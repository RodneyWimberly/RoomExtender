#include <DfrLcdKeypad.h>
#include "RoomExtender.h"

// Local variables used by the X10 Libraries
unsigned long p_sdReceived;
X10Data_s p_x10Data;
HeaderTypes p_headerType = HeaderTypes::notSet;

// Local variable to track the elapsed time
// This is used each loop to determine when the proper timeout has occurred 
// When the timeout occurs we will process certain events and reset the elapsed time.
elapsedMillis p_elapsedTimer;

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

// nRF24L01+ 2.4 GHz Wireless Transceiver
RF24 radio(RADIO1_CE_PIN, RADIO1_CS_PIN);

// Add OSI layer 3 network to the nRF24L01+ radio
RF24Network network(radio);

#if ENABLE_X10_RF
// X10 RF Receiver Library
X10rf x10rf(
	RF_IRQ, // Receive Interrupt Number (0 = Standard Arduino External Interrupt)
	RF_RECEIVER_PIN, // Receive Pin (Pin 2 must be used with interrupt 0)
	&radioFreqEvent // Event triggered when RF message is received
	);
#endif

#if ENABLE_X10_IR
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

#if ENABLE_HID
// Human Input Device
HidClient hidClient(&network);
#endif

// Configuration Register loaded from EEPROM
//ConfigRegister configRegister;

void setup()
{
	// Setup to allow printf (etc) to work when called from any libraries
	printf_begin();

	// Remember to set baud rate in Serial Monitor or lower this to 9600 (default value)
	Serial.begin(115200);

	// Read the Config Register from EEPROM
	//configRegister.begin();

	// Start the x10 PLC Library
	x10ex.begin();

#if ENABLE_X10_RF
	// Start the X10 RF library
	x10rf.begin();
#endif

#if ENABLE_X10_IR
	// Start the X10 IR library
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
	SPI.begin();
	radio.begin();
	radio.setPALevel(RF24_PA_HIGH);
	radio.setDataRate(RF24_250KBPS);
	network.begin(RADIO_CHANNEL, RE_RADIO_NODE_ID);

#if ENABLE_HID
	// Start the Human Input Device
	hidClient.begin(RE_RADIO_NODE_ID, HID_RADIO_NODE_ID);
#endif

	// Set time so it will immediately expire and we won't have to wait for the first update.
	p_elapsedTimer = POLL_INTERVAL_DISPLAY;

	Serial.println(F("X10Ex has loaded and is now monitoring for new events!"));
}

void loop()
{
	network.update();
	while (network.available())
		processIncommingRequests();
	processOutgoingRequests();
}

void processIncommingRequests()
{
	RF24NetworkHeader header;
	uint16_t length = network.peek(header);
	IF_SERIAL_DEBUG(printf_P(PSTR("%lu: APP Received request from 0%o type %d\n\r"), millis(), header.from_node, header.type));
	switch ((HeaderTypes)header.type)
	{
	case HeaderTypes::reX10Standard:
		x10RequestEvent(header, length);
		break;
	case HeaderTypes::reX10ModuleStatus:
		x10RequestEvent(header, length);
		break;
	case HeaderTypes::reEnvironmentSensor:
		environmentRequestEvent(header, length);
		break;
	case HeaderTypes::reSetClock:
		setClockRequestEvent(header, length);
		break;
	case HeaderTypes::reReadClock:
		readClockRequestEvent(header, length);
		break;
	default:
		Serial.print("Invalid type,");
		Serial.print(header.type);
		Serial.println(" throwing away this message");
		network.read(header, 0, 0);
		break;
	}
}

