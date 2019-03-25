#ifndef StealthBoardI2C_h
#define StealthBoardI2C_h

#include "core/CommandEvent.h"
#include <Wire.h>

/**
  * \ingroup i2c
  *
  * \class StealthBoardI2C
  *
  * \brief
  *
  * Encapsulates the available i2c commands that can be sent to the ia-parts.com magic panel.
  */
class StealthBoardI2C : public CommandEvent
{
public:
    /** \brief Default Constructor
      *
      * Encapsulates a Stealh Board at the specific i2c address.
      */
    StealthBoardI2C(const byte i2cAddress = 0) :
        fI2CAddress(i2cAddress)
    {
    }

    /**
      * Stealth Commands start with 'ST'
      */
    virtual void handleCommand(const char* cmd) override
    {
        if (*cmd++ == 'S' && *cmd++ == 'T')
        {
            byte sum = 0;
            Wire.beginTransmission(fI2CAddress);
            for (size_t len = strlen(cmd); len-- > 0; cmd++)
            {
                Wire.write(*cmd);
                sum += byte(*cmd);
            }
            Wire.write(sum);
            Wire.endTransmission();  
        }
    }

private:
    byte fI2CAddress;
};

#endif
