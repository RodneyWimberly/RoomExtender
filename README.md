# HA4IoT Arduino Room Extender

Notes for using this project:
1. This is a Visual Studio 2015 soultion created using the Visual Micro plug-in.
2. This project still should be able to be used with the Arduino IDE if you don't have 1. 
3. The contents of the libraries subfolder should be placed into the shared library location

/****************************************************************************/
/* X10 PLC, RF, IR library w/ nRF24L01+ 2.4 GHz Wireless Transceiver Support*/
/* I have only tested this on the PSC05 X10 PowerLine Interface (PLC).		*/
/****************************************************************************/


Arduino Library to send and receive X10 commands. Currently the requests can come from Serial, RF, IR, or TCP/IP. I will be modifing it to additionally receive requests from the nRF24L01+ Wireless Transiever. The intention is that the RPi (Raspberry Pi 2 running Windows 10 IoT Core) will send requests to Arduinos to control X10 Modules. Currently I have only tested this with PSC05 PowerLine Interface. 

This project was originally created by Thomas Mittet (https://github.com/tmittet/x10) The original project wouldn't compile correctly and I have modified it to now do so. This was written using Visual Studio 2015 using the Visual Micro Extension for the Arduino IDE (https://visualstudiogallery.msdn.microsoft.com/069a905d-387d-4415-bc37-665a5ac9caba). 

***************************************************************************
 X10 PSC05/TW523 RJ11 to Arduino Digital GPIO								
--------------------------------------------------------------------------
 Type	Pin # Color	   X10 Description			| Color   Ardunio Pin/Desc
--------------------------------------------------------------------------
 RJ11 pin 1 Black  = Zero Crossing			    | White = Pin 4 / IRQ 2	
 RJ11 pin 2 Red    = Ground					    | Blue  = Ground			     
 RJ11 pin 3 Green  = Receive (Arduino transmit) | Red   = Pin 5			  
 RJ11 pin 4 Yellow = Transmit (Arduino receive) | Black = Pin 6			
***************************************************************************

Process X10 data messages received from computer over Serial (USB) or Wireless (nRF24L01+, IR, and RF) Transports
 
  X10 messages are 3 or 9 bytes long. Use hex 0-F to address units and send commands.
  Bytes must be sent within one second (defined threshold) from the first to the last
  Below are some examples:
 
  Standard Messages examples:
  A12 (House=A, Unit=2, Command=On)
  AB3 (House=A, Unit=12, Command=Off)
  A_5 (House=A, Unit=N/A, Command=Bright)
  |||
  ||+-- Command 0-F or _  Example: 2 = On, 7 = ExtendedCode and _ = No Command
  |+--- Unit 0-F or _     Example: 0 = Unit 1, F = Unit 16 and _ = No unit
  +---- House code A-P    Example: A = House A and P = House P :)
 
  Extended Message examples:
  A37x31x21 (House=A, Unit=4, Command=ExtendedCode, Extended Command=PreSetDim, Extended Data=33)
  B87x01x0D (House=B, Unit=9, Command=ExtendedCode, Extended Command=ShutterOpen, Extended Data=13)
      |/ |/
      |  +-- Extended Data byte in hex     Example: 01 = 1%, 1F = 50% and 3E = 100% brightness (range is decimal 0-62)
      +----- Extended Command byte in hex  Example: 31 = PreSetDim, for more examples check the X10 ExtendedCode spec.
 
  Scenario Execute examples:
  S03 (Execute scenario 3)
  S14 (Execute scenario 20)
  ||/
  |+--- Scenario byte in hex (Hex: 00-FF, Dec: 0-255)
  +---- Scenario Execute Character
 
  Request Module State examples:
  R** (Request buffered state of all modules)
  RG* (Request buffered state of modules using house code G)
  RA2 (Request buffered state of module A3)
  |||
  ||+-- Unit 0-F or *        Example: 0 = Unit 1, A = Unit 10 and * = All units
  |+--- House code A-P or *  Example: A = House A, P = House P and * = All house codes
  +---- Request Module State Character
 
  Wipe Module State examples:
  RW* (Wipe state data for all modules)
  RWB (Wipe state data for all modules using house code B)
  |||
  ||+-- House code A-P or *  Example: A = House A, P = House P and * = All house codes
  |+--- Wipe Module State Character
  +---- Request Module State Character