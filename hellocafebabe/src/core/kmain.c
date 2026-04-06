#include "../drivers/fb.h"
#include "../drivers/serial.h"
#include "../drivers/io.h"
#include "gdt.h"
#include "../interrupts/interrupts.h"
#include "../interrupts/idt.h"
#include "multiboot.h"
#include "../memory/pfa.h"                                                /* adcionei o header do allocator */
#include "../memory/vmm.h"                                              /* ADICIONADO: header do virtual memory manager */
#include "../memory/kheap.h"

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

    char msg_welcome[] = "Kernel Inicializado com sucesso!\n";
    fb_write(msg_welcome, sizeof(msg_welcome) - 1);

    char msg_interrupts[] = "Interrupcoes de hardware ativadas.\n";
    fb_write(msg_interrupts, sizeof(msg_interrupts) - 1);

    char msg_fb[] = "Driver do framebuffer carregado com sucesso!\n";
    fb_write(msg_fb, sizeof(msg_fb) - 1);

    serial_init(0x3F8);

    char msg_serial[] = "Comunicacao via porta serial do PC ativa com sucesso!\n";
    serial_write(0x3F8, msg_serial, sizeof(msg_serial) - 1);

    /* =========================
       CAPITULO 7 - MULTIBOOT
       ========================= */

    char msg_step1[] = "Passo 1: lendo multiboot...\n";
    fb_write(msg_step1, sizeof(msg_step1) - 1);

    multiboot_info_t *mbinfo = (multiboot_info_t *) (ebx + KERNEL_VIRTUAL_BASE);

    char msg_pfa[] = "Passo 2: inicializando page frame allocator...\n";  
    fb_write(msg_pfa, sizeof(msg_pfa) - 1);                               

    pfa_init(mbinfo);                                                     /* Inicia o allocator */

    char msg_pfa_ok[] = "Page frame allocator inicializado.\n";           
    fb_write(msg_pfa_ok, sizeof(msg_pfa_ok) - 1);                       

    /* Inicializa o VMM: limpa mapeamentos fantasma da região do heap */
    vmm_init();
    char msg_vmm_init[] = "VMM inicializado (regiao do heap liberada).\n";
    fb_write(msg_vmm_init, sizeof(msg_vmm_init) - 1);

                   

    /* =========================================================
       PARTE 4: TESTE DO TEMPORARY MAPPING (VMM)
       ========================================================= */
    char msg_vmm_test[] = "Testando Mapeamento Temporario...\n";
    fb_write(msg_vmm_test, sizeof(msg_vmm_test) - 1);

    // 1. Pede um bloco físico de 4KB para a placa-mãe
    unsigned int frame_fisico = pfa_alloc_frame();
    
    if (frame_fisico != 0) {
        // 2. Mapeia esse bloco físico no endereço virtual 0xC03FF000
        char *temp_page = (char *) vmm_temp_map_page(frame_fisico);

        // 3. Escrevemos direto no endereço virtual (a CPU traduz sozinha pro físico)
        temp_page[0] = '>'; temp_page[1] = '>'; temp_page[2] = ' '; 
        temp_page[3] = 'T'; temp_page[4] = 'E'; temp_page[5] = 'S'; temp_page[6] = 'T'; 
        temp_page[7] = 'E'; temp_page[8] = ' '; temp_page[9] = 'V'; temp_page[10] = 'M'; 
        temp_page[11] = 'M'; temp_page[12] = ' '; temp_page[13] = 'O'; temp_page[14] = 'K'; 
        temp_page[15] = '!'; temp_page[16] = '\n'; temp_page[17] = '\0';

        // 4. Lemos do endereço virtual e mandamos imprimir no monitor
        fb_write(temp_page, 17);

        // 5. Destrói o mapeamento virtual e devolve o bloco físico pro sistema
        vmm_temp_unmap_page();
        pfa_free_frame(frame_fisico);
    } else {
        char msg_vmm_err[] = "ERRO: Sem memoria fisica!\n";
        fb_write(msg_vmm_err, sizeof(msg_vmm_err) - 1);
    }

    /* =========================================================
       ENTREGA FINAL: TESTE DA API BASE (MAP / UNMAP / IS_MAPPED)
       ========================================================= */
    char msg_vmm_api[] = "Testando API base do VMM...\n";
    fb_write(msg_vmm_api, sizeof(msg_vmm_api) - 1);

    {
        // Endereco escolhido dentro da janela da boot_page_table1 e fora do slot temporario.
        // Apos vmm_init(), esta pagina esta desmapeada (pronta para uso).
        unsigned int test_virt = 0xC03E0000;
        unsigned int frame_vmm_api = pfa_alloc_frame();
        int rc;

        if (frame_vmm_api == 0) {
            char msg_api_no_mem[] = "ERRO: Sem frame fisico para teste da API VMM.\n";
            fb_write(msg_api_no_mem, sizeof(msg_api_no_mem) - 1);
        } else {
            // 1) Mapeia a pagina (deve funcionar, pois vmm_init limpou a regiao).
            rc = vmm_map_page(test_virt, frame_vmm_api, 0x02);
            if (rc == VMM_OK) {
                char msg_api_map_ok[] = "OK: vmm_map_page mapeou pagina com sucesso.\n";
                fb_write(msg_api_map_ok, sizeof(msg_api_map_ok) - 1);
            }

            // 2) Tenta mapear de novo no mesmo endereco (deve bloquear sobrescrita).
            rc = vmm_map_page(test_virt, frame_vmm_api, 0x02);
            if (rc == VMM_ERR_ALREADY_USED) {
                char msg_api_overwrite_blocked[] = "OK: VMM bloqueou sobrescrita acidental.\n";
                fb_write(msg_api_overwrite_blocked, sizeof(msg_api_overwrite_blocked) - 1);
            }

            // 3) Verifica se vmm_is_mapped confirma a presenca.
            if (vmm_is_mapped(test_virt)) {
                char msg_api_is_mapped[] = "OK: vmm_is_mapped confirmou pagina presente.\n";
                fb_write(msg_api_is_mapped, sizeof(msg_api_is_mapped) - 1);
            }

            // 4) Escreve e le no endereco virtual recem-mapeado.
            {
                char *test_page = (char *)test_virt;
                test_page[0] = 'V'; test_page[1] = 'M'; test_page[2] = 'M';
                test_page[3] = ' '; test_page[4] = 'O'; test_page[5] = 'K';
                test_page[6] = '\n';
                fb_write(test_page, 7);
            }

            // 5) Remove o mapeamento.
            rc = vmm_unmap_page(test_virt);
            if (rc == VMM_OK) {
                char msg_api_unmap[] = "OK: Unmap executado com sucesso.\n";
                fb_write(msg_api_unmap, sizeof(msg_api_unmap) - 1);
            }

            // 6) Segundo unmap no mesmo endereco deve avisar que ja estava vazio.
            rc = vmm_unmap_page(test_virt);
            if (rc == VMM_ERR_NOT_MAPPED) {
                char msg_api_second_unmap[] = "OK: Unmap detectou pagina ja desmapeada.\n";
                fb_write(msg_api_second_unmap, sizeof(msg_api_second_unmap) - 1);
            }

            pfa_free_frame(frame_vmm_api);
        }
    }
   //----------------------------------------------------
    char *texto_dinamico = (char *) kmalloc(50);
