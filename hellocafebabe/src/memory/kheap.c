#include "kheap.h"
#include "pfa.h"
#include "vmm.h"

// Estrutura do K&R: O Header guarda o tamanho do bloco e aponta pro próximo
typedef long Align;
union header {
    struct {
        union header *ptr;
        unsigned int size;
    } s;
    Align x;
};
typedef union header Header;

static Header base;
static Header *freep = 0;

// O Heap vai começar na marca de 3MB do Kernel (0xC0300000)
static unsigned int heap_curr_vaddr = 0xC0300000; 

// Função que pede mais blocos de 4KB pra placa-mãe (PMM) e mapeia no Virtual (VMM)
static void* morecore(unsigned int nu) {
    unsigned int char_size = nu * sizeof(Header);
    unsigned int pages = (char_size + 4095) / 4096; // Arredonda pra cima (páginas de 4KB)

    void *mem = (void *)heap_curr_vaddr;

    for(unsigned int i = 0; i < pages; i++) {
        unsigned int phys = pfa_alloc_frame(); // Pede pro PMM (Parte 1)
        int rc;

        if (phys == 0) return 0; // Faltou memória física!

        // Mapeia permanentemente a página via API do VMM (ENTREGA FINAL)
        rc = vmm_map_page(heap_curr_vaddr, phys, 0x02); // RW + Present (forçado na API)
        if (rc != VMM_OK) {
            // Se o map falhar, devolvemos o frame alocado nesta iteração.
            pfa_free_frame(phys);
            return 0;
        }

        heap_curr_vaddr += 4096; // Avança o ponteiro virtual
    }

    // Insere o novo bloco gigante na lista de blocos livres
    Header *up = (Header *)mem;
    up->s.size = (pages * 4096) / sizeof(Header);
    kfree((void *)(up + 1)); 
    return freep;
}

// O famoso malloc: procura um bloco livre do tamanho pedido
void *kmalloc(unsigned int nbytes) {
    Header *p, *prevp;
    unsigned int nunits;

    nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;

    if ((prevp = freep) == 0) { // Primeira vez que o kmalloc é chamado
        base.s.ptr = freep = prevp = &base;
        base.s.size = 0;
    }

    for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr) {
        if (p->s.size >= nunits) { // Achou um bloco grande o suficiente!
            if (p->s.size == nunits) {
                prevp->s.ptr = p->s.ptr; // Encaixe exato
            } else {
                p->s.size -= nunits; // Pega só um pedaço do final do bloco
                p += p->s.size;
                p->s.size = nunits;
            }
            freep = prevp;
            return (void *)(p + 1); // Devolve o ponteiro pronto pro usuário
        }
        if (p == freep) { // Deu a volta na lista e não achou espaço
            if ((p = morecore(nunits)) == 0) {
                return 0; // Se o morecore falhar, a memória acabou de verdade
            }
        }
    }
}

// O famoso free: devolve o bloco para a lista e junta com os vizinhos
void kfree(void *ap) {
    Header *bp, *p;
    if (ap == 0) return;

    bp = (Header *)ap - 1; // Aponta de volta para o cabeçalho do bloco

    for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr) {
        if (p >= p->s.ptr && (bp > p || bp < p->s.ptr)) {
            break; // O bloco está no começo ou no fim da memória
        }
    }

    if (bp + bp->s.size == p->s.ptr) { // Junta com o vizinho de cima
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    } else {
        bp->s.ptr = p->s.ptr;
    }

    if (p + p->s.size == bp) { // Junta com o vizinho de baixo
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    } else {
        p->s.ptr = bp;
    }
    freep = p;
}