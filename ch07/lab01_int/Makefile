OBJDIR	:= obj
BINDIR  := bin
BOOTDIR := boot
KERNDIR := kernel
LIBDIR  := include
TINYIMG := $(BINDIR)/os.img

MBR_ENTRY    := 0x7c00
LOADER_ENTRY := 0x900
KERNEL_ENTRY := 0xc0001500

CC      := gcc
LD      := ld
OBJCOPY := objcopy -O binary
CFLAGS  := -march=i686 -fno-builtin -Wall -ggdb -m32 -nostdinc -fno-stack-protector -Os -I $(LIBDIR)
LDFLAGS := -m elf_i386 -nostdlib -N -e main

SRC_C   := $(wildcard kernel/*.c)
SRC_S   := $(wildcard kernel/*.S)
OBJ     := $(patsubst %.c, $(OBJDIR)/%.o, $(SRC_C))
OBJ     += $(patsubst %.S, $(OBJDIR)/%.o, $(SRC_S))
DEP     := $(patsubst %.c, $(OBJDIR)/%.d, $(SRC_C))
DEP     += $(patsubst %.S, $(OBJDIR)/%.d, $(SRC_S))


.PHONY: all

all: mkdir build img

build: $(BINDIR)/mbr.bin $(BINDIR)/loader.bin $(BINDIR)/kernel.bin

mkdir:
	mkdir -p $(BINDIR)
	mkdir -p $(OBJDIR)/$(BOOTDIR)
	mkdir -p $(OBJDIR)/$(KERNDIR)

$(OBJDIR)/%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(OBJDIR)/$*.d -MP
$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ -MD -MF $(OBJDIR)/$*.d -MP
# $(OBJDIR)/%.d: %.c
# 	$(CC) -MM $(CFLAGS) $< -MT "$*.o $@" >$@

-include $(DEP)
$(BINDIR)/kernel.bin: $(OBJ)
	$(LD) $(LDFLAGS) -Ttext $(KERNEL_ENTRY) $^ -o $@

$(BINDIR)/mbr.bin: $(BOOTDIR)/mbr.S
	$(CC) $(CFLAGS) -c $< -o $(OBJDIR)/$(BOOTDIR)/mbr.o
	$(LD) $(LDFLAGS) -Ttext $(MBR_ENTRY) $(OBJDIR)/$(BOOTDIR)/mbr.o -o $(OBJDIR)/mbr
	$(OBJCOPY) $(OBJDIR)/mbr $@

$(BINDIR)/loader.bin: $(BOOTDIR)/loader.S
	$(CC) $(CFLAGS) -c $< -o $(OBJDIR)/$(BOOTDIR)/loader.o
	$(LD) $(LDFLAGS) -Ttext $(LOADER_ENTRY) $(OBJDIR)/$(BOOTDIR)/loader.o -o $(OBJDIR)/loader
	$(OBJCOPY) $(OBJDIR)/loader $@

img:
	qemu-img create -f raw $(TINYIMG) 1M
	dd if=$(BINDIR)/mbr.bin of=$(TINYIMG) bs=512 count=1 conv=notrunc
	dd if=$(BINDIR)/loader.bin of=$(TINYIMG) bs=512 count=4 seek=2 conv=notrunc
	dd if=$(BINDIR)/kernel.bin of=$(TINYIMG) bs=512 count=200 seek=9 conv=notrunc
	@echo "\033[32;1mCreate $(TINYIMG) success!\033[0m"


.PHONY: objdump hexdump qemu debug gdb clean
objdump:
	# objdump -d $(OBJDIR)/$(BOOTDIR)/mbr.o
	# objdump -d $(OBJDIR)/$(BOOTDIR)/loader.o
	objdump -d $(OBJDIR)/$(KERNDIR)/main.o
hexdump:
	hexdump -C $(BINDIR)/mbr.bin
qemu:
	# qemu-system-x86_64 -hda $(TINYIMG) -m 32M -enable-kvm -monitor stdio
	qemu-system-x86_64 -hda $(TINYIMG) -m 32M -enable-kvm -vnc :0 -monitor stdio
debug:
	# qemu-system-x86_64 -S -s -hda $(TINYIMG) -m 32M -monitor stdio
	qemu-system-x86_64 -S -s -hda $(TINYIMG) -m 32M -vnc :0 -monitor stdio
gdb:
	gdb -q -x ../tool/gdbinit
clean:
	rm -rf $(BINDIR)
	rm -rf $(OBJDIR)
