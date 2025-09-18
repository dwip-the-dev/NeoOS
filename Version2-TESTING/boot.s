/* boot.s - Multiboot header + entry point for NeoOS */

.set ALIGN,    1<<0
.set MEMINFO,  1<<1
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot, "aw"
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.set STACK_SIZE, 8192

.section .bss
.align 16
stack_bottom:
    .skip STACK_SIZE
stack_top:

.section .text
.global _start
.extern kernel_main
_start:
    mov $stack_top, %esp
    and $0xFFFFFFF0, %esp  /* 16-byte alignment */

    call kernel_main

    cli
.hang:
    hlt
    jmp .hang

/* --------------------------------------------------
   Common ISR/IRQ handler helpers
-------------------------------------------------- */
.macro ISR_NOERR n
.global isr\n
isr\n:
    cli
    pushl $0          /* dummy error code */
    pushl $\n         /* vector number */
    pusha
    call exception_handler
    popa
    add $8, %esp      /* clean up pushed values */
    sti
    iret
.endm

.macro ISR_ERR n
.global isr\n
isr\n:
    cli
    pushl $\n         /* vector number (error already pushed by CPU) */
    pusha
    call exception_handler
    popa
    add $4, %esp      /* only remove vector */
    sti
    iret
.endm

/* --------------------------------------------------
   Define exception stubs
   (0–31 are CPU exceptions)
   Some push error codes: 8, 10–14, 17
-------------------------------------------------- */
ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31

/* --------------------------------------------------
   IRQs (PIT, keyboard, etc.)
-------------------------------------------------- */
.global irq0
.extern pit_handler
irq0:
    pusha
    call pit_handler
    movb $0x20, %al
    outb %al, $0x20
    popa
    iret

.global irq1
.extern keyboard_handler
irq1:
    pusha
    call keyboard_handler
    movb $0x20, %al
    outb %al, $0x20
    popa
    iret
