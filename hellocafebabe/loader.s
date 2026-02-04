global loader                             ; simbolo de entrada pro ELF

MAGIC_NUMBER equ 0x1BADB002               ; define uma constante com o "número mágico" (veja o README)
FLAGS equ 0X0                             ; flags tambem definidas pelo multiboot
CHECKSUM equ -MAGIC_NUMBER                ; calcula a verificacao do numero magico (veja o README)

section .text:                            ; começo da seção de texto (código em si)
align 4                                   ; faz com que o codigo precise ser alinhado em 4 bits
    dd MAGIC_NUMBER                       ; escreve o numero magico em codigo de maquina,
    dd FLAGS                              ; as flags tambem,
    dd CHECKSUM                           ; e o checksum tambem.

loader:                                   ; o rotulo label (definido como o ponto de entrada do script de linkagem )
    mov eax, 0xCAFEBABE                   ; coloca o numero 0xCAFEBABE no registrador eax 
.loop:
    jmp .loop                             ; loop infinito