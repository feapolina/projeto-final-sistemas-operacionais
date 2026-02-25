<<<<<<< HEAD
#include "fb.h" /* Importa as funções que seu amigo criou */

int kmain(void) {
    /* * A função padrão do livro é: fb_write_cell(posição, caractere, cor_letra, cor_fundo)
     * Cor 2 = Verde, Cor 0 = Preto
     */
    fb_write_cell(0, 'O', 2, 0); 
    fb_write_cell(1, 'L', 2, 0);
    fb_write_cell(2, 'A', 2, 0);

    return 0xCAFEBABE;
=======
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
>>>>>>> 64d480f13091872e47309c6c52cbc8990a4724e0
}