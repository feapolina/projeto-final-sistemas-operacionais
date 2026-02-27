ENTRY(loader)                /* o nome do símbolo de entrada */

SECTIONS {
    . = 0x00100000;          /* o código deve ser carregado em 1 MB */

    .text ALIGN (0x1000) :   /* alinha na página de 4 KB */
    {
        *(.text)             /* todas as seções de texto de todos os arquivos */
    }

    .rodata ALIGN (0x1000) :
    {
        *(.rodata*)          /* todas as seções de dados somente leitura */
    }

    .data ALIGN (0x1000) :
    {
        *(.data)             /* todas as seções de dados */
    }

    .bss ALIGN (0x1000) :
    {
        *(COMMON)            /* todas as seções comuns */
        *(.bss)              /* todas as seções bss */
    }
}