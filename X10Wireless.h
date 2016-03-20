
#ifndef X10Wireless_h
#define X10Wireless_h

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
	X10EventNotification,
	IrEventNotification,
	RfEventNotification,
	EnvironmentSensorRequest,
	EnvironmentSensorResponse,
	SetClockRequest,
	ReadClockRequest,
	ReadClockResponse
} wrPacketTypes;


class X10Wireless
{

public:
	typedef void(*x10ReceiveCallback_t)(X10Wireless* radio, char, uint8_t, uint8_t);
	typedef void(*environmentRequestCallback_t)(X10Wireless* radio);
	typedef void(*setClockRequestCallback_t)(X10Wireless* radio, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
	typedef void(*readClockRequestCallback_t)(X10Wireless* radio);
	X10Wireless(uint8_t cePin, uint8_t csPin, x10ReceiveCallback_t x10ReceiveCallback, environmentRequestCallback_t environmentRequestCallback, 
		setClockRequestCallback_t setClockRequestCallback, readClockRequestCallback_t readClockRequestCallback);
	// Public methods
	void begin(uint64_t receiveAddress, uint64_t sendAddress);
	void receivePacket();
	bool sendPacket(wrPacketTypes wrPacketType, const void* buffer, uint8_t length);

private:
	RF24 *radio;
	byte payload[32];
	x10ReceiveCallback_t x10ReceiveCallback;
	environmentRequestCallback_t environmentRequestCallback;
	setClockRequestCallback_t setClockRequestCallback;
	readClockRequestCallback_t readClockRequestCallback;
};
#endif
