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

void pfa_init(multiboot_info_t *mbinfo)
{
    unsigned int kernel_end_phys = (unsigned int)&kernel_physical_end; 
    
    // Converte para físico se vier como virtual
    if (kernel_end_phys >= KERNEL_VIRTUAL_BASE) {
        kernel_end_phys -= KERNEL_VIRTUAL_BASE;
    }

    // ==========================================================
    // ABORDAGEM SIMPLIFICADA: FOGE DO MAPA CORROMPIDO DO GRUB
    // ==========================================================
    if (mbinfo->flags & 0x01) {
        // A flag 0x01 garante que o mem_upper é válido. 
        // Ele nos dá a memória em KB a partir do primeiro megabyte.
        total_memory_bytes = (mbinfo->mem_upper * 1024) + 0x100000;
    } else {
        // Se o GRUB estiver totalmente maluco, nós assumimos que o PC tem 32 MB
        total_memory_bytes = 32 * 1024 * 1024; 
    }

    // Calcula os frames
    total_frames = align_up(total_memory_bytes, PAGE_SIZE) / PAGE_SIZE; 
    bitmap_size_bytes = (total_frames + 7) / 8; 
    bitmap_size_bytes = align_up(bitmap_size_bytes, PAGE_SIZE); 

    // Posiciona o bitmap
    unsigned int bitmap_phys = align_up(kernel_end_phys, PAGE_SIZE); 
    unsigned int bitmap_virt = bitmap_phys + KERNEL_VIRTUAL_BASE; 
    frame_bitmap = (unsigned char *)bitmap_virt; 

    // ==========================================================
    // O GRANDE TRUQUE: MARCA TUDO COMO LIVRE (0x00)
    // ==========================================================
    memory_set(frame_bitmap, 0x00, bitmap_size_bytes); 
    free_frames = total_frames; 
    used_frames = 0; 

    // ==========================================================
    // AGORA SÓ TRANCA O QUE NÃO PODE SER TOCADO
    // ==========================================================
    pfa_mark_region_used(0, PAGE_SIZE);                                        // Bloqueia a página 0 (Null Pointer)
    pfa_mark_region_used(0xA0000, 0x60000);                                    // Bloqueia memória de Vídeo e BIOS
    pfa_mark_region_used(0x100000, kernel_end_phys - 0x100000);                // Bloqueia o Kernel
    pfa_mark_region_used(bitmap_phys, bitmap_size_bytes);                      // Bloqueia o próprio Bitmap

    // Bloqueia os módulos do GRUB (se houver)
    if (mbinfo->flags & 0x08) { 
        multiboot_module_t *mods = (multiboot_module_t *)(mbinfo->mods_addr + KERNEL_VIRTUAL_BASE);
        unsigned int i;

        pfa_mark_region_used((unsigned int)mbinfo->mods_addr,
                             (unsigned int)(mbinfo->mods_count * sizeof(multiboot_module_t))); 

        for (i = 0; i < (unsigned int)mbinfo->mods_count; i++) {
            pfa_mark_region_used((unsigned int)mods[i].mod_start,
                                 (unsigned int)(mods[i].mod_end - mods[i].mod_start)); 
        }
    }
}

// Parte de Gustavo : ALOCAÇÃO E LIBERAÇÃO EFETIVA


/**
 * @brief Aloca o primeiro frame físico livre de 4KB.
 * @return O endereço físico do frame, ou 0 se a memória acabou.
 */
unsigned int pfa_alloc_frame(void) 
{
    unsigned int i;

    // 1. Verifica se temos memória disponível no sistema
    if (free_frames == 0) {
        return 0; // Out of Memory
    }

    // 2. Varre todos os frames existentes procurando o primeiro com bit '0'
    for (i = 0; i < total_frames; i++) {
        if (!bitmap_test(i)) {
            // Achou um livre! 
            bitmap_set(i);   // Marca como ocupado
            free_frames--;   // Atualiza os contadores
            used_frames++;   
            
            // 3. Transforma o índice (ex: frame 3) num endereço físico (ex: 12288)
            return i * PAGE_SIZE;
        }
    }
    
    return 0; 
}

/**
 * @brief Devolve um frame físico de 4KB para o sistema (marca como livre).
 * @param phys_addr O endereço físico de memória que será liberado.
 */
void pfa_free_frame(unsigned int phys_addr) 
{
    // 1. Descobre qual é o índice (bit) equivalente a esse endereço físico
    unsigned int frame = addr_to_frame(phys_addr);

    // 2. Confere se o frame realmente estava ocupado antes de liberar
    if (bitmap_test(frame)) {
        bitmap_clear(frame); // Marca como livre (0)
        free_frames++;       // Atualiza os contadores
        used_frames--;       
    }
}