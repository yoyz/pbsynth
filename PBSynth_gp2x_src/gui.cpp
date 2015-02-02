#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "gui.h"

#include "knobraw.h"
#include "knobpal.h"
#include "lcdraw.h"
#include "lcdpal.h"

void hideCursor(void);
void showCursor(void);
void moveCursor(int x, int y);




/******** class GUI ********/

GUI::GUI(int width, int height) {

	numWidgets = 0;
	cursorX = cursorY = 0;
	this->width = width;
	this->height = height;
	this->background = 0;
	this->bgColor = 0;
	currentWidget = 0;
}

GUI::~GUI() {

	int i;

	for (i = 0; i < numWidgets; i++)
		delete widgets[i];
}

void GUI::getPalette(GUIColor */*palette*/) {}

int GUI::mustDraw() {

	return update;
}

void GUI::forceDraw() {

	update = 1;
}

void GUI::widgetChanged(GUIWidget */*k*/) {}

int GUI::newWidget(GUIWidget *widget) {

	if (numWidgets >= MAX_WIDGETS || !widget)
		return -1;

	widgets[numWidgets] = widget;

	return numWidgets++;
}

int GUI::newWidget(int xpos, int ypos, widget_type type, int name, int value) {

	GUIWidget *widget;

	switch (type) {

		case WIDGET_TURN:
			widget = new GUIWidgetTurn(this, xpos, ypos, name, value);
			break;
		case WIDGET_BUTTON:
			widget = new GUIWidgetButton(this, xpos, ypos, name, value);
			break;
		case WIDGET_LED:
			widget = new GUIWidgetLED(this, xpos, ypos, name, value);
			break;
		case WIDGET_LCD:
			widget = new GUIWidgetLCD(this, xpos, ypos, name, value);
			break;

		default:
			return -1;
	}

	return newWidget(widget);
}

mfloat GUI::getWidgetValSign(int name) {

	mfloat v;
	GUIWidget *n;

	n = getWidget(name);

	v = n->getValue();

	if (n->getSign()) {

		v = v / 64;
	} else {

		v = (v * 2) / 127;
		v = v - 1;
	}

	return v;
}

void GUI::drawAllWidgets(unsigned char *dest) {

	int i;

	for (i = 0; i < numWidgets; i++)
		widgets[i]->draw(dest);
}

void GUI::drawBackground(unsigned char *dest) {

	int i;
	int d;
	int x, y;

	d = GUI_OFS(0, 0);
	for (y = i = 0; y < height; y++) {

		for (x = 0; x < width; x++, i++) {

			dest[d] = background[i];
			d += GUI_XINC;
		}

		d -= 320 * GUI_XINC;
		d += GUI_YINC;
	}
}

void GUI::draw(unsigned char *dest) {

	if (!mustDraw())
		return;

	if (background)
		drawBackground(dest);
	else {

		int i;

		for (i = 0; i < width * height; i++)
			dest[i] = bgColor;
	}

	drawAllWidgets(dest);

	update = 0;
}

void GUI::setBackground(unsigned char *bg) {

	background = bg;
}

void GUI::setBgColor(unsigned char c) {

	bgColor = c;
}

int GUI::mousePressed() {

	GUIWidget *widget;

	if ((widget = getWidget(cursorX, cursorY))) {

		currentWidget = widget;

		if (currentWidget->getType() == WIDGET_TURN) {

			((GUIWidgetTurn*)currentWidget)->addValue(0);
#ifdef GUI_HAVE_MOUSE_CONTROL
			GUI_HIDECURSOR();
#endif
		} else
		if (currentWidget->getType() == WIDGET_BUTTON) {

			currentWidget->setValue(1);
		}

		forceDraw();

		return 1;
	} else {

		currentWidget = 0;
	}

	return 0;
}

int GUI::mouseReleased() {

	if (currentWidget) {

		if (currentWidget->getType() == WIDGET_TURN) {

#ifdef GUI_HAVE_MOUSE_CONTROL
			GUI_SHOWCURSOR();
#endif
		} else
		if (currentWidget->getType() == WIDGET_BUTTON) {

			if (currentWidget->getValue())
				widgetChanged(currentWidget);

			currentWidget->setValue(0);
		}

		currentWidget = 0;

		forceDraw();

		return 1;
	}

	return 0;
}

void GUI::addValue(int value)
{
    GUIWidget *widget;
	if ((widget = getWidget(cursorX, cursorY)))
	{
	    currentWidget = widget;
        if (currentWidget->getType() == WIDGET_TURN)
        {
            ((GUIWidgetTurn*)currentWidget)->addValue(value);
        }

        forceDraw();
    }
    currentWidget=0;
}

