/****************************************************************************/
/* X10 PLC, RF, IR library w/ nRF24L01+ 2.4 GHz Wireless Transceiver Support*/
/* I have only tested this on the PSC05 X10 PowerLine Interface (PLC).		*/
/****************************************************************************/
/* This library is free software: you can redistribute it and/or modify		*/
/* it under the terms of the GNU General Public License as published by		*/
/* the Free Software Foundation, either version 3 of the License, or		*/                            
/* (at your option) any later version.										*/
/*																			*/
/* This library is distributed in the hope that it will be useful, but		*/
/* WITHOUT ANY WARRANTY; without even the implied warranty of				*/
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU			*/
/* General Public License for more details.									*/
/*																			*/
/* You should have received a copy of the GNU General Public License		*/
/* along with this library. If not, see <http://www.gnu.org/licenses/>.		*/
/*																			*/
/* Written by Thomas Mittet (code@lookout.no) November 2010.				*/
/* Updated by Rodney Wimbery (rodney.wimberly@gmail.com) February 2016		*/
/****************************************************************************/


#include "Arduino.h" 
#include <SPI.h>
#include <Wire.h>
#include "X10Wireless.h"
#include "X10ex.h"
#include "X10ir.h"
#include "X10rf.h"
#include "DHT.h"  
#include "DS3231.h"
#include "X10ExSketch.h"


// Fields used for byte message reception
unsigned long sdReceived;
char bmHouse;
byte bmUnit;
byte bmCommand;
byte bmExtCommand;
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0x544d52687CLL };
byte data[32];

// DHT-11 Digital Humidity and Temperature Sensor
DHT dht(DHT_PIN, DHT21);

// DS3231 Real Time Clock
DS3231 rtc(SDA, SCL);

// Process messages received from X10 modules over the power line
void powerLineEvent(char house, byte unit, byte command, byte extData, byte extCommand, byte remainingBits)
{
  printX10Message(POWER_LINE_MSG, house, unit, command, extData, extCommand, remainingBits);
  byte buffer[5];
}

// X10 Power Line Communication Library
X10ex x10ex(
  X10_IRQ,			// Zero Cross Interrupt Number (2 = "Custom" Pin Change Interrupt)
  X10_RTS_PIN,		// Zero Cross Interrupt Pin (Pin 4-7 can be used with interrupt 2)
  X10_TX_PIN,		// Power Line Transmit Pin 
  X10_RX_PIN,		// Power Line Receive Pin
  true,				// Enable this to see echo of what is transmitted on the power line
  &powerLineEvent,	// Event triggered when power line message is received
  1,				// Number of phases (1 = No Phase Repeat/Coupling)
  60				// The power line AC frequency (e.g. 50Hz in Europe, 60Hz in North America)
  );

// Process X10 commands received from RF24L01+
void x10RequestEvent(X10Wireless* x10Wireless, char house, byte unit, byte command)
{
	Serial.println("x10RequestEvent");
	printX10Message(WIRELESS_DATA_MSG, house, unit, command, 0, 0, 0);
	// Check if command is handled by scenario; if not continue
	if (!handleUnitScenario(house, unit, command, false, false))
	{
		// Make sure that two or more repetitions are used for bright and dim,
		// to avoid that commands are being buffered separately when repeated
		if (command == CMD_BRIGHT || command == CMD_DIM)
		{
			x10ex.sendCmd(house, unit, command, 2);
		}
		// Other commands map directly: just forward to PL interface
		else
		{
			x10ex.sendCmd(house, unit, command, 1);
		}
	}
}

// Process environment request (Temperature/Humidity) received from RF24L01+
void environmentRequestEvent(X10Wireless* x10Wireless)
{
	// Reading temperature or humidity takes about 250 milliseconds!
	// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
	float h = dht.readHumidity();
	// Read temperature as Celsius (the default)
	float t = dht.readTemperature();
	// Read temperature as Fahrenheit (isFahrenheit = true)
	float f = dht.readTemperature(true);

	// Check if any reads failed and exit early (to try again).
	if (isnan(h) || isnan(t) || isnan(f)) {
		//wirelessError("Failed to read from DHT sensor!");
		return;
	}

	// Compute heat index in Fahrenheit (the default)
	float hif = dht.computeHeatIndex(f, h);
	// Compute heat index in Celsius (isFahreheit = false)
	float hic = dht.computeHeatIndex(t, h, false);
	byte buffer[5];
	x10Wireless->sendPacket(EnvironmentSensorResponse, buffer, 5);
}

