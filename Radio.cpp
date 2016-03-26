#include "Radio.h"

Radio *radioInstance = NULL;

void handleInterrupt_wrapper()
{
	if (radioInstance) radioInstance->handleInterrupt();
}

Radio::Radio(
		uint8_t cePin, 
		uint8_t csPin, 
		uint8_t irqPin,
		radioCallback_t radioCallback)
{
	this->irqPin = irqPin;
	this->radio = new RF24(cePin, csPin);
	this->radioCallback = radioCallback;
	radioInstance = this;
}

void Radio::begin(IPAddress receiveAddress, IPAddress sendAddress, bool useIrq)
{
	uint8_t rxAddress[5];
	memcpy(rxAddress, &receiveAddress, sizeof(receiveAddress));
	rxAddress[4] = 0;
	
	uint8_t txAddress[5];
	memcpy(txAddress, &sendAddress, sizeof(sendAddress));
	txAddress[4] = 0;
	
	begin(rxAddress, txAddress, useIrq);
}

void Radio::begin(uint64_t receiveAddress, uint64_t sendAddress, bool useIrq)
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
	this->useIrq = useIrq;
	if (this->useIrq) 
	{ 
		attachInterrupt(this->irqPin, handleInterrupt_wrapper, LOW);
	} 
}

void Radio::handleInterrupt(void)
{
	bool tx, fail, rx;
	radio->whatHappened(tx, fail, rx);   
	if(tx || fail) 
	{
		radio->flush_tx();
		radio->startListening(); 
	}  
	if (rx) { receivePacket(); }
}

void Radio::receivePacket()
{
	while (radio->available())
	{
		uint8_t length = radio->getDynamicPayloadSize();
		if(length < 1) { return; }
		radio->read(&payload, length);

		wrPacketTypes wrPacketType = (wrPacketTypes)payload[0];
		if(radioCallback)
		{
			const uint8_t bufferLength = length - 1;
			char buffer[bufferLength];
			memcpy(payload + 1, buffer, bufferLength);
			radioCallback(this, wrPacketType, &payload, length);
		}
	}
}

void Radio::sendPacket(wrPacketTypes wrPacketType, const void* buffer, uint8_t length)
{
	radio->stopListening();
	radio->flush_tx();
	const byte* current = reinterpret_cast<const byte*>(buffer);
	payload[0] = wrPacketType;
	memcpy(payload + 1, current, length);
	if(this->useIrq)
	{
		radio->startWrite(&payload, length + 1, 0);
	}
	else
	{
		radio->write(&payload, length + 1);
		radio->startListening();
	}
}
