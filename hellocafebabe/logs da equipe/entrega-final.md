# README - Entrega Final (Memoria Virtual)

Este arquivo resume o que foi implementado nesta etapa da entrega final de Gerenciamento de Memoria Virtual e como validar rapidamente.

## O que foi integrado

1. API base do VMM em `src/memory/vmm.c` e `src/memory/vmm.h`
- `vmm_map_page(virt, phys, flags)`
- `vmm_unmap_page(virt)`
- `vmm_is_mapped(virt)`

2. Diagnostico de page fault em `src/interrupts/interrupts.c`
- Leitura de `CR2` (endereco que causou a falha)
- Exibicao de `error_code`
- Decodificacao basica da causa
- Log em framebuffer e serial

3. Testes da API no boot em `src/core/kmain.c`
- Teste de bloqueio de sobrescrita
- Teste de unmap repetido
- Teste de mapeamento valido e leitura/escrita

4. Integracao do heap com VMM em `src/memory/kheap.c`
- `morecore` deixou de escrever direto em `boot_page_table1`
- Agora usa `vmm_map_page` para mapear novas paginas do heap

## Arquivos modificados nesta etapa

- `src/memory/vmm.h`
- `src/memory/vmm.c`
- `src/interrupts/interrupts.c`
- `src/core/kmain.c`
- `src/memory/kheap.c`

## Como testar

Na pasta `hellocafebabe`:

```bash
make clean
make os.iso
make run
```

Mensagens esperadas incluem os testes do VMM no boot e, em caso de falha de pagina, diagnostico no console.

## Limites atuais

- A API de mapeamento esta focada na janela inicial coberta por `boot_page_table1`.
- A politica de page fault ainda e de parada controlada para depuracao.
- Integracoes mais avancadas (ex.: rollback completo em falha de mapeamento multiplo) podem ser evoluidas em etapa futura.
