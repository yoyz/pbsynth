#include "synthgui.h"

#include "synthraw.h"
#include "synthpal.h"
#include "lcdraw.h"
#include "lcdpal.h"
#include "knobraw.h"
#include "knobpal.h"

#define NUM_PROGRAMS 16

#define MAX_DELAYLEN (1 * 44100)
#define SYNTH_GRANULARITY 64
#define NUM_PARAMETERS (DELAY_LEVEL + 1)

#define LCDXPOS 223
#define LCDYPOS 187

SynthGUI::SynthGUI(int width, int height) : GUI(width, height) {

	/* Programs */
	newWidget(226, 228, WIDGET_BUTTON, PROGRAM_PREVIOUS, 0);
	newWidget(275, 228, WIDGET_BUTTON, PROGRAM_NEXT, 0);
	newWidget(250, 228, WIDGET_BUTTON, PROGRAM_SAVE, 0);

	/* LCD */
	newWidget(LCDXPOS, LCDYPOS, WIDGET_LCD, PARAM_LCD, 0);
	((GUIWidgetLCD*)getWidget(PARAM_LCD))->switchOn(0);
	((GUIWidgetLCD*)getWidget(PARAM_LCD))->setNumDigits(3);

	/* ENV1 */
	newWidget(18 + (0 * 20), 149, WIDGET_TURN, ENV1_A, 0);
	newWidget(18 + (1 * 20), 149, WIDGET_TURN, ENV1_D, 0);
	newWidget(18 + (2 * 20), 149, WIDGET_TURN, ENV1_S, 127);
	newWidget(18 + (3 * 20), 149, WIDGET_TURN, ENV1_R, 0);

	/* ENV2 */
	newWidget(18 + (0 * 20), 189, WIDGET_TURN, ENV2_A, 0);
	newWidget(18 + (1 * 20), 189, WIDGET_TURN, ENV2_D, 0);
	newWidget(18 + (2 * 20), 189, WIDGET_TURN, ENV2_S, 127);
	newWidget(18 + (3 * 20), 189, WIDGET_TURN, ENV2_R, 0);

	/* LFO1 */
	newWidget(158, 149, WIDGET_TURN, LFO1_RATE, 96);

	/* LFO2 */
	newWidget(158, 189, WIDGET_TURN, LFO2_RATE, 96);

	/* OSC1 */
	newWidget(8, 38, WIDGET_BUTTON, OSC1_WAVSWITCH, 0);
	newWidget(26, 38 + (0 * 10), WIDGET_LED, OSC1_WAVSAW, 1);
	newWidget(26, 38 + (1 * 10), WIDGET_LED, OSC1_WAVTRI, 0);
	newWidget(26, 38 + (2 * 10), WIDGET_LED, OSC1_WAVRECT, 0);
	newWidget(58, 38, WIDGET_BUTTON, OSC1_OCTSWITCH, 0);
	newWidget(76, 38 + (0 * 10), WIDGET_LED, OSC1_OCTDOWN, 0);
	newWidget(76, 38 + (1 * 10), WIDGET_LED, OSC1_OCTMID, 1);
	newWidget(76, 38 + (2 * 10), WIDGET_LED, OSC1_OCTUP, 0);
	newWidget(108, 34, WIDGET_TURN, OSC1_PW_ENV1MOD, 0);
	getWidget(OSC1_PW_ENV1MOD)->setSign(1);

	/* OSC2 */
	newWidget(8, 88, WIDGET_BUTTON, OSC2_WAVSWITCH, 0);
	newWidget(26, 88 + (0 * 10), WIDGET_LED, OSC2_WAVSAW, 1);
	newWidget(26, 88 + (1 * 10), WIDGET_LED, OSC2_WAVTRI, 0);
	newWidget(26, 88 + (2 * 10), WIDGET_LED, OSC2_WAVRECT, 0);
	newWidget(58, 88, WIDGET_BUTTON, OSC2_OCTSWITCH, 0);
	newWidget(76, 88 + (0 * 10), WIDGET_LED, OSC2_OCTDOWN, 0);
	newWidget(76, 88 + (1 * 10), WIDGET_LED, OSC2_OCTMID, 1);
	newWidget(76, 88 + (2 * 10), WIDGET_LED, OSC2_OCTUP, 0);
	newWidget(108, 84, WIDGET_TURN, OSC2_PW_ENV1MOD, 0);
	getWidget(OSC2_PW_ENV1MOD)->setSign(1);

	/* FILT */
	newWidget(158, 38, WIDGET_BUTTON, FILT_DBSWITCH, 0);
	newWidget(176, 38, WIDGET_LED, FILT_12DB, 1);
	newWidget(176, 48, WIDGET_LED, FILT_24DB, 0);
	newWidget(158, 64, WIDGET_TURN, FILT_FREQ, 127);
	newWidget(158, 94, WIDGET_TURN, FILT_RESO, 0);
	newWidget(198, 64, WIDGET_TURN, FILT_ENV2DEPTH, 0);
	getWidget(FILT_ENV2DEPTH)->setSign(1);
	newWidget(198, 94, WIDGET_TURN, FILT_LFO2DEPTH, 0);
	getWidget(FILT_LFO2DEPTH)->setSign(1);

	/* AMP */
	newWidget(267, 34, WIDGET_TURN, AMP_LEVEL, 127);
	getWidget(AMP_LEVEL)->setOutVal(0.0f, 1.0f);
	newWidget(267, 64, WIDGET_TURN, AMP_LFO1DEPTH, 0);
	getWidget(AMP_LFO1DEPTH)->setSign(1);
	newWidget(267, 94, WIDGET_TURN, AMP_OSCMIX, 0);
	getWidget(AMP_OSCMIX)->setSign(1);
	getWidget(AMP_OSCMIX)->setOutVal(0.0f, 1.0f);

	/* DELAY */
	newWidget(228, 149, WIDGET_TURN, DELAY_TIME, 64);
	getWidget(DELAY_TIME)->setOutVal(0.0f, 1.0f);
	newWidget(248, 149, WIDGET_TURN, DELAY_FB, 64);
	getWidget(DELAY_FB)->setOutVal(0.0f, 1.0f);
	newWidget(268, 149, WIDGET_TURN, DELAY_LEVEL, 0);
	getWidget(DELAY_LEVEL)->setOutVal(0.0f, 1.0f);

	setBackground(synthraw);

	synth = new SynthEngine(SYNTH_GRANULARITY, MAX_DELAYLEN);

	setProgram(0);

	update();
	forceDraw();
	return;

/*
	synth->getEcho()->setLength(((long)getWidget(DELAY_TIME)->getValue() * MAX_DELAYLEN) / 127);
	synth->getEcho()->setLevel((getWidgetValSign(DELAY_LEVEL) + 1) / 2);
	synth->getEcho()->setFeedback((getWidgetValSign(DELAY_FB) + 1) / 2);

	synth->getLFO(0)->setRate(getWidgetValSign(LFO1_RATE));
	synth->getLFO(1)->setRate(getWidgetValSign(LFO2_RATE));

	synth->getEnvelope(0)->setADSR(
							getWidgetValSign(ENV1_A),
							getWidgetValSign(ENV1_D),
							getWidgetValSign(ENV1_S),
							getWidgetValSign(ENV1_R));
	synth->getEnvelope(1)->setADSR(
							getWidgetValSign(ENV2_A),
							getWidgetValSign(ENV2_D),
							getWidgetValSign(ENV2_S),
							getWidgetValSign(ENV2_R));

	doUpdate();
*/
}


