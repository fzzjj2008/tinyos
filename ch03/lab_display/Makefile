OBJDIR	:= obj
BINDIR  := bin
BOOTDIR := boot
TINYIMG := $(BINDIR)/os.img

all:
	mkdir -p $(BINDIR)
	mkdir -p $(OBJDIR)/$(BOOTDIR)
	gcc -march=i686 -fno-builtin -fno-PIC -Wall -ggdb -m32 -gstabs -nostdinc -fno-stack-protector -Os -nostdinc -c $(BOOTDIR)/mbr.S -o $(OBJDIR)/$(BOOTDIR)/mbr.o
	ld -m elf_i386 -nostdlib -N -e main -Ttext 0x7C00 $(OBJDIR)/$(BOOTDIR)/mbr.o -o $(OBJDIR)/mbr
	objcopy -O binary $(OBJDIR)/mbr $(BINDIR)/mbr.bin
	qemu-img create -f raw $(TINYIMG) 1M
	dd if=$(BINDIR)/mbr.bin of=$(TINYIMG) bs=512 count=1 conv=notrunc
objdump:
	# objdump -D -b binary -mi386 -Maddr16,data16,intel $(BINDIR)/mbr.bin
	objdump -d $(OBJDIR)/$(BOOTDIR)/mbr.o
qemu:
	# qemu-system-i386 -hda $(TINYIMG) -enable-kvm -vnc :0 -monitor stdio
	qemu-system-x86_64 -hda $(TINYIMG) -enable-kvm -vnc :0 -monitor stdio
gdb:
	# qemu-system-i386 -S -s -hda $(TINYIMG) -vnc :0 -monitor stdio
	qemu-system-x86_64 -S -s -hda $(TINYIMG) -vnc :0 -monitor stdio
	# gdb -q -x ../tool/gdbinit
clean:
	rm -rf $(BINDIR)
	rm -rf $(OBJDIR)
