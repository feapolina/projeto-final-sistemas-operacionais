#include "vmm.h"

// Importando as tabelas de paginação lá do loader.s
extern unsigned int boot_page_directory[];
extern unsigned int boot_page_table1[];

// O nosso endereço virtual "coringa" (O último slot de 4KB do boot_page_table1)
#define TEMP_VIRT_ADDR 0xC03FF000

// Flags mágicas de paginação da arquitetura x86
#define PTE_PRESENT 0x01
#define PTE_RW      0x02

/**
 * @brief (PARTE 2) Invalida o cache da CPU (TLB) para um endereço específico.
 * Usa um comando inline em Assembly (invlpg) para gritar pro processador ler a tabela de novo.
 */
void vmm_flush_tlb(unsigned int virt_addr) 
{
    asm volatile("invlpg (%0)" :: "r" (virt_addr) : "memory");
}

/**
 * @brief (PARTE 3) Pega um endereço FÍSICO qualquer e "esconde" ele no endereço VIRTUAL temporário.
 */
void* vmm_temp_map_page(unsigned int phys_addr) 
{
    // O endereço 0xC03FF000 fica exatamente no índice 1023 da nossa tabela 1.
    // Nós colocamos o endereço físico lá e ativamos os bits de Presente e Leitura/Escrita.
    boot_page_table1[1023] = (phys_addr & 0xFFFFF000) | PTE_PRESENT | PTE_RW;
    
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
    boot_page_table1[1023] = 0;
    
    // Atualiza a TLB de novo
    vmm_flush_tlb(TEMP_VIRT_ADDR);
}