/************************************************************************/
/* X10 Rx/Tx library for the XM10/TW7223/TW523 interface, v1.6.         */
/*                                                                      */
/* This library is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or    */
/* (at your option) any later version.                                  */
/*                                                                      */
/* This library is distributed in the hope that it will be useful, but  */
/* WITHOUT ANY WARRANTY; without even the implied warranty of           */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     */
/* General Public License for more details.                             */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */
/* along with this library. If not, see <http://www.gnu.org/licenses/>. */
/*                                                                      */
/* Written by Thomas Mittet (code@lookout.no) October 2010.             */
/************************************************************************/

#ifndef X10ExSketch_h
#define X10ExSketch_h

#define DEBUG 1

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

// IRQ Usage
#define IR_IRQ        		0 // IR Interrupt Request (IRQ 0 must be used with pin 2)
#define RF_IRQ        		1 // RF Interrupt Request (IRQ 1 must be used with pin 3)
#define X10_IRQ       		2 // PSC05 Interrupt Request (IRQ 2 is a custom setup that will work with pins 4 -7)

// Digital Pin Usage
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

// Analog Pin Usage
#define LIGHT_DIGITAL_PIN	0 // Digital Light Intensity Sensor Module Photo Resistor
#define LIGHT_ANALOG_PIN	1 // Digital Light Intensity Sensor Module Photo Resistor
#define RADIO2_CS_PIN   	2 // Serial Peripheral Interface - Chip Select 2 (nRF24L01+)
#define RADIO2_CE_PIN   	3 // Serial Peripheral Interface - Chip Enable 2 (nRF24L01+)
#define I2C_SDA_PIN			4 // Inter-IC - Serial Data (HID module)
#define I2C_SCL_PIN			5 // Inter-IC - Serial Clock (HID module)
#define ANALOG_PIN_6		6 // Not Used
#define ANALOG_PIN_7		7 // Not Used

// I2C Addresses 
#define HID_I2C_ADDRESS		9 // (HID module)

typedef enum {
	InstructionSetKeyRate,
	InstructionGetKey,
	InstructionLcdClear,
	InstructionLcdSetCursor,
	InstructionLcdPrint,
	InstructionLcdPrintLine,
	InstructionLcdWrite,
	InstructionLcdCommand
} InstructionTypes;

#endif