// Process set clock request received from RF24L01+
void setClockRequestEvent(X10Wireless* x10Wireless, uint8_t dow, uint8_t day, uint8_t month, uint8_t year, uint8_t hour, uint8_t minute, uint8_t seconds)
{
	rtc.setDOW(dow);
	rtc.setTime(hour, minute, seconds);
	rtc.setDate(day, month, year);
}

// Process read clock request received from RF24L01+
void readClockRequestEvent(X10Wireless* x10Wireless)
{
	rtc.getDOWStr();
	rtc.getDateStr();
	rtc.getTimeStr();
	byte buffer[7];
	x10Wireless->sendPacket(ReadClockResponse, buffer, 7);
}

// nRF24L01+ 2.4 GHz Wireless Transceiver
X10Wireless radio(
	RADIO1_CE_PIN, 
	RADIO1_CS_PIN, 
	&x10RequestEvent,
	&environmentRequestEvent, 
	&setClockRequestEvent, 
	&readClockRequestEvent); // 68

/*
// Process commands received from X10 compatible RF remote
void radioFreqEvent(char house, byte unit, byte command, bool isRepeat)
{
  if (!isRepeat) printX10Message(RADIO_FREQ_MSG, house, unit, command, 0, 0, 0);
  // Check if command is handled by scenario; if not continue
  if (!handleUnitScenario(house, unit, command, isRepeat, false))
  {
	// Make sure that two or more repetitions are used for bright and dim,
	// to avoid that commands are being buffered separately when repeated
	if (command == CMD_BRIGHT || command == CMD_DIM)
	{
	  x10ex.sendCmd(house, unit, command, 2);
	}
	// Other commands map directly: just forward to PL interface
	else if (!isRepeat)
	{
	  x10ex.sendCmd(house, unit, command, 1);
	}
  }
}

// Process commands received from X10 compatible IR remote
void infraredEvent(char house, byte unit, byte command, bool isRepeat)
{
  if (!isRepeat) printX10Message(INFRARED_MSG, house, unit, command, 0, 0, 0);
  // Check if command is handled by scenario; if not continue
  if (!handleUnitScenario(house, unit, command, isRepeat, false))
  {
	// Make sure that two or more repetitions are used for bright and dim,
	// to avoid that commands are being buffered separately when repeated
	if (command == CMD_BRIGHT || command == CMD_DIM)
	{
	  x10ex.sendCmd(house, unit, command, 2);
	}
	// Only repeat bright and dim commands
	else if (!isRepeat)
	{
	  // Handle Address Command (House + Unit)
	  if (command == CMD_ADDRESS)
	  {
		x10ex.sendAddress(house, unit, 1);
	  }
	  // Other commands map directly: just forward to PL interface
	  else
	  {
		x10ex.sendCmd(house, unit, command, 1);
	  }
	}
  }
}

// X10 RF Receiver Library
X10rf x10rf(
	RF_IRQ, // Receive Interrupt Number (0 = Standard Arduino External Interrupt)
	RF_RECEIVER_PIN, // Receive Pin (Pin 2 must be used with interrupt 0)
	&radioFreqEvent // Event triggered when RF message is received
	);

// X10 Infrared Receiver Library
X10ir x10ir(
	IR_IRQ, // Receive Interrupt Number (1 = Standard Arduino External Interrupt)
	IR_RECEIVER_PIN, // Receive Pin (Pin 3 must be used with interrupt 1)
	'A', // Default House Code
	&infraredEvent // Event triggered when IR message is received
	);

*/
void setup()
{
	// Remember to set baud rate in Serial Monitor or lower this to 9600 (default value)
	Serial.begin(115200);

	// Start the x10 PLC, RF and IR libraries
	x10ex.begin();
	//x10rf.begin();
	//x10ir.begin();

	// Start Temperature / Humidity Sensor
	dht.begin();

	// Start Real-Time Clock interface
	rtc.begin();

	// Start the nRF24L01+ library
	radio.begin(pipes[1], pipes[0]);

	Serial.println("X10Ex has loaded and is now monitoring for new events!");
	const char* buffer = { "X10Ex has loaded!" };
	//sendInstructionToHid(InstructionLcdPrintLine, buffer, 1);                                      
}

