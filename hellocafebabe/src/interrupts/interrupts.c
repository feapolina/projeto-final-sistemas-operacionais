/* interrupts.c */
#include "interrupts.h"
#include "../drivers/fb.h"
#include "../drivers/io.h"

// Definições das portas de I/O para comunicação com o chip 8259 PIC
#define PIC1_COMMAND  0x20  // Porta de comando do PIC Master
#define PIC1_DATA     0x21  // Porta de dados do PIC Master
#define PIC2_COMMAND  0xA0  // Porta de comando do PIC Slave
#define PIC2_DATA     0xA1  // Porta de dados do PIC Slave
#define PIC_EOI       0x20  // Comando de End of Interrupt (Fim de Interrupção)

#define KBD_DATA_PORT 0x60  // Porta de dados do controlador de teclado PS/2

/**
 * @brief Remapeia o Controlador de Interrupções Programável (PIC).
 * * O hardware do PIC exige uma sequência exata de 4 comandos de inicialização 
 * (ICW1 a ICW4). Esta função envia esses comandos para mover as IRQs de hardware 
 * (0-15) para os vetores 32-47, evitando conflitos com as exceções da CPU (0-31).
 * Ao final, ela também aplica uma máscara para silenciar as interrupções indesejadas.
 */

 
// Array de mapeamento de Scan Codes para caracteres ASCII (Teclado Americano padrão)
const char kbd_us[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
  '9', '0', '-', '=', '\b', /* Backspace */
  '\t',         /* Tab */
  'q', 'w', 'e', 'r',   /* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter */
    0,          /* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
 '\'', '`',   0,        /* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',            /* 49 */
  'm', ',', '.', '/',   0,                      /* Right shift */
  '*',
    0,  /* Alt */
  ' ',  /* Barra de Espaço */
};





void pic_remap(void) {
    // ICW1: Inicia a configuração e avisa que o ICW4 será enviado
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    // ICW2: Define o offset (Remapeamento real)
    // Master: IRQs 0-7 passam a ser 32-39 (0x20)
    // Slave: IRQs 8-15 passam a ser 40-47 (0x28)
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);

    // ICW3: Configuração de cascata
    // Master: Avisa que o Slave está na linha IRQ2 (0x04 = bit 2)
    // Slave: Informa sua "identidade" na cascata (0x02)
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    // ICW4: Modo de operação
    // 0x01 = Modo 8086/88 (arquitetura x86 padrão)
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    // MÁSCARA DE INTERRUPÇÕES (OCW1)
    // 0 = Ativado (escutando), 1 = Desativado (mascarado)
    
    // 0xFD = (Ativa apenas o bit 1: Teclado/IRQ1. Silencia o Timer/IRQ0)
    outb(PIC1_DATA, 0xFD);
    
    // 0xFF = (Desativa todas as IRQs do PIC2/Slave)
    outb(PIC2_DATA, 0xFF);
}

/**
 * @brief Função central para tratamento de interrupções e exceções.
 * * Todas as interrupções capturadas pelo código em Assembly são repassadas para esta função.
 * Ela avalia o número da interrupção gerada e toma a ação apropriada (ex: imprimir um erro
 * ou ler uma tecla do teclado).
 * * @param cpu Estado dos registradores gerais salvos antes da interrupção.
 * @param interrupt O número do vetor de interrupção (ex: 0 para Divisão por Zero, 33 para Teclado).
 * @param stack Estado da pilha e registradores de controle salvos automaticamente pelo hardware.
 */
void interrupt_handler(struct cpu_state cpu, unsigned int interrupt, struct stack_state stack)
{
    // Prevenção de warnings do compilador para variáveis temporariamente não utilizadas
    (void)cpu;
    (void)stack;
    
    // Tratamento de Exceção da CPU: Divisão por Zero
    if (interrupt == 0) {
        char msg[] = "Erro criado por interrupcao: Divisao por zero detectada!\n";
        fb_write(msg, sizeof(msg) - 1);
    } 
    // Tratamento de Interrupção de Hardware: Teclado (IRQ1 remapeada para 32 + 1 = 33)
    else if (interrupt == 33) {
        // Obrigatorio: ler o scan_code da porta 0x60 para liberar o teclado
        unsigned char scan_code = inb(KBD_DATA_PORT);
        (void)scan_code;

        // Verifica se a tecla foi PRESSIONADA (Make code).
        // Se o bit mais significativo (0x80) for 0, é um Make code. Se for 1, é um Break code (tecla solta).
       // Verifica se é um Make Code (tecla sendo pressionada) e se está dentro do limite do nosso array (0 a 127)
        if (!(scan_code & 0x80) && scan_code < 128) {
            // Busca a letra correspondente no array
            char letra = kbd_us[scan_code];

            // Só imprime se a tecla tiver uma representação visual (ignora Shift, Ctrl, etc.)
            if (letra != 0) {
                // Prepara um array de 2 posições (a letra + o terminador de string nulo)
                char msg_tecla[2] = {letra, '\0'};
                
                // Envia para o driver de vídeo imprimir a letra
                fb_write(msg_tecla, 1);
            }
        
        }
    }
    
    // Controlador (End of Interrupt - EOI)
    // Se for uma interrupção de hardware (IRQs 0-15 remapeadas para 32-47)
    if (interrupt >= 32 && interrupt <= 47) {
        
        // Se veio do PIC2/Slave (IRQs 8-15 -> 40-47), envia EOI para o Slave
        if (interrupt >= 40) {
            outb(PIC2_COMMAND, PIC_EOI);
        }
        
        // Sempre envia EOI para o PIC1/Master nas interrupções de hardware
        outb(PIC1_COMMAND, PIC_EOI);
    }
}