void processOutgoingRequests()
{
#if ENABLE_HID
	if (p_headerType != HeaderTypes::notSet)
	{
		RF24NetworkHeader header(HID_RADIO_NODE_ID, p_headerType);
		network.multicast(header, &p_x10Data, sizeof(X10Data_s), 1);
		p_headerType = HeaderTypes::notSet;
	}
	if (p_elapsedTimer > POLL_INTERVAL_DISPLAY)
	{
		p_elapsedTimer = 0;
		displayTime();
		displayTemperature();
	}
	KeyPadButtons key = KeyPadButtons::None;
	key = hidClient.waitForKey(15);
	if (key != KeyPadButtons::None)
	{
		if (key == KeyPadButtons::Down)
			process3BMessage(WIRELESS_DATA_MSG, 'M', 0, 3);
		else if (key == KeyPadButtons::Up)
			process3BMessage(WIRELESS_DATA_MSG, 'M', 0, 2);
	}
#endif
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
			Serial.print(F(SERIAL_DATA_MSG));
			Serial.println(F(MSG_BUFFER_ERROR));
		}
		p_sdReceived = 0;
	}
	// If partial message was received
	if (Serial.available() && Serial.available() < 3)
	{
		// Store received time
		if (!p_sdReceived)
		{
			p_sdReceived = millis();
		}
		// Clear message if all bytes were not received within threshold
		else if (p_sdReceived > millis() || (millis() - p_sdReceived) > SERIAL_DATA_THRESHOLD)
		{
			p_sdReceived = 0;
			Serial.print(F(SERIAL_DATA_MSG));
			Serial.println(F(MSG_RECEIVE_TIMEOUT));

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

void writeToNetwork(uint16_t fromNode, unsigned char type, const void* message, uint16_t length)
{
	RF24NetworkHeader header = RF24NetworkHeader(fromNode, type);
	network.write(header, &message, length);
}

// Process X10 commands received from RF24L01+
void x10RequestEvent(RF24NetworkHeader& header, uint16_t length)
{
	Serial.println(F("x10RequestEvent"));

	X10Data_s data;
	network.read(header, &data, sizeof(data));

	data.success = !process3BMessage(WIRELESS_DATA_MSG, data.houseCode, data.unitCode, data.commandCode);
	if (!data.success)
	{
		Serial.print(F(WIRELESS_DATA_MSG));
		Serial.println(F(MSG_BUFFER_ERROR));
	}

	writeToNetwork(header.from_node, header.type, &data, sizeof(data));
}

// Get the X10 module status and send back via RF24L01+
void x10ModuleState(RF24NetworkHeader& header, uint16_t length)
{
	Serial.println(F("x10ModuleState"));

	X10Data_s data;
	network.read(header, &data, sizeof(data));

	X10state state = x10ex.getModuleState(data.houseCode, data.unitCode);
	X10info info = x10ex.getModuleInfo(data.houseCode, data.unitCode);
	X10ModuleStatus_s moduleStatus;

	if (state.isSeen)
	{
		moduleStatus.houseCode = data.houseCode;
		moduleStatus.unitCode = data.unitCode;

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
	writeToNetwork(header.from_node, header.type, &moduleStatus, sizeof(X10ModuleStatus_s));
}

#if ENABLE_DHT
// Process environment request (Temperature/Humidity) received from RF24L01+
void environmentRequestEvent(RF24NetworkHeader& header, uint16_t length)
{
	Serial.println(F("environmentRequestEvent"));
	EnvironmentData_s data;
	// Reading temperature or humidity takes about 250 milliseconds!
	// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
	data.humidity = dht.readHumidity();
	// Read temperature as Celsius (the default)
	data.temperatureC = dht.readTemperature();
	// Read temperature as Fahrenheit (isFahrenheit = true)
	data.temperatureF = dht.readTemperature(true);
	// Check if any reads failed and exit early (to try again).
	data.success = !(isnan(data.humidity) || isnan(data.temperatureC) || isnan(data.temperatureF));
	if (data.success)
	{
		// Compute heat index in Fahrenheit (the default)
		data.heatIndexH = dht.computeHeatIndex(data.temperatureF, data.humidity);
		// Compute heat index in Celsius (isFahreheit = false)
		data.heatIndexC = dht.computeHeatIndex(data.temperatureC, data.humidity, false);
	}

	writeToNetwork(header.from_node, header.type, &data, sizeof(data));
}

#if ENABLE_HID
void displayTemperature()
{
	String temperature = String(dht.readTemperature(true));
	char display[6];
	temperature.toCharArray(display, 6, 0);
	hidClient.lcdWriteAt(0, 1, display, 5);
	hidClient.lcdCustomCharacterAt(5, 1, CustomCharacters::DegreeFahrenheit);

	String humidity = String(dht.readHumidity()).substring(0, 2) + String("%");
	humidity.toCharArray(display, 4, 0);
	hidClient.lcdWriteAt(8, 1, display, 3);
}
#endif
#endif

#if ENABLE_RTC
// Process set clock request received from RF24L01+
void setClockRequestEvent(RF24NetworkHeader& header, uint16_t length)
{
	Serial.println(F("setClockRequestEvent"));
	ClockData_s data;
	network.read(header, &data, sizeof(data));

	rtc.setDOW(data.time.dow);
	rtc.setTime(data.time.hour, data.time.min, data.time.sec);
	rtc.setDate(data.time.date, data.time.mon, data.time.year);

	data.success = true;
	writeToNetwork(header.from_node, header.type, &data, sizeof(data));
}

// Process read clock request received from RF24L01+
void readClockRequestEvent(RF24NetworkHeader& header, uint16_t length)
{
	Serial.println(F("readClockRequestEvent"));
	ClockData_s data;
	data.time = rtc.getTime();
	data.success = true;

	writeToNetwork(header.from_node, header.type, &data, sizeof(data));
}
#if ENABLE_HID
void displayTime()
{
	Time time = rtc.getTime();
	uint8_t hour = time.hour - (time.hour > 12 ? 12 : 0);
	String sTime = String(hour > 9 ? "" : "0") + String(hour) + ":" +
		String(time.min > 9 ? "" : "0") + String(time.min) + " " +
		(time.hour > 12 ? "PM" : "AM");
	char display[9];
	sTime.toCharArray(display, 9, 0);
	hidClient.lcdWriteAt(0, 0, display, 8);
}
#endif
#endif

