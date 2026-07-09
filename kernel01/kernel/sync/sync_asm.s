.code32
.section .text

.p2align 4
.global hnm_sync_cpu_serialize
.type hnm_sync_cpu_serialize, @function
hnm_sync_cpu_serialize:
    push %ebx
    xorl %eax, %eax
    cpuid
    pop %ebx
    ret
.size hnm_sync_cpu_serialize, . - hnm_sync_cpu_serialize

.p2align 4
.global hnm_sync_memory_barrier
.type hnm_sync_memory_barrier, @function
hnm_sync_memory_barrier:
    lock; addl $0, hnm_sync_barrier_word
    ret
.size hnm_sync_memory_barrier, . - hnm_sync_memory_barrier

.p2align 4
.global hnm_sync_cpu_pause
.type hnm_sync_cpu_pause, @function
hnm_sync_cpu_pause:
    pause
    ret
.size hnm_sync_cpu_pause, . - hnm_sync_cpu_pause

.p2align 4
.global hnm_sync_irq_save
.type hnm_sync_irq_save, @function
hnm_sync_irq_save:
    pushfl
    pop %eax
    cli
    ret
.size hnm_sync_irq_save, . - hnm_sync_irq_save

.p2align 4
.global hnm_sync_irq_restore
.type hnm_sync_irq_restore, @function
hnm_sync_irq_restore:
    pushl 4(%esp)
    popfl
    ret
.size hnm_sync_irq_restore, . - hnm_sync_irq_restore

.p2align 4
.global hnm_sync_lock_acquire
.type hnm_sync_lock_acquire, @function
hnm_sync_lock_acquire:
    movl 4(%esp), %ecx

.Lsync_lock_try:
    movl $1, %eax
    xchgl %eax, (%ecx)
    testl %eax, %eax
    jz .Lsync_lock_acquired

.Lsync_lock_wait:
    pause
    cmpl $0, (%ecx)
    jne .Lsync_lock_wait
    jmp .Lsync_lock_try

.Lsync_lock_acquired:
    ret
.size hnm_sync_lock_acquire, . - hnm_sync_lock_acquire

.p2align 4
.global hnm_sync_lock_release
.type hnm_sync_lock_release, @function
hnm_sync_lock_release:
    movl 4(%esp), %eax
    movl $0, (%eax)
    ret
.size hnm_sync_lock_release, . - hnm_sync_lock_release

.section .bss
.align 4
hnm_sync_barrier_word:
    .long 0

.section .note.GNU-stack, "", @progbits
