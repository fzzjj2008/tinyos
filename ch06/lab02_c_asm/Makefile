BINDIR  := bin

all:
	mkdir -p $(BINDIR)
	gcc -o $(BINDIR)/main main.c func.S -m32 -g
objdump:
	objdump -d $(BINDIR)/main
debug:
	gdb -q -x tool/gdbinit
clean:
	rm -rf $(BINDIR)
