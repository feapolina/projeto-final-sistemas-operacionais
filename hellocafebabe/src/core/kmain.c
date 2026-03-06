#include "../drivers/fb.h"
#include "../drivers/serial.h"
#include "../drivers/io.h"
#include "gdt.h"
#include "../interrupts/interrupts.h"
#include "../interrupts/idt.h"
#include "multiboot.h"

#define FB_GREEN 2
#define FB_DARK_GREY 8

int kmain(unsigned int ebx)
{
    /* 1. Inicializa a GDT (Segmentação de Memória) */
    init_gdt();
    idt_install();

    /* 2. Remapeia o PIC */
    pic_remap();

    /* 3. Habilita as interrupções de hardware */
    enable_interrupts();

    /* Se a GDT nao funcionar, isso aqui não aparecerá. */
    char msg_welcome[] = "Kernel Inicializado com sucesso!\n";
    fb_write(msg_welcome, sizeof(msg_welcome) - 1);

    /* Se GDT ou IDT nao tiver implementado corretamente, causaria triple fault e isso nao apareceria tambem. */
    char msg_interrupts[] = "Interrupcoes de hardware ativadas.\n";
    fb_write(msg_interrupts, sizeof(msg_interrupts) - 1);

    /* Teste do driver do framebuffer */
    char msg_fb[] = "Driver do framebuffer carregado com sucesso!\n";
    fb_write(msg_fb, sizeof(msg_fb) - 1);

    /* Inicializa COM1 e escreve na serial */
    serial_init(0x3F8);

    char msg_serial[] = "Comunicação via porta serial do PC ativa com sucesso!\n";
    serial_write(0x3F8, msg_serial, sizeof(msg_serial) - 1);

    /* 4. Interpreta o valor de ebx como ponteiro para a estrutura multiboot_info */
    multiboot_info_t *mbinfo = (multiboot_info_t *) ebx;

    /* 5. Verifica se o GRUB carregou módulos */
    if (!(mbinfo->flags & 0x8)) {
        char msg_no_modules[] = "ERRO: GRUB nao carregou modulos.\n";
        fb_write(msg_no_modules, sizeof(msg_no_modules) - 1);
        while (1) {}
    }

    /* 6. Verifica se exatamente um módulo foi carregado */
    if (mbinfo->mods_count != 1) {
        char msg_mod_count[] = "ERRO: mods_count diferente de 1.\n";
        fb_write(msg_mod_count, sizeof(msg_mod_count) - 1);
        while (1) {}
    }

    /* 7. Obtém o endereço do primeiro módulo carregado */
    multiboot_module_t *mods = (multiboot_module_t *) mbinfo->mods_addr;
    unsigned int mod_start = mods[0].mod_start;

    /* 8. Exibe mensagem antes de executar o módulo */
    char msg_module[] = "Executando modulo...\n";
    fb_write(msg_module, sizeof(msg_module) - 1);

    /* 9. Trata o endereço do módulo como função e executa */
    typedef void (*module_entry_t)(void);
    module_entry_t entry = (module_entry_t) mod_start;
    entry();

    /* Se o módulo retornar, o kernel entra em loop infinito */
    while (1) {}
    return 0;
}