SynthGUI::~SynthGUI() {

	delete synth;
}

void SynthGUI::knobValueChangedEvent(GUIWidgetTurn *k) {

	mfloat value;

	value = k->getValue();

	if (k->getSign()) {

		value = value / 64;
	} else {

		value = 2 * value;
		value = value / 127;
		value = -1 + value;
	}

	switch (k->getName()) {

		case LFO1_RATE:
			getSynth()->getLFO(0)->setRate(value);
			break;

		case LFO2_RATE:
			getSynth()->getLFO(1)->setRate(value);
			break;

		case FILT_LFO2DEPTH:
			getSynth()->setParameter(SENGINE_LFO2_TO_CUTOFF, value);
			break;

		case AMP_LFO1DEPTH:
			getSynth()->setParameter(SENGINE_LFO1_TO_AMP, value);
			break;

		case OSC1_PW_ENV1MOD:
			getSynth()->setParameter(SENGINE_ENV1_TO_OSC1PW, value);
			break;

		case OSC2_PW_ENV1MOD:
			getSynth()->setParameter(SENGINE_ENV1_TO_OSC2PW, value);
			break;

		case ENV1_A:
			getSynth()->getEnvelope(0)->setA(value);
			break;
		case ENV1_D:
			getSynth()->getEnvelope(0)->setD(value);
			break;
		case ENV1_S:
			getSynth()->getEnvelope(0)->setS(value);
			break;
		case ENV1_R:
			getSynth()->getEnvelope(0)->setR(value);
			break;

		case ENV2_A:
			getSynth()->getEnvelope(1)->setA(value);
			break;
		case ENV2_D:
			getSynth()->getEnvelope(1)->setD(value);
			break;
		case ENV2_S:
			getSynth()->getEnvelope(1)->setS(value);
			break;
		case ENV2_R:
			getSynth()->getEnvelope(1)->setR(value);
			break;

		case AMP_OSCMIX:
			value = value + 1;
			value = value / 2;
			getSynth()->setParameter(SENGINE_OSCMIX, value);
			break;

		case AMP_LEVEL:
			value = value + 1;
			value = value / 2;
			getSynth()->setParameter(SENGINE_AMPLEVEL, value);
			break;

		case FILT_FREQ:
			getSynth()->setParameter(SENGINE_FILTFREQ, value);
			break;

		case FILT_RESO:
			getSynth()->setParameter(SENGINE_FILTRESO, value);
			break;

		case FILT_ENV2DEPTH:
			getSynth()->setParameter(SENGINE_ENV2_TO_CUTOFF, value);
			break;

		case DELAY_TIME:
			value = value + 1;
			value = value / 2;
			value = value * (int)getSynth()->getEcho()->getBufferLength();
			getSynth()->getEcho()->setLength(((long)k->getValue() * (int)getSynth()->getEcho()->getBufferLength()) / 127);
			break;
		case DELAY_FB:
			value = value + 1;
			value = value / 2;
			getSynth()->getEcho()->setFeedback(value);
			break;
		case DELAY_LEVEL:
			value = value + 1;
			value = value / 2;
			getSynth()->getEcho()->setLevel(value);
			break;

		default:
			return;
			break;
	}
}

