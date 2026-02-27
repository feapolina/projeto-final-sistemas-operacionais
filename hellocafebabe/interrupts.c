/* interrupts.c */
#include "interrupts.h"
#include "fb.h"
#include "pic.h"
#include "io.h" // Necessário para a função read_scan_code

/* 1. Definição da tabela IDT e do ponteiro para a instrução lidt */
struct idt_entry idt[256];
struct idt_ptr idtp;

/* 2. Declaração dos handlers que estão no Assembly (interrupts_asm.s) 
      O 'extern' avisa o C que essas funções foram criadas no Assembly */
extern void interrupt_handler_0(void);
extern void interrupt_handler_1(void);
extern void interrupt_handler_32(void); // Handler do Timer (IRQ0)
extern void interrupt_handler_33(void); // Handler do Teclado (IRQ1)

/* 3. Declaração da função load_idt que está no Assembly */
extern void load_idt(unsigned int idt_ptr_addr);

/* 4. Função auxiliar para preencher uma entrada na IDT */
void idt_set_entry(unsigned char n, unsigned int address, unsigned short sel, unsigned char config) 
{
    idt[n].offset_low = address & 0xFFFF;
    idt[n].offset_high = (address >> 16) & 0xFFFF;
    idt[n].segment_selector = sel;
    idt[n].reserved = 0;
    idt[n].config = config; // Ex: 0x8E para interrupt gates (P=1, DPL=0)
}

/* 5. Função que inicializa a IDT (chamada pelo kmain) */
void idt_init(void) 
{
    // Configura o ponteiro da IDT para o comando lidt
    idtp.size = (sizeof(struct idt_entry) * 256) - 1;
    idtp.address = (unsigned int)&idt;

    /* Registra as interrupções de exceção da CPU */
    idt_set_entry(0, (unsigned int)interrupt_handler_0, 0x08, 0x8E);
    idt_set_entry(1, (unsigned int)interrupt_handler_1, 0x08, 0x8E);

    /* Registra as interrupções de Hardware remapeadas pelo PIC */
    idt_set_entry(32, (unsigned int)interrupt_handler_32, 0x08, 0x8E); // Timer
    idt_set_entry(33, (unsigned int)interrupt_handler_33, 0x08, 0x8E); // Teclado

    // Carrega a IDT no processador 
    load_idt((unsigned int)&idtp);
}

/* 6. Função para ler o scan code do teclado (conforme seção 6.7 do livro) */
unsigned char read_scan_code(void)
{
    return inb(0x60); // Lê o dado da porta I/O do teclado
}

/* 7. O seu tratador genérico em C */
void interrupt_handler(struct cpu_state cpu, unsigned int interrupt, struct stack_state stack)
{
    (void)cpu; 
    (void)stack;

    // Agora 'interrupt' terá o valor correto (0, 32, 33...)
    if (interrupt == 0) {
        fb_write("Erro: Divisao por zero!\n", 24);
    }
    
    if (interrupt == 33) {
        // Lê o scan code para "limpar" o teclado e permitir a próxima tecla
        unsigned char scancode = read_scan_code(); 
        (void)scancode;

        fb_write("Tecla pressionada!\n", 19);
    }

    // Avisa o PIC que terminamos, senão ele trava após o primeiro sinal
    pic_acknowledge(interrupt);
}