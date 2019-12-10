#ifndef MagicPanelI2C_h
#define MagicPanelI2C_h

#include "core/CommandEvent.h"
#include <Wire.h>

/**
  * \ingroup i2c
  *
  * \class MagicPanelI2C
  *
  * \brief
  *
  * Forwards any 'MP' CommandEvent over i2c to a ia-parts magic panel that has been flashed with Reeltwo.
  */
class MagicPanelI2C : public CommandEvent
{
public:
	MagicPanelI2C(const byte i2cAddress = I2C_MAGIC_PANEL) :
		fI2CAddress(i2cAddress)
	{
	}

    /**
      * Magic Panel Commands start with 'MP'
      */
    virtual void handleCommand(const char* cmd) override
    {
        if (cmd[0] == 'M' && cmd[1] == 'P')
        {
            DEBUG_PRINT("SEND: "); DEBUG_PRINTLN(cmd);
            Wire.beginTransmission(fI2CAddress);
            for (size_t len = strlen(cmd); len-- > 0; cmd++)
                Wire.write(*cmd);
            Wire.endTransmission();  
        }
    }

private:
	byte fI2CAddress;
};


/**
  * \ingroup i2c
  *
  * \class MagicPanelClassicI2C
  *
  * \brief
  *
  * Encapsulates the available i2c commands that can be sent to the ia-parts.com magic panel.
  */
class MagicPanelClassicI2C
{
public:
	MagicPanelClassicI2C(const byte i2cAddress = 0x14) :
		fI2CAddress(i2cAddress)
	{
	}

	void on(byte numSeconds = 0)
	{
		switch (numSeconds)
		{
			case 0:
				sendCommand(kPanelOn);
				break;
			case 2:
				sendCommand(kPanelOn2S);
				break;
			case 5:
				sendCommand(kPanelOn5S);
				break;
			case 10:
				sendCommand(kPanelOn10S);
				break;
		}
	}

	void off()
	{
		sendCommand(kPanelOff);
	}

	void random()
	{
		sendCommand(kPanelRandom6S);
	}

private:
	enum
	{
		kPanelOff = 0,
		kPanelOn = 1,
		kPanelOn2S = 2,
		kPanelOn5S = 3,
		kPanelOn10S = 4,
		kPanelToggle = 5,
		kPanelAlert4S = 6,
		kPanelAlert10S = 7,
		kPanelTraceUpType1 = 8,
		kPanelTraceUpType2 = 9,
		kPanelTraceDownType1 = 10,
		kPanelTraceDownType2 = 11,
		kPanelTraceRightType1 = 12,
		kPanelTraceRightType2 = 13,
		kPanelTraceLeftType1 = 14,
		kPanelTraceLeftType2 = 15,
		kPanelExpandType1 = 16,
		kPanelExpandType2 = 17,
		kPanelCompressType1 = 18,
		kPanelCompressType2 = 19,
		kPanelCross3S = 20,
		kPanelCyclonColumn = 21,
		kPanelCyclonRow = 22,
		kPanelEyeScan = 23,
		kPanelFadeOuIn = 24,
		kPanelFaceOut = 25,
		kPanelFlash5S = 26,
		kPanelFlashV = 27,
		kPanelFlashQ = 28,
		kPanelTwoLoop = 29,
		kPanelOneLoop = 30,
		kPanelTestSequenceType1 = 31,
		kPanelTestSequenceType2 = 32,
		kPanelAILogo = 33,
		kPanel2GWDLogo = 34,
		kPanelQuadrantType1 = 35,
		kPanelQuadrantType2 = 36,
		kPanelQuadrantType3 = 37,
		kPanelQuadrantType4 = 38,
		kPanelRandom6S = 39
	};

	void sendCommand(int cmd)
	{
		Wire.beginTransmission(fI2CAddress); 
		Wire.write(cmd);
		Wire.endTransmission();
	}

	byte fI2CAddress;
};

#endif
