#include "io.h"
#include "serial.h"

/* Base do COM1 no PC */
#define SERIAL_COM1_BASE 0x3F8

/* Offsets dos registradores */
#define SERIAL_DATA_PORT(base)              (base)
#define SERIAL_LINE_COMMAND_PORT(base)      (base + 3)
#define SERIAL_FIFO_COMMAND_PORT(base) (base + 2)
#define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base) (base + 5)

/*
 * Configura a taxa de transmissão (baud rate) definindo o divisor.
 * Exemplo: divisor 2 => 115200/2 = 57600
 */
void serial_configure_baud_rate(unsigned short com, unsigned short divisor)
{
    /* Habilita DLAB: diz ao hardware que agora vamos escrever o divisor */
    outb(SERIAL_LINE_COMMAND_PORT(com), 0x80);

    /* Envia o byte alto do divisor */
    outb(SERIAL_DATA_PORT(com), (divisor >> 8) & 0x00FF);

    /* Envia o byte baixo do divisor */
    outb(SERIAL_DATA_PORT(com), divisor & 0x00FF);
}

/*
 * Configura o formato da linha:
 * - 8 bits por caractere
 * - sem paridade
 * - 1 stop bit
 *
 * Valor 0x03: 0000 0011
 * bits 0-1 = 11 (8 bits)
 * bit 2    = 0  (1 stop bit)
 * bits 3-5 = 000 (sem paridade)
 */
void serial_configure_line(unsigned short com)
{
    outb(SERIAL_LINE_COMMAND_PORT(com), 0x03);
}

/*
 * Configura o FIFO (buffers) da serial.
 * 0xC7 é o valor usado pelo livro (habilita FIFO e limpa filas).
 */
void serial_configure_buffers(unsigned short com)
{
    outb(SERIAL_FIFO_COMMAND_PORT(com), 0xC7);
}

/*
 * Configura o modem control.
 * 0x03 é o valor usado pelo livro (ativa RTS e DTR).
 */
void serial_configure_modem(unsigned short com)
{
    outb(SERIAL_MODEM_COMMAND_PORT(com), 0x03);
}

/*
 * Inicializa a porta serial (ex: COM1 = 0x3F8).
 *
 * Etapas realizadas:
 * 1) Configura a taxa de transmissão (baud rate) através do divisor.
 * 2) Define o formato da linha (8 bits, sem paridade, 1 stop bit).
 * 3) Habilita e limpa os buffers FIFO.
 * 4) Configura o controle de modem (ativa RTS e DTR).
 *
 * Após esta função, a porta serial estará pronta para transmitir dados.
 */
int serial_init(unsigned short com)
{
    serial_configure_baud_rate(com, 2);
    serial_configure_line(com);
    serial_configure_buffers(com);
    serial_configure_modem(com);

    return 0;   
}

/* Retorna 1 quando o transmissor está pronto (bit 5 do LSR) */
int serial_is_transmit_fifo_empty(unsigned short com)
{
    return inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
}

/* Escreve 1 caractere quando o transmissor estiver pronto */
void serial_write_char(unsigned short com, char a)
{
    while (serial_is_transmit_fifo_empty(com) == 0) { }
    outb(SERIAL_DATA_PORT(com), a);
}

/*
 * Envia uma sequência de caracteres pela porta serial.
 *
 * Percorre o buffer 'buf' com tamanho 'len' e transmite
 * cada caractere individualmente através de serial_write_char.
 *
 * A transmissão é síncrona: cada caractere só é enviado
 * quando o hardware indica que o buffer está disponível.
 */
void serial_write(unsigned short com, char *buf, unsigned int len)
{
    unsigned int i;
    for (i = 0; i < len; i++) {
        serial_write_char(com, buf[i]);
    }
}