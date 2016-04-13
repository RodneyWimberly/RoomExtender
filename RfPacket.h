#ifndef _RFPACKET_h
#define _RFPACKET_h

#include "Property.h"

#define PACKET_HEADER_TYPE						0
#define PACKET_HEADER_DESTINATION_ADDRESS		1
#define PACKET_HEADER_SOURCE_ADDRESS			2
#define PACKET_HEADER_SUCCESS					3
#define PACKET_HEADER_BODY_LENGTH				4
#define PACKET_HEADER_LENGTH					5
#define PACKET_BODY_LENGTH						27

enum PacketTypes
{
	NotSet,
	hidSetKeyRate,
	hidGetKey,
	hidLcdClear,
	hidLcdSetCursor,
	hidLcdPrint,
	hidLcdPrintLine,
	hidLcdWrite,
	hidLcdCommand,
	reX10Standard,
	reX10Extended,
	reEnvironmentSensor,
	reSetClock,
	reReadClock,
	rePowerLineEvent,
	reSerialDataEvent,
	reInfaredEvent,
	reRadioFrequencyEvent
};

class RfPacket
{
public:
	Property<uint8_t, RfPacket> SourceAddress;
	Property<uint8_t, RfPacket> DestinationAddress;
	Property<uint8_t, RfPacket> Type;
	Property<bool, RfPacket> Success;
	Property<size_t, RfPacket> BodyLength;
	Property<size_t, RfPacket> BytesLength;

	RfPacket();
	RfPacket(byte* bytes, size_t length);
	RfPacket(uint8_t type, uint8_t destinationAddress, uint8_t sourceAddress);
	void* readBody();
	void writeBody(void* body, size_t length);
	size_t readBytes(byte* data);
	void writeBytes(byte* data, size_t length);
	void printDetails();

private:
	byte p_header[5];
	byte p_body[PACKET_BODY_LENGTH];

	uint8_t get_sourceAddress();
	void set_sourceAddress(uint8_t sourceAddress);

	uint8_t get_destinationAddress();
	void set_destinationAddress(uint8_t destinationAddress);

	uint8_t get_type();
	void set_type(uint8_t type);

	bool get_success();
	void set_success(bool success);

	size_t get_bodyLength();

	size_t get_bytesLength();
};

#endif

