#ifndef SERIAL_H
#define SERIAL_H

void serial_configure_baud_rate(unsigned short com, unsigned short divisor);
void serial_configure_line(unsigned short com);
void serial_configure_buffers(unsigned short com);
void serial_configure_modem(unsigned short com);
int serial_init(unsigned short com);
int serial_is_transmit_fifo_empty(unsigned short com);
void serial_write_char(unsigned short com, char a);
void serial_write(unsigned short com, char *buf, unsigned int len);

#endif