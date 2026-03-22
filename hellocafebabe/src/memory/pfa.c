#include "pfa.h"

#define PAGE_SIZE 4096                                                  /* ADICIONADO - cap10: tamanho de cada frame */
#define KERNEL_VIRTUAL_BASE 0xC0000000                                  /* ADICIONADO - cap10: base do higher half */
#define MULTIBOOT_FLAG_MODS 0x00000008                                  /* ADICIONADO - cap10: flag de modulos */
#define MULTIBOOT_FLAG_MMAP 0x00000040                                  /* ADICIONADO - cap10: flag do memory map */

extern unsigned int kernel_physical_end;                                /* ADICIONADO - cap10: vindo do linker */

static unsigned int total_memory_bytes = 0;                             /* ADICIONADO - cap10 */
static unsigned int total_frames = 0;                                   /* ADICIONADO - cap10 */
static unsigned int free_frames = 0;                                    /* ADICIONADO - cap10 */
static unsigned int used_frames = 0;                                    /* ADICIONADO - cap10 */
static unsigned char *frame_bitmap = (unsigned char *)0;                /* ADICIONADO - cap10 */
static unsigned int bitmap_size_bytes = 0;                              /* ADICIONADO - cap10 */

static void memory_set(unsigned char *dest, unsigned char value, unsigned int len)
{
    unsigned int i;
    for (i = 0; i < len; i++) {
        dest[i] = value;                                                /* ADICIONADO - cap10: memset manual */
    }
}

static unsigned int align_up(unsigned int value, unsigned int align)
{
    return (value + align - 1) & ~(align - 1);                          /* ADICIONADO - cap10: sobe para proximo multiplo */
}

static unsigned int align_down(unsigned int value, unsigned int align)
{
    return value & ~(align - 1);                                        /* ADICIONADO - cap10: desce para multiplo anterior */
}

static unsigned int addr_to_frame(unsigned int phys_addr)
{
    return phys_addr / PAGE_SIZE;                                       /* ADICIONADO - cap10: endereco fisico -> indice do frame */
}

static void bitmap_set(unsigned int frame)
{
    frame_bitmap[frame / 8] |= (unsigned char)(1u << (frame % 8));      /* ADICIONADO - cap10: marca frame como usado */
}

static void bitmap_clear(unsigned int frame)
{
    frame_bitmap[frame / 8] &= (unsigned char)~(1u << (frame % 8));     /* ADICIONADO - cap10: marca frame como livre */
}

static int bitmap_test(unsigned int frame)
{
    return (frame_bitmap[frame / 8] & (unsigned char)(1u << (frame % 8))) != 0; /* ADICIONADO - cap10 */
}

static void pfa_mark_region_used(unsigned int start, unsigned int length)
{
    unsigned int region_start = align_down(start, PAGE_SIZE);           /* ADICIONADO - cap10 */
    unsigned int region_end = align_up(start + length, PAGE_SIZE);      /* ADICIONADO - cap10 */
    unsigned int first_frame = addr_to_frame(region_start);             /* ADICIONADO - cap10 */
    unsigned int last_frame = addr_to_frame(region_end);                /* ADICIONADO - cap10 */
    unsigned int frame;

    for (frame = first_frame; frame < last_frame && frame < total_frames; frame++) {
        if (!bitmap_test(frame)) {
            bitmap_set(frame);
            used_frames++;
            if (free_frames > 0) {
                free_frames--;
            }
        }
    }
}

static void pfa_mark_region_free(unsigned int start, unsigned int length)
{
    unsigned int region_start = align_up(start, PAGE_SIZE);             /* ADICIONADO - cap10 */
    unsigned int region_end = align_down(start + length, PAGE_SIZE);    /* ADICIONADO - cap10 */
    unsigned int first_frame;
    unsigned int last_frame;
    unsigned int frame;

    if (region_end <= region_start) {
        return;                                                         /* ADICIONADO - cap10: ignora regioes pequenas demais */
    }

    first_frame = addr_to_frame(region_start);
    last_frame = addr_to_frame(region_end);

    for (frame = first_frame; frame < last_frame && frame < total_frames; frame++) {
        if (bitmap_test(frame)) {
            bitmap_clear(frame);
            free_frames++;
            if (used_frames > 0) {
                used_frames--;
            }
        }
    }
}

