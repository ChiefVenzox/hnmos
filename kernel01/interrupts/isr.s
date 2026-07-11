.code32
.section .text
.global hnm_irq1_stub
.global hnm_irq3_stub
.global hnm_irq12_stub
.type hnm_irq1_stub, @function
hnm_irq1_stub:
    pusha
    movl %esp, %ebp
    andl $-16, %esp
    cld
    call hnm_keyboard_irq_handler
    movl %ebp, %esp
    popa
    iret
.size hnm_irq1_stub, . - hnm_irq1_stub

.type hnm_irq3_stub, @function
hnm_irq3_stub:
    pusha
    movl %esp, %ebp
    andl $-16, %esp
    cld
    call hnm_ai_serial_irq_handler
    movl %ebp, %esp
    popa
    iret
.size hnm_irq3_stub, . - hnm_irq3_stub

.type hnm_irq12_stub, @function
hnm_irq12_stub:
    pusha
    movl %esp, %ebp
    andl $-16, %esp
    cld
    call hnm_mouse_irq_handler
    movl %ebp, %esp
    popa
    iret
.size hnm_irq12_stub, . - hnm_irq12_stub

.section .note.GNU-stack, "", @progbits
