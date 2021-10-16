OBJDIR	:= obj
BINDIR  := bin
BOOTDIR := boot
TINYIMG := $(BINDIR)/os.img

VNC_PORT := 5901

all:
	mkdir -p $(BINDIR)
	nasm -o $(BINDIR)/mbr.bin $(BOOTDIR)/mbr.S
	qemu-img create $(TINYIMG) 1M
	dd if=mbr.bin of=$(TINYIMG) bs=512 count=1 conv=notrunc
qemu:
	qemu-system-x86_64 -enable-kvm -monitor stdio $(TINYIMG)
qemu-vnc:
	qemu-system-x86_64 -enable-kvm -monitor stdio $(TINYIMG) -vnc :$(VNC_PORT)	
clean:
	rm -rf $(BINDIR)
