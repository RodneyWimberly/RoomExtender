# X10ExSketch
Arduino Library to send and receive X10 commands. Currently the requests can come from Serial, RF, IR, or TCP/IP. I will be modifing it to additionally receive requests from the nRF24L01+ Wireless Transiever. The intention is that the RPi (Raspberry Pi 2 running Windows 10 IoT Core) will send requests to Arduinos to control X10 Modules. Currently I have only tested this with PSC05 PowerLine Interface. 

This project was originally created by Thomas Mittet (https://github.com/tmittet/x10) The original project wouldn't compile correctly and I have modified it to now do so. This was written using Visual Studio 2015 using the Visual Micro Extension for the Arduino IDE (https://visualstudiogallery.msdn.microsoft.com/069a905d-387d-4415-bc37-665a5ac9caba). 

