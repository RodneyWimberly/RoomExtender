#include "Arduino.h" 
#include "X10ex.h"
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
		Serial.print("_");
	}
	Serial.println();

	// Send info via RF24L01+
	X10Data_s x10Data;
	x10Data.houseCode = house;
	x10Data.unitCode = unit;
	x10Data.commandCode = command;
	x10Data.extendedData = extData;
	x10Data.extendedCommand = extCommand;

	RfPacket* packet = new RfPacket();
	packet->Type = NotSet;
	packet->SourceAddress = 0;
	packet->DestinationAddress = 1;
	packet->Success = true;
	if (type == SERIAL_DATA_MSG)
	{
		packet->Type = reSerialDataEvent;
	}
	else if (type == POWER_LINE_MSG)
	{
		packet->Type = rePowerLineEvent;
	}
	else if (type == RADIO_FREQ_MSG)
	{
		packet->Type = reRadioFrequencyEvent;
	}
	else if (type == INFRARED_MSG)
	{
		packet->Type = reInfaredEvent;
	} 
	packet->writeBody(&x10Data, sizeof(x10Data));
	radio.sendPacket(packet);
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
		Serial.print("_");
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

void wrPrintModuleState(char house, byte unit, bool printUnseenModules)
{
	X10state state = x10ex.getModuleState(house, unit);
	X10info info = x10ex.getModuleInfo(house, unit);
	X10ModuleStatus_s moduleStatus;

	if (state.isSeen || printUnseenModules)
	{
		moduleStatus.houseCode = house;
		moduleStatus.unitCode = unit;
		
		if (info.type)
		{
			moduleStatus.type = info.type;
		}
		if (strlen(info.name))
		{
			moduleStatus.name = info.name;
		}
		moduleStatus.stateIsKnown = state.isKnown;
		if (state.isKnown)
		{
			moduleStatus.stateIsOn = state.isOn;
			if (state.data)
			{
				moduleStatus.dimPercentage = x10ex.x10BrightnessToPercent(state.data);
			}
		}
	}

	RfPacket *rfPacket = new RfPacket();
	rfPacket->Type = reX10Extended;
	rfPacket->SourceAddress = 0;
	rfPacket->DestinationAddress = 1;
	rfPacket->Success = true;
	rfPacket->writeBody(&moduleStatus, sizeof(moduleStatus));
	radio.sendPacket(rfPacket);
}

void printX10ByteAsHex(byte data)
{
	Serial.print("x");
	if (data <= 0xF) { Serial.print("0"); }
	Serial.print(data, HEX);
}

/******************************************************************************************
/* Debug Helpers
/******************************************************************************************/

#if DEBUG

void printDebugX10Message(const char type[], char house, byte unit, byte command, byte extData, byte extCommand, int remainingBits)
{
	Serial.print("DEBUG=");
	printX10TypeHouseUnit(type, house, unit, command);
	switch (command)
	{
		// This is not a real X10 command, it's a special command used by the IR
		// library to signal that an address and a house code has been received
	case 0x10:
		break;
	case CMD_ALL_UNITS_OFF:
		Serial.println("_AllUnitsOff");
		break;
	case CMD_ALL_LIGHTS_ON:
		Serial.println("_AllLightsOn");
		break;
	case CMD_ON:
		Serial.print("_On");
		printDebugX10Brightness("_Brightness", extData);
		break;
	case CMD_OFF:
		Serial.print("_Off");
		printDebugX10Brightness("_Brightness", extData);
		break;
	case CMD_DIM:
		Serial.println("_Dim");
		break;
	case CMD_BRIGHT:
		Serial.println("_Bright");
		break;
	case CMD_ALL_LIGHTS_OFF:
		Serial.println("_AllLightsOff");
		break;
	case CMD_EXTENDED_CODE:
		Serial.print("_ExtendedCode");
		break;
	case CMD_HAIL_REQUEST:
		Serial.println("_HailReq");
		break;
	case CMD_HAIL_ACKNOWLEDGE:
		Serial.println("_HailAck");
		break;
		// Enable X10_USE_PRE_SET_DIM in X10ex header file
		// to use X10 standard message PRE_SET_DIM commands
	case CMD_PRE_SET_DIM_0:
	case CMD_PRE_SET_DIM_1:
		printDebugX10Brightness("_PreSetDim", extData);
		break;
	case CMD_EXTENDED_DATA:
		Serial.print("_ExtendedData");
		break;
	case CMD_STATUS_ON:
		Serial.println("_StatusOn");
		break;
	case CMD_STATUS_OFF:
		Serial.println("_StatusOff");
		break;
	case CMD_STATUS_REQUEST:
		Serial.println("_StatusReq");
		break;
	case DATA_UNKNOWN:
		Serial.println("_Unknown");
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
			Serial.print("_");
			Serial.print(extCommand, HEX);
			Serial.print("_");
			Serial.println(extData, HEX);
		}
	}
	if (remainingBits)
	{
		printX10TypeHouseUnit(type, house, unit, command);
		Serial.print("_ErrorBitCount=");
		Serial.println(remainingBits, DEC);
	}
}

void printDebugX10Brightness(const char source[], byte extData)
{
	if (extData > 0)
	{
		Serial.print(source);
		Serial.print("_");
		Serial.println(x10ex.x10BrightnessToPercent(extData), DEC);
	}
	else
	{
		Serial.println();
	}
}

#endif

