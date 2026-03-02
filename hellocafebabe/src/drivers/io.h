// Prevenção de dupla inclusão (Include Guard):
// Evita que o compilador inclua este arquivo de cabeçalho mais de uma vez,
// que causaria erros de redefinição durante a compilação.
#ifndef INCLUDE_IO_H
#define INCLUDE_IO_H

/**
 * @brief Escreve um byte em uma porta de entrada/saída (I/O).
 * * @param port O endereço da porta de hardware (ex: 0x60 para o teclado, 0x20 para o PIC).
 * @param data O byte (8 bits) de dado que será enviado para a porta.
 */
void outb(unsigned short port, unsigned char data);

/**
 * @brief Lê um byte de uma porta de entrada/saída (I/O).
 * * @param port O endereço da porta de hardware de onde o dado será lido.
 * @return unsigned char O byte (8 bits) lido da porta especificada.
 */
unsigned char inb(unsigned short port);

/**
 * @brief Habilita as interrupções de hardware.
 * * Geralmente mapeia para a instrução Assembly 'sti' (Set Interrupt Flag) no x86.
 * Permite que a CPU comece a escutar sinais do hardware (como cliques do teclado).
 */
void enable_interrupts(void);

/**
 * @brief Desabilita as interrupções de hardware.
 * * Geralmente mapeia para a instrução Assembly 'cli' (Clear Interrupt Flag) no x86.
 */
void disable_interrupts(void);

#endif 