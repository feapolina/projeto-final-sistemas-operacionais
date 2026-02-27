/* kmain.c */
#include "fb.h"
#include "serial.h"
#include "interrupts.h"
#include "pic.h"
#include "gdt.h"

#define FB_GREEN 2
#define FB_DARK_GREY 8

int kmain(void)
{
    /* 1. Inicializa a GDT (Segmentação de Memória) */
    /* Isso DEVE rodar antes de qualquer outra coisa no sistema! */
    init_gdt();

    /* 2. Inicializa a IDT (Interrupções) */
    /* Deve ser carregada após a GDT para usar os seletores corretos */
    idt_init();

    /* 3. Inicializa o PIC (Controlador Programável de Interrupções) */
    pic_init();

    /* Habilita as interrupções de hardware na CPU. */
    asm volatile("sti");

    fb_write("Kernel Inicializado!", 21);

    /* Teste do driver do framebuffer */
    char msg_fb_idt[] = "IDT Carregada com sucesso!\n";
    fb_write(msg_fb_idt, 27);

   

    /* --- TESTE DE INTERRUPÇÃO --- */
    /* Dispara manualmente a interrupção 0 (Division by Zero) */
    /* Se a IDT estiver correta, o handler imprimirá a mensagem de erro */
    asm volatile("int $0");

    struct cpu_state dummy_cpu = {0};
    struct stack_state dummy_stack = {0};

    interrupt_handler(dummy_cpu, 0, dummy_stack);

    /* Teste rápido do framebuffer (baixo nível) */
    fb_write_cell(0, 'A', FB_GREEN, FB_DARK_GREY);
    fb_move_cursor(1);

    /* Teste do driver do framebuffer (alto nível) */
    char msg_fb_driver[] = "Ola, driver!\n";
    fb_write(msg_fb_driver, 13);

    /* Inicializa COM1 e escreve na serial */
    serial_init(0x3F8);

    char msg_serial[] = "Ola pela serial! Kernel pronto e com interrupcoes\n";
    serial_write(0x3F8, msg_serial, 49);

    while (1){}
    return 0;
}