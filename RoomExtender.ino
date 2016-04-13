#include "RoomExtender.h"

unsigned long sdReceived;
char bmHouse;
byte bmUnit;
byte bmCommand;
byte bmExtCommand; 

// X10 Power Line Communication Library
void powerLineEvent(char house, byte unit, byte command, byte extData, byte extCommand, byte remainingBits);
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

// nRF24L01+ 2.4 GHz Wireless Transceiver
void wirelessEvent(Radio* radio, RfPacket* rfPacket, uint8_t pipeNumber);
Radio radio(
	RADIO1_CE_PIN,
	RADIO1_CS_PIN,
	RADIO1_IRQ,
	&wirelessEvent);

#if ENABLE_X10_RF
// X10 RF Receiver Library
void radioFreqEvent(char house, byte unit, byte command, bool isRepeat);
X10rf x10rf(
RF_IRQ, // Receive Interrupt Number (0 = Standard Arduino External Interrupt)
RF_RECEIVER_PIN, // Receive Pin (Pin 2 must be used with interrupt 0)
&radioFreqEvent // Event triggered when RF message is received
);
#endif

#if ENABLE_X10_IR
// X10 Infrared Receiver Library
void infraredEvent(char house, byte unit, byte command, bool isRepeat);
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

// Human Input Device
HID hid(&radio);

// Configuration Register loaded from EEPROM
ConfigRegister configRegister;

void setup()
{
	// Remember to set baud rate in Serial Monitor or lower this to 9600 (default value)
	Serial.begin(115200);
	printf_begin();

	// Read the Config Register from EEPROM
	configRegister.begin();
	
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
	radio.begin(configRegister.ReAddress, ROOM_EXTENDER_PIPE_ADDRESS, HOST_PIPE_ADDRESS, HUMAN_INPUT_DEVICE_PIPE_ADDRESS, true);
	
	// Start the Human Input Device
	hid.begin(configRegister.HidAddress);

	Serial.println("X10Ex has loaded and is now monitoring for new events!");
}

void loop() 
{
	while (Serial.available())
	{
		Serial.println("lcdPrint");
		char foo[32];
		size_t l = Serial.readBytes(foo, 32);
		Serial.println(l);
		hid.lcdClear();
		hid.lcdPrint(foo, l);
	}
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

// Process messages received from X10 modules over the power line
void powerLineEvent(char house, byte unit, byte command, byte extData, byte extCommand, byte remainingBits)
{
	printX10Message(POWER_LINE_MSG, house, unit, command, extData, extCommand, remainingBits);
}

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
#endif

void printWirelessEvent(RfPacket* rfPacket, uint8_t pipeNumber, const byte* current)
{
	Serial.print(millis());
	Serial.println(": wirelessEvent Fired!");

	Serial.print("Pipe Number: ");
	Serial.println(pipeNumber);

	Serial.print("Source Address: ");
	Serial.println(rfPacket->SourceAddress);

	Serial.print("Destination Address: ");
	Serial.println(rfPacket->DestinationAddress);

	uint8_t type = rfPacket->Type;
	Serial.print("PacketType: ");
	Serial.println((PacketTypes)type);

	uint8_t length = sizeof(current);
	for (int i = 0; i < length; i++)
	{
		Serial.print("Current [");
		Serial.print(i);
		Serial.print("]: ");
		Serial.println(current[i]);
	}
}

// Process messages from the wireless transceiver 
void wirelessEvent(Radio* radio, RfPacket* rfPacket, uint8_t pipeNumber)
{
	const byte* current = reinterpret_cast<const byte*>(rfPacket->readBody());
	printWirelessEvent(rfPacket, pipeNumber, current);
	
	// Is this an ACK, if so send it back to the original sender
	if (pipeNumber == 0)
	{
		if (rfPacket->DestinationAddress == HUMAN_INPUT_DEVICE_PIPE_ADDRESS)
		{
			hid.processAck(rfPacket);
		}
	}

	if (rfPacket->Type == reX10Standard || rfPacket->Type == reX10Extended)
	{
		x10RequestEvent(radio, rfPacket);
	}
#if ENABLE_DHT
	else if (rfPacket->Type == reEnvironmentSensor)
	{
		environmentRequestEvent(radio, rfPacket);
	}
#endif
#if ENABLE_RTC
	else if (rfPacket->Type == reSetClock)
	{
		setClockRequestEvent(radio, rfPacket);
	}
	else if (rfPacket->Type == reReadClock)
	{
		readClockRequestEvent(radio, rfPacket);
	}
#endif
}

// Process X10 commands received from RF24L01+
void x10RequestEvent(Radio* radio, RfPacket* rfPacket)
{
	Serial.println("x10RequestEvent");

	const byte* current = reinterpret_cast<const byte*>(rfPacket->readBody());
	X10Data_s x10Data;
	x10Data.houseCode = current[1];
	x10Data.unitCode = current[2];
	x10Data.commandCode = current[3];
	//x10Data.extendedCommand = current[5];
	//x10Data.extendedData = current[8];

	rfPacket->Success = !process3BMessage(WIRELESS_DATA_MSG, x10Data.houseCode, x10Data.unitCode, x10Data.commandCode);
	if(!rfPacket->Success)
	{
		Serial.print(WIRELESS_DATA_MSG);
		Serial.println(MSG_BUFFER_ERROR);
	}
	radio->sendAckPacket(rfPacket);
	return;
}

#if ENABLE_DHT
// Process environment request (Temperature/Humidity) received from RF24L01+
void environmentRequestEvent(Radio* radio, RfPacket* rfPacket)
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
	rfPacket->Success = !(isnan(data.humidity) || isnan(data.temperatureC) || isnan(data.temperatureF));
	if (rfPacket->Success)
	{
		// Compute heat index in Fahrenheit (the default)
		data.heatIndexH = dht.computeHeatIndex(data.temperatureF, data.humidity);
		// Compute heat index in Celsius (isFahreheit = false)
		data.heatIndexC = dht.computeHeatIndex(data.temperatureC, data.humidity, false);
	}
	
	rfPacket->writeBody(&data, sizeof(data));
	radio->sendAckPacket(rfPacket);
}
#endif

#if ENABLE_RTC
// Process set clock request received from RF24L01+
void setClockRequestEvent(Radio* radio, RfPacket* rfPacket)
{
	Serial.println("setClockRequestEvent");
	ClockData_s data;
	byte* current = reinterpret_cast<byte*>(rfPacket->readBody());
	memcpy(&data, current, rfPacket->BodyLength);

	rtc.setDOW(data.time.dow);
	rtc.setTime(data.time.hour, data.time.min, data.time.sec);
	rtc.setDate(data.time.date, data.time.mon, data.time.year);
	rfPacket->Success = true;
	radio->sendAckPacket(rfPacket);
}

// Process read clock request received from RF24L01+
void readClockRequestEvent(Radio* radio, RfPacket* rfPacket)
{
	Serial.println("readClockRequestEvent");
	ClockData_s data;
	data.time = rtc.getTime();

	rfPacket->Success = true;
	rfPacket->writeBody(&data, sizeof(data));
	radio->sendAckPacket(rfPacket);
}
#endif

