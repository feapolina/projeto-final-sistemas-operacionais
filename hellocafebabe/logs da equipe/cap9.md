Para começar a implementar a paginação e mover o kernel para o higher-half, o primeiro passo é alocar espaço na memória para estruturas de paginação. No caso, é o Page Directory e a Page Table.

Devm ter exatamente 4KB de taamnho e devem ser alinhadas na memória a um limite de 4KB. Cada uma dessas estruturas terá 1024 entradas de 4 bytes.
Precisamos modificar a seção .bss do arquivo loader.s

Definimos um align 4096 pois a MMU exige que o endereço físico do page directory e das page tabls termine com 3 zeros em hexadecimal, pois os 12 bits menos significativos são usados para flags como leitura/escrita, etc.

Alocamos também o 4096 bytes em boot_page_directory pra ser carregado no registrador cr3 mais tarde. E pode gerenciar até 4gb de memória ou 1024 tabelas de página.

e em boot_page_table1 a gente aloca mais 4096 bytes. Essa tabela mapeará os primeiros 4MB da memória física.

Logo em seguida vem o preenchimento das estruturas em tempo de execução com o identity mapping dos primeiros 4MB. O objetivo desse identity mapping é fazer com que o endereço virtual aponte exatamente pro mesmo endereço físico. É crucial na paginação pro processador traduzir os ndereços. Se isso for mapeado incorretamnte o sistema gera uma Page Fault.

Em seguida, o objetivo foi fazer com que o endereço virtual (que fica na zona dos 3gb) aponte pro endereço físico na marca dos 1MB, onde o kernel está carregado fisicamente pelo GRUB. A tabela que fizemos no boot_page_table1 já mapeia os primeiros 4mb de memória física. O que a gente precisa fazer agora é dizer ao Page Directory para usar a mesma tabela numa posição diferente.

A gente divide a entrada que controla o endereço por 4MB, que dá 768 em decimal. Como cada entrada ocupa 4 bytes, a posição na memória será o indice 768 multiplicado por 4.

Quando a paginação for ativada, a memória vai estar "duplicada" digamos assim. Se a CPU tentar ler o endereço virtual, o diretório olha para a entrada 0, usa a boot_page_table1 ee vai ter o endereço físico.
Se a CPU tentar ler o endereço virtual 0xC0100000, o diretório olha para a entrada 768, usa a mesma boot_page_table1 e vai ter o endereço físico 0X00100000.

Depois de setar tudo, hora de fazer o processador começar a usar as estruturas de paginação que fizmos. Para isso, precisamos interagir com 3 registros de controle específicos: CR3, CR4 E CR0.

e aí a partir do momento que a instrução mov cr0, eax é executada, os endereços de memória passam a ser virtuais. Como a gente fez o identity mapping, o processador consegue ler a proxima instrução do código sem dar um Page Fault. E o kernel também já está acessível através do endereço virtual "alto". (0XC0100000)

Após isso, fazemos o salto para o higher-half. O último passo é essencial pra segurança e pra estabilidade que é: já que estamos executando código na higher-half, que é acima dos 3gb, não precisamos mais que os endereços baixos sejam válidos. Remover esse mapeamento inicial traz uma vantagem que se o código C tentar um endereço 0x0, a cpu vai gerar um page fault e avisar do erro imediatamente, ao invés de ler o lixo silenciosamente. Como a gente também alterou as estruturas de paginação, a gente precisa avisar o processador pra esquecer as traduções antigas que estão em cache na TLB.

## Marcos Oliveira - parte 2, cap.9

Como o mapeamento já foi realizado anteriormente, precisamos, primeiramente fazer a alteração no linker script para que o kernel passe a morar no endereço virtual alto. Altera-se, portanto, de 0x00100000 para 0xC0100000 como endereço inicial. Em seguida, utilizamos a diretiva AT(..) para o .text, .rodata, .data, e .bss para que o endereço da seção seja carregado no endereço físico real. A diretiva faz o seguinte: AT(ADDR(.seção) - 0xC0000000), onde o endereço virtual da seção é subtraído pelo offset 0xC0000000, chegando assim, no endereço físico. 

Após isso, será necessário alterar os endereços que foram adicionados nos códigos .C, porque antes da memória virtual os endereços eram acessados diretamente na ram, agora precisamos fazer com que os códigos trabalhem com a memória virtual. Para isso, fb.c será alterado de char *fb = (char*) 0x000B8000; para char *fb = (char*) 0xC00B8000; já que o framebuffer é mapeado na memória ram.

O próximo ajuste será no ponteiro do multiboot, saindo de multiboot_info_t *mbinfo = (multiboot_info_t *) ebx; para multiboot_info_t *mbinfo = (multiboot_info_t *) (ebx + 0xC0000000); o endereço físico passado em ebx pelo GRUB precisa ser convertido manualmente para o endereço virtual correspondente no higher half, somando o offset 0xC0000000.. Assim também, será feito para os outros ponteiros do multiboot.