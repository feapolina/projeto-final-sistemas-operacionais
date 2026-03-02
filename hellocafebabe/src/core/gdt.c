#include "gdt.h"

/* Criamos um array com 3 entradas para a nossa GDT e um ponteiro para ela */
gdt_entry_t gdt_entries[3];
gdt_ptr_t   gdt_ptr;

/* Esta é a declaração da função que vamos escrever em Assembly no próximo passo.
 * Ela é quem realmente entrega a tabela pronta para o processador. */
extern void gdt_flush(unsigned int);

/* Função auxiliar para "montar" cada linha da nossa tabela (quebrar os números nos pedaços certos) */
static void gdt_set_gate(int num, unsigned int base, unsigned int limit, unsigned char access, unsigned char gran)
{
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;

    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access      = access;
}

/* A função que será chamada lá no nosso kmain.c para iniciar tudo */
void init_gdt(void)
{
    /* 1. Configura o tamanho e o endereço do nosso ponteiro da GDT */
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 3) - 1;
    gdt_ptr.base  = (unsigned int)&gdt_entries;

    /* 2. Preenche a tabela */
    
    /* Entrada 0: Segmento Nulo. O processador exige que a primeira linha seja zerada por segurança. */
    gdt_set_gate(0, 0, 0, 0, 0);

    /* Entrada 1: Segmento de Código do Kernel (Pode executar e ler)
     * Vai do endereço 0 até 4GB (0xFFFFFFFF). 
     * Access 0x9A significa: Presente, Ring 0 (Privilégio máximo), Executável. */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* Entrada 2: Segmento de Dados do Kernel (Pode ler e escrever)
     * Vai do endereço 0 até 4GB (0xFFFFFFFF).
     * Access 0x92 significa: Presente, Ring 0, Leitura/Escrita permitida. */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    /* 3. Chama a função em Assembly para carregar a tabela no hardware */
    gdt_flush((unsigned int)&gdt_ptr);
}