#ifndef _KEYPADBUTTONS_h
#define _KEYPADBUTTONS_h

#include <Arduino.h>
#include <LiquidCrystal.h>

enum KeyPadButtons
{
	Right,
	Up,
	Down,
	Left,
	Select,
	None
};

enum CustomCharacters
{
	DegreeCentigrade,
	DegreeFahrenheit,
	ArrowUp,
	ArrowDown
};


class DfrLcdKeyPad : public LiquidCrystal
{
public:
	DfrLcdKeyPad(uint8_t rs, uint8_t enable,
		uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t keyPadPin);
	KeyPadButtons getKeypadButton();
	void writeCustomCharacter(CustomCharacters customCharacter);
	
private:
	uint8_t p_keyPadPin;
	byte p_degreeCentigrade[8] = { B01000, B10100, B01000, B00011, B00100, B00100, B00011, B00000 };
	byte p_degreeFahrenheit[8] = { B01000, B10100, B01000, B00011, B00100, B00111, B00100, B00000 };
	byte p_upArrow[8] = { B00100, B01110, B10101, B00100, B00100, B00100, B00100, B00000 };
	byte p_downArrow[8] = { B00000, B00100, B00100, B00100, B00100, B10101, B01110, B00100 };
};

#endif
