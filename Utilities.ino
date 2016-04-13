// Change a big-endian word into a little-endian
uint16_t flip_endian(uint16_t in)
{
	uint16_t low = in >> 8;
	uint16_t high = in << 8;

	return high | low;
}

byte charHexToDecimal(byte input)
{
	// 0123456789  =>  0-15
	if (input >= 0x30 && input <= 0x39) input -= 0x30;
	// ABCDEF  =>  10-15
	else if (input >= 0x41 && input <= 0x46) input -= 0x37;
	// Return converted byte
	return input;
}

byte stringToDecimal(const char input[], byte startPos, byte endPos)
{
	byte decimal = 0;
	byte multiplier = 1;
	for (byte i = endPos + 1; i > startPos; i--)
	{
		if (input[i - 1] >= 0x30 && input[i - 1] <= 0x39)
		{
			decimal += (input[i - 1] - 0x30) * multiplier;
			multiplier *= 10;
		}
		else if (multiplier > 1)
		{
			break;
		}
	}
	return decimal;
}

short stringIndexOf(const char string[], char find, byte startPos, byte endPos, short notFoundValue)
{
	char *ixStr = strchr(string + startPos, find);
	return
		ixStr - string < 0 || (endPos > 0 && ixStr - string > endPos) ?
		notFoundValue : ixStr - string;
}
