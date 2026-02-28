global outb ;Torna a função visível para outros arquivos (C poderá chamar)

; outb - send a byte to an I/O port
; stack: [esp + 8] the data byte
;        [esp + 4] the I/O port
;        [esp    ] return address

outb:
    mov al, [esp + 8] ; Coloca o dado (8 bits) no registrador AL
    mov dx, [esp + 4] ; Coloca o número da porta (16 bits) no registrador DX
    out dx, al        ; Envia o conteúdo de AL para a porta DX
    ret               ; Retorna para quem chamou (kmain ou fb_move_cursor)

global inb

; inb - retorna 1 byte lido de uma porta de I/O
; stack: [esp + 4] endereço da porta
;        [esp    ] return address

inb:
    mov dx, [esp + 4]
    in  al, dx
    ret

global enable_interrupts
global disable_interrupts

enable_interrupts:
    sti
    ret

disable_interrupts:
    cli
    ret