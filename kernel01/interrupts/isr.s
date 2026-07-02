.section .text
.global hnm_irq1_stub
.global hnm_irq12_stub
.type hnm_irq1_stub, @function
hnm_irq1_stub:
    pusha
    call hnm_keyboard_irq_handler
    popa
    iret

.type hnm_irq12_stub, @function
hnm_irq12_stub:
    pusha
    call hnm_mouse_irq_handler
    popa
    iret

.section .note.GNU-stack, "", @progbits
