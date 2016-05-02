#include "HidClient.h"

HidClient::HidClient(RF24Network* network)
{
	p_network = network;
}

void HidClient::begin(uint16_t myNode, uint16_t hidNode)
{
	p_myNode = myNode;
	p_hidNode = hidNode;
}

KeyPadButtons HidClient::waitForKey(uint8_t timeout)
{
	KeyPadButtons key = KeyPadButtons::None;
	uint8_t args[] = { timeout };
	sendAndWaitForValidResponse(HeaderTypes::hidWaitForKey, &args, sizeof(args), &key, sizeof(KeyPadButtons), ((timeout * 100) + 500));

	return key;
}

KeyPadButtons HidClient::getKey()
{
	KeyPadButtons key = KeyPadButtons::None;
	uint8_t foo = 0;
	sendAndWaitForValidResponse(HeaderTypes::hidGetKey, &foo, sizeof(uint8_t), &key, sizeof(key));

	return key;
}

void HidClient::lcdClear()
{
	uint8_t foo = 0;
	sendAndWaitForValidResponse(HeaderTypes::hidLcdClear, &foo, sizeof(uint8_t));
}

void HidClient::lcdSetCursor(uint8_t col, uint8_t row)
{
	uint8_t data[2] = { col, row };
	sendAndWaitForValidResponse(HeaderTypes::hidLcdSetCursor, &data, sizeof(data));
}

void HidClient::lcdWrite(char* message, size_t length)
{
	sendAndWaitForValidResponse(HeaderTypes::hidLcdWrite, &message, length);
}

void HidClient::lcdWriteAt(uint8_t col, uint8_t row, char* message, size_t length)
{
	uint8_t data[length + 2];
	data[0] = col;
	data[1] = row;

	memcpy(data + 2, message, length);
	sendAndWaitForValidResponse(HeaderTypes::hidLcdWriteAt, &data, sizeof(data));
}

void HidClient::lcdCustomCharacter(CustomCharacters customCharacter)
{
	uint8_t data[1] = { customCharacter };
	sendAndWaitForValidResponse(HeaderTypes::hidLcdCustomCharacter, &data, sizeof(data));
}

void HidClient::lcdCustomCharacterAt(uint8_t col, uint8_t row, CustomCharacters customCharacter)
{
	uint8_t data[3] = { col, row, customCharacter };
	sendAndWaitForValidResponse(HeaderTypes::hidLcdCustomCharacterAt, &data, sizeof(data));
}

void HidClient::lcdCommand(uint8_t value)
{
	uint8_t data[1] = { value };
	sendAndWaitForValidResponse(HeaderTypes::hidLcdCommand, &data, sizeof(data));
}

RF24NetworkHeader HidClient::sendToNetwork(HeaderTypes type, const void* message, uint16_t length)
{
	RF24NetworkHeader header = RF24NetworkHeader(p_hidNode, (unsigned char)type);
	bool success = p_network->write(header, message, length);

	return header;
}

void HidClient::waitForResponse(unsigned long timeout)
{
	elapsedMillis elapsedTimer;
	while (!p_network->available())
	{
		p_network->update();
		if (elapsedTimer > timeout)	break;
	}
}

uint16_t HidClient::validateResponse(RF24NetworkHeader sendHeader, RF24NetworkHeader* receiveHeader)
{
	bool success = false;
	uint16_t length;
	p_network->update();
	while (p_network->available())
	{
		length = p_network->peek(*receiveHeader);
		if (receiveHeader->from_node == sendHeader.to_node &&
			receiveHeader->type == sendHeader.type)
		{
			success = true;
			break;
		}
	}

	return success ? length : 0;
}

bool HidClient::sendAndWaitForValidResponse(HeaderTypes type, const void* sendMessage, uint16_t sendLength, unsigned long timeout)
{
	uint8_t receiveMessage;
	return sendAndWaitForValidResponse(type, sendMessage, sendLength, &receiveMessage, sizeof(uint8_t), timeout);
}

bool HidClient::sendAndWaitForValidResponse(HeaderTypes type, const void* sendMessage, uint16_t sendLength, void* receiveMessage, uint16_t receiveLength, unsigned long timeout)
{
	RF24NetworkHeader receiveHeader;
	RF24NetworkHeader sendHeader = sendToNetwork(type, sendMessage, sendLength);
	waitForResponse(timeout);
	uint16_t length = validateResponse(sendHeader, &receiveHeader);
	if (length)
		length = p_network->read(receiveHeader, receiveMessage, receiveLength);
	return length;
}