void SynthGUI::buttonSelectedEvent(GUIWidgetButton *k) {

	switch (k->getName()) {

		case PROGRAM_PREVIOUS:
			setProgram(getProgram() - 1);
			((GUIWidgetLCD*)getWidget(PARAM_LCD))->switchOn(1);
			getWidget(PARAM_LCD)->setValue(getProgram());
			break;

		case PROGRAM_NEXT:
			setProgram(getProgram() + 1);
			((GUIWidgetLCD*)getWidget(PARAM_LCD))->switchOn(1);
			getWidget(PARAM_LCD)->setValue(getProgram());
			break;

		case PROGRAM_SAVE:
			((GUIWidgetLCD*)getWidget(PARAM_LCD))->switchOn(1);
			getWidget(PARAM_LCD)->setValue(getProgram());
			saveProgram();
			break;

		case FILT_DBSWITCH: {
			int v;

			if (getWidget(FILT_12DB)->getValue())
				v = 0;
			else
				v = 1;

			v ^= 1;

			getWidget(FILT_12DB)->setValue(v == 0);
			getWidget(FILT_24DB)->setValue(v == 1);

			getSynth()->setFilter24dB(v);
			break;
		}

		case OSC1_WAVSWITCH: {
			int v = getOSC1Waveform();

			v++;
			if (v > 2)
				v = 0;

			if (v == 0)
				getSynth()->getOscillator(0)->setWave(OSC_SAWTOOTH);
			else
			if (v == 1)
				getSynth()->getOscillator(0)->setWave(OSC_TRIANGLE);
			else
			if (v == 2)
				getSynth()->getOscillator(0)->setWave(OSC_RECTANGLE);

			setOSC1Waveform(v);
			break;
		}

		case OSC2_WAVSWITCH: {
			int v = getOSC2Waveform();

			v++;
			if (v > 2)
				v = 0;

			if (v == 0)
				getSynth()->getOscillator(1)->setWave(OSC_SAWTOOTH);
			else
			if (v == 1)
				getSynth()->getOscillator(1)->setWave(OSC_TRIANGLE);
			else
			if (v == 2)
				getSynth()->getOscillator(1)->setWave(OSC_RECTANGLE);

			setOSC2Waveform(v);
			break;
		}

		case OSC1_OCTSWITCH: {
			int v = getOSC1Oct();
			long detune = 0;

			v++;
			if (v > 2)
				v = 0;

			if (v == 0) {

				detune = -1 * 128 * 12;
			}
			else
			if (v == 2) {

				detune = 1 * 128 * 12;
			}

			getSynth()->getOscillator(0)->setDetune(detune);

			setOSC1Oct(v);
			break;
		}

		case OSC2_OCTSWITCH: {

			int v = getOSC2Oct();
			long detune = 0;

			v++;
			if (v > 2)
				v = 0;

			if (v == 0) {

				detune = -1 * 128 * 12;
			} else
			if (v == 2) {

				detune = 1 * 128 * 12;
			}

			getSynth()->getOscillator(1)->setDetune(detune);
			setOSC2Oct(v);
			break;
		}
	}
}

