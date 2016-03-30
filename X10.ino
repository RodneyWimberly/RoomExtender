#include "Arduino.h" 
#include "X10ex.h"
#include "RoomExtender.h"

/******************************************************************************************
/* X10 Methods
/******************************************************************************************/

// Processes and executes 3 byte serial and wireless messages
bool process3BMessage(const char type[], byte byte1, byte byte2, byte byte3)
{
	bool x10exBufferError = 0;
	// Convert byte2 from hex to decimal unless command is request module state
	if (byte1 != 'R') byte2 = charHexToDecimal(byte2);
	// Convert byte3 from hex to decimal unless command is wipe module state
	if (byte1 != 'R' || byte2 != 'W') byte3 = charHexToDecimal(byte3);
	// Check if standard message was received (byte1 = House, byte2 = Unit, byte3 = Command)
	if (byte1 >= 'A' && byte1 <= 'P' && (byte2 <= 0xF || byte2 == '_') && (byte3 <= 0xF || byte3 == '_') && (byte2 != '_' || byte3 != '_'))
	{
		// Store first 3 bytes of message to make it possible to process 9 byte serial messages
		bmHouse = byte1;
		bmUnit = byte2 == '_' ? 0 : byte2 + 1;
		bmCommand = byte3 == '_' ? CMD_STATUS_REQUEST : byte3;
		bmExtCommand = 0;
		// Send standard message if command type is not extended
		if (bmCommand != CMD_EXTENDED_CODE && bmCommand != CMD_EXTENDED_DATA)
		{
			printX10Message(type, bmHouse, bmUnit, byte3, 0, 0, 8 * Serial.available());
			// Check if command is handled by scenario; if not continue
			if (!handleUnitScenario(bmHouse, bmUnit, bmCommand, false, true))
			{
				x10exBufferError = x10ex.sendCmd(bmHouse, bmUnit, bmCommand, bmCommand == CMD_BRIGHT || bmCommand == CMD_DIM ? 2 : 1);
			}
			bmHouse = 0;
		}
	}
	// Check if extended message was received (byte1 = Hex Separator, byte2 = Hex 1, byte3 = Hex 2)
	else if (byte1 == 'X' && bmHouse)
	{
		byte data = byte2 * 16 + byte3;
		// No extended command set, assume that we are receiving command
		if (!bmExtCommand)
		{
			bmExtCommand = data;
		}
		// Extended command set, we must be receiving extended data
		else
		{
			printX10Message(type, bmHouse, bmUnit, bmCommand, data, bmExtCommand, 8 * Serial.available());
			x10exBufferError = x10ex.sendExt(bmHouse, bmUnit, bmCommand, data, bmExtCommand, 1);
			bmHouse = 0;
		}
	}
	// Check if scenario execute command was received (byte1 = Scenario Character, byte2 = Hex 1, byte3 = Hex 2)
	else if (byte1 == 'S')
	{
		byte scenario = byte2 * 16 + byte3;
		Serial.print(type);
		Serial.print("S");
		if (scenario <= 0xF) { Serial.print("0"); }
		Serial.println(scenario, HEX);
		handleSdScenario(scenario);
	}
	// Check if request module state command was received (byte1 = Request State Character, byte2 = House, byte3 = Unit)
	else if (byte1 == 'R' && ((byte2 >= 'A' && byte2 <= 'P') || byte2 == '*'))
	{
		Serial.print(type);
		Serial.print("R");
		Serial.print(byte2);
		// All modules
		if (byte2 == '*')
		{
			Serial.println('*');
			for (short i = 0; i < 256; i++)
			{
				sdPrintModuleState((i >> 4) + 0x41, i & 0xF);
			}
		}
		else
		{
			if (byte3 <= 0xF)
			{
				Serial.println(byte3, HEX);
				sdPrintModuleState(byte2, byte3);
			}
			// All units using specified house code
			else
			{
				Serial.println('*');
				for (byte i = 0; i < 16; i++)
				{
					sdPrintModuleState(byte2, i);
				}
			}
		}
	}
	// Check if request wipe module state command was received (byte1 = Request State Character, byte2 = Wipe Character, byte3 = House)
	else if (byte1 == 'R' && byte2 == 'W' && ((byte3 >= 'A' && byte3 <= 'P') || byte3 == '*'))
	{
		Serial.print(type);
		Serial.print("RW");
		Serial.println((char)byte3);
		x10ex.wipeModuleState(byte3);
		Serial.print(MODULE_STATE_MSG);
		Serial.print(byte3 >= 'A' && byte3 <= 'P' ? (char)byte3 : '*');
		Serial.println("__");
	}
	// Unknown command/data
	else
	{
		Serial.print(type);
		Serial.println(MSG_DATA_ERROR);
	}
	return x10exBufferError;
}

