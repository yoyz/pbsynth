#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "archdep.h"
#include "synthgui.h"

#ifndef GP32
/*#define WFILE*/
#endif

#ifdef WFILE
FILE *fh;
#endif

#define SAMPLELEN (1024 * 256)
#define NUMSTEPS 1024
mfloat x[BUFLEN];


SynthGUI *synthgui;


void audio_callback(void */*userdata*/, unsigned char *stream, int len) {

	int i;
	int l;

	l = len / 2;

	synthgui->getSynth()->process(x, l);

	for (i = 0; i < len / 2; i++) {
#ifdef FIXED
		long v = x[i].v >> (FBITS - 15);
#else
		long v = (long)(x[i] * 32768.0f);
#endif

		if (v < -32768) v = 32768; else
		if (v > 32767) v = 32767;

		((unsigned short*)stream)[i] = v
#ifdef GP32
	^ 0x8000
#endif
;

	}
#ifdef WFILE
		fwrite(stream, len, 1, fh);
#endif
}

void triggerNote(int note) {

	synthgui->getSynth()->triggerNote(note);
}

void triggerNoteOsc(int osc, int note) {

	synthgui->getSynth()->triggerNoteOsc(osc, note);
}

void releaseNote(void) {

	synthgui->getSynth()->releaseNote();
}

void pressMouseButtonEvent(void) {

	synthgui->mousePressed();
}

void releaseMouseButtonEvent(void) {

	synthgui->mouseReleased();
}

void moveMouseEvent(int x, int y) {

	synthgui->mouseMoved(x, y);
}

void printScreen(void) {

	synthgui->draw(getPixels());
	refreshScreen();
}

void setSynthPalette(void) {

	int i;
	GUIColor palette[256];

	synthgui->getPalette(palette);

	for (i = 0; i < 256; i++) {

		unsigned char *color;

		color = (unsigned char*)&palette[i];
		setPal(i, color);
	}
}

#ifdef GP32
void GpMain(void */*param*/) {
#else
int main(void) {
#endif

	synthgui = new SynthGUI(320, 240);

  initVideo();

#ifdef WFILE
	fh = fopen("out.pcm", "wb");
#endif

	initAudio(audio_callback);

	setSynthPalette();
	commitPal(128);

  showCursor();

  while (1) {

    if (pollmachine() < 0) break;
    if (synthgui->mustDraw())
    	printScreen();
  }

#ifdef WFILE
	fclose(fh);
#endif

	stopAudio();
	delete synthgui;
	exitApp();

#ifndef GP32
	return 0;
#endif
}
