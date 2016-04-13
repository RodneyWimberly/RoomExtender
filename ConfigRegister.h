#ifndef _CONFIGREGISTER_h
#define _CONFIGREGISTER_h

#include "EEPROMex.h"
#include "EEPROMvar.h"
#include "Property.h"

#define EEPROM_MAX_ALLOWED_WRITES		20
#define EEPROM_MEM_BASE					120

#define DEFAULT_CONFIG_REGISTER			0x321

#define HOST_ADDRESS_MASK				0b1111
#define HOST_ADDRESS_SHIFT				0
#define RE_ADDRESS_MASK					0b1111
#define RE_ADDRESS_SHIFT				4
#define HID_ADDRESS_MASK				0b1111
#define HID_ADDRESS_SHIFT				8

class ConfigRegister
{
public:
	Property<uint8_t, ConfigRegister> HostAddress;
	Property<uint8_t, ConfigRegister> ReAddress;
	Property<uint8_t, ConfigRegister> HidAddress;

	ConfigRegister();
	ConfigRegister(long defaultValue);
	void begin();

private:
	EEPROMVar<long> *p_configRegister;
	long p_defaultValue;

	uint8_t getHostAddress();
	void setHostAddress(uint8_t hostAddress);

	uint8_t getReAddress();
	void setReAddress(uint8_t reAddress);

	uint8_t getHidAddress();
	void setHidAddress(uint8_t hidAddress);

	uint8_t getByte(int shift, byte mask);
	void setByte(uint8_t value, int shift, byte mask);
};

#endif

