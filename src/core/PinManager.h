#ifndef PinManager_h
#define PinManager_h

#include "ReelTwo.h"

class PinManager
{
public:
	virtual void begin()
	{
	}
	virtual bool digitalRead(uint8_t pin)
	{
		return ::digitalRead(pin);
	}
	virtual void digitalWrite(uint8_t pin, uint8_t val)
	{
		::digitalWrite(pin, val);
	}
	virtual void pinMode(uint8_t pin, uint8_t mode)
	{
		::pinMode(pin, mode);
	}
};

#endif
