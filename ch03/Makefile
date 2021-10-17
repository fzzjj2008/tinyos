OBJDIR	:= obj
BINDIR  := bin
BOOTDIR := boot
TINYIMG := $(BINDIR)/os.img

all:
	mkdir -p $(OBJDIR)
	mkdir -p $(BINDIR)
	nasm -o $(BINDIR)/mbr.bin $(BOOTDIR)/mbr.S
	# nasm -f elf -g -F stabs -o $(OBJDIR)/mbr.o $(BOOTDIR)/mbr.S
	# nasm -f elf -o $(OBJDIR)/mbr.o $(BOOTDIR)/mbr.S
	# ld -m elf_i386 -Ttext 0x7c00 -o $(BINDIR)/mbr.bin $(OBJDIR)/mbr.o
	# ld -g -m elf_i386 -Ttext 0x7c00 -o $(BINDIR)/mbr.bin $(OBJDIR)/mbr.o
	qemu-img create $(TINYIMG) 1M
	dd if=$(BINDIR)/mbr.bin of=$(TINYIMG) bs=512 count=1 conv=notrunc
qemu:
	qemu-system-x86_64 -hda $(TINYIMG) -enable-kvm -monitor stdio -vnc :0
gdb:
	qemu-system-x86_64 -S -s -hda $(TINYIMG) -enable-kvm -monitor stdio -vnc :0
clean:
	rm -rf $(BINDIR)
	rm -rf $(OBJDIR)