if (texto_dinamico != 0) {
    texto_dinamico[0] = 'H'; texto_dinamico[1] = 'E'; texto_dinamico[2] = 'A'; texto_dinamico[3] = 'P'; 
    texto_dinamico[4] = ' '; texto_dinamico[5] = 'O'; texto_dinamico[6] = 'K'; texto_dinamico[7] = '\n'; 
    texto_dinamico[8] = '\0';

    fb_write(texto_dinamico, 8);
    kfree(texto_dinamico);
}



    /* Passo 3: verifica se o GRUB carregou modulos */
    char msg_step2[] = "Passo 3: verificando flags...\n";                
    fb_write(msg_step2, sizeof(msg_step2) - 1);

    if (!(mbinfo->flags & 0x8)) {
        char msg_no_modules[] = "ERRO: GRUB nao carregou modulos.\n";
        fb_write(msg_no_modules, sizeof(msg_no_modules) - 1);
        while (1) {}
    }

    /* Passo 4: verifica se exatamente um modulo foi carregado */
    char msg_step3[] = "Passo 4: verificando mods_count...\n";            /* ALTERADO - numeracao */
    fb_write(msg_step3, sizeof(msg_step3) - 1);

    if (mbinfo->mods_count != 1) {
        char msg_mod_count[] = "ERRO: mods_count diferente de 1.\n";
        fb_write(msg_mod_count, sizeof(msg_mod_count) - 1);
        while (1) {}
    }

    /* Passo 5: obtem o endereco do primeiro modulo carregado */
    char msg_step4[] = "Passo 5: obtendo modulo...\n";                    /* ALTERADO - numeracao */
    fb_write(msg_step4, sizeof(msg_step4) - 1);

    multiboot_module_t *mods = (multiboot_module_t *) (mbinfo->mods_addr + KERNEL_VIRTUAL_BASE);
    unsigned int mod_start = (unsigned int) (mods[0].mod_start + KERNEL_VIRTUAL_BASE);

    /* Passo 6: exibe mensagem antes de executar o modulo */
    char msg_step5[] = "Passo 6: executando modulo...\n";
    fb_write(msg_step5, sizeof(msg_step5) - 1);

    typedef void (*module_entry_t)(void);
    module_entry_t entry = (module_entry_t) mod_start;
    entry();

    while (1) {}
    return 0;
}