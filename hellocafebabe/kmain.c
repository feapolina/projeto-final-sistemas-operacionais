int kmain(void) {
    /* 1. Ponteiro direto para a placa de vídeo */
    char *fb = (char *) 0x000B8000;
    
    /* 2. Injetamos as letras manualmente na memória */
    fb[0] = 'O'; fb[1] = 0x28;
    fb[2] = 'L'; fb[3] = 0x28;
    fb[4] = 'A'; fb[5] = 0x28;
    
    /* 3. A TRAVA MAGICA: Um loop infinito! 
       Isso faz o processador ficar rodando em círculos aqui para sempre, 
       mantendo a tela ligada com o nosso texto. */
    while(1) {
        /* Não faz nada, só espera */
    }

    return 0xCAFEBABE;
}