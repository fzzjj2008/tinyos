OBJDIR	:= obj
BINDIR  := bin

CC      := gcc
LD      := ld
CFLAGS  := -m32 -g
LDFLAGS := -m elf_i386 -e main

all:
	mkdir -p $(BINDIR)
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c main.c -o $(OBJDIR)/main.o
	$(LD) $(LDFLAGS) $(OBJDIR)/main.o -o $(BINDIR)/main
objdump:
	objdump -d $(BINDIR)/main
debug:
	gdb -q -x tool/gdbinit
clean:
	rm -rf $(BINDIR)
	rm -rf $(OBJDIR)
