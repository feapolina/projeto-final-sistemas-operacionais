/* interrupts.c */
#include "interrupts.h"
#include "../drivers/fb.h"
#include "../drivers/io.h"
#include "../drivers/serial.h"

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

<<<<<<< HEAD

=======
// Array para quando o Shift estiver pressionado (Maiúsculas e Símbolos)
const char kbd_us_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', /* 9 */
  '(', ')', '_', '+', '\b', /* Backspace */
  '\t',         /* Tab */
  'Q', 'W', 'E', 'R',   /* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', /* Enter */
    0,          /* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', /* 39 */
 '"', '~',   0,        /* Left shift */
 '|', 'Z', 'X', 'C', 'V', 'B', 'N',            /* 49 */
  'M', '<', '>', '?',   0,                      /* Right shift */
  '*',
    0,  /* Alt */
  ' ',  /* Barra de Espaço */
};

// Variável global para "lembrar" se o Shift está pressionado (0 = Não, 1 = Sim)
static int shift_pressed = 0;
>>>>>>> 78d63dbc7c7c7ca29b2e54f885643ed1c03c12ae



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
    // Prevenção de warnings
    (void)cpu;
    (void)stack;
    
    // 1. Tratamento de Divisão por Zero
    if (interrupt == 0) {
        char msg[] = "Erro: Divisao por zero!\n";
        serial_write(0x3F8, msg, sizeof(msg) - 1);
    } 
    // 2. Tratamento do Teclado
    else if (interrupt == 33) {
        unsigned char scan_code = inb(KBD_DATA_PORT);

        // Verifica Shift solto
        if (scan_code == 0xAA || scan_code == 0xB6) {
            shift_pressed = 0; 
        }
        // Verifica Shift apertado
        else if (scan_code == 0x2A || scan_code == 0x36) {
            shift_pressed = 1; 
        }
        // Tecla normal (Make Code)
        else if (!(scan_code & 0x80) && scan_code < 128) {
            char letra = (shift_pressed) ? kbd_us_shift[scan_code] : kbd_us[scan_code];

            if (letra != 0) {
                char msg_tecla[2] = {letra, '\0'};
                fb_write(msg_tecla, 1);
            }
        }
    }

    // 3. Avisar o Controlador (EOI) - ISSO DEVE ESTAR DENTRO DA FUNÇÃO
    if (interrupt >= 32 && interrupt <= 47) {
        if (interrupt >= 40) {
            outb(PIC2_COMMAND, PIC_EOI);
        }
        outb(PIC1_COMMAND, PIC_EOI);
    }
} // <--- Esta é a única chave que deve fechar a função inteira!