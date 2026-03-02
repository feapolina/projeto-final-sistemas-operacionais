global loader                   ; Declara o símbolo 'loader' como global para ser acessado externamente
extern kmain                   ; Declara a função 'kmain' como externa, definida em outro arquivo

MAGIC_NUMBER equ 0x1BADB002     ; Número mágico usado pelo Multiboot para identificar o kernel
FLAGS        equ 0x0            ; Flags para o cabeçalho Multiboot
CHECKSUM     equ -MAGIC_NUMBER  ; Checksum para validar o cabeçalho Multiboot

section .text:                  ; Seção de código executável
align 4                         ; Alinha o código a um limite de 4 bytes
    dd MAGIC_NUMBER             ; Escreve o número mágico no cabeçalho
    dd FLAGS                    ; Escreve os flags no cabeçalho
    dd CHECKSUM                 ; Escreve o checksum no cabeçalho

loader:
    mov esp, kernel_stack + KERNEL_STACK_SIZE   ; Configura o ponteiro de pilha para o topo da pilha do kernel
    call kmain                                  ; Chama a função principal do kernel

    .loop:
        jmp .loop                              ; Loop infinito para evitar execução fora do kernel

KERNEL_STACK_SIZE equ 4096      ; Define o tamanho da pilha do kernel como 4 KB

section .bss                    ; Seção de dados não inicializados
align 4                         ; Alinha os dados a um limite de 4 bytes
kernel_stack:
    resb KERNEL_STACK_SIZE      ; Reserva espaço para a pilha do kernel