int GUI::getValue()
{
    GUIWidget *widget;
	if ((widget = getWidget(cursorX, cursorY)))
	{
	    currentWidget = widget;
        if (currentWidget->getType() == WIDGET_TURN)
        {
            return ((GUIWidgetTurn*)currentWidget)->getValue();
        }
    }
    currentWidget=0;
}

int GUI::cursorOnWidget()
{
    GUIWidget *widget;
	if ((widget = getWidget(cursorX, cursorY)))
	{
	    currentWidget = widget;
        if (currentWidget->getType() == WIDGET_TURN || currentWidget->getType() == WIDGET_BUTTON)
        {
            currentWidget=0;
            return 1;
        }

        //forceDraw();
    }
    currentWidget=0;

    return 0;
}

int GUI::mouseMoved(int x, int y) {

	if (currentWidget) {

		if (x != cursorX || y != cursorY) {

			int xrel, yrel;

			xrel = x - cursorX;
			yrel = y - cursorY;

			if (currentWidget->getType() == WIDGET_TURN) {

				((GUIWidgetTurn*)currentWidget)->addValue(xrel);
				((GUIWidgetTurn*)currentWidget)->addValue(-yrel);
#ifdef GUI_HAVE_MOUSE_CONTROL
				GUI_MOVECURSOR(cursorX, cursorY);
				x = cursorX;
				y = cursorY;
#endif
			} else
			if (currentWidget->getType() == WIDGET_BUTTON) {

				if (currentWidget->pointInWidget(x, y))
					currentWidget->setValue(1);
				else
					currentWidget->setValue(0);
			}

			forceDraw();

			cursorX = x;
			cursorY = y;
			return 1;
		}
	} else {

		cursorX = x;
		cursorY = y;
	}

	return 0;
}




/******** class GUIElement ********/

GUIWidget::GUIWidget(GUI *gui, int xpos, int ypos, widget_type type, int name, int value) {

	this->xpos = xpos;
	this->ypos = ypos;
	this->type = type;
	this->name = name;
	this->value = value;
	this->gui = gui;
	this->sign = 0;
	setOutVal(-1.0f, 1.0f);
}

GUIWidget::~GUIWidget() {
}

void GUIWidget::draw(unsigned char */*dest*/) {}

int GUIWidget::pointInWidget(int /*x*/, int /*y*/) { return 0; }

int GUIWidget::getName() {

	return name;
}

void GUIWidget::setOutVal(mfloat min, mfloat max) {

	this->min = min;
	this->max = max;
}

mfloat GUIWidget::getRealValue() {

	mfloat ret;
	int v = getValue();

	if (getSign())
		v += 64;

	ret = v;
	ret = ((ret / 127) * (max - min)) + min;
	ret = ret * (max - min);
	ret = ret + min;

	return ret;
}

int GUIWidget::setRealValue(mfloat v) {

	int x;
	mfloat mm1, mm2, mm3;

	mm1 = v - min;
	mm2 = max - min;

	mm1 = mm1 / 16;
	mm2 = mm2 / 16;

	mm3 = mm1 / mm2;
	mm3 = mm3 * 127;

	x = (int)mm3;

	if (getSign())
		x -= 63;

	setValue(x);

	return x;
}

GUIWidget *GUI::getWidget(int name) {

	int i;

	for (i = 0; i < numWidgets; i++) {

		if (widgets[i]->getName() == name)
			return widgets[i];
	}

	return 0;
}

GUIWidget *GUI::getWidget(int x, int y) {

	int i;

	for (i = 0; i < numWidgets; i++) {

		if (widgets[i]->pointInWidget(x, y))
			return widgets[i];
	}

	return 0;
}

int GUIWidget::getValue() {

	return value;
}

int GUIWidget::getSign() {

	return sign;
}

void GUIWidget::setSign(int sign) {

	this->sign = sign;
}

void GUIWidget::setValue(int value) {

	this->value = value;
}

int GUIWidget::getType() { return -1; }




/******** class GUIKnob ********/

GUIWidgetTurn::GUIWidgetTurn(GUI *gui, int xpos, int ypos, int name, int value) :
	GUIWidget(gui, xpos, ypos, WIDGET_TURN, name, value) {

}

GUIWidgetTurn::~GUIWidgetTurn() {
}

int GUIWidgetTurn::getType() { return WIDGET_TURN; }

int GUIWidgetTurn::pointInWidget(int x, int y) {

	if ((x > xpos) && (x < (xpos + 16)) &&
	    (y > ypos) && (y < (ypos + 16)))
		return 1;
	else
		return 0;
}

int GUIWidgetTurn::addValue(int val) {

	value += val;

	if (sign) {

		if (value > 63) {

			value = 63;
		} else
		if (value < -64) {

			value = -64;
		}
	} else {

		if (value > 127) {

			value = 127;
		} else
		if (value < 0) {

			value = 0;
		}
	}

	gui->widgetChanged(this);

	return value;
}

