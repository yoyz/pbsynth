#ifndef __GUI_H__
#define __GUI_H__

/* General GUI classes */

#include "mytypes.h"
#include "archdep.h"

#define MAX_WIDGETS 64

#if defined(GP32)
#define GUI_HAVE_MOUSE_CONTROL
#endif

#ifdef GUI_HAVE_MOUSE_CONTROL
#define GUI_MOVECURSOR(x, y) moveCursor(x, y)
#define GUI_SHOWCURSOR() showCursor()
#define GUI_HIDECURSOR() hideCursor()
#endif

enum widget_type {

	WIDGET_BUTTON,
	WIDGET_LED,
	WIDGET_TURN,
	WIDGET_LCD
};

class GUI;
class GUIWidget;
class GUIWidgetTurn;
class GUIWidgetLED;
class GUIWidgetButton;
class GUIWidgetLCD;

typedef struct GUIColor {

	unsigned char r, g, b;
} GUIColor;

class GUI {

public:
	GUI(int width, int height);
	virtual ~GUI();
	virtual void getPalette(GUIColor *palette);
	virtual int mousePressed();
	virtual int mouseReleased();
	virtual void addValue(int value);
	virtual int getValue();
	virtual int cursorOnWidget();
	virtual int mouseMoved(int x, int y);
	virtual void widgetChanged(GUIWidget *k);
	int newWidget(int xpos, int ypos, widget_type type, int name, int value);
	int newWidget(GUIWidget *widget);
	void draw(unsigned char *dest);
	void setBackground(unsigned char *bg);
	void setBgColor(unsigned char c);
	int mustDraw();
	void forceDraw();
	GUIWidget *getWidget(int name);

protected:
	int width, height;
	unsigned char *background;
	unsigned char bgColor;
	GUIWidget *widgets[MAX_WIDGETS];
	GUIWidget *currentWidget;
	int numWidgets;
	int cursorX, cursorY;
	int update;

	void drawAllWidgets(unsigned char *dest);
	void drawBackground(unsigned char *dest);
	int pointInWidget(int i, int x, int y);
	mfloat getWidgetValSign(int name);
	GUIWidget *getWidget(int x, int y);
};

class GUIWidget {

public:
	GUIWidget(GUI *gui, int xpos, int ypos, widget_type type, int name, int value);
	virtual ~GUIWidget();
	int getName();
	virtual int getValue();
	virtual void setValue(int value);
	int getSign();
	void setSign(int sign);
	mfloat getRealValue();
	int setRealValue(mfloat v);
	void setOutVal(mfloat min, mfloat max);
	virtual int getType();
	virtual void draw(unsigned char *dest);
	virtual int pointInWidget(int x, int y);

protected:
	int xpos, ypos;
	int width, height;
	mfloat min, max;
	int type;
	int name;
	int value;
	int sign;
	GUI *gui;
};


class GUIWidgetTurn : public GUIWidget {

public:
	GUIWidgetTurn(GUI *gui, int xpos, int ypos, int name, int value);
	~GUIWidgetTurn();
	void draw(unsigned char *dest);
	int pointInWidget(int x, int y);
	int addValue(int val);
	int getType();
};

class GUIWidgetButton : public GUIWidget {

public:
	GUIWidgetButton(GUI *gui, int xpos, int ypos, int name, int value);
	~GUIWidgetButton();
	void draw(unsigned char *dest);
	int pointInWidget(int x, int y);
	int getType();
};

class GUIWidgetLED : public GUIWidget {

public:
	GUIWidgetLED(GUI *gui, int xpos, int ypos, int name, int value);
	~GUIWidgetLED();
	void draw(unsigned char *dest);
	int pointInWidget(int x, int y);
	int getType();
};

class GUIWidgetLCD : public GUIWidget {

public:
	GUIWidgetLCD(GUI *gui, int xpos, int ypos, int name, int value);
	~GUIWidgetLCD();
	void draw(unsigned char *dest);
	void setNumDigits(int numDigits);
	void setValue(int value);
	int getType();
	void switchOn(int on);

private:
	void drawDigit(unsigned char *dest, int n, char digit);
	int numDigits;
	char lcdNumber[3];
	int on;
};

#endif
