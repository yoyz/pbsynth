#include <stdlib.h>
#include <gpmm.h>
#include <gpdef.h>
#include <gpstdio.h>
#include <gpgraphic.h>
#include "knobraw.h"
#include "archdep.h"
#include "synthgui.h"
#include "gui.h"

extern SynthGUI *synthgui;

extern void updateScreen(void);
extern void triggerNote(int note);
extern void releaseNote(void);


static GPDRAWSURFACE gpDraw[2];
static int curSurf;
static int key;
static void *buffer;
static unsigned int *soundPos;
static volatile int idx_buf;
static unsigned int frame;
static int mouseX, mouseY;
static int mouseVisible;
static int mouseIncX, mouseIncY;
static int mouseFX, mouseFY;
static int base, note;
struct {
	
	unsigned char r, g, b;
} palette[256];

static GP_PALETTEENTRY rpalette[256];


#define GP_RGB24(r,g,b) ((((((r)>>3))&0x1f)<<11)|(((((g)>>3))&0x1f)<<6)|(((((b)>>3))&0x1f)<<1))

#define MOUSEACCEL 0x4000
#define MOUSEACCEL_PARAM 0x1000
#define MOUSEMAX   0x60000

static void (*audiocallback)(void *, unsigned char *, int);

#define BRIGHTNESS 148

static void soundtimer (void) {
	
  unsigned int t =
    (((unsigned int)(*soundPos) - (unsigned int)buffer) >> 1) >= BUFLEN ? 1 : 0;

  if (t != frame) {

    unsigned int offs = ((frame == 1) ? (BUFLEN << 1) : 0);
    audiocallback(0, (unsigned char*)((unsigned int)buffer + offs), BUFLEN << 1);
    frame = t;
  }
}

unsigned char *getPixels(void) {
	
	return (unsigned char*)gpDraw[curSurf].ptbuffer;
}

static void printMouse(unsigned char *dest, int xp, int yp) {
	
	int d = GUI_OFS(xp, yp);
	int s = 536 * 16;
	int x, y;
	int height;
	
	height = 16;
	
	if (yp > 240 - 16)
		height = 240 - yp;
	
	for (y = 0; y < height; y++) {
		
		for (x = 0; x < 16; x++) {
			
			if (knobraw[s] != 4)
				dest[d + (x * GUI_XINC)] = knobraw[s] + 238;

			s++;
		}
		
		d += GUI_YINC;
	}
}

void initVideo(void) {

	int *t = 0;
	
	curSurf = 0;
	key = 0;
	mouseX = 160;
	mouseY = 120;
	mouseVisible = 1;
	mouseIncX = mouseIncY = 0;
	mouseFX = mouseX << 16;
	mouseFY = mouseY << 16;
	note = 0;
	base = 2;
	
	GpClockSpeedChange (132000000, 0x3a011, 3);
	/*GpClockSpeedChange (66000000, 0x3a012, 1);*/
	
  GpFatInit();
  GpKeyInit();

  GpGraphicModeSet(8, t);

  GpLcdSurfaceGet(&gpDraw[0], 0);
  GpLcdSurfaceGet(&gpDraw[1], 1);
  GpSurfaceSet(&gpDraw[curSurf ^ 1]);
  GpLcdEnable();
}

void initAudio(void (*audio_callback)(void *, unsigned char *, int)) {

	int i;
	
	frame = 0;	
	buffer = gm_malloc(BUFLEN * 2);
	
	for (i = 0; i < BUFLEN; i++)
		((unsigned short*)buffer)[i] = 32768;

	audiocallback = audio_callback;

  GpTimerOptSet(0, 400, 0, soundtimer);
  GpTimerSet(0);

  GpPcmInit(PCM_M44, PCM_16BIT);
  GpPcmPlay((unsigned short*)buffer, BUFLEN * 4, 1);
  GpPcmLock((unsigned short*)buffer, (int*)&idx_buf, (unsigned int*)&soundPos);
}

void stopAudio(void) {
	
	GpPcmStop();
	GpTimerKill(0);
}

void refreshScreen(void) {

	synthgui->draw(getPixels());

	if (mouseVisible)
		printMouse(getPixels(), mouseX, mouseY);

	GpSurfaceSet(&gpDraw[curSurf]);
	curSurf = curSurf ^ 1;
}

void commitPal(int brightness) {
	
	int i;
	
	for (i = 0; i < 256; i++) {
		
		int r, g, b;
		
		r = ((int)palette[i].r * brightness) / 128;
		g = ((int)palette[i].g * brightness) / 128;
		b = ((int)palette[i].b * brightness) / 128;

		if (r > 255) r = 255;
		if (g > 255) g = 255;
		if (b > 255) b = 255;

		rpalette[i] = GP_RGB24(r, g, b);
	}
	
	GpPaletteEntryChange(0, 256, rpalette, 1);
}

