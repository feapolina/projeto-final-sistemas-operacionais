/* interrupts.c */
#include "interrupts.h"
#include "fb.h"

void interrupt_handler(struct cpu_state cpu, struct stack_state stack, unsigned int interrupt)
{

    (void)cpu;
    (void)stack;
    // Exemplo: Se ocorrer uma divisão por zero (0)
    if (interrupt == 0) {
        char msg[] = "Erro: Divisao por zero!\n";
        fb_write(msg, sizeof(msg) - 1);
    }
    
    
}