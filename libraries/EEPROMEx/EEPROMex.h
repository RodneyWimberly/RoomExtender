/*
  EEPROMEx.h - Extended EEPROM library
  Copyright (c) 2012 Thijs Elenbaas.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef EEPROMEX_h
#define EEPROMEX_h

#include <Arduino.h> 
#include <inttypes.h>

struct EERef {

	EERef(const int index)
		: index(index) {}

	//Access/read members.
	uint8_t operator*() const { return eeprom_read_byte((uint8_t*)index); }
	operator const uint8_t() const { return **this; }

	//Assignment/write members.
	EERef &operator=(const EERef &ref) { return *this = *ref; }
	EERef &operator=(uint8_t in) { return eeprom_write_byte((uint8_t*)index, in), *this; }
	EERef &operator +=(uint8_t in) { return *this = **this + in; }
	EERef &operator -=(uint8_t in) { return *this = **this - in; }
	EERef &operator *=(uint8_t in) { return *this = **this * in; }
	EERef &operator /=(uint8_t in) { return *this = **this / in; }
	EERef &operator ^=(uint8_t in) { return *this = **this ^ in; }
	EERef &operator %=(uint8_t in) { return *this = **this % in; }
	EERef &operator &=(uint8_t in) { return *this = **this & in; }
	EERef &operator |=(uint8_t in) { return *this = **this | in; }
	EERef &operator <<=(uint8_t in) { return *this = **this << in; }
	EERef &operator >>=(uint8_t in) { return *this = **this >> in; }

	EERef &update(uint8_t in) { return  in != *this ? *this = in : *this; }

	/** Prefix increment/decrement **/
	EERef& operator++() { return *this += 1; }
	EERef& operator--() { return *this -= 1; }

	/** Postfix increment/decrement **/
	uint8_t operator++ (int) {
		uint8_t ret = **this;
		return ++(*this), ret;
	}

	uint8_t operator-- (int) {
		uint8_t ret = **this;
		return --(*this), ret;
	}

	int index; //Index of current EEPROM cell.
};

/***
EEPtr class.

This object is a bidirectional pointer to EEPROM cells represented by EERef objects.
Just like a normal pointer type, this can be dereferenced and repositioned using
increment/decrement operators.
***/

struct EEPtr {

	EEPtr(const int index)
		: index(index) {}

	operator const int() const { return index; }
	EEPtr &operator=(int in) { return index = in, *this; }

	//Iterator functionality.
	bool operator!=(const EEPtr &ptr) { return index != ptr.index; }
	EERef operator*() { return index; }

	/** Prefix & Postfix increment/decrement **/
	EEPtr& operator++() { return ++index, *this; }
	EEPtr& operator--() { return --index, *this; }
	EEPtr operator++ (int) { return index++; }
	EEPtr operator-- (int) { return index--; }

	int index; //Index of current EEPROM cell.
};

/***
EEPROMClass class.

This object represents the entire EEPROM space.
It wraps the functionality of EEPtr and EERef into a basic interface.
This class is also 100% backwards compatible with earlier Arduino core releases.
***/

struct EEPROMClass {

	//Basic user access methods.
	EERef operator[](const int idx) { return idx; }
	uint8_t read(int idx) { return EERef(idx); }
	void write(int idx, uint8_t val) { (EERef(idx)) = val; }
	void update(int idx, uint8_t val) { EERef(idx).update(val); }

	//STL and C++11 iteration capability.
	EEPtr begin() { return 0x00; }
	EEPtr end() { return length(); } //Standards requires this to be the item after the last valid entry. The returned pointer is invalid.
	uint16_t length() { return E2END + 1; }

	//Functionality to 'get' and 'put' objects to and from EEPROM.
	template< typename T > T &get(int idx, T &t) {
		EEPtr e = idx;
		uint8_t *ptr = (uint8_t*)&t;
		for (int count = sizeof(T); count; --count, ++e)  *ptr++ = *e;
		return t;
	}

	template< typename T > const T &put(int idx, const T &t) {
		EEPtr e = idx;
		const uint8_t *ptr = (const uint8_t*)&t;
		for (int count = sizeof(T); count; --count, ++e)  (*e).update(*ptr++);
		return t;
	}
};


// Boards with ATmega328, Duemilanove, Uno, UnoSMD, Lilypad - 1024 bytes (1 kilobyte)
// Boards with ATmega1280 or 2560, Arduino Mega series – 4096 bytes (4 kilobytes)
// Boards with ATmega168, Lilypad, old Nano, Diecimila  – 512 bytes (1/2 kilobyte)

