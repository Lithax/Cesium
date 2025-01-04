ENTRY(_start)

SECTIONS
{
    . = 0x1000; /* Start at address 0x1000 */

    .multiboot_header : {
        *(.multiboot)
    }

    .text : {
        *(.text)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }
}
