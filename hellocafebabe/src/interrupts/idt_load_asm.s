global idt_load
idt_load:
    mov eax, [esp + 4]  ; Pega o ponteiro da IDT passado como argumento
    lidt [eax]          ; Carrega a IDT
    ret