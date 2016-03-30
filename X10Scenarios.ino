#include "Arduino.h" 
#include "X10ex.h"
#include "RoomExtender.h"

#if ENABLE_X10_SCENARIO

// Handles scenario execute commands received as serial data message
void handleSdScenario(byte scenario)
{
	switch (scenario)
	{
		//////////////////////////////
		// Sample Code
		// Replace with your own setup
		//////////////////////////////
	case 0x01: sendMasterBedroomOn(); break;
	case 0x02: sendHallAndKitchenOn(); break;
	case 0x03: sendLivingRoomOn(); break;
	case 0x04: sendLivingRoomTvScenario(); break;
	case 0x05: sendLivingRoomMovieScenario(); break;
	case 0x11: sendAllLightsOff(); break;
	case 0x12: sendHallAndKitchenOff(); break;
	case 0x13: sendLivingRoomOff(); break;
	}
}

// Handles scenarios triggered when receiving unit on/off commands
// Use this method to trigger scenarios from simple RF/IR remotes
bool handleUnitScenario(char house, byte unit, byte command, bool isRepeat, bool failOnBufferError)
{
	//////////////////////////////
	// Sample Code
	// Replace with your own setup
	//////////////////////////////

	bool bufferError = 0;

	// Ignore all house codes except A
	if (house != 'A') return 0;

	// Unit 4 is an old X10 lamp module that doesn't remember state on its own. Use the
	// following method to revert to buffered state when these modules are turned on
	if (unit == 4)
	{
		bufferError = handleOldLampModuleState(house, unit, command, isRepeat);
	}
	// Unit 5 is a placeholder unit code used by RF and IR remotes that triggers HallAndKitchen scenario
	else if (unit == 5)
	{
		if (!isRepeat)
		{
			if (command == CMD_ON) bufferError = sendHallAndKitchenOn();
			else if (command == CMD_OFF) bufferError = sendHallAndKitchenOff();
		}
	}
	// Unit 10 is a placeholder unit code used by RF and IR remotes that triggers AllLights scenario
	else if (unit == 10)
	{
		if (!isRepeat)
		{
			if (command == CMD_ON) bufferError = sendAllLightsOn();
			else if (command == CMD_OFF) bufferError = sendAllLightsOff();
		}
	}
	// Unit 11 is a placeholder unit code used by RF and IR remotes that triggers LivingRoom scenario
	else if (unit == 11)
	{
		if (!isRepeat)
		{
			if (command == CMD_ON) bufferError = sendLivingRoomOn();
			else if (command == CMD_OFF) bufferError = sendLivingRoomOff();
		}
	}
	// Unit 12 is a placeholder unit code used by RF and IR remotes that triggers LivingRoomTv scenario
	else if (unit == 12)
	{
		if (!isRepeat)
		{
			if (command == CMD_ON) bufferError = sendLivingRoomTvScenario();
			else if (command == CMD_OFF) bufferError = sendLivingRoomOn();
		}
	}
	// Unit 13 is a placeholder unit code used by RF and IR remotes that triggers LivingRoomMovie scenario
	else if (unit == 13)
	{
		if (!isRepeat)
		{
			if (command == CMD_ON) bufferError = sendLivingRoomMovieScenario();
			else if (command == CMD_OFF) bufferError = sendLivingRoomOn();
		}
	}
	else
	{
		return 0;
	}
	return !failOnBufferError || !bufferError;
}

// Handles old X10 lamp modules that don't remember state on their own
bool handleOldLampModuleState(char house, byte unit, byte command, bool isRepeat)
{
	if (!isRepeat)
	{
		bool bufferError = 0;
		if (command == CMD_ON)
		{
			// Get state from buffer, and set to last brightness when turning on
			X10state state = x10ex.getModuleState(house, unit);
			if (state.isKnown && !state.isOn && state.data > 0)
			{
				bufferError = x10ex.sendExt(house, unit, CMD_EXTENDED_CODE, state.data, EXC_PRE_SET_DIM, 1);
			}
		}
		return bufferError || x10ex.sendCmd(house, unit, command, 1);
	}
	return 0;
}

bool sendAllLightsOn()
{
	return
		// Bedroom
		x10ex.sendExtDim('A', 7, 80, EXC_DIM_TIME_4, 1) ||
		// Dining Table
		x10ex.sendExtDim('A', 2, 70, EXC_DIM_TIME_4, 1) ||
		// Hall
		x10ex.sendExtDim('A', 8, 75, EXC_DIM_TIME_4, 1) ||
		// Couch
		x10ex.sendExtDim('A', 3, 90, EXC_DIM_TIME_4, 1) ||
		// Kitchen
		x10ex.sendExtDim('A', 9, 100, EXC_DIM_TIME_4, 1) ||
		// TV Backlight
		x10ex.sendExtDim('A', 4, 40, EXC_DIM_TIME_4, 1);
}

bool sendAllLightsOff()
{
	return
		x10ex.sendCmd('A', 7, CMD_OFF, 1) ||
		x10ex.sendCmd('A', 2, CMD_OFF, 1) ||
		x10ex.sendCmd('A', 8, CMD_OFF, 1) ||
		x10ex.sendCmd('A', 3, CMD_OFF, 1) ||
		x10ex.sendCmd('A', 9, CMD_OFF, 1) ||
		x10ex.sendCmd('A', 4, CMD_OFF, 1);
}

bool sendHallAndKitchenOn()
{
	return
		x10ex.sendExtDim('A', 8, 75, EXC_DIM_TIME_4, 1) ||
		x10ex.sendExtDim('A', 9, 100, EXC_DIM_TIME_4, 1);
}

bool sendHallAndKitchenOff()
{
	return
		x10ex.sendCmd('A', 8, CMD_OFF, 1) ||
		x10ex.sendCmd('A', 9, CMD_OFF, 1);
}

bool sendLivingRoomOn()
{
	return
		x10ex.sendExtDim('A', 2, 70, EXC_DIM_TIME_4, 1) ||
		x10ex.sendExtDim('A', 3, 90, EXC_DIM_TIME_4, 1) ||
		x10ex.sendExtDim('A', 4, 40, EXC_DIM_TIME_4, 1);
}

bool sendLivingRoomOff()
{
	return
		x10ex.sendCmd('A', 2, CMD_OFF, 1) ||
		x10ex.sendCmd('A', 3, CMD_OFF, 1) ||
		x10ex.sendCmd('A', 4, CMD_OFF, 1);
}

bool sendLivingRoomTvScenario()
{
	return
		x10ex.sendExtDim('A', 2, 40, EXC_DIM_TIME_4, 1) ||
		x10ex.sendExtDim('A', 3, 30, EXC_DIM_TIME_4, 1) ||
		x10ex.sendExtDim('A', 4, 25, EXC_DIM_TIME_4, 1);
}

bool sendMasterBedroomOn()
{
	return
		x10ex.sendCmd('A', 0, CMD_ON, 1) ||
		x10ex.sendCmd('A', 1, CMD_ON, 1);
}

bool sendLivingRoomMovieScenario()
{
	return
		x10ex.sendCmd('A', 2, CMD_OFF, 1) ||
		x10ex.sendCmd('A', 3, CMD_OFF, 1) ||
		x10ex.sendExtDim('A', 4, 25, EXC_DIM_TIME_4, 1);
}
#endif
