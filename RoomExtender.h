#ifndef RoomExtender_h
#define RoomExtender_h

//#define DEBUG 1
//#define SERIAL_DEBUG

// Enable Hardware Options
#define ENABLE_X10_IR			0
#define ENABLE_X10_RF			0
#define ENABLE_X10_SCENARIO		0
#define ENABLE_RTC				1
#define ENABLE_DHT				1
#define ENABLE_HID				1

// Includes
#include <Arduino.h>
#include <RF24_config.h>
#include <RF24.h>
#include <printf.h>
#include <nRF24L01.h>
#include <Sync.h>
#include <RF24Network_config.h>
#include <RF24Network.h>
#include <SPI.h>
#include <EEPROMex.h>
#include <X10ex.h>
#include <printf.h>
#include <elapsedMillis.h>
#include "ConfigRegister.h"

#if ENABLE_HID
#include "HidClient.h"
#endif

#if ENABLE_DHT
#include <DHT.h>
#endif

#if ENABLE_RTC
#include <DS3231.h>
#endif

#if ENABLE_X10_IR
#include <X10ir.h>
#endif

#if ENABLE_X10_RF
#include <X10rf.h>
#endif

#define POLL_INTERVAL_DISPLAY	30000UL
#define RADIO_CHANNEL			90
#define RE_RADIO_NODE_ID		00
#define HID_RADIO_NODE_ID		01

#define POWER_LINE_MSG 			"PL:"
#define POWER_LINE_MSG_TIME 	1400
#define RADIO_FREQ_MSG 			"RF:"
#define INFRARED_MSG 			"IR:"
#define WIRELESS_DATA_MSG 		"WR:"
#define SERIAL_DATA_MSG 		"SD:"
#define SERIAL_DATA_THRESHOLD	1000
#define MODULE_STATE_MSG 		"MS:"
#define MSG_BUFFER_ERROR 		"_ExBuffer"
#define MSG_DATA_ERROR 			"_ExSyntax"
#define MSG_RECEIVE_TIMEOUT 	"_ExTimOut"
#define MSG_AUTH_ERROR 			"_ExNoAuth"
#define MSG_METHOD_ERROR 		"_ExMethod"
#define USE_UNO					1

// IRQ Usage
#define IR_IRQ        		0 // IR Interrupt Request (IRQ 0 must be used with pin 2)
#define RADIO1_IRQ			0 // nRF24L01+ Interrupt Request (IRQ 0 must be used with pin 2)
#define RF_IRQ        		1 // RF Interrupt Request (IRQ 1 must be used with pin 3)
#define RADIO2_IRQ			1 // nRF24L01+ Interrupt Request (IRQ 1 must be used with pin 3)
#define X10_IRQ       		2 // PSC05 Interrupt Request (IRQ 2 is a custom setup that will work with pins 4 -7)

// Digital Pin Usage
#if USE_UNO
#if ENABLE_DHT
#define DHT_TYPE			DHT11
#endif
#define UART_TX_PIN     	0 // Universal Asynchronous Receiver Transmitter - Transmit
#define UART_RX_PIN     	1 // Universal Asynchronous Receiver Transmitter - Receive
#define IR_RECEIVER_PIN		2 // 38KHz Infrared Receiver Module (VS1838B)
#define RF_RECEIVER_PIN 	3 // 433MHz Radio Frequency Receiver (RF-433-4A40201 MX-05V)
#define X10_RTS_PIN     	4 // X10 Controller - Zero Crossing (PSC05)
#define X10_RX_PIN      	5 // X10 Controller - Receive (PSC05)
#define X10_TX_PIN      	9 // X10 Controller - Transmit (PSC05)
#define DHT_PIN       		8 // Digital Humidity and Tempature Sensor (DHT-11)
#define RADIO1_CS_PIN  		7 // Serial Peripheral Interface - Chip Select 1 (nRF24L01+)
#define RADIO1_CE_PIN   	6 // Serial Peripheral Interface - Chip Enable 1 (nRF24L01+)
#define RF_TRANSMIT_PIN 	10 // 315MHz/330MHz/433MHz Radio Frequency Transmitter (RF-433-4A40201 MX-FS-03V)
#define SPI_MOSI_PIN    	11 // Serial Peripheral Interface - Master Output, Slave Input (output from master)
#define SPI_MISO_PIN    	12 // Serial Peripheral Interface - Master Input, Slave Output (output from slave)
#define SPI_SCLK_PIN     	13 // Serial Peripheral Interface - Serial Clock (output from master)
#else
#if ENABLE_DHT
#define DHT_TYPE			DHT22
#endif
#define UART_TX_PIN     	0 // Universal Asynchronous Receiver Transmitter - Transmit
#define UART_RX_PIN     	1 // Universal Asynchronous Receiver Transmitter - Receive
#define IR_RECEIVER_PIN		2 // 38KHz Infrared Receiver Module (VS1838B)
#define RF_RECEIVER_PIN 	3 // 433MHz Radio Frequency Receiver (RF-433-4A40201 MX-05V)
#define X10_RTS_PIN     	4 // X10 Controller - Zero Crossing (PSC05)
#define X10_RX_PIN      	5 // X10 Controller - Receive (PSC05)
#define X10_TX_PIN      	6 // X10 Controller - Transmit (PSC05)
#define DHT_PIN       		7 // Digital Humidity and Tempature Sensor (DHT-11)
#define RADIO1_CS_PIN  		8 // Serial Peripheral Interface - Chip Select 1 (nRF24L01+)
#define RADIO1_CE_PIN   	9 // Serial Peripheral Interface - Chip Enable 1 (nRF24L01+)
#define RF_TRANSMIT_PIN 	10 // 315MHz/330MHz/433MHz Radio Frequency Transmitter (RF-433-4A40201 MX-FS-03V)
#define SPI_MOSI_PIN    	11 // Serial Peripheral Interface - Master Output, Slave Input (output from master)
#define SPI_MISO_PIN    	12 // Serial Peripheral Interface - Master Input, Slave Output (output from slave)
#define SPI_SCLK_PIN     	13 // Serial Peripheral Interface - Serial Clock (output from master)
#endif

// Analog Pin Usage
#define LIGHT_DIGITAL_PIN	0 // Digital Light Intensity Sensor Module Photo Resistor
#define LIGHT_ANALOG_PIN	1 // Digital Light Intensity Sensor Module Photo Resistor
#define RADIO2_CS_PIN   	2 // Serial Peripheral Interface - Chip Select 2 (nRF24L01+)
#define RADIO2_CE_PIN   	3 // Serial Peripheral Interface - Chip Enable 2 (nRF24L01+)
#define I2C_SDA_PIN			4 // Inter-IC - Serial Data (HID module)
#define I2C_SCL_PIN			5 // Inter-IC - Serial Clock (HID module)

struct X10Data_s
{
	char houseCode;
	uint8_t unitCode;
	uint8_t commandCode;
	uint8_t extendedData;
	uint8_t extendedCommand;
	bool success;
};

struct X10ModuleStatus_s
{
	char houseCode;
	uint8_t unitCode;
	uint8_t type;
	char* name;
	bool stateIsKnown;
	bool stateIsOn;
	uint8_t dimPercentage;
	bool success;
};

#if ENABLE_DHT
struct EnvironmentData_s
{
	float humidity;
	float temperatureC;
	float temperatureF;
	float heatIndexC;
	float heatIndexH;
	bool success;
};
#endif

#if ENABLE_RTC
struct ClockData_s
{
	Time time;
	bool success;
};
#endif

#endif