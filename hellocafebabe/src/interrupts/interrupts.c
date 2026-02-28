/* interrupts.c */
#include "interrupts.h"
#include "../drivers/fb.h"
#include "../drivers/io.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1
#define PIC_EOI      0x20

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

    // Ativar todas as IRQs (máscara zero)
    outb(PIC1_DATA, 0x00);
    outb(PIC2_DATA, 0x00);
}

void interrupt_handler(struct cpu_state cpu, struct stack_state stack, unsigned int interrupt)
{

    (void)cpu;
    (void)stack;
    
    // Exemplo: Se ocorrer uma divisão por zero (0)
    if (interrupt == 0) {
        char msg[] = "Erro criado por interrupcao: Divisao por zero detectada!\n";
        fb_write(msg, sizeof(msg) - 1);
    }
    
    // Se for uma interrupção de hardware (IRQ 0-15 remapeadas para 32-47)
    else if (interrupt >= 32 && interrupt <= 47){
        // Enviar EOI para o PIC2 se for IRQ 8-15
        if (interrupt >= 40){
            if (interrupt >= 40){
                outb(0xA0, 0x20);
            }
            // Enviar EOI para o PC1
            outb(0x20, 0x20);
        }
    }
    
}