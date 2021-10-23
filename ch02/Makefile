OBJDIR	:= obj
BINDIR  := bin
BOOTDIR := boot
TINYIMG := $(BINDIR)/os.img

all:
	mkdir -p $(BINDIR)
	nasm -o $(BINDIR)/mbr.bin $(BOOTDIR)/mbr.S -g
	qemu-img create $(TINYIMG) 1M
	dd if=$(BINDIR)/mbr.bin of=$(TINYIMG) bs=512 count=1 conv=notrunc
qemu:
	qemu-system-x86_64 -hda $(TINYIMG) -enable-kvm -monitor stdio -vnc :0
clean:
	rm -rf $(BINDIR)
