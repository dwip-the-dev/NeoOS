ENTRY(_start)

SECTIONS {
    . = 0x100000; /* Kernel load address (1MB) */

    /* Multiboot header must be at the very beginning */
    .multiboot ALIGN(4) : {
        *(.multiboot)
    }

    .text ALIGN(4K) : {
        *(.text)
    }

    .rodata ALIGN(4K) : {
        *(.rodata*)
    }

    .data ALIGN(4K) : {
        *(.data)
    }

    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
        . = ALIGN(16);
        stack_bottom = .;
        . += 8192; /* 8KB stack */
        stack_top = .;
    }

    /* Discard unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
    }
}
