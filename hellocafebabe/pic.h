#ifndef INCLUDE_PIC_H
#define INCLUDE_PIC_H

#define PIC1_PORT_A 0x20
#define PIC1_PORT_B 0x21
#define PIC2_PORT_A 0xA0
#define PIC2_PORT_B 0xA1

/* O PIC deve ser remapeado para evitar conflitos com interrupções de CPU (0-31) */
#define PIC1_START_INTERRUPT 0x20
#define PIC2_START_INTERRUPT 0x28
#define PIC2_END_INTERRUPT   (PIC2_START_INTERRUPT + 7)

#define PIC_ACK     0x20

void pic_init(void);
void pic_acknowledge(unsigned int interrupt);

#endif