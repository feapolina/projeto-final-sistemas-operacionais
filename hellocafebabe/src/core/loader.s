global loader                     ; Declara o símbolo 'loader' como global para ser acessado externamente
extern kmain                      ; Declara a função 'kmain' como externa, definida em outro arquivo

MAGIC_NUMBER   equ 0x1BADB002     ; Número mágico usado pelo Multiboot para identificar o kernel
ALIGN_MODULES  equ 0x00000001     ; Flag para alinhar módulos carregados pelo GRUB
CHECKSUM       equ -(MAGIC_NUMBER + ALIGN_MODULES) ; Checksum para validar o cabeçalho Multiboot

section .text                     ; Seção de código executável
align 4                           ; Alinha o código a um limite de 4 bytes

dd MAGIC_NUMBER                   ; Escreve o número mágico no cabeçalho
dd ALIGN_MODULES                  ; Escreve as flags no cabeçalho
dd CHECKSUM                       ; Escreve o checksum no cabeçalho

loader:
    mov esp, kernel_stack + KERNEL_STACK_SIZE   ; Configura o ponteiro de pilha para o topo da pilha do kernel

    ; ---------------------------------------------------------
    ; ↓ Implementação do Identity Mapping dos primeiros 4 MB
    ; ---------------------------------------------------------

    ; ↓ Preencher a boot_page_table1 (Mapear 1024 páginas de 4 KB (totalizando 4 MB).)
    mov edi, boot_page_table1   ; EDI aponta para o início da Page Table
    mov edx, 0x00000003         ; [CORREÇÃO] Usando EDX para preservar o EBX (Multiboot)!
    mov ecx, 1024               ; Número de entradas a preencher (1024)

.fill_table_loop:
    mov [edi], edx                     ; Escreve a entrada atual na tabela
    add edx, 4096                      ; Avança 4 KB (0x1000) para o próximo endereço físico
    add edi, 4                         ; Avança 4 bytes para a próxima entrada na tabela
    loop .fill_table_loop              ; Decrementa ECX e repete até ser 0

    ; ↓ Inserir a boot_page_table1 no boot_page_directory
    ; ↓ O identity mapping dos primeiros 4 MB corresponde à entrada 0 do Page Directory.

    mov eax, boot_page_table1      ; Carrega o endereço da tabela que acabámos de preencher
    or eax, 0x00000003             ; Adiciona as flags de Presente (bit 0) e Leitura/Escrita (bit 1)
    mov [boot_page_directory], eax ; Guarda na primeira entrada (índice 0) do diretório

    ; ---------------------------------------------------------
    ; ↓ Adicionar a entrada para 0xC0100000 -> 0x00100000
    ; ---------------------------------------------------------
    ; ↓ Inserir a MESMA tabela na entrada 768 do diretório (Higher-Half Mapping)
    mov eax, boot_page_table1
    or eax, 0x00000003
    mov [boot_page_directory + 768 * 4], eax

    ; ---------------------------------------------------------
    ; ↓ Ativar a Paginação (CR3, CR4, CR0)
    ; ---------------------------------------------------------

    ; ↓ 1. Informar ao CPU onde está o nosso Page Directory (Registo CR3)
    mov eax, boot_page_directory
    mov cr3, eax

    ; ↓ 2. Configurar o Registo CR4 (Extensão de Tamanho de Página)
    ; O bit 4 do CR4 ativa o PSE (Page Size Extension). Embora estejamos a usar 
    ; páginas de 4 KB agora, é bom ativá-lo caso a gente queira usar páginas de 4 MB no futuro.
    mov eax, cr4
    or eax, 0x00000010          ; Define o bit 4 (PSE) a 1
    mov cr4, eax

    ; ↓ 3. Ligar efetivamente a Paginação (Registo CR0)
    ; O bit 31 do CR0 é o bit PG (Paging). Quando o ligamos, o CPU passa a traduzir
    ; todos os endereços virtuais para físicos usando as nossas tabelas.
    mov eax, cr0
    or eax, 0x80000000          ; Define o bit 31 (PG) a 1
    mov cr0, eax

    ; ---------------------------------------------------------
    ; ↓ Fazer o jump para o higher-half
    ; ---------------------------------------------------------

    lea ecx, [higher_half]      ; Carrega o endereço da label (que o linker acha que é 0x00100xxx)
    add ecx, 0xC0000000         ; Soma 3 GB para apontar para o endereço virtual correto
    jmp ecx                     ; Faz um salto absoluto para a zona alta da memória

higher_half:
    ; =========================================================
    ; ↓ A partir desta linha, o EIP (Instruction Pointer) está na zona dos 3 GB (0xC01000xx)
    ; =========================================================

    ; ↓ Remover identity mapping temporário e invalidar TLB

    ; ↓ 1. Colocar a entrada 0 do Page Directory a zero (desmapeia os primeiros 4 MB)
    mov dword [boot_page_directory], 0

    ; ↓ 2. Invalidar a TLB
    ; 'invlpg' serve para invalidar páginas individuais (4 KB).
    ; Como acabámos de desmapear um bloco inteiro de 4 MB (uma entrada inteira do diretório),
    ; a forma correta e mais eficiente no x86 é recarregar todo o registo CR3.
    ; Ao escrever no CR3, o CPU limpa automaticamente toda a TLB não global.
    mov ecx, cr3
    mov cr3, ecx

    ; ↓ 3. Atualizar o Stack Pointer (Pilha)
    ; Como o linker script (link.ld) ainda aponta para 1 MB físico, o ESP 
    ; configurado no início está a apontar para o endereço físico desmapeado.
    ; Precisamos de somar 3 GB ao ESP para que a pilha funcione no novo mapeamento!
    add esp, 0xC0000000

    ; ↓ (O código para chamar o kernel em C)
    push ebx                    ; [CORRETO] Agora sim, EBX contém os dados do Multiboot intocados!
    call kmain                  
    add esp, 4

.loop:
    jmp .loop                      ; Loop infinito para evitar execução fora do kernel

KERNEL_STACK_SIZE equ 4096         ; Define o tamanho da pilha do kernel como 4 KB

section .bss                       ; Seção de dados não inicializados
align 4096                         ; [CORREÇÃO] Alinha os dados a um limite de 4096 bytes (4 KB)

; ---------------------------------------------------------
; ↓ Estruturas de Paginação (Paging)
; ---------------------------------------------------------

global boot_page_directory
boot_page_directory:
    resb 4096                      ; Reserva 4 KB para o Page Directory (1024 entradas)

global boot_page_table1
boot_page_table1:
    resb 4096                      ; Reserva 4 KB para a primeira Page Table (cobre os primeiros 4 MB)

; ---------------------------------------------------------
; ↓ Pilha do Kernel
; ---------------------------------------------------------
align 4                            ; A pilha não precisa de alinhamento de 4 KB, 4 bytes é suficiente
kernel_stack:
    resb KERNEL_STACK_SIZE         ; Reserva espaço para a pilha do kernel