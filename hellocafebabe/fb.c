#include "io.h"

/** fb_write_cell:
* Escreve um caracter com a cor e a cor do fundo passados como paramêtro na posição i
* no framebuffer
*
* @param i A posição no framebuffer
* @param c O caracter
* @param fg A cor do caracter
* @param bg A cor do fundo
*/

char *fb = (char*) 0x000B8000;

void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg)
{
fb[i] = c;
fb[i + 1] = ((fg & 0x0F) << 4) | (bg & 0x0F);
}

/* Portas de I/O do controlador VGA */
#define FB_COMMAND_PORT 0x3D4 // Porta usada para indicar qual registrador será modificado
#define FB_DATA_PORT    0x3D5 // Porta usada para enviar o valor ao registrador

/* Códigos internos do VGA para cursor */
#define FB_HIGH_BYTE_COMMAND 14 // Registrador que armazena o byte alto da posição do cursor 
#define FB_LOW_BYTE_COMMAND  15 // Registrador que armazena o byte baixo da posição do cursor

void fb_move_cursor(unsigned short pos)
{
    /* Seleciona o registrador 14 (parte alta da posição) */
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    /* Envia os 8 bits mais significativos da posição */
    outb(FB_DATA_PORT, (pos >> 8) & 0x00FF);
    /* Seleciona o registrador 15 (parte baixa da posição) */
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    /* Envia os 8 bits menos significativos da posição */
    outb(FB_DATA_PORT, pos & 0x00FF);
}