#ifndef IDT_H
#define IDT_H

void idt_install(void);
void idt_load(unsigned int idtp);

#endif
struct idt_entry {
    unsigned short offset_low;     // Offset bits 0..15
    unsigned short selector;       // Seletor do segmento de código (GDT)
    unsigned char  zero;           // Sempre zero
    unsigned char  type_attr;      // Tipo e atributos (P, DPL, Tipo)
    unsigned short offset_high;    // Offset bits 16..31
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit;          // Tamanho da IDT - 1
    unsigned int   base;           // Endereço base da IDT
} __attribute__((packed));