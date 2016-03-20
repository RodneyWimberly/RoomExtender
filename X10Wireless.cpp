#include "X10Wireless.h"

X10Wireless::X10Wireless(
		uint8_t cePin, 
		uint8_t csPin, 
		x10ReceiveCallback_t x10ReceiveCallback, 
		environmentRequestCallback_t environmentRequestCallback,
		setClockRequestCallback_t setClockRequestCallback, 
		readClockRequestCallback_t readClockRequestCallback)
{
	this->radio = new RF24(cePin, csPin);
	this->x10ReceiveCallback = x10ReceiveCallback;
	this->environmentRequestCallback = environmentRequestCallback;
	this->setClockRequestCallback = setClockRequestCallback;
	this->readClockRequestCallback = readClockRequestCallback;
}

void X10Wireless::begin(uint64_t receiveAddress, uint64_t sendAddress)
{
	radio->begin();
	radio->setChannel(1);
	radio->setPALevel(RF24_PA_MAX);
	radio->setDataRate(RF24_1MBPS);
	radio->setAutoAck(1);
	radio->setRetries(15, 15);
	radio->setCRCLength(RF24_CRC_8);
	radio->openWritingPipe(sendAddress);
	radio->openReadingPipe(0, sendAddress);
	radio->openReadingPipe(1, receiveAddress);
	radio->enableDynamicPayloads();
	radio->startListening();
	radio->powerUp();
}

void X10Wireless::receivePacket()
{
	while (radio->available())
	{
		uint8_t length = radio->getDynamicPayloadSize();
		byte buffer[length];
		radio->read(&buffer, length);

		wrPacketTypes wrPacketType = (wrPacketTypes)buffer[0];
		if (wrPacketType == X10StandardRequest && length >= 4 && x10ReceiveCallback)
		{
			x10ReceiveCallback(this, buffer[1], buffer[2], buffer[3]);
		}
		else if (wrPacketType == X10ExtendedRequest && length >= 10 && x10ReceiveCallback)
		{
			for (int index = 1; index <= 9; index++)
			{
				x10ReceiveCallback(this, buffer[index * 1], buffer[index * 2], buffer[index * 3]);
			}
		}
		else if (wrPacketType == EnvironmentSensorRequest && environmentRequestCallback)
		{
			environmentRequestCallback(this);
		}
		else if (wrPacketType == SetClockRequest && length >= 8 && setClockRequestCallback)
		{
			setClockRequestCallback(this, buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
		}
		else if (wrPacketType == ReadClockRequest && readClockRequestCallback)
		{
			readClockRequestCallback(this);
		}
	}
}

bool X10Wireless::sendPacket(wrPacketTypes wrPacketType, const void* buffer, uint8_t length)
{
	const byte* current = reinterpret_cast<const byte*>(buffer);
	payload[0] = wrPacketType;
	memcpy(payload + 1, current, length);
	return true;// radio->writeFast(&payload, length + 1);
}
