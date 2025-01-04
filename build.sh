#!/bin/bash
set -e

echo "Cleaning up old files..."
rm -f kernel.o kernel.elf kernel.bin

echo "Compiling dependencies..."
for file in lib/*.c; do
    gcc -m32 -ffreestanding -c "$file" -o "${file%.c}.o"
done

gcc -m32 -ffreestanding -nostdlib -o kernel.bin *.o

echo "Compiling kernel..."
gcc -m32 -ffreestanding -c kernel.c -o kernel.o

echo "Assembling Multiboot header..."
nasm -f elf32 multiboot_header.asm -o multiboot_header.o

echo "Linking kernel..."
ld -m elf_i386 -T linker.ld -o kernel.elf kernel.o multiboot_header.o

echo "Copying kernel to ISO directory..."
rm iso/boot/kernel.elf
cp kernel.elf iso/boot/kernel.elf

echo "Creating bootable ISO..."
grub-mkrescue -o mykernel.iso iso

echo "Testing ISO with QEMU..."
qemu-system-i386 -cdrom mykernel.iso