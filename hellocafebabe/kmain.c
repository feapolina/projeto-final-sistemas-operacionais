/* kmain.c */
#include "fb.h"

#define FB_GREEN 2
#define FB_DARK_GREY 8

int kmain(void) {
     /* Escreve o caractere 'A' na primeira célula do framebuffer */
    fb_write_cell(0,'A',FB_GREEN, FB_DARK_GREY);
    /* Move o cursor para a próxima posição (coluna 1) */
    fb_move_cursor(1);
     /* Loop infinito para impedir retorno */
    while(1);
}