#ifndef __SYNTHGUI_H__
#define __SYNTHGUI_H__

#include "gui.h"
#include "synthengine.h"

enum gui_element {
	
	ENV1_A = 0,
	ENV1_D,
	ENV1_S,
	ENV1_R,
	ENV2_A,
	ENV2_D,
	ENV2_S,
	ENV2_R,
	LFO1_RATE,
	LFO2_RATE,
	OSC1_WAVSWITCH,
	OSC1_WAVSAW,
	OSC1_WAVTRI,
	OSC1_WAVRECT,
	OSC1_OCTSWITCH,
	OSC1_OCTDOWN,
	OSC1_OCTMID,
	OSC1_OCTUP,
	OSC1_PW,
	OSC1_PW_ENV1MOD,
	OSC2_WAVSWITCH,
	OSC2_WAVSAW,
	OSC2_WAVTRI,
	OSC2_WAVRECT,
	OSC2_OCTSWITCH,
	OSC2_OCTDOWN,
	OSC2_OCTMID,
	OSC2_OCTUP,
	OSC2_PW,
	OSC2_PW_ENV1MOD,
	FILT_DBSWITCH,
	FILT_12DB,
	FILT_24DB,
	FILT_FREQ,
	FILT_RESO,
	FILT_ENV2DEPTH,
	FILT_LFO2DEPTH,
	AMP_LEVEL,
	AMP_LFO1DEPTH,
	AMP_OSCMIX,
	DELAY_TIME,
	DELAY_FB,
	DELAY_LEVEL,
	PARAM_LCD,
	PROGRAM_PREVIOUS,
	PROGRAM_NEXT,
	PROGRAM_SAVE
};

class SynthGUI : public GUI {

public:
	SynthGUI(int width, int height);
	~SynthGUI();
	void widgetChanged(GUIWidget *k);
	SynthEngine *getSynth();
	void getPalette(GUIColor *palette);
	void update();
	void getProgramFileName(char *name, int num);
	void setProgram(int i);
	int getProgram();
	void saveProgram();
	int getNumPrograms();

private:
	void buttonSelectedEvent(GUIWidgetButton* k);
	void knobValueChangedEvent(GUIWidgetTurn *k);
	
	int getOSC1Waveform();
	void setOSC1Waveform(int v);
	int getOSC2Waveform();
	void setOSC2Waveform(int v);
	int getOSC1Oct();
	int getOSC2Oct();
	void setOSC1Oct(int v);
	void setOSC2Oct(int v);
	
	int currentProgram;

	SynthEngine *synth;
};

#endif
