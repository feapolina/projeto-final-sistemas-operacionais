# Entrega Final — Gerenciador de Memória Virtual (VMM)

## O que foi feito

### 1. vmm_init — Limpeza da região do heap

O `loader.s` preenche todas as 1024 entradas da `boot_page_table1` no boot (4 MB inteiros mapeados). Isso inclui a faixa do heap (a partir de `0xC0300000`). Sem limpar essas entradas, o `vmm_map_page` acha que tudo já tá em uso e o `kmalloc` não consegue crescer.

O `vmm_init` zera os índices 768 a 1022 da tabela (região do heap), deixando o índice 1023 intacto porque é usado pelo mapeamento temporário. É chamado no `kmain` logo depois do `pfa_init`.

### 2. vmm_map_page — Mapeamento permanente

Escreve na tabela de páginas associando um endereço virtual a um frame físico. Antes de gravar, faz as seguintes verificações:
- Endereços alinhados em 4KB
- Dentro da janela da `boot_page_table1` (`0xC0000000` a `0xC03FFFFF`)
- Não é o slot 1023 (reservado pro temp map)
- Não sobrescreve mapeamento existente (retorna `VMM_ERR_ALREADY_USED`)

### 3. vmm_unmap_page — Desmapeamento

Zera a entrada correspondente na tabela. Se a entrada já tava vazia, retorna `VMM_ERR_NOT_MAPPED` pra avisar que não tinha nada ali.

### 4. vmm_is_mapped — Consulta

Checa se a entrada da tabela tem o bit Present ligado. Retorna 1 se sim, 0 se não.

### 5. Códigos de retorno (vmm.h)

- `VMM_OK` (0) — operação deu certo
- `VMM_ERR_INVALID_ADDR` (-1) — endereço desalinhado, fora da janela ou slot reservado
- `VMM_ERR_ALREADY_USED` (-2) — página já mapeada
- `VMM_ERR_NOT_MAPPED` (-3) — tentou desmapear página vazia

### 6. Integração com o heap (kheap.c)

O `morecore` deixou de escrever direto na `boot_page_table1` e agora usa `vmm_map_page`. Se o mapeamento falhar, devolve o frame pro PFA.

### 7. Demand Paging — Paginação sob Demanda (interrupts.c) — Capítulo 10

Quando ocorre um page fault por "página não presente" (bit 0 do error_code = 0) dentro da janela gerenciável do kernel (`0xC0000000` a `0xC03FEFFF`), o handler executa os 6 passos descritos no livro:

1. **Trap** — a CPU gera a interrupção 14 e o handler lê o endereço faltante do registrador CR2
2. **Localizar** — verifica se o endereço está dentro da janela que o VMM consegue gerenciar
3. **Alocar** — chama `pfa_alloc_frame()` para obter um frame físico livre
4. **Mapear** — grava a entrada na tabela de páginas via `vmm_map_page()` (Present + RW)
5. **Zero-fill** — zera os 4096 bytes da nova página por segurança (Zero-Fill-On-Demand)
6. **Reiniciar** — retorna da interrupção; o `iret` do assembly reinicia a instrução que falhou, que agora encontra a página presente

Se a falha não pode ser resolvida (violação de proteção, endereço fora da janela ou sem memória), o sistema imprime diagnóstico completo (endereço CR2, error_code decodificado) e trava.

### 8. Testes no boot (kmain.c)

Bloco de testes que roda durante o boot e valida cada funcionalidade:

**API base do VMM:**
- Mapeamento com sucesso
- Bloqueio de sobrescrita (`VMM_ERR_ALREADY_USED`)
- Confirmação via `vmm_is_mapped`
- Leitura e escrita no endereço mapeado
- Desmapeamento com sucesso
- Detecção de double unmap (`VMM_ERR_NOT_MAPPED`)
- `kmalloc` usando a cadeia completa (PFA → VMM → heap)

**Demand Paging:**
- Confirma que a página `0xC03D0000` não está mapeada (desmapeada pelo `vmm_init`)
- Escreve diretamente no endereço desmapeado → dispara page fault (INT 14)
- O handler resolve a falha sob demanda: aloca frame → mapeia → zero-fill → retorna
- A CPU reinicia a instrução de escrita, que agora funciona normalmente
- Confirma que a página agora está mapeada (o handler resolveu)

## Arquivos modificados

| Arquivo | O que mudou |
|---------|-------------|
| `src/memory/vmm.c` | `vmm_init`, `vmm_map_page`, `vmm_unmap_page`, `vmm_is_mapped` |
| `src/memory/vmm.h` | Códigos de retorno e declaração do `vmm_init` |
| `src/memory/kheap.c` | `morecore` usa `vmm_map_page` |
| `src/core/kmain.c` | Chamada do `vmm_init` + testes da API + teste de demand paging |
| `src/interrupts/interrupts.c` | Handler de page fault com demand paging |

