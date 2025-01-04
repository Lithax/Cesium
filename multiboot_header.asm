[bits 32]
[section .multiboot]
align 4
dd 0xe85250d6          ; Magic number for Multiboot 2
dd 0                   ; Architecture (0 for i386)
dd multiboot_end - multiboot_start ; Header length
dd -(0xe85250d6 + 0 + (multiboot_end - multiboot_start)) ; Checksum

multiboot_start:
    dd 0               ; End tag (terminator)

multiboot_end:
