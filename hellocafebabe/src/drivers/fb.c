#include "fb.h"
#include "io.h"

/* Definindo o tema do SO */
#define THEME_FG FB_WHITE     /* Texto Branco */
#define THEME_BG FB_BLACK     /* Fundo Preto */

/** fb_write_cell:
* Escreve um caracter com a cor e a cor do fundo passados como paramêtro na posição i
* no framebuffer
*
* @param i A posição no framebuffer
* @param c O caracter
* @param fg A cor do caracter
* @param bg A cor do fundo
*/
void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg)
{
    /* Ponteiro de memória */
    char *fb = (char*) 0xC00B8000;
    fb[i] = c;
    fb[i + 1] = ((bg & 0x0F) << 4) | (fg & 0x0F);
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

/*definição do tamanho de linhas e colunas*/
#define FB_COLS 80
#define FB_ROWS 25

/* Posição atual do cursor em células (0..(80*25 - 1))*/
static unsigned int fb_cursor_pos = 0;

/* Limpa a tela preenchendo todos os caracteres com espaços em branco utilizando a cor padrão */
void fb_clear(void)
{
    unsigned int i;
    for (i = 0; i < FB_COLS * FB_ROWS; i++) {
        fb_write_cell(i * 2, ' ', THEME_FG, THEME_BG);
    }
    fb_cursor_pos = 0;
    fb_move_cursor((unsigned short)fb_cursor_pos);
}

/* A função fb_write escreve len caracteres do buffer buf na tela e avança o cursor 
automaticamente a cada caractere. */
int fb_write_color(char *buf, unsigned int len, unsigned char fg, unsigned char bg)
{
    unsigned int i;

    for (i = 0; i < len; i++) {
        if (buf[i] == '\n') {
            fb_cursor_pos += FB_COLS - (fb_cursor_pos % FB_COLS);
        } 
        else if (buf[i] == '\b') {
            if (fb_cursor_pos > 0) {
                fb_cursor_pos--;
                fb_write_cell(fb_cursor_pos * 2, ' ', fg, bg); 
            }
        } 
        else {
            fb_write_cell(fb_cursor_pos * 2, buf[i], fg, bg);
            fb_cursor_pos++;
        }

        fb_move_cursor((unsigned short)fb_cursor_pos);

        if (fb_cursor_pos >= (FB_COLS * FB_ROWS)) {
            fb_cursor_pos = 0;
            fb_move_cursor((unsigned short)fb_cursor_pos);
        }
    }

    return (int)len;
}

/* A fb_write agora é um "wrapper" que chama a nova função usando as cores padrão */
int fb_write(char *buf, unsigned int len)
{
    return fb_write_color(buf, len, THEME_FG, THEME_BG);
}

/* Função auxiliar simples para calcular o tamanho da string */
static unsigned int kstrlen(char *str) {
    unsigned int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

/* Imprime um log informativo em Ciano */
void log_info(char *msg) {
    fb_write_color("[INFO] ", 7, FB_LIGHT_CYAN, THEME_BG);
    fb_write_color(msg, kstrlen(msg), THEME_FG, THEME_BG);
    fb_write_color("\n", 1, THEME_FG, THEME_BG);
}

/* Imprime um log de sucesso em Verde */
void log_success(char *msg) {
    fb_write_color("[ OK ] ", 7, FB_LIGHT_GREEN, THEME_BG);
    fb_write_color(msg, kstrlen(msg), THEME_FG, THEME_BG);
    fb_write_color("\n", 1, THEME_FG, THEME_BG);
}

/* Imprime um log de erro em Vermelho */
void log_error(char *msg) {
    fb_write_color("[ERRO] ", 7, FB_LIGHT_RED, THEME_BG);
    fb_write_color(msg, kstrlen(msg), FB_LIGHT_RED, THEME_BG); /* Deixa a mensagem toda vermelha para destacar */
    fb_write_color("\n", 1, THEME_FG, THEME_BG);
}

/* Imprime um aviso em Amarelo */
void log_warning(char *msg) {
    fb_write_color("[AVISO] ", 8, FB_YELLOW, THEME_BG);
    fb_write_color(msg, kstrlen(msg), THEME_FG, THEME_BG);
    fb_write_color("\n", 1, THEME_FG, THEME_BG);
}