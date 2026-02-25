/* kmain.c */
#include "fb.h"
#include "serial.h"

#define FB_GREEN 2
#define FB_DARK_GREY 8

int kmain(void) {
     /* Escreve o caractere 'A' na primeira célula do framebuffer */
    fb_write_cell(0,'A',FB_GREEN, FB_DARK_GREY);
    /* Move o cursor para a próxima posição (coluna 1) */
    fb_move_cursor(1);
     /* Loop infinito para impedir retorno */

    char msg_driver[] = "Ola, driver!\n";
    /*mensagem armazenada na variavel msg*/
    fb_write(msg_driver,13);
    /*mensagem escrita na tela através do driver*/

    serial_init(0x3F8);

    char msg_serial[] = "Ola pela serial!\n";
    /*mensagem armazenada na variavel msg_serial*/
    serial_write(0x3F8, msg_serial, 16);
    /*mensagem escrita na tela através do driver*/
    while(1);
}