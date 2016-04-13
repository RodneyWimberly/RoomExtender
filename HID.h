// HID.h

#ifndef _HID_h
#define _HID_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Radio.h"

#define SAMPLE_WAIT -1
#define NO_KEY 0
#define UP_KEY 3
#define DOWN_KEY 4
#define LEFT_KEY 2
#define RIGHT_KEY 5
#define SELECT_KEY 1

class HID
{
 public:
	HID(Radio* radio);
	void begin(uint8_t destinationAddress);
	int getKey();
	void setRate(int);
	void lcdClear();
	void lcdSetCursor(uint8_t col, uint8_t row);
	void lcdPrint(char* message, size_t length);
	void lcdPrintLine(char* message, size_t length);
	void lcdWrite(char* message, size_t length);
	void lcdCommand(uint8_t value);
	void processAck(RfPacket* rfPacket);

private:
	bool p_ackArrived;
	Radio* p_radio;
	RfPacket* p_packet;

	bool sendAndWaitForAck(PacketTypes packetType, int timeOut = 1000);
};

#endif

