global loader
extern kmain

MAGIC_NUMBER      equ 0x1BADB002
ALIGN_MODULES     equ 0x00000001
CHECKSUM          equ -(MAGIC_NUMBER + ALIGN_MODULES)

KERNEL_VIRTUAL_BASE equ 0xC0000000
KERNEL_STACK_SIZE   equ 4096

section .multiboot                                  ; ADICIONADO - header multiboot em seção própria
align 4                                             ; ADICIONADO - alinhamento exigido pelo GRUB
dd MAGIC_NUMBER                                     ; ADICIONADO - magic multiboot
dd ALIGN_MODULES                                    ; ADICIONADO - flags
dd CHECKSUM  

section .text
align 4

dd MAGIC_NUMBER
dd ALIGN_MODULES
dd CHECKSUM

loader:
    ; Antes do paging, o kernel ainda está sendo executado a partir do endereço físico.
    ; Como os símbolos foram linkados no higher half, precisamos converter para físico.
    mov esp, kernel_stack + KERNEL_STACK_SIZE - KERNEL_VIRTUAL_BASE

    ; ---------------------------------------------------------
    ; Identity mapping dos primeiros 4 MB
    ; ---------------------------------------------------------

    ; Preenche boot_page_table1: 1024 entradas de 4 KB = 4 MB
    mov edi, boot_page_table1 - KERNEL_VIRTUAL_BASE
    mov edx, 0x00000003
    mov ecx, 1024

.fill_table_loop:
    mov [edi], edx
    add edx, 4096
    add edi, 4
    loop .fill_table_loop

    ; ---------------------------------------------------------
    ; Inserir a page table no page directory
    ; ---------------------------------------------------------

    ; Entrada 0 -> identity mapping
    mov eax, boot_page_table1 - KERNEL_VIRTUAL_BASE
    or eax, 0x00000003
    mov [boot_page_directory - KERNEL_VIRTUAL_BASE], eax

    ; Entrada 768 -> higher half (0xC0000000)
    mov eax, boot_page_table1 - KERNEL_VIRTUAL_BASE
    or eax, 0x00000003
    mov [boot_page_directory - KERNEL_VIRTUAL_BASE + 768 * 4], eax

    ; ---------------------------------------------------------
    ; Ativar a paginação
    ; ---------------------------------------------------------

    ; CR3 recebe o endereço físico do page directory
    mov eax, boot_page_directory - KERNEL_VIRTUAL_BASE
    mov cr3, eax

    ; Ativa PSE no CR4
    mov eax, cr4
    or eax, 0x00000010
    mov cr4, eax

    ; Ativa paging no CR0
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    ; ---------------------------------------------------------
    ; Jump para o higher half
    ; ---------------------------------------------------------

    ; higher_half já é um símbolo virtual alto por causa do linker script.
    mov ecx, higher_half
    jmp ecx

higher_half:
    ; =========================================================
    ; A partir daqui, o kernel já está executando em endereços virtuais altos.
    ; =========================================================

    ; Remover identity mapping temporário
    mov dword [boot_page_directory], 0

    ; Invalidar a TLB recarregando CR3
    mov ecx, cr3
    mov cr3, ecx

    ; Ajustar a pilha para o espaço virtual alto
    add esp, KERNEL_VIRTUAL_BASE

    ; Passa o ponteiro multiboot ao kmain
    push ebx
    call kmain
    add esp, 4

.loop:
    jmp .loop

section .bss
align 4096

global boot_page_directory
boot_page_directory:
    resb 4096

global boot_page_table1
boot_page_table1:
    resb 4096

align 4
kernel_stack:
    resb KERNEL_STACK_SIZE