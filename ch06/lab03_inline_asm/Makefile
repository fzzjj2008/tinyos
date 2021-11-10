BINDIR  := bin
CC      := gcc
CFLAGS  := -m32 -g

all:
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $(BINDIR)/main main.c
objdump:
	objdump -d $(BINDIR)/main
debug:
	gdb -q -x tool/gdbinit
clean:
	rm -rf $(BINDIR)
