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

#include "RoomExtender.h"
#include "Arduino.h" 
#include <SPI.h>
#include "EEPROMex.h"
#include "EEPROMvar.h"
#include "IPAddress.h"
#include "Radio.h"
#include "X10ex.h"
#if ENABLE_X10_IR
#include "X10ir.h"
#endif
#if ENABLE_X10_RF
#include "X10rf.h"
#endif
#if ENABLE_DHT
#include "DHT.h"
#endif
#if ENABLE_RTC
#include "DS3231.h"
#endif

// Fields used for byte message reception
unsigned long sdReceived;
char bmHouse;
byte bmUnit;
byte bmCommand;
byte bmExtCommand; 
const uint64_t addresses[2] = { 0xABCDABCD71LL, 0x544d52687CLL };
//const IPAddress addresses[2] = { IPAddress(1, 1, 1, 1), IPAddress(1, 1, 2, 1) };
byte data[32];

// Process messages received from X10 modules over the power line
void powerLineEvent(char house, byte unit, byte command, byte extData, byte extCommand, byte remainingBits)
{
  printX10Message(POWER_LINE_MSG, house, unit, command, extData, extCommand, remainingBits);
}

// X10 Power Line Communication Library
X10ex x10ex(
	X10_IRQ,			// Zero Cross Interrupt Number (2 = "Custom" Pin Change Interrupt)
	X10_RTS_PIN,		// Zero Cross Interrupt Pin (Pin 4-7 can be used with interrupt 2)
	X10_TX_PIN,			// Power Line Transmit Pin 
	X10_RX_PIN,			// Power Line Receive Pin
	true,				// Enable this to see echo of what is transmitted on the power line
	&powerLineEvent,	// Event triggered when power line message is received
	1,					// Number of phases (1 = No Phase Repeat/Coupling)
	60					// The power line AC frequency (e.g. 50Hz in Europe, 60Hz in North America)
	);

// Process messages from the wireless transceiver 
void wirelessEvent(Radio* radio, wrPacketTypes wrPacketType, const void* buffer, uint8_t length)
{
	const byte* current = reinterpret_cast<const byte*>(buffer);
	Serial.println(millis());
	Serial.print("Length: ");
	Serial.println(length);
	Serial.print("0: ");
	Serial.println(current[0]);
	Serial.print("1: ");
	Serial.println(current[1]);
	Serial.print("2: ");
	Serial.println(current[2]);
	Serial.print("3: ");
	Serial.println(current[3]);
	Serial.print("4: ");
	Serial.println(current[4]);
	Serial.print("PacketType: ");
	Serial.println(wrPacketType);
	if (wrPacketType == X10StandardRequest)
	{
		X10Data_s x10Data;
		x10Data.houseCode = current[1];
		x10Data.unitCode = current[2];
		x10Data.commandCode = current[3];

		x10RequestEvent(radio, x10Data);
	}
	else if (wrPacketType == X10ExtendedRequest)
	{
		X10Data_s x10Data;
		x10Data.houseCode = current[1];
		x10Data.unitCode = current[2];
		x10Data.commandCode = current[3];
		x10Data.extendedCommand = current[5];
		x10Data.extendedData = current[8];
		x10RequestEvent(radio, x10Data);
	}
#if ENABLE_DHT
	else if (wrPacketType == EnvironmentSensorRequest)
	{
		environmentRequestEvent(radio);
	}
#endif
#if ENABLE_RTC
	else if (wrPacketType == SetClockRequest)
	{
		ClockData_s data;
		memcpy(&data, current, sizeof(data));
		setClockRequestEvent(radio, data);
	}
	else if (wrPacketType == ReadClockRequest)
	{
		readClockRequestEvent(radio);
	}
#endif
}

// nRF24L01+ 2.4 GHz Wireless Transceiver
Radio radio(
	RADIO1_CE_PIN,
	RADIO1_CS_PIN,
	RADIO1_IRQ,
	&wirelessEvent); // 68

#if ENABLE_X10_RF
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

// X10 RF Receiver Library
X10rf x10rf(
RF_IRQ, // Receive Interrupt Number (0 = Standard Arduino External Interrupt)
RF_RECEIVER_PIN, // Receive Pin (Pin 2 must be used with interrupt 0)
&radioFreqEvent // Event triggered when RF message is received
);
#endif

#if ENABLE_X10_IR

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

// X10 Infrared Receiver Library
X10ir x10ir(
IR_IRQ, // Receive Interrupt Number (1 = Standard Arduino External Interrupt)
IR_RECEIVER_PIN, // Receive Pin (Pin 3 must be used with interrupt 1)
'A', // Default House Code
&infraredEvent // Event triggered when IR message is received
);
#endif

