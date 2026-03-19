#ifndef PFA_H
#define PFA_H

#include "../core/multiboot.h"

void pfa_init(multiboot_info_t *mbinfo);

unsigned int pfa_get_total_memory(void);
unsigned int pfa_get_total_frames(void);
unsigned int pfa_get_free_frames(void);
unsigned int pfa_get_used_frames(void);

#endif