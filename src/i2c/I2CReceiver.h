#ifndef I2CReceiver_h
#define I2CReceiver_h

#include "ReelTwo.h"
#include "core/CommandEvent.h"
#include <Wire.h>

#define USE_I2C
#define SUBCLASS_I2C public I2CEvent

/**
  * \ingroup i2c
  *
  * \class I2CReceiver
  *
  * \brief Automatic forwarder from i2c to CommandEvent
  *
  * Create an instance of this template to automatically forward i2c string commands to CommandEvent 
  *
  * \code
  * I2CReceiver<> i2cReceiver(0x19);
  * \endcode
  */
template<int bufferSize = 32>
class I2CReceiver : public AnimatedEvent
{
public:
	I2CReceiver(byte i2caddress = 0x19)
	{
		*myself() = this;
		Wire.begin(i2caddress);                  // Connects to I2C Bus and establishes address.
		Wire.onReceive(i2cEvent);                // Register event so when we receive something we jump to i2cEvent();
	}

	virtual void animate() override
	{
		if (fCmdReady && *fCmdString != 0)
		{
			CommandEvent::process(fCmdString);
		}
		fCmdReady = false;
	}

	void handleEvent(int howMany)
	{
		/* ignore any new i2c event until previous one has been processed */
		if (fCmdReady)
			return;
		char* b = fCmdString;
		for (byte i = 0; Wire.available();)
		{
			char ch = (char)Wire.read();
			// Dont add leading whitespace
			if (i < sizeof(fCmdString) - 1 && (i != 0 || !isspace(ch)))
			{
				fCmdString[i++] = ch;
				fCmdString[i] = 0;
			}
		}
		fCmdReady = true;
	}

	static void i2cEvent(int howMany)
	{
		(*myself())->handleEvent(howMany);
	}

private:
	char fCmdString[bufferSize];
	bool fCmdReady = false;

	static I2CReceiver<bufferSize>** myself()
	{
		static I2CReceiver<bufferSize>* self;
		return &self;
	}
};
#endif
