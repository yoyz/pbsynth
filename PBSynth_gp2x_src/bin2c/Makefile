CC=gcc
CFLAGS=-Wall -W -ansi -pedantic

LNK=$(CC)
LFLAGS=-s


all: bin2c.exe

bin2c.exe: bin2c.o getopts.o
	$(LNK) $(LFLAGS) -o $@ $^

bin2c.o: bin2c.c
	$(CC) $(CFLAGS) -c $<

getopts.o: getopts.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f bin2c.exe getopts.o bin2c.o
