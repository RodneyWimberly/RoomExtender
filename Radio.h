
#ifndef Radio_h
#define Radio_h

#include <SPI.h>
#include "Arduino.h" 
#include "RF24_config.h"
#include "nRF24L01.h"
#include "RF24.h"

typedef enum
{
	NotSet,
	X10StandardRequest,
	X10ExtendedRequest,
	X10ExtendedResponse,
	PlEventNotification,
	SdEventNotification,
	IrEventNotification,
	RfEventNotification,
	EnvironmentSensorRequest,
	EnvironmentSensorResponse,
	SetClockRequest,
	ReadClockRequest,
	ReadClockResponse
} wrPacketTypes;


class Radio
{

public:
	typedef void(*radioCallback_t)(Radio* radio, wrPacketTypes wrPacketType, const void* buffer, uint8_t length);
	Radio(uint8_t cePin, uint8_t csPin, uint8_t irqPin, radioCallback_t radioCallback);
	// Public methods
	void begin(uint64_t receiveAddress, uint64_t sendAddress, bool useIrq);
	void receivePacket();
	void sendPacket(wrPacketTypes wrPacketType, const void* buffer, uint8_t length);
	void handleInterrupt(void);

private:
	RF24 *radio;
	byte payload[32];
	uint8_t irqPin;
	bool useIrq;
	radioCallback_t radioCallback;
};

/*
typedef struct _X10StandardRequest_s
 {
 	char houseCode;
	uint8_t unitCode;
	uint8_t commandCode;
 } X10StandardRequest_s;  

 typedef union _X10StandardRequest_u
 {
 	X10StandardRequest_s x10StandardRequest;
 	char buffer[sizeof(X10StandardRequest_s)];
 } X10StandardRequest_u;

X10StandardRequest_u x10StandardRequest;
x10StandardRequest.buffer = buffer;
*/

#endif
