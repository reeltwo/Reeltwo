#ifndef PeriscopeI2C_h
#define PeriscopeI2C_h

#include "ReelTwo.h"
#include <Wire.h>

/**
  * \ingroup i2c
  *
  * \class PeriscopeI2C
  *
  * \brief
  *
  * Encapsulates the available i2c commands that can be sent to the ia-parts.com periscope lifter and periscope light kight.
  */
class PeriscopeI2C
{
public:
    /** \brief Constructor
      *
      * Default i2c address for the periscope is 0x20
      */
	PeriscopeI2C(const byte i2cAddress = 0x20) :
		fI2CAddress(i2cAddress),
		fPeriscopeUp(false),
		fToggleMode(true),
		fLastCmdMillis(0)
	{
	}

	/**
	  * Only allow a new command every 4 seconds
	  */
	inline bool readyForNewCmd()
	{
		return (fLastCmdMillis + 4000 < millis());
	}

	bool isPeriscopeUp()
	{
		return fPeriscopeUp;
	}

	inline void setToggleMode(bool toggleMode)
	{
		fToggleMode = toggleMode;
	}

	void up()
	{
		if (!readyForNewCmd())
			return;
		if (fPeriscopeUp)
		{
			if (fToggleMode)
				down();
		}
		else
		{
			sendCommand(kUp);
			fPeriscopeUp = true;
		}
	}

	/**
	  * Down just means down no toggle
	  */
	void down()
	{
		if (!readyForNewCmd())
			return;
		if (fPeriscopeUp)
		{
			sendCommand(kDown);
			fPeriscopeUp = false;
		}
	}

	void randomFast()
	{
		if (!readyForNewCmd())
			return;
		if (fPeriscopeUp)
		{
			if (fToggleMode)
				down();
		}
		else
		{
			sendCommand(kRandomFast);
			fPeriscopeUp = true;
		}
	}

	void randomSlow()
	{
		if (!readyForNewCmd())
			return;
		if (fPeriscopeUp)
		{
			if (fToggleMode)
				down();
		}
		else
		{
			sendCommand(kRandomSlow);
			fPeriscopeUp = true;
		}
	}

	void searchLightCW()
	{
		if (!readyForNewCmd())
			return;
		if (fPeriscopeUp)
		{
			sendCommand(kSearchLightCW);
		}
	}

	void searchLightCCW()
	{
		if (!readyForNewCmd())
			return;
		if (fPeriscopeUp)
		{
			sendCommand(kSearchLightCCW);
		}
	}

	void faceForward()
	{
		if (!readyForNewCmd())
			return;
		if (fPeriscopeUp)
		{
			sendCommand(kFaceForward);
		}
	}

private:
	enum PeriscopeCmd
	{
		kDown = 1,
		kUp = 2,
		kSearchLightCCW = 3,
		kRandomFast = 4,
		kRandomSlow = 5,
		kFaceForward = 6,
		kSearchLightCW = 7
	};

	void sendCommand(int cmd)
	{
		Wire.beginTransmission(fI2CAddress); 
		Wire.write(cmd);
		Wire.endTransmission();
		fLastCmdMillis = millis();
	}

	byte fI2CAddress;
	bool fPeriscopeUp;
	bool fToggleMode;
	unsigned long fLastCmdMillis;
};

#endif
