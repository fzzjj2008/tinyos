OBJDIR	:= obj
BINDIR  := bin

CC      := gcc
LD      := ld
LDFLAGS := -dynamic-linker /lib64/ld-linux-x86-64.so.2
LDFLAGS += /usr/lib/x86_64-linux-gnu/crt1.o /usr/lib/x86_64-linux-gnu/crti.o
LDFLAGS += /usr/lib/x86_64-linux-gnu/crtn.o -lc

all:
	mkdir -p $(BINDIR)
	mkdir -p $(OBJDIR)
	$(CC) -g -c asm.S -o $(OBJDIR)/asm.o
	$(CC) -g -c main.c -o $(OBJDIR)/main.o
	$(LD) $(LDFLAGS) $(OBJDIR)/main.o $(OBJDIR)/asm.o -o $(BINDIR)/main
objdump:
	objdump -d $(BINDIR)/main
debug:
	gdb -q -x tool/gdbinit
clean:
	rm -rf $(BINDIR)
	rm -rf $(OBJDIR)
