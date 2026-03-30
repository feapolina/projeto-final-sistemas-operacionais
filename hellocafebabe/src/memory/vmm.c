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
 * @brief (PARTE 2) Invalida o cache da CPU (TLB) para um endereço específico.
 * Usa um comando inline em Assembly (invlpg) para gritar pro processador ler a tabela de novo.
 */
void vmm_flush_tlb(unsigned int virt_addr) 
{
    asm volatile("invlpg (%0)" :: "r" (virt_addr) : "memory");
}

/**
 * @brief (ENTREGA FINAL) Mapeia uma página virtual para um frame físico.
 *
 * Regras:
 * - Aceita apenas endereços alinhados em 4KB.
 * - Aceita apenas a janela da boot_page_table1 (0xC0000000..0xC03FFFFF).
 * - Reserva o índice 1023 para o mapeamento temporário.
 */
int vmm_map_page(unsigned int virt_addr, unsigned int phys_addr, unsigned int flags)
{
    unsigned int index;
    unsigned int final_flags;

    // Verifica se os dois endereços estão alinhados em 4KB (tamanho da página).
    if (!is_page_aligned(virt_addr) || !is_page_aligned(phys_addr)) {
        return VMM_ERR_INVALID_ADDR;
    }

    // Garante que o endereço virtual está dentro da janela coberta pela tabela inicial.
    if (!is_in_boot_pt_window(virt_addr)) {
        return VMM_ERR_INVALID_ADDR;
    }

    index = boot_pt_index(virt_addr);
    // Protege o slot reservado para mapeamento temporário.
    if (index == TEMP_PAGE_INDEX) {
        return VMM_ERR_INVALID_ADDR;
    }

    // Evita sobrescrita acidental: se ja existe mapeamento, retornamos erro.
    if (vmm_is_mapped(virt_addr)) {
        return VMM_ERR_ALREADY_USED;
    }

    // Mantemos apenas os 12 bits de flags e forçamos o bit Present.
    final_flags = (flags & 0xFFF) | PTE_PRESENT;
    boot_page_table1[index] = (phys_addr & 0xFFFFF000) | final_flags;
    vmm_flush_tlb(virt_addr);
    return VMM_OK;
}

/**
 * @brief (ENTREGA FINAL) Remove o mapeamento de uma página virtual.
 */
int vmm_unmap_page(unsigned int virt_addr)
{
    unsigned int index;

    // Para remover, o endereço virtual também precisa estar alinhado em 4KB.
    if (!is_page_aligned(virt_addr)) {
        return VMM_ERR_INVALID_ADDR;
    }

    // Só permitimos unmap da faixa que essa implementação controla.
    if (!is_in_boot_pt_window(virt_addr)) {
        return VMM_ERR_INVALID_ADDR;
    }

    index = boot_pt_index(virt_addr);
    // O índice temporário não pode ser manipulado por esta API.
    if (index == TEMP_PAGE_INDEX) {
        return VMM_ERR_INVALID_ADDR;
    }

    // Unmap de pagina ja vazia retorna erro explicito.
    if (!vmm_is_mapped(virt_addr)) {
        return VMM_ERR_NOT_MAPPED;
    }

    boot_page_table1[index] = 0;
    vmm_flush_tlb(virt_addr);
    return VMM_OK;
}

/**
 * @brief (ENTREGA FINAL) Verifica se uma página virtual está marcada como presente.
 *
 * Definição adotada aqui: "mapeada" significa apenas bit Present ligado.
 * Esta função não valida permissões (RW/US) nem consistência do frame físico.
 */
int vmm_is_mapped(unsigned int virt_addr)
{
    unsigned int index;

    // Endereço desalinhado não representa uma página válida.
    if (!is_page_aligned(virt_addr)) {
        return 0;
    }

    // Fora da janela inicial, esta rotina não consegue afirmar mapeamento.
    if (!is_in_boot_pt_window(virt_addr)) {
        return 0;
    }

    index = boot_pt_index(virt_addr);
    // O slot temporário fica fora desta checagem para evitar falso positivo.
    if (index == TEMP_PAGE_INDEX) {
        return 0;
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