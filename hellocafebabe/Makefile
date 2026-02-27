CORE = src/core/kmain.c src/core/gdt.c
DRIVERS = src/drivers/fb.c src/drivers/serial.c
INTERRUPTS = src/interrupts/interrupts.c src/interrupts/idt.c
ASM = src/core/loader.s src/drivers/io.s src/core/gdt_asm.s src/interrupts/interrupts_asm.s src/interrupts/idt_load_asm.s

OBJECTS = $(CORE:.c=.o) $(DRIVERS:.c=.o) $(INTERRUPTS:.c=.o) $(ASM:.s=.o)
CC = gcc
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c \
         -Isrc/core -Isrc/drivers -Isrc/interrupts
LDFLAGS = -T src/core/link.ld -m elf_i386
AS = nasm
ASFLAGS = -f elf

all: kernel.elf

kernel.elf: $(OBJECTS)
	ld $(LDFLAGS) $(OBJECTS) -o kernel.elf

os.iso: kernel.elf
	mkdir -p iso/boot/grub
	cp kernel.elf iso/boot/kernel.elf
	cp stage2_eltorito iso/boot/grub/
	genisoimage -R                              \
	            -b boot/grub/stage2_eltorito    \
	            -no-emul-boot                   \
	            -boot-load-size 4               \
	            -A os                           \
	            -input-charset utf8             \
	            -quiet                          \
	            -boot-info-table                \
	            -o os.iso                       \
	            iso

run: os.iso
	qemu-system-i386 -cdrom os.iso

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -rf src/core/*.o src/drivers/*.o src/interrupts/*.o *.o kernel.elf os.iso