OBJDIR	:= obj
BINDIR  := bin
BOOTDIR := boot
KERNDIR := kernel
LIBDIR  := lib
TINYIMG := $(BINDIR)/os.img

CC      := gcc
LD      := ld
OBJCOPY := objcopy -O binary
CFLAGS  := -march=i686 -fno-builtin -Wall -ggdb -m32 -nostdinc -fno-stack-protector -Os -nostdinc
LDFLAGS := -m elf_i386 -nostdlib -N -e main

all:
	mkdir -p $(BINDIR)
	mkdir -p $(OBJDIR)/$(BOOTDIR)
	mkdir -p $(OBJDIR)/$(KERNDIR)
	qemu-img create -f raw $(TINYIMG) 1M

	# mbr
	$(CC) $(CFLAGS) -c $(BOOTDIR)/mbr.S -o $(OBJDIR)/$(BOOTDIR)/mbr.o
	$(LD) $(LDFLAGS) -Ttext 0x7C00 $(OBJDIR)/$(BOOTDIR)/mbr.o -o $(OBJDIR)/mbr
	$(OBJCOPY) $(OBJDIR)/mbr $(BINDIR)/mbr.bin
	dd if=$(BINDIR)/mbr.bin of=$(TINYIMG) bs=512 count=1 conv=notrunc

	# loader
	$(CC) $(CFLAGS) -c $(BOOTDIR)/loader.S -o $(OBJDIR)/$(BOOTDIR)/loader.o
	$(LD) $(LDFLAGS) -Ttext 0x900 $(OBJDIR)/$(BOOTDIR)/loader.o -o $(OBJDIR)/loader
	$(OBJCOPY) $(OBJDIR)/loader $(BINDIR)/loader.bin
	dd if=$(BINDIR)/loader.bin of=$(TINYIMG) bs=512 count=4 seek=2 conv=notrunc

	# kernel
	$(CC) $(CFLAGS) -c $(KERNDIR)/print.S -o $(OBJDIR)/$(KERNDIR)/print.o
	$(CC) $(CFLAGS) -c $(KERNDIR)/main.c -I $(LIBDIR) -o $(OBJDIR)/$(KERNDIR)/main.o
	$(LD) $(LDFLAGS) -Ttext 0xc0001500 $(OBJDIR)/$(KERNDIR)/main.o $(OBJDIR)/$(KERNDIR)/print.o -o $(BINDIR)/kernel.bin
	dd if=$(BINDIR)/kernel.bin of=$(TINYIMG) bs=512 count=200 seek=9 conv=notrunc
objdump:
	# objdump -d $(OBJDIR)/$(BOOTDIR)/mbr.o
	# objdump -d $(OBJDIR)/$(BOOTDIR)/loader.o
	objdump -d $(OBJDIR)/$(KERNDIR)/main.o
hexdump:
	hexdump -C $(BINDIR)/mbr.bin
qemu:
	qemu-system-x86_64 -hda $(TINYIMG) -m 32M -enable-kvm -vnc :0 -monitor stdio
debug:
	qemu-system-x86_64 -S -s -hda $(TINYIMG) -m 32M -vnc :0 -monitor stdio
gdb:
	gdb -q -x ../tool/gdbinit
clean:
	rm -rf $(BINDIR)
	rm -rf $(OBJDIR)
