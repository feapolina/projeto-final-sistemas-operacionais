# 🚀 Mini Sistema Operacional - Projeto Final

Este projeto foi desenvolvido para a disciplina de Sistemas Operacionais I, lecionada pelo professor Davi. O objetivo principal é a implementação passo a passo de um sistema operacional, seguindo as instruções e a arquitetura propostas pelo repositório e livro [The Little OS Book](https://littleosbook.github.io/).

Importante ressaltar que existem alguns logs de desenvolvimento, detalhando melhor a implementação de algumas coisas. O caminho da pasta é src/logs da equipe/
---

## 👥 Equipe

O projeto foi construído em equipe pelos seguintes membros:

- Felipe Cavalcanti Apolinario
- Marcos Eduardo de Oliveira Souza
- Gustavo de Andrade Lacerda
- Isabella Nascimento de Sousa
- Yan Feitosa Cláudio

---

## 🛠️ Tecnologias e Ferramentas Utilizadas

Para garantir o controle preciso do código gerado e o acesso direto à memória, as seguintes tecnologias foram adotadas:

- **C (GCC):** Linguagem principal do kernel, escolhida por permitir manipulação de baixo nível e ser a linguagem padrão do livro.
- **Assembly (NASM):** Utilizado para escrever o código de inicialização (`loader.s`) e instruções específicas de hardware.
- **Bash & Make:** Utilizados como linguagens de script e automação da compilação através de arquivos Makefile.
- **GRUB (0.97):** Utilizado como bootloader (através da especificação Multiboot) para evitar a complexidade de interagir diretamente com a BIOS e carregar o arquivo executável ELF do kernel no local correto da memória.
- **QEMU:** Escolhido como emulador (testado no Ubuntu 24.04 LTS), uma vez que o Bochs apresentou problemas de compatibilidade com instruções de distribuições Linux modernas.
- **Genisoimage:** Ferramenta utilizada para gerar a imagem ISO (`os.iso`) inicializável do sistema.

---

## ▶️ Como Rodar o Projeto

Para executar o sistema operacional localmente em sua máquina, siga os passos abaixo:

### 1. Clonando o Repositório

Primeiramente, você precisa clonar o repositório oficial do projeto:

```bash
git clone [https://github.com/feapolina/projeto-final-sistemas-operacionais.git](https://github.com/feapolina/projeto-final-sistemas-operacionais.git)
cd projeto-final-sistemas-operacionais/hellocafebabe
```

### 2. Compilando e Executando

```bash
make clean
make os.iso
make run
```

O QEMU vai abrir com o sistema operacional rodando. As mensagens no framebuffer mostram o estado de cada subsistema durante o boot.

---

## 📂 Estrutura do Projeto

```
hellocafebabe/
├── src/
│   ├── core/           # Kernel principal (kmain, loader, GDT, linker script)
│   ├── drivers/        # Framebuffer, porta serial, I/O
│   ├── interrupts/     # IDT, handlers de interrupção, PIC
│   └── memory/         # PFA, VMM, Heap (kmalloc/kfree)
├── iso/                # Estrutura da ISO (GRUB + kernel + módulos)
├── logs da equipe/     # Documentação de cada etapa
└── Makefile
```

---

## 🧠 Entrega Final — VMM

O Virtual Memory Manager centraliza o controle da tabela de páginas do kernel. O que foi implementado:

- **`vmm_init()`** — Limpa os mapeamentos da região do heap que vieram prontos do boot, permitindo que o `kmalloc` funcione corretamente
- **`vmm_map_page()`** — Mapeia uma página virtual pra um frame físico, com proteção contra sobrescrita
- **`vmm_unmap_page()`** — Remove mapeamentos com detecção de double unmap
- **`vmm_is_mapped()`** — Consulta se uma página tá presente na tabela
- **Integração com o heap** — `morecore` agora usa `vmm_map_page` em vez de escrever direto na tabela
- **Diagnóstico de page fault** — Handler da interrupção 14 com leitura de CR2 e decodificação do error_code
- **Testes automatizados no boot** — Validam cada funcionalidade durante a inicialização

Mais detalhes em `logs da equipe/entrega-final.md`.
