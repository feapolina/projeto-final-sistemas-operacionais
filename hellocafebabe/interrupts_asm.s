; interrupts_asm.s
section .text
extern interrupt_handler        ; Função definida no arquivo C

; --- 6.5 DO LIVRO (IDT) ---
global load_idt
load_idt:
    mov     eax, [esp + 4]    ; Carrega o endereço do descritor da IDT (stack: [esp+4]) [cite: 141, 145-146]
    lidt    [eax]             ; Executa a instrução lidt 
    ret                       ; Retorna para a função chamadora [cite: 150, 152]
; ---------------------------------------------------

%macro no_error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
    push    dword 0             ; Código de erro fictício
    push    dword %1            ; Número da interrupção
    jmp     common_interrupt_handler
%endmacro

%macro error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
    push    dword %1            ; Número da interrupção (o erro já foi empilhado pela CPU)
    jmp     common_interrupt_handler
%endmacro

common_interrupt_handler:
    ; 1. Salva os registradores (Estado da CPU)
    ; A ordem deve bater com a struct cpu_state em C
    push    ebp
    push    edi
    push    esi
    push    edx
    push    ecx
    push    ebx
    push    eax

    ; 2. Chama a função C
    call    interrupt_handler

    ; 3. Restaura os registradores
    pop     eax
    pop     ebx
    pop     ecx
    pop     edx
    pop     esi
    pop     edi
    pop     ebp

    ; 4. Limpa o número da interrupção e o código de erro da pilha
    add     esp, 8

    ; 5. Retorna da interrupção
    iret


; Definição das interrupções (Exemplos baseados no livro) [cite: 268, 343-349]
no_error_code_interrupt_handler 0 ; Divide-by-zero
no_error_code_interrupt_handler 1 ; Debug
; ... (Interrupções 2 a 6)
error_code_interrupt_handler    8 ; Double Fault (tem código de erro)
error_code_interrupt_handler    10 ; Invalid TSS
error_code_interrupt_handler    11 ; Segment Not Present
error_code_interrupt_handler    12 ; Stack-Segment Fault
error_code_interrupt_handler    13 ; General Protection Fault

; IRQs principais
no_error_code_interrupt_handler 32 ; Timer (IRQ0)
no_error_code_interrupt_handler 33 ; Teclado (IRQ1)
error_code_interrupt_handler    14 ; Page Fault
