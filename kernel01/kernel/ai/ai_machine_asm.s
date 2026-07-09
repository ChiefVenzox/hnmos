.code32
.section .text

.p2align 4
.global hnm_ai_machine_read_eflags
.type hnm_ai_machine_read_eflags, @function
hnm_ai_machine_read_eflags:
    pushfl
    pop %eax
    ret
.size hnm_ai_machine_read_eflags, . - hnm_ai_machine_read_eflags

.p2align 4
.global hnm_ai_machine_read_cr0
.type hnm_ai_machine_read_cr0, @function
hnm_ai_machine_read_cr0:
    mov %cr0, %eax
    ret
.size hnm_ai_machine_read_cr0, . - hnm_ai_machine_read_cr0

.section .note.GNU-stack, "", @progbits
