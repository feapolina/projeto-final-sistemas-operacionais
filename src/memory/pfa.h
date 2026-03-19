#ifndef PFA_H
#define PFA_H

#include "../core/multiboot.h"

void pfa_init(multiboot_info_t *mbinfo);                                /* ADICIONADO - cap10: inicializa o page frame allocator */

unsigned int pfa_get_total_memory(void);                                /* ADICIONADO - cap10 */
unsigned int pfa_get_total_frames(void);                                /* ADICIONADO - cap10 */
unsigned int pfa_get_free_frames(void);                                 /* ADICIONADO - cap10 */
unsigned int pfa_get_used_frames(void);                                 /* ADICIONADO - cap10 */

#endif