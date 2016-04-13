#ifndef Radio_h
#define Radio_h

#include <SPI.h>
#include "Arduino.h" 
#include "RF24_config.h"
#include "nRF24L01.h"
#include "RF24.h"
#include "RfPacket.h"

// Pipes 2-5 only need the last byte to be give because they use the first 4 bytes of pipe 1. 
#define HOST_PIPE_ADDRESS						0x01
#define ROOM_EXTENDER_PIPE_ADDRESS				0x02
#define HUMAN_INPUT_DEVICE_PIPE_ADDRESS			0x03
#define THIRD_PIPE_ADDRESS						0x04
#define FOURTH_PIPE_ADDRESS						0x05
#define FIFTH_PIPE_ADDRESS						0x06

class Radio
{
public:
	typedef void(*radioCallback_t)(Radio* radio, RfPacket* rfPacket, uint8_t pipeNumber);
	Radio(uint8_t cePin, uint8_t csPin, uint8_t irqPin, radioCallback_t radioCallback);
	void begin(uint8_t myAddress, uint64_t myPipeAddress, uint64_t pipe1Address, uint64_t pipe2Address, bool useIrq);
	void receivePacket();
	void sendPacket(RfPacket* rfPacket);
	void sendAckPacket(RfPacket* rfPacket);
	void handleInterrupt(void);
	RfPacket* createPacket(uint8_t type, uint8_t destinationAddress);
private:
	RF24 *radio;
	RfPacket *packet;
	uint8_t pipeNumber;
	uint8_t irqPin;
	uint8_t myAddress;
	uint64_t myPipeAddress;
	uint64_t pipe1Address;
	uint64_t pipe2Address;
	bool useIrq;
	radioCallback_t radioCallback;
};

#endif