void GUIWidgetTurn::draw(unsigned char *dest) {

	int lvalue = value;
	int n;
	int d;
	int s = 16 * 16;
	int x, y;

	lvalue = ((lvalue + 1) / 4) - 1;

	if (sign)
		lvalue += 16;

	n = lvalue;

	if (n < 0) n = 0; else
	if (n > 31) n = 31;

	d = GUI_OFS(xpos, ypos);

	s *= n;

	for (y = 0; y < 16; y++) {

		for (x = 0; x < 16; x++) {


			if (knobraw[s] != 4)
				dest[d + (x * GUI_XINC)] = knobraw[s] + 238;

			s++;
		}

		d += GUI_YINC;
	}
}




/******** class GUILED ********/

GUIWidgetLED::GUIWidgetLED(GUI *gui, int xpos, int ypos, int name, int value) :
	GUIWidget(gui, xpos, ypos, WIDGET_LED, name, value) {
}

GUIWidgetLED::~GUIWidgetLED() {
}

int GUIWidgetLED::getType() { return WIDGET_LED; }

int GUIWidgetLED::pointInWidget(int x, int y) {

	if ((x > xpos) && (x < (xpos + 8)) &&
	    (y > ypos) && (y < (ypos + 8)))
		return 1;
	else
		return 0;
}

void GUIWidgetLED::draw(unsigned char *dest) {

	int d = GUI_OFS(xpos, ypos);
	int s = 528 * 16;
	int x, y;

	if (value)
		s += 8;

	for (y = 0; y < 8; y++) {

		for (x = 0; x < 8; x++) {


			if (knobraw[s] != 4)
				dest[d + (x * GUI_XINC)] = knobraw[s] + 238;

			s++;
		}

		s += 8;
		d += GUI_YINC;
	}
}




/******** class GUIButton ********/

GUIWidgetButton::GUIWidgetButton(GUI *gui, int xpos, int ypos, int name, int value) :
	GUIWidget(gui, xpos, ypos, WIDGET_BUTTON, name, value) {
}

GUIWidgetButton::~GUIWidgetButton() {
}

int GUIWidgetButton::getType() { return WIDGET_BUTTON; }

void GUIWidgetButton::draw(unsigned char *dest) {

	int d = GUI_OFS(xpos, ypos);
	int s = 512 * 16;
	int x, y;

	if (value)
		s += 16 * 8;

	for (y = 0; y < 8; y++) {

		for (x = 0; x < 16; x++) {

			dest[d + (x * GUI_XINC)] = knobraw[s] + 238;
			s++;
		}

		d += GUI_YINC;
	}
}

int GUIWidgetButton::pointInWidget(int x, int y) {

	if ((x > xpos) && (x < (xpos + 16)) &&
	    (y > ypos) && (y < (ypos + 8)))
		return 1;
	else
		return 0;
}




/******** GUILCD ********/

GUIWidgetLCD::GUIWidgetLCD(GUI *gui, int xpos, int ypos, int name, int value) :
	GUIWidget(gui, xpos, ypos, WIDGET_BUTTON, name, value) {

	setNumDigits(4);
	on = 0;
}

GUIWidgetLCD::~GUIWidgetLCD() {
}

void GUIWidgetLCD::setNumDigits(int numDigits) {

	if (numDigits < 0)
		return;

	this->numDigits = numDigits;
	width = 24 * numDigits;
	height = 32;
	setValue(value);
}

int GUIWidgetLCD::getType() { return WIDGET_LCD; }

void GUIWidgetLCD::setValue(int value) {

	int i;
	char n[32];

	GUIWidget::setValue(value);

	for (i = 0; i < numDigits; i++)
		lcdNumber[i] = ' ';

	if (on) {

		sprintf(n, "%d", value);
		memcpy(&lcdNumber[3 - strlen(n)], n, strlen(n));
	}

	gui->forceDraw();
}

void GUIWidgetLCD::drawDigit(unsigned char *dest, int n, char digit) {

	int d = GUI_OFS(xpos + (n * 24), ypos);
	int s = 24 * 32;
	int x, y;

	s *= (int)digit;

	for (y = 0; y < 32; y++) {

		for (x = 0; x < 24; x++) {

			dest[d + (x * GUI_XINC)] = lcdraw[s] + 128;

			s++;
		}

		d += GUI_YINC;
	}
}

void GUIWidgetLCD::draw(unsigned char *dest) {

	int i;

	for (i = 0; i < numDigits; i++) {

		int d = lcdNumber[i];

		if (d >= '0' && d <= '9')
			d -= '0';
		else
		if (d == '-')
			d = 10;
		else
		if (d == ' ')
			d = 11;

		drawDigit(dest, i, d);
	}
}

void GUIWidgetLCD::switchOn(int on) {

	this->on = on;
	setValue(value);
}