#if ENABLE_DHT
// DHT-11 Digital Humidity and Temperature Sensor
DHT dht(DHT_PIN, DHT_TYPE);
#endif

#if ENABLE_RTC
// DS3231 Real Time Clock
DS3231 rtc(SDA, SCL);
#endif

void setup()
{
	// Remember to set baud rate in Serial Monitor or lower this to 9600 (default value)
	Serial.begin(115200);

	// start reading from position memBase (address 0) of the EEPROM. Set maximumSize to EEPROMSizeUno 
	// Writes before membase or beyond EEPROMSizeUno will only give errors when _EEPROMEX_DEBUG is set
	EEPROM.setMemPool(memBase, EEPROMSizeUno);

	// Set maximum allowed writes to maxAllowedWrites. 
	// More writes will only give errors when _EEPROMEX_DEBUG is set
	EEPROM.setMaxAllowedWrites(maxAllowedWrites);
	//EEPROMVar<float> eepromFloat(5.5);  // initial value 5.5
	//eepromFloat.save();     // store EEPROMVar to EEPROM
	//eepromFloat = 0.0;      // reset 
	//eepromFloat.restore();  // restore EEPROMVar to EEPROM

	// Start the x10 PLC, RF and IR libraries
	x10ex.begin();
#if ENABLE_X10_RF
	x10rf.begin();
#endif
#if ENABLE_X10_IR
	x10ir.begin();
#endif
#if ENABLE_DHT
	// Start Temperature / Humidity Sensor
	dht.begin();
#endif
#if ENABLE_RTC
	// Start Real-Time Clock interface
	rtc.begin();
#endif
	// Start the nRF24L01+ library
	radio.begin(addresses[1], addresses[0], true);

	Serial.println("X10Ex has loaded and is now monitoring for new events!");
}

/******************************************************************************************
/* Process Loop (Not used, this sketch is completely event driven)
/******************************************************************************************/
void loop() { }

/******************************************************************************************
/* Event Handlers
/******************************************************************************************/
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

// Process X10 commands received from RF24L01+
void x10RequestEvent(Radio* radio, X10Data_s data)
{
	Serial.println("x10RequestEvent");
	if(process3BMessage(WIRELESS_DATA_MSG, data.houseCode, data.unitCode, data.commandCode))
	{
		Serial.print(WIRELESS_DATA_MSG);
		Serial.println(MSG_BUFFER_ERROR);
	}
	return;
}

#if ENABLE_DHT
// Process environment request (Temperature/Humidity) received from RF24L01+
void environmentRequestEvent(Radio* radio)
{
	Serial.println("environmentRequestEvent");
	EnvironmentData_s data;
	// Reading temperature or humidity takes about 250 milliseconds!
	// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
	data.humidity = dht.readHumidity();
	// Read temperature as Celsius (the default)
	data.temperatureC = dht.readTemperature();
	// Read temperature as Fahrenheit (isFahrenheit = true)
	data.temperatureF = dht.readTemperature(true);
	// Check if any reads failed and exit early (to try again).
	if (isnan(data.humidity) || isnan(data.temperatureC) || isnan(data.temperatureF)) {
		//wirelessError("Failed to read from DHT sensor!");
		return;
	}
	// Compute heat index in Fahrenheit (the default)
	data.heatIndexH = dht.computeHeatIndex(data.temperatureF, data.humidity);
	// Compute heat index in Celsius (isFahreheit = false)
	data.heatIndexC = dht.computeHeatIndex(data.temperatureC, data.humidity, false);
	
	size_t size = sizeof(data);
	char buffer[size];
	memcpy(buffer, &data, size);
	radio->sendPacket(EnvironmentSensorResponse, buffer, size);
}
#endif

#if ENABLE_RTC
// Process set clock request received from RF24L01+
void setClockRequestEvent(Radio* radio, ClockData_s data)
{
	Serial.println("setClockRequestEvent");
	rtc.setDOW(data.time.dow);
	rtc.setTime(data.time.hour, data.time.min, data.time.sec);
	rtc.setDate(data.time.date, data.time.mon, data.time.year);
}

// Process read clock request received from RF24L01+
void readClockRequestEvent(Radio* radio)
{
	Serial.println("readClockRequestEvent"); 
	ClockData_s data;
	data.time = rtc.getTime();

	size_t size = sizeof(data);
	char buffer[size];
	memcpy(buffer, &data, size);
	radio->sendPacket(ReadClockResponse, buffer, size);
}
#endif