#define EEPROMSizeATmega168   512     
#define EEPROMSizeATmega328   1024     
#define EEPROMSizeATmega1280  4096     
#define EEPROMSizeATmega32u4  1024
#define EEPROMSizeAT90USB1286 4096
#define EEPROMSizeMK20DX128   2048

#define EEPROMSizeUno         EEPROMSizeATmega328     
#define EEPROMSizeUnoSMD      EEPROMSizeATmega328
#define EEPROMSizeLilypad     EEPROMSizeATmega328
#define EEPROMSizeDuemilanove EEPROMSizeATmega328
#define EEPROMSizeMega        EEPROMSizeATmega1280
#define EEPROMSizeDiecimila   EEPROMSizeATmega168
#define EEPROMSizeNano        EEPROMSizeATmega168
#define EEPROMSizeTeensy2     EEPROMSizeATmega32u4
#define EEPROMSizeLeonardo    EEPROMSizeATmega32u4
#define EEPROMSizeMicro       EEPROMSizeATmega32u4
#define EEPROMSizeYun         EEPROMSizeATmega32u4
#define EEPROMSizeTeensy2pp   EEPROMSizeAT90USB1286
#define EEPROMSizeTeensy3     EEPROMSizeMK20DX128

class EEPROMClassEx
{

public:
	EEPROMClassEx();
	bool 	 isReady();
	int 	 writtenBytes();
	void 	 setMemPool(int base, int memSize);
	void  	 setMaxAllowedWrites(int allowedWrites);
	int 	 getAddress(int noOfBytes);

	uint8_t  read(int);
	bool 	 readBit(int, byte);
	uint8_t  readByte(int);
	uint16_t readInt(int);
	uint32_t readLong(int);
	float    readFloat(int);
	double   readDouble(int);

	bool     write(int, uint8_t);
	bool 	 writeBit(int, uint8_t, bool);
	bool     writeByte(int, uint8_t);
	bool 	 writeInt(int, uint16_t);
	bool 	 writeLong(int, uint32_t);
	bool 	 writeFloat(int, float);
	bool 	 writeDouble(int, double);

	bool     update(int, uint8_t);
	bool 	 updateBit(int, uint8_t, bool);
	bool     updateByte(int, uint8_t);
	bool 	 updateInt(int, uint16_t);
	bool 	 updateLong(int, uint32_t);
	bool 	 updateFloat(int, float);
	bool 	 updateDouble(int, double);


	// Use template for other data formats


	template <class T> int readBlock(int address, const T value[], int items)
	{
		if (!isWriteOk(address + items*sizeof(T))) return 0;
		unsigned int i;
		for (i = 0; i < (unsigned int)items; i++)
			readBlock<T>(address + (i*sizeof(T)), value[i]);
		return i;
	}

	template <class T> int readBlock(int address, const T& value)
	{
		eeprom_read_block((void*)&value, (const void*)address, sizeof(value));
		return sizeof(value);
	}

	template <class T> int writeBlock(int address, const T value[], int items)
	{
		if (!isWriteOk(address + items*sizeof(T))) return 0;
		unsigned int i;
		for (i = 0; i < (unsigned int)items; i++)
			writeBlock<T>(address + (i*sizeof(T)), value[i]);
		return i;
	}

	template <class T> int writeBlock(int address, const T& value)
	{
		if (!isWriteOk(address + sizeof(value))) return 0;
		eeprom_write_block((void*)&value, (void*)address, sizeof(value));
		return sizeof(value);
	}

	template <class T> int updateBlock(int address, const T value[], int items)
	{
		int writeCount = 0;
		if (!isWriteOk(address + items*sizeof(T))) return 0;
		unsigned int i;
		for (i = 0; i < (unsigned int)items; i++)
			writeCount += updateBlock<T>(address + (i*sizeof(T)), value[i]);
		return writeCount;
	}

	template <class T> int updateBlock(int address, const T& value)
	{
		int writeCount = 0;
		if (!isWriteOk(address + sizeof(value))) return 0;
		const byte* bytePointer = (const byte*)(const void*)&value;
		for (unsigned int i = 0; i < (unsigned int)sizeof(value); i++) {
			if (read(address) != *bytePointer) {
				write(address, *bytePointer);
				writeCount++;
			}
			address++;
			bytePointer++;
		}
		return writeCount;
	}



private:
	//Private variables
	static int _base;
	static int _memSize;
	static int _nextAvailableaddress;
	static int _writeCounts;
	int _allowedWrites;
	bool checkWrite(int base, int noOfBytes);
	bool isWriteOk(int address);
	bool isReadOk(int address);
};

extern EEPROMClassEx EEPROM;

#endif

