#ifndef PeriscopeI2C_h
#define PeriscopeI2C_h

#include "ReelTwo.h"
#include "core/CommandEvent.h"
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
class PeriscopeI2C :
    public CommandEvent
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
        // Issue the down command even if our safe guard variable isn't set
		// if (fPeriscopeUp)
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

    /**
      * Periscope Commands start with 'PS'
      */
    virtual void handleCommand(const char* cmd) override
    {
        if (*cmd++ == 'P' && *cmd++ == 'S')
        {
            if (cmd[0] == 'U' && cmd[1] == 'P')
            {
                up();
            }
            else if (cmd[0] == 'D' && cmd[1] == 'O' && cmd[2] == 'W' && cmd[3] == 'N')
            {
                down();
            }
            else if (cmd[0] == 'F' && cmd[1] == 'A' && cmd[2] == 'S' && cmd[3] == 'T')
            {
                randomFast();
            }
            else if (cmd[0] == 'S' && cmd[1] == 'L' && cmd[2] == 'O' && cmd[3] == 'W')
            {
                randomSlow();
            }
            else if (cmd[0] == 'S' && cmd[1] == 'C' && cmd[2] == 'W')
            {
                searchLightCW();
            }
            else if (cmd[0] == 'S' && cmd[1] == 'C' && cmd[2] == 'C' && cmd[3] == 'W')
            {
                searchLightCCW();
            }
            else if (cmd[0] == 'F' && cmd[1] == 'W' && cmd[2] == 'D')
            {
                faceForward();
            }
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
