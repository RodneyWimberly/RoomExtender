#include "RfPacket.h"

// Source Address 
uint8_t RfPacket::get_sourceAddress()
{
	return p_header[PACKET_HEADER_SOURCE_ADDRESS];
}

void RfPacket::set_sourceAddress(uint8_t sourceAddress)
{
	p_header[PACKET_HEADER_SOURCE_ADDRESS] = sourceAddress;
}

// Destination Address
uint8_t RfPacket::get_destinationAddress()
{
	return p_header[PACKET_HEADER_DESTINATION_ADDRESS];
}

void RfPacket::set_destinationAddress(uint8_t destinationAddress)
{
	p_header[PACKET_HEADER_DESTINATION_ADDRESS] = destinationAddress;
}

// Type
uint8_t RfPacket::get_type()
{
	return p_header[PACKET_HEADER_TYPE];
}

void RfPacket::set_type(uint8_t packetType)
{
	p_header[PACKET_HEADER_TYPE] = packetType;
}

// Success
bool RfPacket::get_success()
{
	return p_header[PACKET_HEADER_SUCCESS];
}

void RfPacket::set_success(bool success)
{
	p_header[PACKET_HEADER_SUCCESS] = success;
}

// Body
size_t RfPacket::get_bodyLength()
{
	return p_header[PACKET_HEADER_BODY_LENGTH];
}

void* RfPacket::readBody()
{
	return p_body;
}

void RfPacket::writeBody(void* body, size_t length)
{
	memcpy(p_body, body, length);
	p_header[PACKET_HEADER_BODY_LENGTH] = length;
}

// Bytes
size_t RfPacket::get_bytesLength()
{
	return BodyLength + PACKET_HEADER_LENGTH;
}

size_t RfPacket::readBytes(byte* bytes)
{
	memcpy(bytes, p_header, PACKET_HEADER_LENGTH);
	memcpy(bytes + PACKET_HEADER_LENGTH, p_body, BodyLength);
	return BytesLength;
}

void RfPacket::writeBytes(byte* bytes, size_t length)
{
	printf("RfPacket::writeBytes started\r\n");
	if (length >= PACKET_HEADER_LENGTH)
	{
		p_header[PACKET_HEADER_BODY_LENGTH] = length - PACKET_HEADER_LENGTH;
		memcpy(p_header, bytes, PACKET_HEADER_LENGTH);
		if (length > PACKET_HEADER_LENGTH)
		{
			writeBody(bytes + PACKET_HEADER_LENGTH, length - PACKET_HEADER_LENGTH);
		}
	}
	printf("RfPacket::writeBytes finished\r\n");
}

// ctor
RfPacket::RfPacket()
{
	SourceAddress.Set_Class(this);
	SourceAddress.Set_Get(&RfPacket::get_sourceAddress);
	SourceAddress.Set_Set(&RfPacket::set_sourceAddress);

	DestinationAddress.Set_Class(this);
	DestinationAddress.Set_Get(&RfPacket::get_destinationAddress);
	DestinationAddress.Set_Set(&RfPacket::set_destinationAddress);

	Type.Set_Class(this);
	Type.Set_Get(&RfPacket::get_type);
	Type.Set_Set(&RfPacket::set_type);

	Success.Set_Class(this);
	Success.Set_Get(&RfPacket::get_success);
	Success.Set_Set(&RfPacket::set_success);

	BodyLength.Set_Class(this);
	BodyLength.Set_Get(&RfPacket::get_bodyLength);

	BytesLength.Set_Class(this);
	BytesLength.Set_Get(&RfPacket::get_bytesLength);

	p_header[PACKET_HEADER_TYPE] = NotSet;
	p_header[PACKET_HEADER_DESTINATION_ADDRESS] = 0;
	p_header[PACKET_HEADER_SOURCE_ADDRESS] = 0;
	p_header[PACKET_HEADER_SUCCESS] = false;
	p_header[PACKET_HEADER_BODY_LENGTH] = 0;
}

RfPacket::RfPacket(byte* bytes, size_t length) :
	RfPacket::RfPacket()
{
	writeBytes(bytes, length);
}

RfPacket::RfPacket(uint8_t type, uint8_t destinationAddress, uint8_t sourceAddress) :
	RfPacket::RfPacket()
{
	Type = type;
	DestinationAddress = destinationAddress;
	SourceAddress = sourceAddress;
	Success = false;
	p_header[PACKET_HEADER_BODY_LENGTH] = 0;
}

void RfPacket::printDetails()
{
	int sourceAddress = this->SourceAddress,
		destinationAddress = this->DestinationAddress,
		bodyLength = this->BodyLength,
		bytesLength = this->BytesLength,
		type = this->Type,
		success = this->Success;
	PacketTypes pt = (PacketTypes)type;
	printf("printPacketDetails SourceAddress:%x\r\n", sourceAddress);
	printf("printPacketDetails DestinationAddress:%x\r\n", destinationAddress);
	printf("printPacketDetails Type:%x\r\n", pt);
	//printf("printPacketDetails Success:%x\r\n", success);
	printf("printPacketDetails BodyLength:%x\r\n", bodyLength);
	printf("printPacketDetails BytesLength:%x\r\n", bytesLength);

	/*byte* buffer = reinterpret_cast<byte*>(this->readBody());
	uint8_t length1 = sizeof(buffer);
	int bufferValue = 0;
	for (int i = 0; i < length1; i++)
	{
	bufferValue = buffer[i];
	printf("Buffer[%x]: %x", i, bufferValue);
	}
	*/
}
