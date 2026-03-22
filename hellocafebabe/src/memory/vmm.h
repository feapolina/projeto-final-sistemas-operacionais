#ifndef VMM_H
#define VMM_H

// Função da Parte 2: Atualizar a TLB
void vmm_flush_tlb(unsigned int virt_addr);

// Funções da Parte 3: Mapeamento Temporário
void* vmm_temp_map_page(unsigned int phys_addr);
void vmm_temp_unmap_page(void);

#endif