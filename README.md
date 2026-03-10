# 🚀 Mini Sistema Operacional - Projeto Final

Este projeto foi desenvolvido para a disciplina de Sistemas Operacionais I, lecionada pelo professor Davi. O objetivo principal é a implementação passo a passo de um sistema operacional, seguindo as instruções e a arquitetura propostas pelo repositório e livro [The Little OS Book](https://littleosbook.github.io/).

---

## 👥 Equipe

O projeto foi construído em equipe pelos seguintes membros:

- Felipe Cavalcanti Apolinario
- Marcos Eduardo de Oliveira Souza
- Gustavo de Andrade Lacerda
- Isabella Nascimento de Sousa

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
