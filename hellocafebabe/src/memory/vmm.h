#ifndef VMM_H
#define VMM_H

// Códigos de retorno da API de mapeamento
#define VMM_OK                0
#define VMM_ERR_INVALID_ADDR -1
#define VMM_ERR_ALREADY_USED -2
#define VMM_ERR_NOT_MAPPED   -3

// Inicialização: limpa mapeamentos da região do heap herdados do boot
void vmm_init(void);

// Função da Parte 2: Atualizar a TLB
void vmm_flush_tlb(unsigned int virt_addr);

// Funções da Entrega Final: API básica de mapeamento permanente
int vmm_map_page(unsigned int virt_addr, unsigned int phys_addr, unsigned int flags);
int vmm_unmap_page(unsigned int virt_addr);
int vmm_is_mapped(unsigned int virt_addr);

// Funções da Parte 3: Mapeamento Temporário
void* vmm_temp_map_page(unsigned int phys_addr);
void vmm_temp_unmap_page(void);

#endif