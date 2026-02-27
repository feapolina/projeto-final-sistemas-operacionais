# Felipe Cavalcanti Apolinario

# Interrupções e Input

O capítulo 6 fala sobre a capacidade de adicionar Interrupções para que possamos ler informações sobre o teclado. Uma interrupção ocorre quando um dispositivo de hardware, como o teclado, a portal serial do timer sinaliza pra CPU que o estado do dispositivo mudou. A CPU por si própria também pode lançar interrupções por conta de erros de programas. Por exemplo: quando um programa referencia uma memória que não tem acesso, ou quando um programa divide um número por zero. E também tem as interrupções de software, que são interrupções que são causadas pela instrução de código assembly, que são comumente usada pra chamadas de sistema.

Interrupçõe são cuidadas pelo IDT (Tabela Descritora de Interrupção). O IDT descreve um handler para cada interrupção. As interrupções são numeradas de 0 a 255, e o handler para a interrupção i é definida para a i-ésima posição na tabela.

# O que foi desenvolvido por mim

Foi criado 3 arquivos:

- interrupts_asm.s (Contém os macros, como no livro. Tem as rotinas em assembly pra lidar com as interrupções, tem os stubs de ISRs) e IRQs (Interrupt Requests). Lá foi definido os pontos de entrada pra cada interrupção, empilha os registradores necessários, chama o handler em C, e retorna do tratamento da interrupção. Ele é essencial pra poder conectar o Assembly ao C.

- interrupts.h (É definido as interfaces e estruturas necessárias pro gerenciamento de interrupções no kernel, incluindo os protótipos de funções pra instalar e manipular handlers de interrupção. Também tem definições de tipos e constantes pra IDT, ISRs e IRQs. É o ponto central pra declarar funções e estruturas usadas na implementação do sistema de interrupções.)

- interrupts.c (Esse arquivo é o tratador em C, basicamente vai servir pra saber se está funcionando corretamente. Usei como exemplo aqui a divisão por zero, que geraria uma interrupção. Isso deve ser mostrado na interface do Qemu. E é mostrado.)

Tudo isso foi adicionado ao makefile. E conforme o livro, as interrupções que nativamente empilham códigos de erros são _8, 10, 11, 12, 13, 14 e 17_. Pras outras tem que usar o macro no_error_code_interrupt_handler pra manter essa pilha consistente.

Pra garantir que a compilação e a linkagem tá correta, podemos rodar:
nm kernel.elf | grep interrupt_handler

dá pra ver se os símbolos das interrupções criados pelo macro no Assembly estão lá.

Pra testar isso tudo antes de implementar a IDT, criei uma chamada manual no kmain, pra testar se a lógica de escrita no framebuffer pra interrupção 0 funciona. E isso aparece lá no Qemu.

# IDT

Seguindo o que o livro diz em seguida, devemos criar um IDT, então é adicionado um trecho de código ao assembly.

Com os handlers prontos, implementamos a IDT (Interrupt Descriptor Table) para catalogar essas rotinas e permitir que a CPU as localize. Isso envolveu a definição de estruturas para as entradas da tabela (idt_entry) e para o ponteiro de registro (idt_ptr), além de uma função de instalação para configurar cada "gate" com os seletores de segmento e atributos de privilégio corretos. Para ativar a tabela, foi desenvolvida a função idt_load em assembly, que executa a instrução lidt, informando formalmente ao processador o endereço base e o limite da nossa IDT.

Por fim, integramos todo o sistema no kmain.c, garantindo que a IDT seja carregada imediatamente após a inicialização da GDT. A validação do sistema foi feita através de testes práticos, utilizando tanto chamadas manuais ao handler quanto interrupções de software via instrução int $0. Esse processo confirmou que o fluxo de execução está funcionando corretamente: a CPU intercepta a interrupção, passa pelo stub assembly, executa a lógica em C e retorna com sucesso para a execução normal do kernel, conforme evidenciado pelas mensagens de status exibidas no terminal.
