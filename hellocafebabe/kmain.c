#include "fb.h" /* Importa as funções que seu amigo criou */

int kmain(void) {
    /* * A função padrão do livro é: fb_write_cell(posição, caractere, cor_letra, cor_fundo)
     * Cor 2 = Verde, Cor 0 = Preto
     */
    fb_write_cell(0, 'O', 2, 0); 
    fb_write_cell(1, 'L', 2, 0);
    fb_write_cell(2, 'A', 2, 0);

    return 0xCAFEBABE;
}