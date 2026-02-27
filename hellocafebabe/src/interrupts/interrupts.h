/* interrupts.h */
#ifndef INCLUDE_INTERRUPTS_H
#define INCLUDE_INTERRUPTS_H

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

// Declaração da função para que o kmain saiba que ela existe
void interrupt_handler(struct cpu_state cpu, struct stack_state stack, unsigned int interrupt);

#endif