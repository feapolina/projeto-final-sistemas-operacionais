#ifndef INCLUDE_IO_H
#define INCLUDE_IO_H

void outb(unsigned short port, unsigned char data);
unsigned char inb(unsigned short port);
void enable_interrupts(void);
void disable_interrupts(void);

#endif