void pfa_init(multiboot_info_t *mbinfo)
{
    unsigned int kernel_end_phys = (unsigned int)&kernel_physical_end;  /* ADICIONADO - cap10: fim fisico do kernel */
    unsigned int bitmap_phys;
    unsigned int bitmap_virt;
    unsigned int max_phys_addr = 0;
    unsigned int mmap_start;
    unsigned int mmap_end;
    unsigned int current;

    if ((mbinfo->flags & MULTIBOOT_FLAG_MMAP) == 0) {                   /* ADICIONADO - cap10: valida memory map */
        while (1) { }
    }

    mmap_start = (unsigned int)(mbinfo->mmap_addr + KERNEL_VIRTUAL_BASE); /* ADICIONADO - cap10: endereco virtual do mapa */
    mmap_end = mmap_start + (unsigned int)mbinfo->mmap_length;            /* ADICIONADO - cap10 */

    current = mmap_start;
    while (current < mmap_end) {
        multiboot_memory_map_t *entry = (multiboot_memory_map_t *)current; /* ADICIONADO - cap10 */
        unsigned long long region_end = entry->addr + entry->len;          /* ADICIONADO - cap10 */

        if (region_end > max_phys_addr) {
            max_phys_addr = (unsigned int)region_end;                      /* ADICIONADO - cap10: descobre o maior endereco fisico */
        }

        current += entry->size + sizeof(entry->size);                      /* ADICIONADO - cap10: avanca para a proxima entrada */
    }

    total_memory_bytes = max_phys_addr;                                    /* ADICIONADO - cap10 */
    total_frames = align_up(total_memory_bytes, PAGE_SIZE) / PAGE_SIZE;    /* ADICIONADO - cap10 */
    bitmap_size_bytes = (total_frames + 7) / 8;                            /* ADICIONADO - cap10: 1 bit por frame */
    bitmap_size_bytes = align_up(bitmap_size_bytes, PAGE_SIZE);            /* ADICIONADO - cap10: bitmap alinhado em 4 KB */

    bitmap_phys = align_up(kernel_end_phys, PAGE_SIZE);                    /* ADICIONADO - cap10: bitmap logo apos o kernel */
    bitmap_virt = bitmap_phys + KERNEL_VIRTUAL_BASE;                       /* ADICIONADO - cap10 */
    frame_bitmap = (unsigned char *)bitmap_virt;                           /* ADICIONADO - cap10 */

    memory_set(frame_bitmap, 0xFF, bitmap_size_bytes);                     /* ADICIONADO - cap10: comeca marcando tudo como usado */
    used_frames = total_frames;                                            /* ADICIONADO - cap10 */
    free_frames = 0;                                                       /* ADICIONADO - cap10 */

    current = mmap_start;
    while (current < mmap_end) {
        multiboot_memory_map_t *entry = (multiboot_memory_map_t *)current; /* ADICIONADO - cap10 */

        if (entry->type == 1) {                                            /* ADICIONADO - cap10: tipo 1 = memoria disponivel */
            pfa_mark_region_free((unsigned int)entry->addr, (unsigned int)entry->len);
        }

        current += entry->size + sizeof(entry->size);
    }

    pfa_mark_region_used(0, PAGE_SIZE);                                    /* ADICIONADO - cap10: pagina zero reservada */
    pfa_mark_region_used(0xA0000, 0x60000);                                /* ADICIONADO - cap10: regiao de video/ROM BIOS */
    pfa_mark_region_used(0x100000, kernel_end_phys - 0x100000);            /* ADICIONADO - cap10: reserva o kernel */
    pfa_mark_region_used(bitmap_phys, bitmap_size_bytes);                  /* ADICIONADO - cap10: reserva o proprio bitmap */

    if (mbinfo->flags & MULTIBOOT_FLAG_MODS) {                             /* ADICIONADO - cap10: reserva modulos do GRUB */
        multiboot_module_t *mods = (multiboot_module_t *)(mbinfo->mods_addr + KERNEL_VIRTUAL_BASE);
        unsigned int i;

        pfa_mark_region_used((unsigned int)mbinfo->mods_addr,
                             (unsigned int)(mbinfo->mods_count * sizeof(multiboot_module_t))); /* ADICIONADO - cap10 */

        for (i = 0; i < (unsigned int)mbinfo->mods_count; i++) {
            pfa_mark_region_used((unsigned int)mods[i].mod_start,
                                 (unsigned int)(mods[i].mod_end - mods[i].mod_start));         /* ADICIONADO - cap10 */
        }
    }
}

unsigned int pfa_get_total_memory(void)
{
    return total_memory_bytes;                                            /* ADICIONADO - cap10 */
}

unsigned int pfa_get_total_frames(void)
{
    return total_frames;                                                  /* ADICIONADO - cap10 */
}

unsigned int pfa_get_free_frames(void)
{
    return free_frames;                                                   /* ADICIONADO - cap10 */
}

unsigned int pfa_get_used_frames(void)
{
    return used_frames;                                                   /* ADICIONADO - cap10 */
}