#include "../drivers/fb.h"
#include "../drivers/serial.h"
#include "../drivers/io.h"
#include "gdt.h"
#include "../interrupts/interrupts.h"
#include "../interrupts/idt.h"
#include "multiboot.h"
#define KERNEL_VIRTUAL_BASE 0XC0000000
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

    char msg_serial[] = "Comunicacao via porta serial do PC ativa com sucesso!\n";
    serial_write(0x3F8, msg_serial, sizeof(msg_serial) - 1);

    /* =========================
       CAPITULO 7 - MULTIBOOT
       ========================= */

    /* Passo 1: interpreta o valor de ebx como ponteiro para a estrutura multiboot_info */
    char msg_step1[] = "Passo 1: lendo multiboot...\n";
    fb_write(msg_step1, sizeof(msg_step1) - 1);

    multiboot_info_t *mbinfo = (multiboot_info_t *) (ebx + KERNEL_VIRTUAL_BASE);

    /* Passo 2: verifica se o GRUB carregou modulos */
    char msg_step2[] = "Passo 2: verificando flags...\n";
    fb_write(msg_step2, sizeof(msg_step2) - 1);

    /* bit 3 (0x8) indica que mods_count e mods_addr sao validos */
    if (!(mbinfo->flags & 0x8)) {
        char msg_no_modules[] = "ERRO: GRUB nao carregou modulos.\n";
        fb_write(msg_no_modules, sizeof(msg_no_modules) - 1);
        while (1) {}
    }

    /* Passo 3: verifica se exatamente um modulo foi carregado */
    char msg_step3[] = "Passo 3: verificando mods_count...\n";
    fb_write(msg_step3, sizeof(msg_step3) - 1);

    if (mbinfo->mods_count != 1) {
        char msg_mod_count[] = "ERRO: mods_count diferente de 1.\n";
        fb_write(msg_mod_count, sizeof(msg_mod_count) - 1);
        while (1) {}
    }

    /* Passo 4: obtem o endereco do primeiro modulo carregado */
    char msg_step4[] = "Passo 4: obtendo modulo...\n";
    fb_write(msg_step4, sizeof(msg_step4) - 1);

    multiboot_module_t *mods = (multiboot_module_t *) (mbinfo->mods_addr + KERNEL_VIRTUAL_BASE);
    unsigned int mod_start = (unsigned int) (mods[0].mod_start + KERNEL_VIRTUAL_BASE);

    /* Passo 5: exibe mensagem antes de executar o modulo */
    char msg_step5[] = "Passo 5: executando modulo...\n";
    fb_write(msg_step5, sizeof(msg_step5) - 1);

    /* Passo 6: trata o endereco do modulo como funcao e executa */
    typedef void (*module_entry_t)(void);
    module_entry_t entry = (module_entry_t) mod_start;
    entry();

    /* Se o modulo retornar, o kernel entra em loop infinito */
    while (1) {}
    return 0;
}