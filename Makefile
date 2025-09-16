CC = gcc
LD = ld
CFLAGS = -nostdlib -nostdinc -fno-builtin -fno-stack-protector -O2 -Wall -Wextra -m32 -Iinclude

OBJS = boot.o kernel.o vga.o keyboard.o shell.o commands.o


all: neoos.iso

boot.o: boot.s
	$(CC) $(CFLAGS) -c boot.s -o boot.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

vga.o: vga.c vga.h
	$(CC) $(CFLAGS) -c vga.c -o vga.o

keyboard.o: keyboard.c keyboard.h
	$(CC) $(CFLAGS) -c keyboard.c -o keyboard.o

shell.o: shell.c shell.h
	$(CC) $(CFLAGS) -c shell.c -o shell.o

commands.o: commands.c commands.h
	$(CC) $(CFLAGS) -c commands.c -o commands.o

kernel.bin: $(OBJS)
	$(LD) -m elf_i386 -T linker.ld -nostdlib $(OBJS) -o kernel.bin

neoos.iso: kernel.bin grub.cfg
	rm -rf iso
	mkdir -p iso/boot/grub
	cp kernel.bin iso/boot/
	cp grub.cfg iso/boot/grub/
	grub-mkrescue -o neoos.iso iso || (echo "grub-mkrescue failed — ensure grub-pc-bin & xorriso installed"; exit 1)

clean:
	rm -f *.o *.bin neoos.iso
	rm -rf iso

run: all
	qemu-system-i386 -cdrom neoos.iso -m 512M -boot d
