#include "fb.h"
#include "serial.h"

#define FB_GREEN 2
#define FB_DARK_GREY 8

int kmain(void) {
    /* Usando a função fb_write */
    fb_write(0, 'O', FB_GREEN, FB_DARK_GREY);
    fb_write(1, 'L', FB_GREEN, FB_DARK_GREY);
    fb_write(2, 'A', FB_GREEN, FB_DARK_GREY);
    
    fb_move_cursor(3);

    return 0xCAFEBABE;
}