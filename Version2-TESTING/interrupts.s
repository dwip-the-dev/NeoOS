/* interrupts.s - IDT and interrupt related assembly functions */

.global idt_load
idt_load:
    movl 4(%esp), %eax
    lidt (%eax)
    ret

/* Remove keyboard_handler since it's already defined in boot.s */
