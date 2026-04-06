#include "vmm.h"

// Importando as tabelas de paginação lá do loader.s
extern unsigned int boot_page_directory[];
extern unsigned int boot_page_table1[];

// O nosso endereço virtual "coringa" (O último slot de 4KB do boot_page_table1)
#define TEMP_VIRT_ADDR 0xC03FF000
#define TEMP_PAGE_INDEX 1023

// Janela virtual coberta pela boot_page_table1 (4 MB a partir de 0xC0000000)
#define KERNEL_VIRTUAL_BASE 0xC0000000
#define BOOT_PT_WINDOW_END 0xC0400000

// Início da região do heap (3 MB acima do KERNEL_VIRTUAL_BASE)
#define HEAP_REGION_START 0xC0300000

// Flags mágicas de paginação da arquitetura x86
#define PTE_PRESENT 0x01
#define PTE_RW      0x02

static int is_page_aligned(unsigned int addr)
{
    return (addr & 0xFFF) == 0;
}

static int is_in_boot_pt_window(unsigned int virt_addr)
{
    return virt_addr >= KERNEL_VIRTUAL_BASE && virt_addr < BOOT_PT_WINDOW_END;
}

static unsigned int boot_pt_index(unsigned int virt_addr)
{
    return (virt_addr - KERNEL_VIRTUAL_BASE) >> 12;
}

/**
 * @brief (ENTREGA FINAL) Limpa os mapeamentos da região do heap que vieram prontos do boot.
 * O loader preenche as 1024 entradas da tabela inteira, então a faixa do heap
 * (0xC0300000+) já chega "ocupada". Sem essa limpeza o vmm_map_page acha que
 * tudo já tá em uso e o kmalloc não consegue crescer.
 * Zeramos os índices 768..1022 e deixamos o 1023 intacto (slot temporário).
 */
void vmm_init(void)
{
    unsigned int addr;

    for (addr = HEAP_REGION_START; addr < TEMP_VIRT_ADDR; addr += 0x1000) {
        unsigned int idx = boot_pt_index(addr);
        boot_page_table1[idx] = 0;
        vmm_flush_tlb(addr);
    }
}

/**
 * @brief (PARTE 2) Invalida o cache da CPU (TLB) para um endereço específico.
 * Usa um comando inline em Assembly (invlpg) para gritar pro processador ler a tabela de novo.
 */
void vmm_flush_tlb(unsigned int virt_addr) 
{
    asm volatile("invlpg (%0)" :: "r" (virt_addr) : "memory");
}

/**
 * @brief (ENTREGA FINAL) Mapeia uma página virtual pra um frame físico na boot_page_table1.
 * Só aceita endereços alinhados em 4KB e dentro da janela de 4 MB do kernel.
 */
int vmm_map_page(unsigned int virt_addr, unsigned int phys_addr, unsigned int flags)
{
    unsigned int index;
    unsigned int final_flags;

    // Os dois endereços precisam ser múltiplos de 4KB
    if (!is_page_aligned(virt_addr) || !is_page_aligned(phys_addr)) {
        return VMM_ERR_INVALID_ADDR;
    }

    // Tem que estar dentro da janela da tabela (0xC0000000 .. 0xC03FFFFF)
    if (!is_in_boot_pt_window(virt_addr)) {
        return VMM_ERR_INVALID_ADDR;
    }

    index = boot_pt_index(virt_addr);
    // O slot 1023 é do mapeamento temporário, não pode mexer
    if (index == TEMP_PAGE_INDEX) {
        return VMM_ERR_INVALID_ADDR;
    }

    // Se já tem alguém mapeado aqui, não sobrescreve
    if (vmm_is_mapped(virt_addr)) {
        return VMM_ERR_ALREADY_USED;
    }

    // Junta as flags do usuário com o bit Present e grava na tabela
    final_flags = (flags & 0xFFF) | PTE_PRESENT;
    boot_page_table1[index] = (phys_addr & 0xFFFFF000) | final_flags;
    vmm_flush_tlb(virt_addr);
    return VMM_OK;
}

/**
 * @brief (ENTREGA FINAL) Remove o mapeamento de uma página virtual (zera a entrada na tabela).
 */
int vmm_unmap_page(unsigned int virt_addr)
{
    unsigned int index;

    if (!is_page_aligned(virt_addr)) {
        return VMM_ERR_INVALID_ADDR;
    }

    if (!is_in_boot_pt_window(virt_addr)) {
        return VMM_ERR_INVALID_ADDR;
    }

    index = boot_pt_index(virt_addr);
    if (index == TEMP_PAGE_INDEX) {
        return VMM_ERR_INVALID_ADDR;
    }

    // Não faz unmap de página que já tá vazia
    if (!vmm_is_mapped(virt_addr)) {
        return VMM_ERR_NOT_MAPPED;
    }

    boot_page_table1[index] = 0;
    vmm_flush_tlb(virt_addr);
    return VMM_OK;
}

/**
 * @brief (ENTREGA FINAL) Checa se uma página virtual tá presente na tabela (bit Present ligado).
 */
int vmm_is_mapped(unsigned int virt_addr)
{
    unsigned int index;

    if (!is_page_aligned(virt_addr)) {
        return 0;
    }

    if (!is_in_boot_pt_window(virt_addr)) {
        return 0;
    }

    index = boot_pt_index(virt_addr);
    if (index == TEMP_PAGE_INDEX) {
        return 0; // ignora o slot temporário
    }

    return (boot_page_table1[index] & PTE_PRESENT) != 0;
}

/**
 * @brief (PARTE 3) Pega um endereço FÍSICO qualquer e "esconde" ele no endereço VIRTUAL temporário.
 */
void* vmm_temp_map_page(unsigned int phys_addr) 
{
    // O endereço 0xC03FF000 fica exatamente no índice 1023 da nossa tabela 1.
    // Nós colocamos o endereço físico lá e ativamos os bits de Presente e Leitura/Escrita.
    boot_page_table1[TEMP_PAGE_INDEX] = (phys_addr & 0xFFFFF000) | PTE_PRESENT | PTE_RW;
    
    // Avisamos ao processador que o mapa mudou!
    vmm_flush_tlb(TEMP_VIRT_ADDR);
    
    // Devolvemos o endereço virtual prontinho para o SO usar
    return (void*)TEMP_VIRT_ADDR;
}

/**
 * @brief (PARTE 3) Desfaz o mapeamento temporário para evitar bagunça.
 */
void vmm_temp_unmap_page(void) 
{
    // Zera o índice 1023
    boot_page_table1[TEMP_PAGE_INDEX] = 0;
    
    // Atualiza a TLB de novo
    vmm_flush_tlb(TEMP_VIRT_ADDR);
}