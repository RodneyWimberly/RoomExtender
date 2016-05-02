#ifndef _HIDCLIENT_h
#define _HIDCLIENT_h

#include <RF24_config.h>
#include <RF24.h>
#include <nRF24L01.h>
#include <Sync.h>
#include <RF24Network_config.h>
#include <RF24Network.h>
#include <SPI.h>
#include <elapsedMillis.h>
#include <DfrLcdKeypad.h>

enum HeaderTypes
{
	/* These types will NOT receive network acknowledgements */
	notSet = 0,
	rePowerLineEvent = 1,
	reSerialDataEvent = 2,
	reInfaredEvent = 3,
	reRadioFrequencyEvent = 4,

	/* These types will receive network acknowledgements */
	hidGetKey = 66,
	hidWaitForKey = 67,
	hidLcdClear = 68,
	hidLcdSetCursor = 69,
	hidLcdWrite = 70,
	hidLcdWriteAt = 71,
	hidLcdCustomCharacter = 72,
	hidLcdCustomCharacterAt = 73,
	hidLcdCommand = 74,
	reX10Standard = 75,
	reX10ModuleStatus = 76,
	reEnvironmentSensor = 77,
	reSetClock = 78,
	reReadClock = 79,
};

class HidClient
{

public:
	HidClient(RF24Network* network);
	void begin(uint16_t myNode, uint16_t hidNode);

	KeyPadButtons getKey();
	KeyPadButtons waitForKey(uint8_t timeout = 10);
	void lcdClear();
	void lcdSetCursor(uint8_t col, uint8_t row);
	void lcdWrite(char* message, size_t length);
	void lcdWriteAt(uint8_t col, uint8_t row, char* message, size_t length);
	void lcdCustomCharacter(CustomCharacters customCharacter);
	void lcdCustomCharacterAt(uint8_t col, uint8_t row, CustomCharacters customCharacter);
	void lcdCommand(uint8_t value);

private:
	RF24Network* p_network;
	uint16_t p_myNode;
	uint16_t p_hidNode;
	elapsedMillis p_lastButtonReceived;
	KeyPadButtons p_lastButton;

	RF24NetworkHeader sendToNetwork(HeaderTypes type, const void* message, uint16_t length);
	void waitForResponse(unsigned long timeout);
	uint16_t validateResponse(RF24NetworkHeader sendHeader, RF24NetworkHeader* receiveHeader);
	bool sendAndWaitForValidResponse(HeaderTypes type, const void* sendMessage, uint16_t sendLength, unsigned long timeout = 500);
	bool sendAndWaitForValidResponse(HeaderTypes type, const void* sendMessage, uint16_t sendLength, void* receiveMessage, uint16_t receiveLength, unsigned long timeout = 500);
};
#endif