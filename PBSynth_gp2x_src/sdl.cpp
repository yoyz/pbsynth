#include <stdlib.h>
#include <sdl/sdl.h>
#include <time.h>
#include "archdep.h"
#include "synthgui.h"
#include "gui.h"

#ifdef GP2X
    #include "gp2x.h"
#endif

extern SynthGUI *synthgui;

static SDL_Surface *surf;
static int mouseX, mouseY;
static int mouseVisible;
static int mouseIncX, mouseIncY;
static int mouseFX, mouseFY;
int oscBase[2], note[2];
static int curSurf;
static int key;
static long stepCounter, arpCounter, elapsedTime, elapsedArpTime;
static int oscSelection = -1;
static int arpOctave[2];
int arpMode = 0;
int arpSpeed = 200;
int arpOctaveMax = 4;
static int arpInc = 1;

static int holdNote = 0;
#define DELAY_STEP_VALUE 25
int delayValueSteps = DELAY_STEP_VALUE;
int delayStepsMul = 1;

#define JOYPAD_BUTTON_COUNT 19

static int joyArray[JOYPAD_BUTTON_COUNT];

SDL_Joystick *joy;

#define MOUSEACCEL 0x1800
#define MOUSEACCEL_PARAM 0x1000
#define MOUSEMAX   0x29000

extern void triggerNote(int note);
extern void triggerNoteOsc(int osc, int note);
extern void releaseNote(void);

struct {

	unsigned char r, g, b;
} palette[256];

static SDL_Color rpalette[256];

void initVideo(void) {

  oscBase[0] = 2;
  oscBase[1] = 2;
  note[0] = 0;
  note[1] = 0;
  arpOctave[0] = 0;
  arpOctave[1] = 0;
  curSurf = 0;
  key = 0;
  mouseX = 160;
  mouseY = 120;
  mouseVisible = 1;
  mouseIncX = mouseIncY = 0;
  mouseFX = mouseX << 16;
  mouseFY = mouseY << 16;

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {

    fprintf(stderr, "Couldn't initialize SDL\n");
    exit(1);
  }
  atexit(SDL_Quit);

  surf = SDL_SetVideoMode(320, 240, 8, SDL_HWSURFACE | SDL_HWPALETTE/*| SDL_HWPALETTE | SDL_DOUBLEBUF*/
                          /*(SDL_GetVideoInfo()->hw_available ?
                             SDL_SWSURFACE|SDL_HWSURFACE : SDL_SWSURFACE) |
                               SDL_HWPALETTE | SDL_DOUBLEBUF*/
                         );

    SDL_WarpMouse(160,120);

    #ifdef GP2X
    joy = SDL_JoystickOpen(0);
    for(int i=0; i<JOYPAD_BUTTON_COUNT; i++)
        joyArray[i] = 0;

    stepCounter = SDL_GetTicks();
    arpCounter = SDL_GetTicks();
    #endif

    srand(time(NULL));
}

void initAudio(void (*audio_callback)(void *, unsigned char *, int)) {

  SDL_AudioSpec spec;

  spec.freq = 44100;
  spec.format = AUDIO_S16;
  spec.channels = 1;
  spec.samples = BUFLEN;
  spec.callback = audio_callback;
  spec.userdata = 0;

  if (SDL_OpenAudio(&spec, 0) < 0) {

    fprintf(stderr, "Couldn't start audio: %s\n", SDL_GetError());
    exit(1);
  }

	SDL_PauseAudio(0);
}

void stopAudio(void) {

	SDL_PauseAudio(1);
	SDL_CloseAudio();
}

void refreshScreen(void) {

	SDL_UpdateRect(surf, 0, 0, 0, 0);
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

		rpalette[i].r = r;
		rpalette[i].g = g;
		rpalette[i].b = b;
	}

	SDL_SetPalette(surf, SDL_LOGPAL | SDL_PHYSPAL, rpalette, 0, 256);
}

