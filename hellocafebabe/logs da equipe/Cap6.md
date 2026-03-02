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

Proximo passo: ativação das Interrupções Globais (STI)

Foi identificado que as interrupções de hardware estavam desligadas por padrão.

Criaram-se as funções enable_interrupts (que executa a instrução assembly sti) e disable_interrupts (com cli) no ficheiro io.s, com as respetivas declarações no io.h.

A chamada a enable_interrupts() foi integrada na sequência de arranque do kmain.c.

2. Remapeamento do PIC (Programmable Interrupt Controller)

Para evitar conflitos entre as IRQs de hardware e as exceções da própria CPU (índices 0-31), implementou-se a função pic_remap().

Esta função reconfigura o PIC1 e o PIC2 para mapear as interrupções de hardware para os índices da IDT a partir do 32 (ex: IRQ 0 passa a ser 32, IRQ 1 passa a ser 33).

Ajuste de Máscaras (Masking): Durante os testes, o Timer do sistema (PIT - IRQ 0) estava a inundar a CPU com interrupções, causando o bloqueio do sistema de forma silenciosa. A função pic_remap foi ajustada para enviar as máscaras 0xFD (PIC1) e 0xFF (PIC2), silenciando todas as interrupções exceto a do teclado (bit 1 ativado).

3. Registo da Interrupção do Teclado na IDT

Foi adicionada a macro no_error_code_interrupt_handler 33 no ficheiro interrupts_asm.s para lidar com a IRQ 1 (Teclado).

A entrada foi devidamente registada na tabela através de idt_set_gate(33, ...) no ficheiro idt.c.

4. Correção de Bugs Críticos na Passagem de Estado e EOI

Correção do EOI (End of Interrupt): O sinal de confirmação para o PIC1 estava incorretamente isolado dentro de um bloco condicional destinado apenas ao PIC2. O código foi corrigido para garantir que o PIC1 recebe sempre o sinal outb(0x20, 0x20) nas interrupções de hardware. Sem isto, o teclado bloqueava após o primeiro toque.

Alinhamento da Pilha (Stack): Havia uma divergência entre a ordem com que o Assembly colocava os dados na pilha e a assinatura da função em C. A assinatura da função foi corrigida no interrupts.c e interrupts.h para void interrupt_handler(struct cpu_state cpu, unsigned int interrupt, struct stack_state stack), garantindo que a variável interrupt recebesse o número correto disparado pelo hardware.

5. Leitura de Scan Codes e Filtro de Eventos

A interrupção 33 foi configurada para ler obrigatoriamente a porta de dados do teclado (0x60 via inb()), uma exigência do hardware para esvaziar o buffer e permitir a leitura de teclas subsequentes.

Implementou-se uma lógica de filtragem com operações de bits (!(scan_code & 0x80)) para diferenciar Make Codes (tecla pressionada) de Break Codes (tecla solta).

Com esta filtragem, o kernel reage agora de forma precisa, imprimindo a mensagem "tecla pressionada!" no ecrã exclusivamente no momento em que o utilizador prime a tecla, ignorando o evento redundante gerado ao soltá-la.
