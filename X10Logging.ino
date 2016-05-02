#include "RoomExtender.h"

void printX10Message(const char type[], char house, byte unit, byte command, byte extData, byte extCommand, int remainingBits)
{
	printX10TypeHouseUnit(type, house, unit, command);
	// Ignore non X10 commands like the CMD_ADDRESS command used by the IR library
	if (command <= 0xF)
	{
		Serial.print(command, HEX);
		if (extCommand || (extData && (command == CMD_STATUS_ON || command == CMD_STATUS_OFF)))
		{
			printX10ByteAsHex(extCommand);
			printX10ByteAsHex(extCommand == EXC_PRE_SET_DIM ? extData & B111111 : extData);
		}
	}
	else
	{
		Serial.print(F("_"));
	}
	Serial.println();

	// Send info via RF24L01+
	p_x10Data.houseCode = house;
	p_x10Data.unitCode = unit;
	p_x10Data.commandCode = command;
	p_x10Data.extendedData = extData;
	p_x10Data.extendedCommand = extCommand;

	if (strcmp(type, SERIAL_DATA_MSG) == 0)
	{
		p_headerType = HeaderTypes::reSerialDataEvent;
	}
	else if (strcmp(type, POWER_LINE_MSG) == 0)
	{
		p_headerType = HeaderTypes::rePowerLineEvent;
	}
	else if (strcmp(type, RADIO_FREQ_MSG) == 0)
	{
		p_headerType = HeaderTypes::reRadioFrequencyEvent;
	}
	else if (strcmp(type, INFRARED_MSG) == 0)
	{
		p_headerType = HeaderTypes::reInfaredEvent;
	}

#if DEBUG
	printDebugX10Message(type, house, unit, command, extData, extCommand, remainingBits);
#endif
}

void printX10TypeHouseUnit(const char type[], char house, byte unit, byte command)
{
	Serial.print(type);
	Serial.print(house);
	if (
		unit &&
		unit != DATA_UNKNOWN/* &&
							command != CMD_ALL_UNITS_OFF &&
							command != CMD_ALL_LIGHTS_ON &&
							command != CMD_ALL_LIGHTS_OFF &&
							command != CMD_HAIL_REQUEST*/)
	{
		Serial.print(unit - 1, HEX);
	}
	else
	{
		Serial.print(F("_"));
	}
}

void sdPrintModuleState(char house, byte unit)
{
	unit++;
	X10state state = x10ex.getModuleState(house, unit);
	if (state.isSeen)
	{
		printX10Message(
			MODULE_STATE_MSG, house, unit,
			state.isKnown ? state.isOn ? CMD_STATUS_ON : CMD_STATUS_OFF : DATA_UNKNOWN,
			state.data,
			0, 0);
	}
}

void printX10ByteAsHex(byte data)
{
	Serial.print(F("x"));
	if (data <= 0xF) { Serial.print(F("0")); }
	Serial.print(data, HEX);
}

/******************************************************************************************
Debug Helpers
******************************************************************************************/

#if DEBUG

void printDebugX10Message(const char type[], char house, byte unit, byte command, byte extData, byte extCommand, int remainingBits)
{
	Serial.print(F("DEBUG="));
	printX10TypeHouseUnit(type, house, unit, command);
	switch (command)
	{
		// This is not a real X10 command, it's a special command used by the IR
		// library to signal that an address and a house code has been received
	case 0x10:
		break;
	case CMD_ALL_UNITS_OFF:
		Serial.println(F("_AllUnitsOff"));
		break;
	case CMD_ALL_LIGHTS_ON:
		Serial.println(F("_AllLightsOn"));
		break;
	case CMD_ON:
		Serial.print(F("_On"));
		printDebugX10Brightness("_Brightness", extData);
		break;
	case CMD_OFF:
		Serial.print(F("_Off"));
		printDebugX10Brightness("_Brightness", extData);
		break;
	case CMD_DIM:
		Serial.println(F("_Dim"));
		break;
	case CMD_BRIGHT:
		Serial.println(F("_Bright"));
		break;
	case CMD_ALL_LIGHTS_OFF:
		Serial.println(F("_AllLightsOff"));
		break;
	case CMD_EXTENDED_CODE:
		Serial.print(F("_ExtendedCode"));
		break;
	case CMD_HAIL_REQUEST:
		Serial.println(F("_HailReq"));
		break;
	case CMD_HAIL_ACKNOWLEDGE:
		Serial.println(F("_HailAck"));
		break;
		// Enable X10_USE_PRE_SET_DIM in X10ex header file
		// to use X10 standard message PRE_SET_DIM commands
	case CMD_PRE_SET_DIM_0:
	case CMD_PRE_SET_DIM_1:
		printDebugX10Brightness("_PreSetDim", extData);
		break;
	case CMD_EXTENDED_DATA:
		Serial.print(F("_ExtendedData"));
		break;
	case CMD_STATUS_ON:
		Serial.println(F("_StatusOn"));
		break;
	case CMD_STATUS_OFF:
		Serial.println(F("_StatusOff"));
		break;
	case CMD_STATUS_REQUEST:
		Serial.println(F("_StatusReq"));
		break;
	case DATA_UNKNOWN:
		Serial.println(F("_Unknown"));
		break;
	default:
		Serial.println();
	}
	if (extCommand)
	{
		switch (extCommand)
		{
		case EXC_PRE_SET_DIM:
			printDebugX10Brightness("_PreSetDim", extData);
			break;
		default:
			Serial.print(F("_"));
			Serial.print(extCommand, HEX);
			Serial.print(F("_"));
			Serial.println(extData, HEX);
		}
	}
	if (remainingBits)
	{
		printX10TypeHouseUnit(type, house, unit, command);
		Serial.print(F("_ErrorBitCount="));
		Serial.println(remainingBits, DEC);
	}
}

void printDebugX10Brightness(const char source[], byte extData)
{
	if (extData > 0)
	{
		Serial.print(source);
		Serial.print(F("_"));
		Serial.println(x10ex.x10BrightnessToPercent(extData), DEC);
	}
	else
	{
		Serial.println();
	}
}

#endif


