/* kmain.c */
#include "fb.h"
#include "serial.h"
#include "gdt.h" 
#include "interrupts.h"
#include "idt.h"

#define FB_GREEN 2
#define FB_DARK_GREY 8

int kmain(void)
{
    /* 1. Inicializa a GDT (Segmentação de Memória) */
    /* Isso DEVE rodar antes de qualquer outra coisa no sistema! */
    init_gdt();

    idt_install();

    // Dispara manualmente a interrupção 0
    __asm__ __volatile__("int $0");

    struct cpu_state dummy_cpu = {0};
    struct stack_state dummy_stack = {0};

    char msg_welcome[] = "Kernel Inicializado com sucesso!\n";
    fb_write(msg_welcome, sizeof(msg_welcome) - 1);

    // Testa se a lógica de escrita no framebuffer para interrupção 0 funciona
    interrupt_handler(dummy_cpu, dummy_stack, 0);

    /* Teste do driver do framebuffer (alto nível) */
    char msg_fb[] = "Driver do framebuffer carregado com sucesso!\n";
    fb_write(msg_fb, sizeof(msg_fb) - 1);

    /* Inicializa COM1 e escreve na serial */
    serial_init(0x3F8);

    char msg_serial[] = "Comunicação via porta serial do PC ativa com sucesso!\n";
    serial_write(0x3F8, msg_serial, sizeof(msg_serial) - 1);

    while (1){}
    return 0;
}