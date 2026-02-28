/* interrupts.c */
#include "interrupts.h"
#include "../drivers/fb.h"
#include "../drivers/io.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1
#define PIC_EOI      0x20
#define KBD_DATA_PORT 0x60

void pic_remap(void) {
    // Inicialização do PIC em modo cascata
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    // Definir o offset (IRQs 0-7 -> 32-39, IRQs 8-15 -> 40-47)
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);

    // Configuração de cascata
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    // Modo 8086
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    // MÁSCARA DE INTERRUPÇÕES:
    // 0xFD = 1111 1101 em binário (Ativa apenas o bit 1, que é o Teclado/IRQ1, silenciando o Timer)
    outb(PIC1_DATA, 0xFD);
    
    // 0xFF = 1111 1111 em binário (Desativa todas as IRQs do PIC2)
    outb(PIC2_DATA, 0xFF);
}

void interrupt_handler(struct cpu_state cpu, unsigned int interrupt, struct stack_state stack)
{
    (void)cpu;
    (void)stack;
    
    if (interrupt == 0) {
        char msg[] = "Erro criado por interrupcao: Divisao por zero detectada!\n";
        fb_write(msg, sizeof(msg) - 1);
    } else if (interrupt == 33){
        // Obrigatorio ler scan code da porta 0x60
        unsigned char scan_code = inb(KBD_DATA_PORT);
        (void)scan_code;

        if (!(scan_code & 0x80)){
            char msg_tecla[] = "tecla pressionada!\n";
            fb_write(msg_tecla, sizeof(msg_tecla) - 1);
        }
    }
    
    // Se for uma interrupção de hardware (IRQ 0-15 remapeadas para 32-47)
    if (interrupt >= 32 && interrupt <= 47){
        // Enviar EOI para o PIC2 se for IRQ 8-15 (interrupções 40 a 47)
        if (interrupt >= 40){
            outb(0xA0, 0x20);
        }
        // Enviar EOI para o PIC1 SEMPRE que for uma interrupção de hardware
        outb(0x20, 0x20);
    }
}