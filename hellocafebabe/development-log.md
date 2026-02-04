# Felipe Cavalcanti Apolinario

## A ideia aqui é treinar um mini OS, que vai ser utilizado com o GRUB. A única coisa que esse sistema operacional faz é escrever:

0xCAFEBABE, no registrador eax.

Pra compilar esse mini sistema operacional, devemos criar um arquivo loader.s, e escrever um pouco de código assembly.

## ELF

ELF é um formato de arquivo usado para executáveis e objetos de código. O kernel é um ELF nesse projeto. O livro instrui a compilar o kernel desse sistema operacional como um arquivo executável ELF de 32 bits. A utilização dele deve-se ao fato de que o bootloader já entende o formato ELF. O GRUB sabe como ler o arquivo, carregar ele na posição correta de memória, sem se preocupar com esses detalhes. 

Mais pro final do final do livro, é sugerido não usar ELF inicialmente, por conta de alguns detalhes. Mas por enquanto, pra realizar esse exercício do hellocafebabe do início do livro, vamos utilizar.

## SOBRE O VALOR MÁGICO

O livro diz:

O GRUB transferirá o controle para o sistema operacional ao pular para uma posição na memória. Antes do salto, o GRUB procurará por um número mágico para garantir que está realmente pulando para um sistema operacional e não para algum código aleatório. Esse número mágico faz parte da especificação multiboot [@multiboot], à qual o GRUB adere. Uma vez que o GRUB realiza o salto, o sistema operacional tem controle total do computador.

 -> O VALOR MOSTRADO NO CÓDIGO NÃO É ALEATÓRIO, É DEFINIDO PELO MULTIBOOT (0x1BADB002)

## CÁLCULO DO CHECKSUM

O Cálculo do checksum é feito da seguinte forma: (número mágico + checksum + flags deve resultar em 0)

## COMPILAÇÃO

Pra compilar em ELF 32 bits: 

`nasm -f elf32 loader.s`

## LINKANDO O KERNEL

Agora o código tem que ser linkado pra produzir um arquivo executável. O Grub tem que carregar o kernel num endereço de memória que seja maior ou igual 0x00100000 (1 MB), porque endereços menores que 1MB são usados pelo próprio GRUB, BIOS e dispositivos de entrada e saída. Ou seja, vai estar ocupado. Então pra definir isso, criaremos um arquivo link.ld

## OBTENDO O GRUB 

Aqui a gente vai copiar um arquivo, que é o arquivo binário do GRUB 0.97. Colocar na mesma pasta que o loader, link, etc.

## CRIANDO A ISO

Vamos criar a imagem ISO do kernel com o programa genisoimage. Tem que ter uma estrutura de pastas específica, então foi criada uma ramificação de pasta iso/boot/grub. E copiar o arquivo de grub e o arquivo de kernel. Deve ser também criado um arquivo de configuração chamado menu.lst para o GRUB. Esse arquivo diz pro GRUB onde o kernel tá localizado e algumas configurações a mais.

## CONTINUA A PARTIR DAQUI: 

The ISO image can then be generated with the following command:
