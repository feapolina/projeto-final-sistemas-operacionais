/* interrupts.h */

// Prevenção de dupla inclusão (Include Guard)
#ifndef INCLUDE_INTERRUPTS_H
#define INCLUDE_INTERRUPTS_H

/**
 * @brief Estrutura que representa o estado dos registradores de propósito geral.
 * * Durante uma interrupção, o código em Assembly normalmente 
 * usa a instrução 'pushad' (Push All Double) para empilhar esses registradores 
 * na memória. A ordem aqui de baixo para cima reflete a ordem em que o 'pushad'
 * os coloca na pilha.
 * * O '__attribute__((packed))' avisa ao compilador C para NÃO adicionar bytes 
 * vazios (padding) de alinhamento na estrutura. Isso é obrigatório, pois 
 * precisamos ler os dados da pilha exatamente como o processador os empilhou.
 */
struct cpu_state {
    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;
    unsigned int esi;
    unsigned int edi;
    unsigned int ebp;
} __attribute__((packed));

/**
 * @brief Estrutura que representa o estado salvo automaticamente pela CPU.
 * * Quando uma interrupção ocorre (antes mesmo de rodar o seu código Assembly), 
 * o próprio hardware do processador empilha algumas informações vitais para que 
 * ele saiba como voltar ao programa interrompido depois.
 */
struct stack_state {
    unsigned int error_code; // Código de erro (algumas exceções da CPU geram isso, outras não)
    unsigned int eip;        // Instruction Pointer: o endereço da próxima instrução que ia ser executada antes de  
                             // ser interrompido
    unsigned int cs;         // Code Segment: o segmento de código onde a interrupção ocorreu
    unsigned int eflags;     // Flags da CPU (estado lógico das operações matemáticas, etc)
} __attribute__((packed));

/**
 * @brief Função central em C para tratamento de interrupções.
 * * O código Assembly (handler) irá chamar esta função, passando o estado
 * dos registradores, o número exato da interrupção (ex: 32 para o Timer, 33 para o teclado)
 * e o estado da pilha.
 */
void interrupt_handler(struct cpu_state cpu, unsigned int interrupt, struct stack_state stack);

/**
 * @brief Remapeia o Controlador de Interrupções Programável (PIC).
 * * Por padrão, o PIC mapeia as interrupções de hardware (IRQs 0 a 15) para os números 
 * 8 a 23. O problema é que a Intel reservou os números de 0 a 31 para exceções 
 * internas da CPU (como divisão por zero). Essa função move as IRQs para o intervalo
 * 32 a 47 para evitar esse conflito.
 */
void pic_remap(void);

#endif 