void SynthGUI::widgetChanged(GUIWidget *k) {

	if (k->getType() == WIDGET_TURN) {

		GUIWidgetLCD *lcd = (GUIWidgetLCD*)getWidget(PARAM_LCD);

		knobValueChangedEvent((GUIWidgetTurn*)k);
		lcd->setValue(currentWidget->getValue());
		lcd->switchOn(1);
	} else
	if (k->getType() == WIDGET_BUTTON) {

		buttonSelectedEvent((GUIWidgetButton*)k);
	}
}

SynthEngine *SynthGUI::getSynth() {

	return synth;
}

int SynthGUI::getOSC1Waveform() {

	int v = 0;

	if (getWidget(OSC1_WAVSAW)->getValue())
		v = 0;
	else
	if (getWidget(OSC1_WAVTRI)->getValue())
		v = 1;
	else
	if (getWidget(OSC1_WAVRECT)->getValue())
		v = 2;

	return v;
}

void SynthGUI::setOSC1Waveform(int v) {

	getWidget(OSC1_WAVSAW)->setValue(v == 0);
	getWidget(OSC1_WAVTRI)->setValue(v == 1);
	getWidget(OSC1_WAVRECT)->setValue(v == 2);
}

int SynthGUI::getOSC2Waveform() {

	int v = 0;

	if (getWidget(OSC2_WAVSAW)->getValue())
		v = 0;
	else
	if (getWidget(OSC2_WAVTRI)->getValue())
		v = 1;
	else
	if (getWidget(OSC2_WAVRECT)->getValue())
		v = 2;

	return v;
}

