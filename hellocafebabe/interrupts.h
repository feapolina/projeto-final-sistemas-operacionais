/* interrupts.h */
#ifndef INCLUDE_INTERRUPTS_H
#define INCLUDE_INTERRUPTS_H

struct idt_entry {
    unsigned short offset_low;      // bits 0..15 do endereço 
    unsigned short segment_selector;// seletor de segmento no GDT 
    unsigned char  reserved;        // reservado 
    unsigned char  config;          // P, DPL, D e tipo 
    unsigned short offset_high;     // bits 16..31 do endereço 
} __attribute__((packed));

struct idt_ptr {
    unsigned short size;
    unsigned int address;
} __attribute__((packed));

void idt_init(void);

struct cpu_state {
    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;
    unsigned int esi;
    unsigned int edi;
    unsigned int ebp;
} __attribute__((packed));

struct stack_state {
    unsigned int error_code;
    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
} __attribute__((packed));

void interrupt_handler(struct cpu_state cpu, unsigned int interrupt, struct stack_state stack);

#endif