void loop()
{ 
	if (!Serial.available()) 
		radio.receivePacket();
	/*Time t;
	fireEventsBasedOnTime(t);*/
}


void serialEvent()
{
	// Read 3 bytes from serial buffer
	if (Serial.available() >= 3)
	{
		byte byte1 = toupper(Serial.read());
		byte byte2 = toupper(Serial.read());
		byte byte3 = toupper(Serial.read());
		if (process3BMessage(SERIAL_DATA_MSG, byte1, byte2, byte3))
		{
			// Return error message if message sent to X10ex was not buffered successfully
			Serial.print(SERIAL_DATA_MSG);
			Serial.println(MSG_BUFFER_ERROR);
		}
		sdReceived = 0;
	}
	// If partial message was received
	if (Serial.available() && Serial.available() < 3)
	{
		// Store received time
		if (!sdReceived)
		{
			sdReceived = millis();
		}
		// Clear message if all bytes were not received within threshold
		else if (sdReceived > millis() || (millis() - sdReceived) > SERIAL_DATA_THRESHOLD)
		{
			bmHouse = 0;
			bmExtCommand = 0;
			sdReceived = 0;
			Serial.print(SERIAL_DATA_MSG);
			Serial.println(MSG_RECEIVE_TIMEOUT);
	  
			// Clear serial input buffer
			while (Serial.read() != -1);
		}
	}
}

void fireEventsBasedOnTime()
{

}

/*
void sendInstructionToHid(InstructionTypes instructionType, const char* buffer, int length)
{
	byte sendBuffer[length + 1];
	sendBuffer[0] = instructionType;
	memcpy(sendBuffer + 1, buffer, sizeof(byte) * (length));

	Wire.beginTransmission(HID_I2C_ADDRESS);
	Wire.write(sendBuffer, length);
	Wire.endTransmission();
}
*/
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
	//byte* buffer[5];
	data[0] = house;
	data[1] = unit;
	data[2] = command;
	data[3] = extData;
	data[4] = extCommand;
	wrPacketTypes packetType = NotSet;
	if (type == POWER_LINE_MSG)
	{
		packetType = X10EventNotification;
	}
	else if (type == RADIO_FREQ_MSG)
	{
		packetType = RfEventNotification;
	}
	else if (type == INFRARED_MSG)
	{
		packetType = IrEventNotification;
	}
	Serial.println("Sending Radio Packet");
	radio.sendPacket(packetType, &data, 5);
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

bool wrPrintModuleState(char house, byte unit, bool printUnseenModules)
{
	//X10state state = x10ex.getModuleState(house, unit);
	//X10info info = x10ex.getModuleInfo(house, unit);
	//byte buffer[7];
	//if (state.isSeen || printUnseenModules)
	//{
	//	buffer[0] = house;
	//	buffer[1] = unit;
	//	
	//	if (info.type)
	//	{
	//		buffer[3] = info.type;
	//	}
	//	if (strlen(info.name))
	//	{
	//		//buffer[4] = (byte)info.name;
	//	}
	//	buffer[5] = state.isKnown;
	//	if (state.isKnown)
	//	{
	//		buffer[6] = state.isOn;
	//		if (state.data)
	//		{
	//			buffer[7] = x10ex.x10BrightnessToPercent(state.data);
	//		}
	//	}
	//}
	//return radio.sendPacket(X10EventNotification, buffer, 7);
	return true;
}


void printX10ByteAsHex(byte data)
{
	Serial.print("x");
	if (data <= 0xF) { Serial.print("0"); }
	Serial.print(data, HEX);
}

byte charHexToDecimal(byte input)
{
	// 0123456789  =>  0-15
	if (input >= 0x30 && input <= 0x39) input -= 0x30;
	// ABCDEF  =>  10-15
	else if (input >= 0x41 && input <= 0x46) input -= 0x37;
	// Return converted byte
	return input;
}