void SynthGUI::setOSC2Waveform(int v) {

	getWidget(OSC2_WAVSAW)->setValue(v == 0);
	getWidget(OSC2_WAVTRI)->setValue(v == 1);
	getWidget(OSC2_WAVRECT)->setValue(v == 2);
}

int SynthGUI::getOSC1Oct() {

	int v = 0;

	if (getWidget(OSC1_OCTDOWN)->getValue())
		v = 0;
	else
	if (getWidget(OSC1_OCTMID)->getValue())
		v = 1;
	else
	if (getWidget(OSC1_OCTUP)->getValue())
		v = 2;

	return v;
}

int SynthGUI::getOSC2Oct() {

	int v = 0;

	if (getWidget(OSC2_OCTDOWN)->getValue())
		v = 0;
	else
	if (getWidget(OSC2_OCTMID)->getValue())
		v = 1;
	else
	if (getWidget(OSC2_OCTUP)->getValue())
		v = 2;

	return v;
}

void SynthGUI::setOSC1Oct(int v) {

	getWidget(OSC1_OCTDOWN)->setValue(v == 0);
	getWidget(OSC1_OCTMID)->setValue(v == 1);
	getWidget(OSC1_OCTUP)->setValue(v == 2);
}

void SynthGUI::setOSC2Oct(int v) {

	getWidget(OSC2_OCTDOWN)->setValue(v == 0);
	getWidget(OSC2_OCTMID)->setValue(v == 1);
	getWidget(OSC2_OCTUP)->setValue(v == 2);
}

void SynthGUI::getPalette(GUIColor *palette) {

	int i;

	for (i = 0; i < 18; i++) {

		palette[i + 256 - 18].r = knobpal[(i * 3) + 0];
		palette[i + 256 - 18].g = knobpal[(i * 3) + 1];
		palette[i + 256 - 18].b = knobpal[(i * 3) + 2];
	}

	for (i = 0; i < 4; i++) {

		palette[i + 128].r = lcdpal[(i * 3) + 0];
		palette[i + 128].g = lcdpal[(i * 3) + 1];
		palette[i + 128].b = lcdpal[(i * 3) + 2];
	}

	for (i = 0; i < 128; i++) {

		palette[i].r = synthpal[(i * 3) + 0];
		palette[i].g = synthpal[(i * 3) + 1];
		palette[i].b = synthpal[(i * 3) + 2];
	}
}

