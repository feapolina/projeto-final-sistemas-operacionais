/* kmain.c */
#include "fb.h"
#include "serial.h"
#include "gdt.h" 
#include "interrupts.h"

#define FB_GREEN 2
#define FB_DARK_GREY 8

int kmain(void)
{
    /* 1. Inicializa a GDT (Segmentação de Memória) */
    /* Isso DEVE rodar antes de qualquer outra coisa no sistema! */
    init_gdt();

    struct cpu_state dummy_cpu = {0};
    struct stack_state dummy_stack = {0};

    // Testa se a lógica de escrita no framebuffer para interrupção 0 funciona
    interrupt_handler(dummy_cpu, dummy_stack, 0);

    /* Teste rápido do framebuffer (baixo nível) */
    fb_write_cell(0, 'A', FB_GREEN, FB_DARK_GREY);
    fb_move_cursor(1);

    /* Teste do driver do framebuffer (alto nível) */
    char msg_fb[] = "Ola, driver!\n";
    fb_write(msg_fb, sizeof(msg_fb) - 1);

    /* Inicializa COM1 e escreve na serial */
    serial_init(0x3F8);

    char msg_serial[] = "Ola pela serial!\n";
    serial_write(0x3F8, sizeof(msg_serial) - 1);

    while (1){}
    return 0;
}