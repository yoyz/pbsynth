CC=gcc
CFLAGS=-Wall -W -ansi -pedantic

LNK=$(CC)
LFLAGS=-s

all: pcx2raw.exe

pcx2raw.exe: pcx.o pcx2raw.o
	$(LNK) $(LFLAGS) -o $@ $^

pcx.o: pcx.c
	$(CC) $(CFLAGS) -c $<

pcx2raw.o: pcx2raw.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f pcx.o pcx2raw.o pcx2raw.exe
