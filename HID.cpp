#include "HID.h"

HID::HID(Radio* radio)
{
	p_radio = radio;
}

void HID::begin(uint8_t destinationAddress)
{
	p_packet = p_radio->createPacket(NotSet, destinationAddress);
}

void HID::setRate(int rate)
{
	int data[1] = { rate };
	p_packet->writeBody(data, 1);
	sendAndWaitForAck(hidSetKeyRate);
}

int HID::getKey()
{
	int key = NO_KEY;
	if (sendAndWaitForAck(hidGetKey))
	{
		int* data = reinterpret_cast<int*>(p_packet->readBody());
		key = data[0];
	}

	return key;
}

void HID::lcdClear()
{
	sendAndWaitForAck(hidLcdClear);
}

void HID::lcdSetCursor(uint8_t col, uint8_t row)
{
	uint8_t data[2] = { col, row };
	p_packet->writeBody(data, 2);
	sendAndWaitForAck(hidLcdSetCursor);
}

void HID::lcdPrint(char* message, size_t length)
{
	p_packet->writeBody(message, length);
	sendAndWaitForAck(hidLcdPrint);
}

void HID::lcdPrintLine(char* message, size_t length)
{
	p_packet->writeBody(message, length);
	sendAndWaitForAck(hidLcdPrintLine);
}

void HID::lcdWrite(char* message, size_t length)
{
	p_packet->writeBody(message, length);
	sendAndWaitForAck(hidLcdWrite);
}

void HID::lcdCommand(uint8_t value)
{
	uint8_t data[1] = { value };
	p_packet->writeBody(data, 1);
	sendAndWaitForAck(hidLcdCommand);
}

void HID::processAck(RfPacket* rfPacket)
{
	p_packet = rfPacket;
	p_ackArrived = true;
}

bool HID::sendAndWaitForAck(PacketTypes packetType, int timeOut)
{
	p_ackArrived = false;
	p_packet->Type = packetType;
	p_radio->sendPacket(p_packet);
	unsigned long timestamp = millis();
	while (!p_ackArrived)
	{
		if (millis() - timestamp > timeOut) break;
	}
	return p_ackArrived && p_packet->Type == packetType;
}