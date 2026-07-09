#ifndef HNM_SYNC_ASM_H
#define HNM_SYNC_ASM_H

#include "../types.h"

void hnm_sync_cpu_serialize(void);
void hnm_sync_memory_barrier(void);
void hnm_sync_cpu_pause(void);
u32 hnm_sync_irq_save(void);
void hnm_sync_irq_restore(u32 flags);
void hnm_sync_lock_acquire(volatile u32 *lock);
void hnm_sync_lock_release(volatile u32 *lock);

#endif
