#ifndef GDT_H
#define GDT_H

/* Estrutura de uma entrada na GDT (cada linha da tabela). 
 * Usamos '__attribute__((packed))' para proibir o compilador (gcc) de adicionar
 * espaços vazios de otimização. O hardware exige que cada entrada tenha EXATAMENTE 8 bytes.
 */
struct gdt_entry_struct {
    unsigned short limit_low;   /* Os 16 bits mais baixos do limite (tamanho do segmento) */
    unsigned short base_low;    /* Os 16 bits mais baixos do endereço base */
    unsigned char  base_middle; /* Os próximos 8 bits da base */
    unsigned char  access;      /* Flags de acesso (define se é código ou dados, privilégios, etc) */
    unsigned char  granularity; /* Flags de granularidade e os 4 bits finais do limite */
    unsigned char  base_high;   /* Os últimos 8 bits da base */
} __attribute__((packed));

typedef struct gdt_entry_struct gdt_entry_t;

/* Estrutura do ponteiro da GDT. 
 * É este "pacotinho" que vamos entregar para o processador ler a nossa tabela inteira.
 */
struct gdt_ptr_struct {
    unsigned short limit; /* O tamanho máximo da tabela inteira em bytes (menos 1) */
    unsigned int   base;  /* O endereço de memória onde a tabela começa */
} __attribute__((packed));

typedef struct gdt_ptr_struct gdt_ptr_t;

/* A função principal que vamos chamar lá no nosso kmain.c para inicializar tudo */
void init_gdt(void);

#endif