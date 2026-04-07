#ifndef FB_H
#define FB_H

/* Definições das 16 cores padrão do modo texto VGA */
#define FB_BLACK         0
#define FB_BLUE          1
#define FB_GREEN         2
#define FB_CYAN          3
#define FB_RED           4
#define FB_MAGENTA       5
#define FB_BROWN         6
#define FB_LIGHT_GRAY    7
#define FB_DARK_GRAY     8
#define FB_LIGHT_BLUE    9
#define FB_LIGHT_GREEN   10
#define FB_LIGHT_CYAN    11
#define FB_LIGHT_RED     12
#define FB_PINK          13
#define FB_YELLOW        14
#define FB_WHITE         15

void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg);
void fb_move_cursor(unsigned short pos);

/* função para limpar a tela */
void fb_clear(void); 

/* funções de escrita e logs */
int fb_write_color(char *buf, unsigned int len, unsigned char fg, unsigned char bg);
int fb_write(char *buf, unsigned int len);

/* Atalhos práticos para mensagens do sistema */
void log_info(char *msg);
void log_success(char *msg);
void log_error(char *msg);
void log_warning(char *msg);

#endif