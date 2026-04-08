# SOBRE A IMPLEMENTAÇÃO DE CI/CD COM GITHUB ACTIONS

Para garantir a estabilidade do sistema operacional e automatizar a geração e distribuição da imagem `.iso`, implementamos um fluxo de Integração Contínua (CI) e Entrega Contínua (CD) utilizando o GitHub Actions.

O processo foi dividido em duas frentes independentes que rodam em máquinas virtuais Ubuntu providenciadas pelo GitHub.

## 1. Integração Contínua (CI) - Validação de Builds

O objetivo do CI seria impedir que código quebrado seja integrado à branch principal. Ele é acionado automaticamente a cada `push` ou `pull_request` nas branches principais (`main` ou `master`). Porém, no nosso caso, por algum motivo, mesmo tendo erros no código, a verificação do github passa. Provavelmente há algum problema no meio do caminho.

**O que o workflow de CI faz:**

1. **Prepara o ambiente:** Cria uma instância limpa do Ubuntu.
2. **Instala dependências:** Baixa as ferramentas necessárias para compilar o SO: `build-essential` (GCC/Make), `nasm` (Assembly), e utilitários de disco como `genisoimage`, `grub-pc-bin` e `mtools`.
3. **Compilação:** Executa o comando `make` dentro da pasta `hellocafebabe`.
4. **Validação:** Caso ocorra qualquer erro de sintaxe no C ou Assembly, o processo falha e marca o commit com um erro (X vermelho), alertando a equipe.
5. **Artefatos:** Se a compilação for bem-sucedida, o arquivo `os.iso` é salvo como um artefato temporário que pode ser baixado diretamente da aba _Actions_.

## 2. Entrega Contínua (CD) - Geração de Releases

O CD automatiza a distribuição de versões fechadas do sistema, eliminando a necessidade de gerar a ISO manualmente para envio ao professor ou avaliadores. Ele é disparado apenas quando uma **Tag** de versão (ex: `v1.0`) é enviada ao repositório.

**O que o workflow de CD faz:**

1. **Build oficial:** Realiza o mesmo processo de compilação do CI para garantir a integridade.
2. **Criação de Release:** Cria automaticamente uma nova página na seção **Releases** do GitHub.
3. **Distribuição:** Gera um log de alterações automático e anexa a imagem final `os.iso` como um arquivo pronto para download público.

## Como testar e utilizar

- **Para testar o CI:** Crie um commit e suba para o GitHub. O Actions indicará se falhou ou passou.
- **Para acionar o CD:** Use os comandos de tag no terminal:
  ```bash
  git tag v1.0
  git push origin v1.0
  ```
  A ISO estará disponível em `Releases` após alguns minutos.
