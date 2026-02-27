#include "pic.h"
#include "io.h"

/**
 * pic_acknowledge:
 * Envia o sinal de confirmação (EOI - End of Interrupt) para os controladores PIC.
 */
void pic_acknowledge(unsigned int interrupt)
{
    if (interrupt < PIC1_START_INTERRUPT || interrupt > PIC2_END_INTERRUPT) {
        return;
    }

    if (interrupt >= PIC2_START_INTERRUPT) {
        outb(PIC2_PORT_A, PIC_ACK);
    }
    outb(PIC1_PORT_A, PIC_ACK);
}

/**
 * pic_init:
 * Remapeia as interrupções do hardware de (0-15) para (0x20-0x2F).
 */
void pic_init(void) {
    /* ICW1 - Inicia a inicialização */
    outb(PIC1_PORT_A, 0x11);
    outb(PIC2_PORT_A, 0x11);

    /* ICW2 - Remapeia o offset (Vetor base) */
    outb(PIC1_PORT_B, 0x20); /* PIC1 -> 0x20 (32) */
    outb(PIC2_PORT_B, 0x28); /* PIC2 -> 0x28 (40) */

    /* ICW3 - Configura a conexão mestre/escravo */
    outb(PIC1_PORT_B, 0x04);
    outb(PIC2_PORT_B, 0x02);

    /* ICW4 - Modo 8086 */
    outb(PIC1_PORT_B, 0x01);
    outb(PIC2_PORT_B, 0x01);

    /* Máscaras - Ativa todas as interrupções por enquanto (0x00) */
    outb(PIC1_PORT_B, 0x00);
    outb(PIC2_PORT_B, 0x00);
}