void setPal(int i, unsigned char *c) {

	palette[i].r = c[0];
	palette[i].g = c[1];
	palette[i].b = c[2];
}

unsigned char *getPixels(void) {

	return (unsigned char*)surf->pixels;
}

void moveCursor(int x, int y) {
	SDL_WarpMouse(x, y);
}

void showCursor(void) {

  SDL_ShowCursor(SDL_ENABLE);
}

void hideCursor(void) {

  SDL_ShowCursor(SDL_DISABLE);
}

void waitKey(void) {

	SDL_Event event;

	while (1) {

		if (SDL_PollEvent(&event)) {
#ifdef GP2X
            if (event.type == SDL_JOYBUTTONDOWN)
#else
			if (event.type == SDL_KEYDOWN)
#endif
				return;
		}
	}
}

int pollmachine(void) {
    //srand(time(NULL));
#ifdef GP2X
    long ticks = SDL_GetTicks();
    elapsedTime = ticks-stepCounter;
    elapsedArpTime = ticks-arpCounter;

    for(int i=0; i<JOYPAD_BUTTON_COUNT; i++)
        joyArray[i] = SDL_JoystickGetButton(joy, i);
#endif
    SDL_Event event;
    SDL_Delay(1);

    while (SDL_PollEvent(&event) != 0) {

    switch (event.type) {

        case SDL_MOUSEMOTION:
            moveMouseEvent(event.motion.x, event.motion.y);
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT)
                pressMouseButtonEvent();
            break;

        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT)
                releaseMouseButtonEvent();
            break;
#ifndef GP2X

        case SDL_QUIT:
            return -1;
            break;

        case SDL_KEYDOWN:
        case SDL_KEYUP: {

				int t = event.type == SDL_KEYDOWN;

				switch (event.key.keysym.sym) {

					case SDLK_ESCAPE:
						return -1;
						break;

					default:
						break;
				}

				break;
			}
#else
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP: {

				int t = event.type == SDL_JOYBUTTONDOWN;

				switch (event.jbutton.button) {
				    case GP2X_BUTTON_START: // reset oscs freq
                        if(!t)
                        {
                            oscBase[0] = 2;
                            oscBase[1] = 2;
                            note[0] = 0;
                            note[1] = 0;
                        }
                        break;

                    case GP2X_BUTTON_VOLDOWN:
                        if(holdNote && !t)
                        {
                            holdNote = 0;
                            releaseNote();
                        }
                        else if(!t)
                        {
                            arpOctave[0] = 0;
                            arpOctave[1] = 0;
                            holdNote = 1;
                        }
                        break;

                    case GP2X_BUTTON_A:
                        if(!t)
                        {
                            if(synthgui->cursorOnWidget() == 1)
                            {
                                pressMouseButtonEvent();
                                releaseMouseButtonEvent();
                            }
                            else
                            {
                                arpOctaveMax++;
                                if(arpOctaveMax > 5)
                                    arpOctaveMax = 1;
                                synthgui->getWidget(PARAM_LCD)->setValue(arpOctaveMax);
                            }
                        }
                        break;

                    case GP2X_BUTTON_B:
                        if(!t && holdNote == 0)
                        {
                            arpOctave[0] = 0;
                            arpOctave[1] = 0;
                        }
                        break;

                    case GP2X_BUTTON_Y:
                        if(!t)
                        {
                            oscSelection++;
                            if(oscSelection > 1)
                            {
                                oscSelection = -1;
                                synthgui->getWidget(PARAM_LCD)->setValue(oscSelection);
                            }
                            else
                                synthgui->getWidget(PARAM_LCD)->setValue(oscSelection+1);
                        }
                        break;

                    case GP2X_BUTTON_SELECT:
                        if(!t)
                        {
                            delayStepsMul++;
                            if(delayStepsMul > 7)
                                delayStepsMul = 1;

                            delayValueSteps = DELAY_STEP_VALUE*delayStepsMul;
                            synthgui->getWidget(PARAM_LCD)->setValue(delayStepsMul);
                        }
                        break;

                    case GP2X_BUTTON_X:
                        if(!t)
                        {
                            arpMode++;
                            if(arpMode > 7)
                            {
                                arpMode = 0;
                            }
                            arpOctave[0] = 0;
                            arpOctave[1] = 0;
                            synthgui->getWidget(PARAM_LCD)->setValue(arpMode);
                        }
                        break;
/*
					case SDLK_F11:
						synthgui->getSynth()->loadProgram("00.dat");
						synthgui->update();
						break;

					case SDLK_F12:
						synthgui->getSynth()->saveProgram("00.dat");
						break;
*/

					default:
						break;
				}

				break;
			}
#endif

      default:
        break;
    }
  }

