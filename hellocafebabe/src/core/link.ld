ENTRY(loader)                                                           /* o nome do símbolo de entrada */

SECTIONS
{
    . = 0xC0100000;                                                     /* o passa a ser linkado a partir endereço virtual */

    .text ALIGN(0x1000) : AT(ADDR(.text) - 0xC0000000)                  /* alinha a seção em 4 KB e define o endereço físico de carga correspondente */
    {
        *(.text)                                                        /* todas as seções de texto de todos os arquivos */
    }

    .rodata ALIGN(0x1000) : AT(ADDR(.rodata) - 0xC0000000)
    {
        *(.rodata*)                                                     /* todas as seções de dados somente leitura */
    }

    .data ALIGN(0x1000) : AT(ADDR(.data) - 0xC0000000)
    {
        *(.data)                                                        /* todas as seções de dados */
    }

    .bss ALIGN(0x1000) : AT(ADDR(.bss) - 0xC0000000)
    {
        *(COMMON)                                                       /* todas as seções comuns */
        *(.bss)                                                         /* todas as seções bss */
    }
}