byte stringToDecimal(const char input[], byte startPos, byte endPos)
{
	byte decimal = 0;
	byte multiplier = 1;
	for (byte i = endPos + 1; i > startPos; i--)
	{
		if (input[i - 1] >= 0x30 && input[i - 1] <= 0x39)
		{
			decimal += (input[i - 1] - 0x30) * multiplier;
			multiplier *= 10;
		}
		else if (multiplier > 1)
		{
			break;
		}
	}
	return decimal;
}

short stringIndexOf(const char string[], char find, byte startPos, byte endPos, short notFoundValue)
{
	char *ixStr = strchr(string + startPos, find);
	return
		ixStr - string < 0 || (endPos > 0 && ixStr - string > endPos) ?
		notFoundValue : ixStr - string;
}

// Handles scenario execute commands received as serial data message
void handleSdScenario(byte scenario)
{
	switch (scenario)
	{
		//////////////////////////////
		// Sample Code
		// Replace with your own setup
		//////////////////////////////
	case 0x01: sendMasterBedroomOn(); break;
	case 0x02: sendHallAndKitchenOn(); break;
	case 0x03: sendLivingRoomOn(); break;
	case 0x04: sendLivingRoomTvScenario(); break;
	case 0x05: sendLivingRoomMovieScenario(); break;
	case 0x11: sendAllLightsOff(); break;
	case 0x12: sendHallAndKitchenOff(); break;
	case 0x13: sendLivingRoomOff(); break;
	}
}

// Handles scenarios triggered when receiving unit on/off commands
// Use this method to trigger scenarios from simple RF/IR remotes
bool handleUnitScenario(char house, byte unit, byte command, bool isRepeat, bool failOnBufferError)
{
	//////////////////////////////
	// Sample Code
	// Replace with your own setup
	//////////////////////////////

	bool bufferError = 0;

	// Ignore all house codes except A
	if (house != 'A') return 0;

	// Unit 4 is an old X10 lamp module that doesn't remember state on its own. Use the
	// following method to revert to buffered state when these modules are turned on
	if (unit == 4)
	{
		bufferError = handleOldLampModuleState(house, unit, command, isRepeat);
	}
	// Unit 5 is a placeholder unit code used by RF and IR remotes that triggers HallAndKitchen scenario
	else if (unit == 5)
	{
		if (!isRepeat)
		{
			if (command == CMD_ON) bufferError = sendHallAndKitchenOn();
			else if (command == CMD_OFF) bufferError = sendHallAndKitchenOff();
		}
	}
	// Unit 10 is a placeholder unit code used by RF and IR remotes that triggers AllLights scenario
	else if (unit == 10)
	{
		if (!isRepeat)
		{
			if (command == CMD_ON) bufferError = sendAllLightsOn();
			else if (command == CMD_OFF) bufferError = sendAllLightsOff();
		}
	}
	// Unit 11 is a placeholder unit code used by RF and IR remotes that triggers LivingRoom scenario
	else if (unit == 11)
	{
		if (!isRepeat)
		{
			if (command == CMD_ON) bufferError = sendLivingRoomOn();
			else if (command == CMD_OFF) bufferError = sendLivingRoomOff();
		}
	}
	// Unit 12 is a placeholder unit code used by RF and IR remotes that triggers LivingRoomTv scenario
	else if (unit == 12)
	{
		if (!isRepeat)
		{
			if (command == CMD_ON) bufferError = sendLivingRoomTvScenario();
			else if (command == CMD_OFF) bufferError = sendLivingRoomOn();
		}
	}
	// Unit 13 is a placeholder unit code used by RF and IR remotes that triggers LivingRoomMovie scenario
	else if (unit == 13)
	{
		if (!isRepeat)
		{
			if (command == CMD_ON) bufferError = sendLivingRoomMovieScenario();
			else if (command == CMD_OFF) bufferError = sendLivingRoomOn();
		}
	}
	else
	{
		return 0;
	}
	return !failOnBufferError || !bufferError;
}

