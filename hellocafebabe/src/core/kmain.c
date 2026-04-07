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

int kmain(unsigned int ebx)
{
    /* Limpa textos residuais deixados pela BIOS ou GRUB*/
    fb_clear();
    log_info("Iniciando o Sistema Operacional...");
    /* 1. Inicializa a GDT (Segmentação de Memória) */
    init_gdt();
    idt_install();

    /* 2. Remapeia o PIC */
    pic_remap();

    /* 3. Habilita as interrupções de hardware */
    enable_interrupts();

    log_success("Kernel Inicializado com sucesso!");
    log_success("Interrupcoes de hardware ativadas.");
    log_success("Driver do framebuffer carregado.");

    /* Inicializa Serial */
    serial_init(0x3F8);
    char msg_serial[] = "Comunicacao via porta serial do PC ativa com sucesso!\n";
    serial_write(0x3F8, msg_serial, sizeof(msg_serial) - 1);
    log_success("Comunicacao via porta serial (COM1) ativa.");

    /* =========================
       CAPITULO 7 - MULTIBOOT
       ========================= */

    log_info("Lendo estruturas do Multiboot...");
    multiboot_info_t *mbinfo = (multiboot_info_t *) (ebx + KERNEL_VIRTUAL_BASE);

    log_info("Passo 2: inicializando page frame allocator...");
    pfa_init(mbinfo);                                                     /* Inicia o allocator */
    log_success("Page frame allocator inicializado.");

    /* Inicializa o VMM: limpa mapeamentos fantasma da região do heap */
    vmm_init();
    log_success("VMM inicializado (regiao do heap liberada).");

                   

    /* =========================================================
       PARTE 4: TESTE DO TEMPORARY MAPPING (VMM)
       ========================================================= */
    log_info("Testando Mapeamento Temporario (VMM)...");

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
        log_error("Falha no VMM temporario: Sem memoria fisica!");
    }

    /* =========================================================
       ENTREGA FINAL: TESTE DA API BASE (MAP / UNMAP / IS_MAPPED)
       ========================================================= */
        
       log_info("Testando API base do VMM...");

    {
        // Endereco escolhido dentro da janela da boot_page_table1 e fora do slot temporario.
        // Apos vmm_init(), esta pagina esta desmapeada (pronta para uso).
        unsigned int test_virt = 0xC03E0000;
        unsigned int frame_vmm_api = pfa_alloc_frame();
        int rc;

        if (frame_vmm_api == 0) {
            log_error("Sem frame fisico para teste da API VMM.");
        } else {
            // 1) Mapeia a pagina (deve funcionar, pois vmm_init limpou a regiao).
            rc = vmm_map_page(test_virt, frame_vmm_api, 0x02);
            if (rc == VMM_OK) {
                log_success("OK: vmm_map_page mapeou pagina com sucesso.");
            }

            // 2) Tenta mapear de novo no mesmo endereco (deve bloquear sobrescrita).
            rc = vmm_map_page(test_virt, frame_vmm_api, 0x02);
            if (rc == VMM_ERR_ALREADY_USED) {
                log_success("VMM bloqueou sobrescrita acidental.");
            }

            // 3) Verifica se vmm_is_mapped confirma a presenca.
            if (vmm_is_mapped(test_virt)) {
                log_success("OK: vmm_is_mapped confirmou pagina presente.");
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
                log_success("OK: Unmap executado com sucesso.");
            }

            // 6) Segundo unmap no mesmo endereco deve avisar que ja estava vazio.
            rc = vmm_unmap_page(test_virt);
            if (rc == VMM_ERR_NOT_MAPPED) {
                log_success("Unmap detectou pagina ja desmapeada.");
            }

            pfa_free_frame(frame_vmm_api);
        }
    }
    /* =========================================================
       TESTE DO KHEAP
       ========================================================= */
    char *texto_dinamico = (char *) kmalloc(50);
    if (texto_dinamico != 0) {
        texto_dinamico[0] = 'H'; texto_dinamico[1] = 'E'; texto_dinamico[2] = 'A'; texto_dinamico[3] = 'P'; 
        texto_dinamico[4] = ' '; texto_dinamico[5] = 'O'; texto_dinamico[6] = 'K'; texto_dinamico[7] = '\n'; 
        texto_dinamico[8] = '\0';

        log_success("Heap alocado com sucesso.");
        kfree(texto_dinamico);
    } else {
        log_error("Falha ao alocar memoria via kmalloc!");
    }

    /* =========================================================
       TESTE DO DEMAND PAGING (ENTREGA FINAL)
       ========================================================= */
    {
        char msg_dp[] = "Testando Demand Paging...\n";
        fb_write(msg_dp, sizeof(msg_dp) - 1);

        // Endereco na regiao do heap, desmapeado pelo vmm_init().
        // Ninguem fez vmm_map_page aqui — a pagina NAO existe na tabela.
        unsigned int demand_virt = 0xC03D0000;

        // 1) Confirma que a pagina nao esta mapeada antes do acesso.
        if (!vmm_is_mapped(demand_virt)) {
            char msg_not[] = "  Pagina 0xC03D0000 NAO mapeada (esperado).\n";
            fb_write(msg_not, sizeof(msg_not) - 1);
        }

        // 2) Escreve no endereco desmapeado.
        //    Isso dispara uma Page Fault (INT 14).
        //    O handler resolve a falha via demand paging:
        //      aloca frame -> mapeia -> zero-fill -> retorna (iret).
        //    A CPU reinicia esta instrucao e ela funciona normalmente.
        char *dp = (char *)demand_virt;
        dp[0] = 'D'; dp[1] = 'E'; dp[2] = 'M'; dp[3] = 'A';
        dp[4] = 'N'; dp[5] = 'D'; dp[6] = ' '; dp[7] = 'P';
        dp[8] = 'A'; dp[9] = 'G'; dp[10] = 'I'; dp[11] = 'N';
        dp[12] = 'G'; dp[13] = ' '; dp[14] = 'O'; dp[15] = 'K';
        dp[16] = '!'; dp[17] = '\n';

        fb_write(dp, 18);

        // 3) Confirma que agora a pagina esta mapeada (o handler resolveu).
        if (vmm_is_mapped(demand_virt)) {
            char msg_ok[] = "  Pagina mapeada sob demanda com sucesso!\n";
            fb_write(msg_ok, sizeof(msg_ok) - 1);
        }
    }

    /* Passo 3: verifica se o GRUB carregou modulos */
    char msg_step2[] = "Passo 3: verificando flags...\n";                
    fb_write(msg_step2, sizeof(msg_step2) - 1);
    /* =========================================================
       VERIFICAÇÃO DE MÓDULOS E TRANSIÇÃO PARA USERMODE
       ========================================================= */
    log_info("Verificando flags do GRUB...");

    if (!(mbinfo->flags & 0x8)) {
        log_error("GRUB nao carregou modulos.");
        while (1) {}
    }

    log_info("Verificando contagem de modulos (mods_count)...");
    if (mbinfo->mods_count != 1) {
        log_error("mods_count diferente de 1.");
        while (1) {}
    }

    log_success("Modulo de usuario detectado com sucesso.");
    multiboot_module_t *mods = (multiboot_module_t *) (mbinfo->mods_addr + KERNEL_VIRTUAL_BASE);
    unsigned int mod_start = (unsigned int) (mods[0].mod_start + KERNEL_VIRTUAL_BASE);

    log_warning("Executando modulo de usuario (Userland)...");

    typedef void (*module_entry_t)(void);
    module_entry_t entry = (module_entry_t) mod_start;
    entry();

    while (1) {}
    return 0;
}