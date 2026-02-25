int kmain(void) {
    /* 1. Criamos o ponteiro direto para a placa de vídeo */
    char *fb = (char *) 0x000B8000;
    
    /* 2. Injetamos as letras manualmente na memória 
       (O código 0x28 significa letra verde com fundo cinza escuro) */
    
    fb[0] = 'O';
    fb[1] = 0x28;
    
    fb[2] = 'L';
    fb[3] = 0x28;
    
    fb[4] = 'A';
    fb[5] = 0x28;
    
    return 0xCAFEBABE;
}