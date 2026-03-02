global gdt_flush

; A função gdt_flush recebe 1 parâmetro (o endereço do ponteiro da GDT)
gdt_flush:
    ; Pega o parâmetro que o C passou (o endereço) da pilha de memória
    mov eax, [esp+4]
    
    ; Carrega a nova GDT no processador usando o comando especial 'lgdt'
    lgdt [eax]

    ; Agora precisamos atualizar os registradores de segmento de dados (DS, ES, FS, GS, SS)
    ; O valor 0x10 é o "Deslocamento" (offset) do nosso Segmento de Dados na GDT.
    ; Lembra que ele era a Entrada 2 na tabela? (2 * 8 bytes cada linha = 16 = 0x10 em hexadecimal)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Por fim, precisamos atualizar o registrador de segmento de código (CS).
    ; Para isso, fazemos um "Far Jump" (Salto Longo).
    ; O valor 0x08 é o offset do nosso Segmento de Código.
    ; (Entrada 1 na tabela: 1 * 8 bytes = 8 = 0x08)
    jmp 0x08:.flush

.flush:
    ret ; Retorna para o código em C