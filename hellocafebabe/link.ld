ENTRY(loader) /* rotulo de entrada */

SECTIONS {
    . = 0x00100000;              /* o codigo deve ser carregado em 1MB */

    .text ALIGN (0x1000) :       /* ALINHE EM 4 KB */
    {
        *(.text)                  /* TODOS AS SEÇÕES DE TEXTO DE TODOS OS ARQUIVOS*/
    }

    .rodata ALIGN (0x1000):       /* ALINHE EM 4 KB */
    {
        *(.rodata*)                /* TODAS AS SEÇÕES DE DADOS APENAS DE LEITURA DE TODOS OS ARQUIVOS */
    }

    .data ALIGN (0x1000):         /* ALINHE EM 4 KB */
    {
        *(.data)                  /* TODAS AS SEÇÕES DE DADOS DE TODOS ARQUIVOS */
    }

    .bss ALIGN (0x1000):         /* ALINHE EM 4 KB */
    {
        *(COMMON)                /* TODAS SEÇÕES COMMON DE TODOS OS ARQUIVOS */
        *(.bss)                   /* TODAS AS SEÇÕES BSS DE TODOS OS ARQUIVOS */
    }
}