CC=mingw32-gcc
AR=ar
LIBS=-lmingw32 -lstdc++ -lSDLmain -lSDL
COPTS=-O2 -Os -W -Wall -ansi -pedantic

BIN2C=bin2c/bin2c.exe
PCX2RAW=pcx2raw/pcx2raw.exe

SYNTHLIB = libsynth.a

SOBJS = filter.o coeff.o synthengine.o osc.o osctab.o env.o envtab.o echo.o knobraw.o knobpal.o synthraw.o synthpal.o lcdraw.o lcdpal.o lfo.o lfotab.o gui.o synthgui.o

OBJS = main.o sdl.o

%.o: %.cpp
	$(CC) $(COPTS) -c $< -o $@
	
%.o: %.c
	$(CC) $(COPTS) -c $< -o $@

all: pbsynth.exe

$(BIN2C):
	make -C bin2c

$(PCX2RAW):
	make -C pcx2raw

$(SYNTHLIB): $(SOBJS)
	$(AR) -r $@ $^
	
gui.o: gui.cpp synthraw.h synthpal.h knobraw.h knobpal.h lcdraw.h lcdpal.h

osctab.c: ./mkosctab.exe
	./mkosctab.exe >osctab.c
	
envtab.c: ./mkenvtab.exe
	./mkenvtab.exe >envtab.c

lfotab.c: ./mklfotab.exe
	./mklfotab.exe >lfotab.c

./mkosctab.exe: mkosctab.c
	$(CC) $< -o $@
	
./mkenvtab.exe: mkenvtab.c
	$(CC) $< -o $@
	
./mklfotab.exe: mklfotab.c
	$(CC) $< -o $@

pbsynth.exe: $(OBJS) $(SYNTHLIB)
	$(CC) $^ -o $@ $(LIBS)

synth.pal synth.raw: synth.pcx $(PCX2RAW)
	$(PCX2RAW) $<

synthpal.c synthpal.h: synth.pal $(BIN2C)
	$(BIN2C) -c -i $< -o synthpal.c -g synthpal.h -l synthpal

synthraw.c synthraw.h: synth.raw $(BIN2C)
	$(BIN2C) -c -i $< -o synthraw.c -g synthraw.h -l synthraw

knob.pal knob.raw: knob.pcx $(PCX2RAW)
	$(PCX2RAW) $<

knobpal.c knobpal.h: knob.pal $(BIN2C)
	$(BIN2C) -c -i $< -o knobpal.c -g knobpal.h -l knobpal

knobraw.c knobraw.h: knob.raw $(BIN2C)
	$(BIN2C) -c -i $< -o knobraw.c -g knobraw.h -l knobraw

lcd.pal lcd.raw: lcd.pcx $(PCX2RAW)
	$(PCX2RAW) $<

lcdpal.c lcdpal.h: lcd.pal $(BIN2C)
	$(BIN2C) -c -i $< -o lcdpal.c -g lcdpal.h -l lcdpal

lcdraw.c lcdraw.h: lcd.raw $(BIN2C)
	$(BIN2C) -c -i $< -o lcdraw.c -g lcdraw.h -l lcdraw

clean:
	make -C bin2c clean
	make -C pcx2raw clean
	rm -f $(OBJS) pbsynth.exe
	rm -f ./mkosctab.exe osctab.c
	rm -f ./mkenvtab.exe envtab.c
	rm -f ./mklfotab.exe lfotab.c
	rm -f synth.pal synth.raw
	rm -f synthpal.c synthpal.h
	rm -f synthraw.c synthraw.h
	rm -f knob.pal knob.raw
	rm -f knobpal.c knobpal.h
	rm -f knobraw.c knobraw.h
	rm -f lcd.pal lcd.raw
	rm -f lcdpal.c lcdpal.h
	rm -f lcdraw.c lcdpal.h
	rm -f $(SOBJS)
	rm -f $(SYNTHLIB)
