#include "ConfigRegister.h"

ConfigRegister::ConfigRegister() :
	ConfigRegister(DEFAULT_CONFIG_REGISTER)
{
}

ConfigRegister::ConfigRegister(long defaultValue) : p_defaultValue(defaultValue)
{
	HostAddress.Set_Class(this);
	HostAddress.Set_Get(&ConfigRegister::getHostAddress);
	HostAddress.Set_Set(&ConfigRegister::setHostAddress);

	ReAddress.Set_Class(this);
	ReAddress.Set_Get(&ConfigRegister::getReAddress);
	ReAddress.Set_Set(&ConfigRegister::setReAddress);

	HidAddress.Set_Class(this);
	HidAddress.Set_Get(&ConfigRegister::getHidAddress);
	HidAddress.Set_Set(&ConfigRegister::setHidAddress);

	p_configRegister = NULL;
}

void ConfigRegister::begin()
{
	EEPROM.setMemPool(EEPROM_MEM_BASE, EEPROMSizeUno);
	EEPROM.setMaxAllowedWrites(EEPROM_MAX_ALLOWED_WRITES);

	p_configRegister = new EEPROMVar<long>(p_defaultValue);
	p_configRegister->restore();
}

ConfigRegister::~ConfigRegister()
{
	if(p_configRegister)
		delete p_configRegister;
}

uint8_t ConfigRegister::getHostAddress()
{
	return getByte(HOST_ADDRESS_SHIFT, HOST_ADDRESS_MASK);
}

void ConfigRegister::setHostAddress(uint8_t hostAddress)
{
	setByte(hostAddress, HOST_ADDRESS_SHIFT, HOST_ADDRESS_MASK);
}

uint8_t ConfigRegister::getReAddress()
{
	return getByte(RE_ADDRESS_SHIFT, RE_ADDRESS_MASK);
}

void ConfigRegister::setReAddress(uint8_t reAddress)
{
	setByte(reAddress, RE_ADDRESS_SHIFT, RE_ADDRESS_MASK);
}

uint8_t ConfigRegister::getHidAddress()
{
	return getByte(HID_ADDRESS_SHIFT, HID_ADDRESS_MASK);
}

void ConfigRegister::setHidAddress(uint8_t hidAddress)
{
	setByte(hidAddress, HID_ADDRESS_SHIFT, HID_ADDRESS_MASK);
}

uint8_t ConfigRegister::getByte(int shift, byte mask)
{
	return (*p_configRegister >> shift) & mask;
}

void ConfigRegister::setByte(uint8_t value, int shift, byte mask)
{
	value = value << shift;
	*p_configRegister = (value & mask) | ((long)*p_configRegister & ~mask);
	p_configRegister->update();
}
