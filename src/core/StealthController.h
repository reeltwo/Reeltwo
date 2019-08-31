#ifndef StealthController_h
#define StealthController_h

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include <Wire.h>

/**
  * \ingroup core
  *
  * \class StealthController
  *
  * \brief
  *
  * Stealth Controller directly attached to the Arduino to read J1/J2 and S1/S2 status header and
  * using an external DAC via i2c to set the dome AutoDome position.
  */
class StealthController :
    public SetupEvent, AnimatedEvent
{
public:
    const byte S1_FLAG = 1<<0;
    const byte S2_FLAG = 1<<1;
    const byte J1_FLAG = 1<<2;
    const byte J2_FLAG = 1<<3;

    /** \brief Default Constructor
      *
      * Stealth Controller.
      */
    StealthController(const byte pinS1 = 26, const byte pinJ1 = 28, const byte pinS2 = 27, const byte pinJ2 = 29) :
        fPosDegrees(~0),
        fPinS1(pinS1),
        fPinJ1(pinJ1),
        fPinS2(pinS2),
        fPinJ2(pinJ2)
    {
    }

    /**
      * Setup pin mode for analog read
      */
    virtual void setup() override
    {
        pinMode(fPinS1, INPUT_PULLUP);
        pinMode(fPinJ1, INPUT_PULLUP);
        pinMode(fPinS2, INPUT_PULLUP);
        pinMode(fPinJ2, INPUT_PULLUP);
    }

    void setDomePosition(uint16_t degrees)
    {
        if (degrees != fPosDegrees)
        {
            fPosDegrees = degrees;
            int output = map(degrees, 0, 360, 0, 4095);

            Wire.beginTransmission(0x62);
            Wire.write(MCP4726_CMD_WRITEDAC);
            // Upper bits (D11.D10.D9.D8.D7.D6.D5.D4)
            Wire.write(output / 16);
            // Lower bits (D3.D2.D1.D0.x.x.x.x)
            Wire.write((output % 16) << 4);
            Wire.endTransmission();
        }
    }

    inline bool getS1()
    {
        return ((fStatus & S1_FLAG) != 0);
    }

    inline bool getS2()
    {
        return ((fStatus & S2_FLAG) != 0);
    }

    inline bool getJ1()
    {
        return ((fStatus & J1_FLAG) != 0);
    }

    inline bool getJ2()
    {
        return ((fStatus & J2_FLAG) != 0);
    }

    inline byte getStatusFlags()
    {
        return fStatus;
    }

    inline bool getStatusChanged()
    {
        return fChanged;
    }

    /**
      * Read the pot once through the loop
      */
    virtual void animate() override
    {
        byte status = 0;
        fChanged = false;
        if (digitalRead(fPinS1) == HIGH)
            status |= S1_FLAG;
        if (digitalRead(fPinS2) == HIGH)
            status |= S2_FLAG;
        if (digitalRead(fPinJ1) == HIGH)
            status |= J1_FLAG;
        if (digitalRead(fPinJ2) == HIGH)
            status |= J2_FLAG;
        if (status != fStatus)
        {
            fStatus = status;
            fChanged = true;
        }
    }

private:
    const byte MCP4726_CMD_WRITEDAC = 0x40;  // Writes data to the DAC
    const byte MCP4726_CMD_WRITEDACEEPROM = 0x60;  // Writes data to the DAC and the EEPROM (persisting the assigned value after reset)
    uint16_t fPosDegrees;
    bool fChanged;
    byte fStatus;
    byte fPinS1;
    byte fPinJ1;
    byte fPinS2;
    byte fPinJ2;
};

#endif