// Handles old X10 lamp modules that don't remember state on their own
bool handleOldLampModuleState(char house, byte unit, byte command, bool isRepeat)
{
	if (!isRepeat)
	{
		bool bufferError = 0;
		if (command == CMD_ON)
		{
			// Get state from buffer, and set to last brightness when turning on
			X10state state = x10ex.getModuleState(house, unit);
			if (state.isKnown && !state.isOn && state.data > 0)
			{
				bufferError = x10ex.sendExt(house, unit, CMD_EXTENDED_CODE, state.data, EXC_PRE_SET_DIM, 1);
			}
		}
		return bufferError || x10ex.sendCmd(house, unit, command, 1);
	}
	return 0;
}

//////////////////////////////
// Scenarios
// Replace with your own setup
//////////////////////////////

bool sendAllLightsOn()
{
	return
		// Bedroom
		x10ex.sendExtDim('A', 7, 80, EXC_DIM_TIME_4, 1) ||
		// Dining Table
		x10ex.sendExtDim('A', 2, 70, EXC_DIM_TIME_4, 1) ||
		// Hall
		x10ex.sendExtDim('A', 8, 75, EXC_DIM_TIME_4, 1) ||
		// Couch
		x10ex.sendExtDim('A', 3, 90, EXC_DIM_TIME_4, 1) ||
		// Kitchen
		x10ex.sendExtDim('A', 9, 100, EXC_DIM_TIME_4, 1) ||
		// TV Backlight
		x10ex.sendExtDim('A', 4, 40, EXC_DIM_TIME_4, 1);
}

bool sendAllLightsOff()
{
	return
		x10ex.sendCmd('A', 7, CMD_OFF, 1) ||
		x10ex.sendCmd('A', 2, CMD_OFF, 1) ||
		x10ex.sendCmd('A', 8, CMD_OFF, 1) ||
		x10ex.sendCmd('A', 3, CMD_OFF, 1) ||
		x10ex.sendCmd('A', 9, CMD_OFF, 1) ||
		x10ex.sendCmd('A', 4, CMD_OFF, 1);
}

bool sendHallAndKitchenOn()
{
	return
		x10ex.sendExtDim('A', 8, 75, EXC_DIM_TIME_4, 1) ||
		x10ex.sendExtDim('A', 9, 100, EXC_DIM_TIME_4, 1);
}

bool sendHallAndKitchenOff()
{
	return
		x10ex.sendCmd('A', 8, CMD_OFF, 1) ||
		x10ex.sendCmd('A', 9, CMD_OFF, 1);
}

bool sendLivingRoomOn()
{
	return
		x10ex.sendExtDim('A', 2, 70, EXC_DIM_TIME_4, 1) ||
		x10ex.sendExtDim('A', 3, 90, EXC_DIM_TIME_4, 1) ||
		x10ex.sendExtDim('A', 4, 40, EXC_DIM_TIME_4, 1);
}

bool sendLivingRoomOff()
{
	return
		x10ex.sendCmd('A', 2, CMD_OFF, 1) ||
		x10ex.sendCmd('A', 3, CMD_OFF, 1) ||
		x10ex.sendCmd('A', 4, CMD_OFF, 1);
}

bool sendLivingRoomTvScenario()
{
	return
		x10ex.sendExtDim('A', 2, 40, EXC_DIM_TIME_4, 1) ||
		x10ex.sendExtDim('A', 3, 30, EXC_DIM_TIME_4, 1) ||
		x10ex.sendExtDim('A', 4, 25, EXC_DIM_TIME_4, 1);
}

bool sendMasterBedroomOn()
{
	return
		x10ex.sendCmd('A', 0, CMD_ON, 1) ||
		x10ex.sendCmd('A', 1, CMD_ON, 1);
}

bool sendLivingRoomMovieScenario()
{
	return
		x10ex.sendCmd('A', 2, CMD_OFF, 1) ||
		x10ex.sendCmd('A', 3, CMD_OFF, 1) ||
		x10ex.sendExtDim('A', 4, 25, EXC_DIM_TIME_4, 1);
}

#if DEBUG

void printDebugX10Message(const char type[], char house, byte unit, byte command, byte extData, byte extCommand, int remainingBits)
{
	Serial.print("DEBUG=");
	printX10TypeHouseUnit(type, house, unit, command);
	switch (command)
	{
		// This is not a real X10 command, it's a special command used by the IR
		// library to signal that an address and a house code has been received
	case CMD_ADDRESS:
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