void setPal(int i, unsigned char *c) {

	int r, g, b;
	
	r = (c[0] * BRIGHTNESS) / 128;
	g = (c[1] * BRIGHTNESS) / 128;
	b = (c[2] * BRIGHTNESS) / 128;
	
	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;
	
	palette[i].r = r;
	palette[i].g = g;
	palette[i].b = b;
}

void moveCursor(int x, int y) {
	
	if (x < 0)
		x = 0;
	else
	if (x > 319 - 16)
		x = 319 - 16;
	
	if (y < 0)
		y = 0;
	else
	if (y > 239)
		y = 239;

/*	if ((mouseX != x) || (mouseY != y))*/ {
		
		mouseX = x;
		mouseY = y;
		mouseFX = (mouseFX & 0xffff) | (x << 16);
		mouseFY = (mouseFY & 0xffff) | (y << 16);
		moveMouseEvent(mouseX, mouseY);
		synthgui->forceDraw();
		refreshScreen();
	}
}

void showCursor(void) {

	mouseVisible = 1;
	synthgui->forceDraw();
	refreshScreen();
}

void hideCursor(void) {

	mouseVisible = 0;
	synthgui->forceDraw();
	refreshScreen();
}

int pollmachine(void) {
	
	int lastKey;
	
	lastKey = key;
	key = GpKeyGet();

	if ((key & GPC_VK_FA) && !(lastKey & GPC_VK_FA))
		pressMouseButtonEvent();
		
	if (!(key & GPC_VK_FA) && (lastKey & GPC_VK_FA))
		releaseMouseButtonEvent();

	if (!(key & GPC_VK_SELECT) || !mouseVisible) {
		
		int mmx = mouseX;
		int mmy = mouseY;
		
		if (key & GPC_VK_UP) {

			mouseFY -= mouseIncY;
			mmy = mouseFY >> 16;
			mouseIncY += MOUSEACCEL;
			if (mouseIncY > MOUSEMAX)
				mouseIncY = MOUSEMAX;
		} else
		if (key & GPC_VK_DOWN) {

			mouseFY = mouseFY + mouseIncY;
			mmy = mouseFY >> 16;
			mouseIncY += MOUSEACCEL;
			if (mouseIncY > MOUSEMAX)
				mouseIncY = MOUSEMAX;
		} else
			mouseIncY = 0;

		if (key & GPC_VK_LEFT) {
		
			mouseFX -= mouseIncX;
			mmx = mouseFX >> 16;
			if (mouseVisible)
				mouseIncX += MOUSEACCEL;
			else
				mouseIncX = MOUSEACCEL_PARAM;
			if (mouseIncX > MOUSEMAX)
				mouseIncX = MOUSEMAX;
		} else
		if (key & GPC_VK_RIGHT) {
		
			mouseFX += mouseIncX;
			mmx = mouseFX >> 16;
			if (mouseVisible)
				mouseIncX += MOUSEACCEL;
			else
				mouseIncX = MOUSEACCEL_PARAM;
			if (mouseIncX > MOUSEMAX)
				mouseIncX = MOUSEMAX;
		} else
			mouseIncX = 0;
		
		moveCursor(mmx, mmy);
	}

	if ((key & GPC_VK_FB) && !(lastKey & GPC_VK_FB)) {
		
		triggerNote(note + (base * 12));
	} else
	if (!(key & GPC_VK_FB) && (lastKey & GPC_VK_FB)) {
		
		releaseNote();
	}

	if ((key & GPC_VK_SELECT) && mouseVisible) {
			
		if ((key & GPC_VK_LEFT) && !(lastKey & GPC_VK_LEFT)) {
		
			note -= 1;
			if (note < 0) {
				
				note = 11;
				base -= 1;
				if (base < 0) {
					
					base = 0;
					note = 0;
				}
			}
			synthgui->getWidget(PARAM_LCD)->setValue(note + 1);
		} else
		if ((key & GPC_VK_RIGHT) && !(lastKey & GPC_VK_RIGHT)) {
		
			note += 1;
			if (note > 11) {
				
				note = 0;
				base += 1;
				if (base > 7) {
					
					base = 7;
					note = 11;
				}
			}
			synthgui->getWidget(PARAM_LCD)->setValue(note + 1);
		} else
		if ((key & GPC_VK_UP) && !(lastKey & GPC_VK_UP)) {
			
			base += 1;
			if (base > 7)
				base = 7;
			
			synthgui->getWidget(PARAM_LCD)->setValue(base);
		} else
		if ((key & GPC_VK_DOWN) && !(lastKey & GPC_VK_DOWN)) {
			
			base -= 1;
			if (base < 0)
				base = 0;
			
			synthgui->getWidget(PARAM_LCD)->setValue(base);
		}
	}

	if (GpKeyGet() & GPC_VK_FL)
		return -1;
		
  return SYNTH_OK;
}

void exitApp(void) {
	
	GpClockSpeedChange (66000000, 0x3a012, 1);
	GpAppExit();
}

void waitKey(void) {
	
	while (GpKeyGet());
	while (!GpKeyGet());
}
