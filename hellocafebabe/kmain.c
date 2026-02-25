#include "fb.h"
#include "serial.h"

int kmain(void) {
    /* Criamos a nossa frase */
    char texto[] = "OLA";
    
    /* Usamos a função do seu amigo: passamos o texto e dizemos que tem 3 letras */
    fb_write(texto, 3);

    return 0xCAFEBABE;
}