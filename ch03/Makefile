OBJDIR	:= obj
BINDIR  := bin
BOOTDIR := boot
TINYIMG := $(BINDIR)/os.img

all:
	mkdir -p $(BINDIR)
	nasm -o $(BINDIR)/mbr.bin $(BOOTDIR)/mbr.S
	qemu-img create -f raw $(TINYIMG) 1M
	dd if=$(BINDIR)/mbr.bin of=$(TINYIMG) bs=512 count=1 conv=notrunc
objdump:
	objdump -D -b binary -mi386 -Maddr16,data16,intel $(BINDIR)/mbr.bin
qemu:
	qemu-system-x86_64 -hda $(TINYIMG) -enable-kvm -monitor stdio -vnc :0
gdb:
	mkdir -p $(OBJDIR)/$(BOOTDIR)
	cp -f $(BOOTDIR)/mbr.S $(OBJDIR)/$(BOOTDIR)/mbr.S
	sed -i 's/^.*vstart=0x7c00$$//g' $(OBJDIR)/$(BOOTDIR)/mbr.S
	nasm -f elf -g -F stabs -o $(OBJDIR)/mbr.o $(OBJDIR)/$(BOOTDIR)/mbr.S
	ld -g -m elf_i386 -e main -Ttext 0x7c00 -o $(OBJDIR)/mbr.bin $(OBJDIR)/mbr.o
	qemu-system-x86_64 -S -s -hda $(TINYIMG) -enable-kvm -monitor stdio -vnc :0
clean:
	rm -rf $(BINDIR)
	rm -rf $(OBJDIR)
