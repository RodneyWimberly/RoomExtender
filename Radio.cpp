#include "Radio.h"

Radio *radioInstance = NULL;

void handleInterrupt_wrapper()
{
	if (radioInstance) radioInstance->handleInterrupt();
}

Radio::Radio(uint8_t cePin, uint8_t csPin, uint8_t irqPin, radioCallback_t radioCallback)
{
	this->irqPin = irqPin;
	this->radio = new RF24(cePin, csPin);
	this->packet = new RfPacket();
	this->radioCallback = radioCallback;
	radioInstance = this;
}

void Radio::begin(uint8_t myAddress, uint64_t myPipeAddress, uint64_t pipe1Address, uint64_t pipe2Address, bool useIrq)
{
	this->myAddress = myAddress;
	this->myPipeAddress = myPipeAddress;
	this->pipe1Address = pipe1Address;
	this->pipe2Address = pipe2Address;
	this->useIrq = useIrq;
	radio->begin();
	radio->setChannel(1);
	radio->setPALevel(RF24_PA_MAX);
	radio->setDataRate(RF24_1MBPS);
	radio->setAutoAck(1);
	radio->setRetries(15, 15);
	radio->setCRCLength(RF24_CRC_8);
	radio->openWritingPipe(this->myPipeAddress);
	radio->openReadingPipe(0, this->myPipeAddress);
	radio->openReadingPipe(1, this->pipe1Address);
	radio->openReadingPipe(2, this->pipe2Address);
	radio->enableDynamicPayloads();
	radio->powerUp();
	radio->startListening();

	radio->printDetails();
	if (this->useIrq)
	{
		attachInterrupt(this->irqPin, handleInterrupt_wrapper, LOW);
	}
}

void Radio::handleInterrupt(void)
{
	bool tx, fail, rx;
	radio->whatHappened(tx, fail, rx);
	printf("handleInterrupt TX:%x RX:%x Fail:%x\r\n", tx, rx, fail);
	if (tx || fail)
	{
		radio->flush_tx();
		radio->startListening();
	}
	if (rx) { receivePacket(); }
}

void Radio::receivePacket()
{
	printf("receivePacket start\r\n");
	while (radio->available(&this->pipeNumber))
	{
		printf("receivePacket PipeNumber:%x\r\n", this->pipeNumber);
		uint8_t length = radio->getDynamicPayloadSize();
		printf("receivePacket Payload Size:%x\r\n", length);
		if (length < 1) return;

		byte bytes[length];
		radio->read(&bytes, length);
		this->packet->writeBytes(bytes, length);
		this->packet->printDetails();

		if (radioCallback)
		{
			printf("receivePacket radioCallback\r\n");
			radioCallback(this, this->packet, this->pipeNumber);
		}
	}
	printf("receivePacket end\r\n");
}

void Radio::sendAckPacket(RfPacket* rfPacket)
{
	if (!this->pipeNumber) return;

	radio->stopListening();
	radio->flush_tx();
	this->packet = rfPacket;
	// Normal delay will not work here, so cycle through some no-operations (16nops @16mhz = 1us delay)
	for (uint32_t i = 0; i<130; i++) {
		__asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
	}

	byte bytes[32];
	size_t bytesLength = this->packet->readBytes(bytes);

	radio->writeAckPayload(this->pipeNumber, &bytes, bytesLength);
	radio->flush_tx();
	radio->startListening();
}

void Radio::sendPacket(RfPacket* rfPacket)
{
	radio->stopListening();
	radio->flush_tx();
	this->packet = rfPacket;
	byte bytes[32];
	size_t bytesLength = this->packet->readBytes(bytes);

	if (this->useIrq)
	{
		radio->startWrite(&bytes, bytesLength, 0);
	}
	else
	{
		radio->write(&bytes, bytesLength);
		radio->startListening();
	}
}

RfPacket* Radio::createPacket(uint8_t type, uint8_t destinationAddress)
{
	RfPacket* packet = new RfPacket();
	packet->Type = type;
	packet->DestinationAddress = destinationAddress;
	packet->SourceAddress = this->myAddress;

	return packet;
}