void SynthGUI::update() {

	float t;

	/* Filter */
	getWidget(FILT_ENV2DEPTH)->setRealValue(synth->getParameter(SENGINE_ENV2_TO_CUTOFF));
	getWidget(FILT_LFO2DEPTH)->setRealValue(synth->getParameter(SENGINE_LFO2_TO_CUTOFF));
	getWidget(FILT_12DB)->setValue(!synth->getFilter24dB());
	getWidget(FILT_24DB)->setValue(synth->getFilter24dB());
	getWidget(FILT_FREQ)->setRealValue(synth->getParameter(SENGINE_FILTFREQ));
	getWidget(FILT_RESO)->setRealValue(synth->getParameter(SENGINE_FILTRESO));

	/* Envelopes */
	getWidget(ENV1_A)->setRealValue(synth->getEnvelope(0)->getA());
	getWidget(ENV1_D)->setRealValue(synth->getEnvelope(0)->getD());
	getWidget(ENV1_S)->setRealValue(synth->getEnvelope(0)->getS());
	getWidget(ENV1_R)->setRealValue(synth->getEnvelope(0)->getR());
	getWidget(ENV2_A)->setRealValue(synth->getEnvelope(1)->getA());
	getWidget(ENV2_D)->setRealValue(synth->getEnvelope(1)->getD());
	getWidget(ENV2_S)->setRealValue(synth->getEnvelope(1)->getS());
	getWidget(ENV2_R)->setRealValue(synth->getEnvelope(1)->getR());

	/* Amplifier */
	getWidget(AMP_LEVEL)->setRealValue(synth->getParameter(SENGINE_AMPLEVEL));
	getWidget(AMP_LFO1DEPTH)->setRealValue(synth->getParameter(SENGINE_LFO1_TO_AMP));
	getWidget(AMP_OSCMIX)->setRealValue(synth->getParameter(SENGINE_OSCMIX));

	/* LFO1 */
	getWidget(LFO1_RATE)->setRealValue(synth->getLFO(0)->getRate());

	/* LFO2 */
	getWidget(LFO2_RATE)->setRealValue(synth->getLFO(1)->getRate());

	/* OSC1 */
	getWidget(OSC1_WAVSAW)->setValue(synth->getOscillator(0)->getWave() == OSC_SAWTOOTH);
	getWidget(OSC1_WAVRECT)->setValue(synth->getOscillator(0)->getWave() == OSC_RECTANGLE);
	getWidget(OSC1_WAVTRI)->setValue(synth->getOscillator(0)->getWave() == OSC_TRIANGLE);
	getWidget(OSC1_OCTUP)->setValue(synth->getOscillator(0)->getDetune() > 0);
	getWidget(OSC1_OCTMID)->setValue(synth->getOscillator(0)->getDetune() == 0);
	getWidget(OSC1_OCTDOWN)->setValue(synth->getOscillator(0)->getDetune() < 0);
	getWidget(OSC1_PW_ENV1MOD)->setRealValue(synth->getParameter(SENGINE_ENV1_TO_OSC1PW));

	/* OSC2 */
	getWidget(OSC2_WAVSAW)->setValue(synth->getOscillator(1)->getWave() == OSC_SAWTOOTH);
	getWidget(OSC2_WAVRECT)->setValue(synth->getOscillator(1)->getWave() == OSC_RECTANGLE);
	getWidget(OSC2_WAVTRI)->setValue(synth->getOscillator(1)->getWave() == OSC_TRIANGLE);
getWidget(OSC2_OCTUP)->setValue(synth->getOscillator(1)->getDetune() > 0);
	getWidget(OSC2_OCTMID)->setValue(synth->getOscillator(1)->getDetune() == 0);
	getWidget(OSC2_OCTDOWN)->setValue(synth->getOscillator(1)->getDetune() < 0);
	getWidget(OSC2_PW_ENV1MOD)->setRealValue(synth->getParameter(SENGINE_ENV1_TO_OSC2PW));

	/* Delay */
	t = synth->getEcho()->getLength();
	t = t / synth->getEcho()->getBufferLength();
	getWidget(DELAY_TIME)->setRealValue((mfloat)t);
	getWidget(DELAY_FB)->setRealValue(synth->getEcho()->getFeedback());
	getWidget(DELAY_LEVEL)->setRealValue(synth->getEcho()->getLevel());

	forceDraw();
}

void SynthGUI::saveProgram() {

	char programName[256];

	getProgramFileName(programName, getProgram());
	getSynth()->saveProgram(programName);
}

void SynthGUI::getProgramFileName(char *name, int num) {

	sprintf(name, "PRG%02d.PBS", num);
}

void SynthGUI::setProgram(int i) {

	char programName[256];

	if ((i >= getNumPrograms()) || (i < 0)) {

		((GUIWidgetLCD*)getWidget(PARAM_LCD))->switchOn(1);
		getWidget(PARAM_LCD)->setValue(getProgram());
		update();
		forceDraw();
		return;
	}

	currentProgram = i;
	getProgramFileName(programName, getProgram());

	if (getSynth()->loadProgram(programName) != 0) {

		getSynth()->reset();
	}

	((GUIWidgetLCD*)getWidget(PARAM_LCD))->switchOn(1);
	getWidget(PARAM_LCD)->setValue(getProgram());
	update();
	forceDraw();
}

int SynthGUI::getProgram() {

	return currentProgram;
}

int SynthGUI::getNumPrograms() {

	return NUM_PROGRAMS;
}
