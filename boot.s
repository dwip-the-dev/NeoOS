/* boot.s - Multiboot header + entry point for NeoOS */

.set ALIGN,    1<<0
.set MEMINFO,  1<<1
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
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
_start:
    mov $stack_top, %esp  /* set up stack */
    call kernel_main      /* call C kernel */
    cli

hang:
    hlt
    jmp hang
