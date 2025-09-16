.global idt_load
idt_load:
    movl 4(%esp), %eax
    lidt (%eax)
    ret

.global keyboard_handler
keyboard_handler:
    pusha
    call keyboard_handler_main
    popa
    iretl
