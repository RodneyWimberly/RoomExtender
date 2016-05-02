#include "DfrLcdKeypad.h"

DfrLcdKeyPad::DfrLcdKeyPad(uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t keyPadPin) : 
	LiquidCrystal(rs, enable, d0, d1, d2, d3)
{
	p_keyPadPin = keyPadPin;
	createChar(CustomCharacters::DegreeCentigrade, p_degreeCentigrade);
	createChar(CustomCharacters::DegreeFahrenheit, p_degreeFahrenheit);
	createChar(CustomCharacters::ArrowUp, p_upArrow);
	createChar(CustomCharacters::ArrowDown, p_downArrow);
}

KeyPadButtons DfrLcdKeyPad::getKeypadButton()
{
	int adcValue = analogRead(p_keyPadPin);
	KeyPadButtons keyPadButton = KeyPadButtons::None;
	if (adcValue < 50)   keyPadButton = KeyPadButtons::Right;
	else if (adcValue < 195)  keyPadButton = KeyPadButtons::Up;
	else if (adcValue < 380)  keyPadButton = KeyPadButtons::Down;
	else if (adcValue < 555)  keyPadButton = KeyPadButtons::Left;
	else if (adcValue < 790)  keyPadButton = KeyPadButtons::Select;
	return keyPadButton;
}

void DfrLcdKeyPad::writeCustomCharacter(CustomCharacters customCharacter)
{
	write((uint8_t)customCharacter);
}
