#include "idt.h"

// Handlers definidos no arquivo assembly (interrupts_asm.s)
extern void interrupt_handler_0();
extern void interrupt_handler_8();
extern void interrupt_handler_13();
extern void interrupt_handler_14();
extern void interrupt_handler_32();
extern void interrupt_handler_33();

struct idt_entry idt[256];
struct idt_ptr   idtp;

// Função auxiliar para configurar cada entrada
void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags) {
    idt[num].offset_low = (base & 0xFFFF);
    idt[num].offset_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].type_attr = flags;
}

void idt_install() {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (unsigned int)&idt;

    // Limpa a IDT com zeros
    for(int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    // Registra seus handlers 
    // 0x08 é o seletor de código da GDT, 0x8E define como Interrupt Gate presente
    idt_set_gate(0, (unsigned long)interrupt_handler_0, 0x08, 0x8E);
    idt_set_gate(8, (unsigned long)interrupt_handler_8, 0x08, 0x8E);
    idt_set_gate(13, (unsigned long)interrupt_handler_13, 0x08, 0x8E);
    idt_set_gate(14, (unsigned long)interrupt_handler_14, 0x08, 0x8E);
    idt_set_gate(32, (unsigned long) interrupt_handler_32, 0x08, 0x8E);
    idt_set_gate(33, (unsigned long) interrupt_handler_33, 0x08, 0x8E);

    // Carrega a IDT
    idt_load((unsigned int)&idtp);
}