#ifdef GP2X
    if(joyArray[GP2X_BUTTON_START] && joyArray[GP2X_BUTTON_SELECT])
        return -1;

    Uint8 mS = SDL_GetMouseState(&mouseX, &mouseY);

    // trigger note
	if(joyArray[GP2X_BUTTON_B])
	{
        if(!holdNote)
        {
            triggerNoteOsc(0, note[0] + ((oscBase[0]+arpOctave[0]) * 12));
            triggerNoteOsc(1, note[1] + ((oscBase[1]+arpOctave[1]) * 12));
        }
	}
	else if(!joyArray[GP2X_BUTTON_B])
	{
        if(!holdNote)
            releaseNote();
	}

    // change value l&r
    if(elapsedTime >= delayValueSteps)
    {
        if(joyArray[GP2X_BUTTON_VOLUP])
        {
            arpSpeed+=2;
            if(arpSpeed > 999)
                arpSpeed = 0;
            stepCounter = SDL_GetTicks();
            synthgui->getWidget(PARAM_LCD)->setValue(arpSpeed);
        }

        if(joyArray[GP2X_BUTTON_L])
        {
            synthgui->addValue(-1);
            stepCounter = SDL_GetTicks();
        }
        else if(joyArray[GP2X_BUTTON_R])
        {
            synthgui->addValue(1);
            stepCounter = SDL_GetTicks();
        }

        // change osc semitone/octave
        if (joyArray[GP2X_BUTTON_B]) {

            if (joyArray[GP2X_BUTTON_LEFT]) {
                if(oscSelection < 0)
                {
                    for(int i=0; i<2; i++)
                    {
                        note[i] -= 1;
                        if (note[i] < 0) {

                            note[i] = 11;
                            oscBase[i] -= 1;
                            if (oscBase[i] < 0) {

                                oscBase[i] = 0;
                                note[i] = 0;
                            }
                        }
                    }
                    synthgui->getWidget(PARAM_LCD)->setValue(note[0] + 1);
                }
                else
                {
                    note[oscSelection] -= 1;
                    if (note[oscSelection] < 0) {

                        note[oscSelection] = 11;
                        oscBase[oscSelection] -= 1;
                        if (oscBase[oscSelection] < 0) {

                            oscBase[oscSelection] = 0;
                            note[oscSelection] = 0;
                        }
                    }
                    synthgui->getWidget(PARAM_LCD)->setValue(note[oscSelection] + 1);
                }
                stepCounter = SDL_GetTicks();
            } else
            if (joyArray[GP2X_BUTTON_RIGHT]) {
                if(oscSelection < 0)
                {
                    for(int i=0; i<2; i++)
                    {
                        note[i] += 1;
                        if (note[i] > 11) {

                            note[i] = 0;
                            oscBase[i] += 1;
                            if (oscBase[i] > 7) {

                                oscBase[i] = 7;
                                note[i] = 11;
                            }
                        }
                    }
                    synthgui->getWidget(PARAM_LCD)->setValue(note[0] + 1);
                }
                else
                {
                    note[oscSelection] += 1;
                    if (note[oscSelection] > 11) {

                        note[oscSelection] = 0;
                        oscBase[oscSelection] += 1;
                        if (oscBase[oscSelection] > 7) {

                            oscBase[oscSelection] = 7;
                            note[oscSelection] = 11;
                        }
                    }
                    synthgui->getWidget(PARAM_LCD)->setValue(note[oscSelection] + 1);
                }
                stepCounter = SDL_GetTicks();
            } else
            if (joyArray[GP2X_BUTTON_UP]) {
                if(oscSelection < 0)
                {
                    for(int i=0; i<2; i++)
                    {
                        oscBase[i] += 1;
                        if (oscBase[i] > 7)
                            oscBase[i] = 7;
                    }
                    synthgui->getWidget(PARAM_LCD)->setValue(oscBase[0] + 1);
                }
                else
                {
                    oscBase[oscSelection] += 1;
                    if (oscBase[oscSelection] > 7)
                        oscBase[oscSelection] = 7;
                    synthgui->getWidget(PARAM_LCD)->setValue(oscBase[oscSelection]);
                }
                stepCounter = SDL_GetTicks();
            } else
            if (joyArray[GP2X_BUTTON_DOWN]) {
                if(oscSelection < 0)
                {
                    for(int i=0; i<2; i++)
                    {
                        oscBase[i] -= 1;
                        if (oscBase[i] < 0)
                            oscBase[i] = 0;
                    }
                    synthgui->getWidget(PARAM_LCD)->setValue(oscBase[0] + 1);
                }
                else
                {
                    oscBase[oscSelection] -= 1;
                    if (oscBase[oscSelection] < 0)
                        oscBase[oscSelection] = 0;
                    synthgui->getWidget(PARAM_LCD)->setValue(oscBase[oscSelection]);
                }
                stepCounter = SDL_GetTicks();
            }
        }
    }

    if(!joyArray[GP2X_BUTTON_B]) { // dpad mouse handling

		int mmx = mouseX;
		int mmy = mouseY;

        // check boundaries
		int tmpMFX = mouseFX>>16, tmpMFY = mouseFY>>16;
        if(tmpMFX < 0)
            mouseFX = 0;
        else if(tmpMFX > 320)
            mouseFX = 320<<16;
        if(tmpMFY < 0)
            mouseFY = 0;
        else if(tmpMFY > 240)
            mouseFY = 240<<16;

		if (joyArray[GP2X_BUTTON_UP] || joyArray[GP2X_BUTTON_UPLEFT]
            || joyArray[GP2X_BUTTON_UPRIGHT]) {

                mouseFY -= mouseIncY;
                mmy = mouseFY >> 16;
                mouseIncY += MOUSEACCEL;
                if (mouseIncY > MOUSEMAX)
                    mouseIncY = MOUSEMAX;
		} else
		if (joyArray[GP2X_BUTTON_DOWN] || joyArray[GP2X_BUTTON_DOWNLEFT]
            || joyArray[GP2X_BUTTON_DOWNRIGHT]) {

                mouseFY = mouseFY + mouseIncY;
                mmy = mouseFY >> 16;
                mouseIncY += MOUSEACCEL;
                if (mouseIncY > MOUSEMAX)
                    mouseIncY = MOUSEMAX;
		} else
			mouseIncY = 0;

		if (joyArray[GP2X_BUTTON_LEFT] || joyArray[GP2X_BUTTON_UPLEFT]
            || joyArray[GP2X_BUTTON_DOWNLEFT]) {

                mouseFX -= mouseIncX;
                mmx = mouseFX >> 16;
                if (mouseVisible)
                    mouseIncX += MOUSEACCEL;
                else
                    mouseIncX = MOUSEACCEL_PARAM;
                if (mouseIncX > MOUSEMAX)
                    mouseIncX = MOUSEMAX;
		} else
		if (joyArray[GP2X_BUTTON_RIGHT] || joyArray[GP2X_BUTTON_UPRIGHT]
            || joyArray[GP2X_BUTTON_DOWNRIGHT] ) {

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

    if(holdNote == 1)
    {
        triggerNoteOsc(0, note[0] + ((oscBase[0]+arpOctave[0]) * 12));
        triggerNoteOsc(1, note[1] + ((oscBase[1]+arpOctave[1]) * 12));
    }

    if(arpMode != 0)
        if(elapsedArpTime >= arpSpeed)
        {
            switch(arpMode)
            {
                case 1: // up
                    arpOctave[0]++;
                    arpOctave[1]++;
                    if(arpOctave[0] > arpOctaveMax)
                    {
                        arpOctave[0] = 0;
                        arpOctave[1] = 0;
                    }
                    break;

                case 2: // down
                    arpOctave[0]--;
                    arpOctave[1]--;
                    if(arpOctave[0] < -arpOctaveMax)
                    {
                        arpOctave[0] = 0;
                        arpOctave[1] = 0;
                    }
                    break;

                case 3:
                    arpOctave[0]++;
                    arpOctave[1]--;
                    if(arpOctave[0] > arpOctaveMax)
                    {
                        arpOctave[0] = 0;
                    }

                    if(arpOctave[1] < -arpOctaveMax)
                    {
                        arpOctave[1] = 0;
                    }
                    break;

                case 4: // random
                    int rndValue = ((int)((arpOctaveMax*2) * (rand()/(RAND_MAX+1.0)))-arpOctaveMax);
                    arpOctave[0]=rndValue;
                    arpOctave[1]=rndValue;
                    break;

                case 5: // random (rand osc independantly)
                    arpOctave[0]=((int)((arpOctaveMax*2) * (rand()/(RAND_MAX+1.0)))-arpOctaveMax);
                    arpOctave[1]=((int)((arpOctaveMax*2) * (rand()/(RAND_MAX+1.0)))-arpOctaveMax);
                    break;

                case 6: // more randomness by randomizizing arp speed
                    arpOctave[0]=((int)((arpOctaveMax*2) * (rand()/(RAND_MAX+1.0)))-arpOctaveMax);
                    arpOctave[1]=((int)((arpOctaveMax*2) * (rand()/(RAND_MAX+1.0)))-arpOctaveMax);
                    arpSpeed = rand()%999;
                    break;

                case 7: // randomize osc notes
                    note[0]=((int)(12 * (rand()/(RAND_MAX+1.0))));
                    note[1]=((int)(12 * (rand()/(RAND_MAX+1.0))));
                    arpOctave[0]=((int)((arpOctaveMax*2) * (rand()/(RAND_MAX+1.0)))-arpOctaveMax);
                    arpOctave[1]=((int)((arpOctaveMax*2) * (rand()/(RAND_MAX+1.0)))-arpOctaveMax);
                    break;

                default:
                    break;
            }

            if(oscBase[0]+arpOctave[0] > 7)
            {
                arpOctave[0] = 0;
            }
            else if(oscBase[0]+arpOctave[0] < 0)
            {
                arpOctave[0] = 0;
            }

            if(oscBase[1]+arpOctave[1] > 7)
                arpOctave[1] = 0;
            else if(oscBase[1]+arpOctave[1] < 0)
                arpOctave[1] = 0;

            arpCounter = SDL_GetTicks();
            if(!holdNote)
                releaseNote();
        }
#endif

  return SYNTH_OK;
}

void exitApp(void) {
    #ifdef GP2X
    SDL_JoystickClose(joy);
    SDL_Quit();

	chdir("/usr/gp2x");
	execl("/usr/gp2x/gp2xmenu", "/usr/gp2x/gp2xmenu", NULL);
	#endif
	exit(0);
}
