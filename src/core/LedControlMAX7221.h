
#ifndef LedControlMAX7221_h
#define LedControlMAX7221_h

#include "ReelTwo.h"

/// \private
class LedControl
{
public:
    LedControl()
    {
    }

    enum {
        kMaxBrightness = 15
    };

    /**
      * Add device to chain and return index. You must ensure that enough
      * devices were reserved for it to function.
      */
    virtual byte addDevice(byte count = 1) = 0;

    virtual int getDeviceCount() = 0;

    virtual bool isPowered(byte device) = 0;

    virtual void setPower(byte device, bool onState, byte numDevices = 1) = 0;

    virtual void setScanLimit(byte device, byte limit, byte numDevices = 1) = 0;

    virtual void setIntensity(byte device, byte intensity, byte numDevices = 1) = 0;

    virtual void clearDisplay(byte device, byte numDevices = 1) = 0;

    /** 
      * Set the status of a single Led.
      * Params :
      * device   address of the display 
      * row      the row of the Led (0..7)
      * col      the column of the Led (0..7)
      * state    If true the led is switched on, 
      *      if false it is switched off
      */
    virtual void setLed(byte device, int row, int column, boolean state) = 0;

    /** 
      * Set all 8 Led's in a row to a new state
      * Params:
      * device   address of the display
      * row      row which is to be set (0..7)
      * value    each bit set to 1 will light up the
      *      corresponding Led.
      */
    virtual void setRow(byte device, int row, byte value) = 0;

    /** 
      * Set all 8 Led's in a row to a new state
      * Params:
      * device   address of the display
      * row      row which is to be set (0..7)
      * value    each bit set to 1 will light up the
      *      corresponding Led.
      */
    virtual void setRowNoCache(byte device, int row, byte value) = 0;
    
    /** 
      * Set all 8 Led's in a column to a new state
      * Params:
      * device   address of the display
      * column   column which is to be set (0..7)
      * value    each bit set to 1 will light up the
      *      corresponding Led.
      */
    virtual void setColumn(byte device, int column, byte value) = 0;

    virtual byte getRow(byte device, byte row) = 0;

    void setAllPower(bool onState)
    {
        setPower(0, onState, getDeviceCount());
    }

    void clearAllDisplays()
    {
        clearDisplay(0, getDeviceCount());
    }

    /**
      * Utility to generate random LED patterns
      * Mode goes from 0 to 6. The lower the mode
      * the less the LED density that's on.
      * Modes 4 and 5 give the most organic feel
      */
    static byte randomRow(byte randomMode)
    {
        switch(randomMode)
        {
            case 0:  // stage -3
                return (random(256)&random(256)&random(256)&random(256));
            case 1:  // stage -2
                return (random(256)&random(256)&random(256));
            case 2:  // stage -1
                return (random(256)&random(256));
            case 3: // legacy "blocky" mode
                return random(256);
            case 4:  // stage 1
                return (random(256)|random(256));
            case 5:  // stage 2
                return (random(256)|random(256)|random(256));
            case 6:  // stage 3
                return (random(256)|random(256)|random(256)|random(256));
        }
        return random(256);
    }
};

/**
  * \ingroup Core
  *
  * \class LedControlMAX7221
  *
  * \brief LED MAX7221 device chain
  *
  * Encapsulates an MAX7221 device chain of "numDevices" on "dataPin", "clkPin", and "csPin"
  */
template <byte numDevices>
class LedControlMAX7221 : public LedControl
{
public:
    /**
      * \brief Constructor
      */
    LedControlMAX7221(byte dataPin, byte clkPin, byte csPin) :
        fIndex(0),
        fPowerMask(0),
        fSPI_MOSI(dataPin),
        fSPI_CLK(clkPin),
        fSPI_CS(csPin)
    {
        pinMode(fSPI_MOSI, OUTPUT);
        pinMode(fSPI_CLK, OUTPUT);
        pinMode(fSPI_CS, OUTPUT);
        digitalWrite(fSPI_CS, HIGH);
        // set everything to 0xff so clearDisplay will zero out
        for (byte i = 0; i < sizeof(fBits); i++)
            fBits[i] = ~0;
        for (byte i = 0; i < numDevices; i++)
        {
            spiTransfer(i, kOP_DISPLAYTEST, 0);
            //scanlimit is set to max on startup
            setScanLimit(i, 7);
            //decode is done in source
            spiTransfer(i, kOP_DECODEMODE, 0);
            clearDisplay(i);
            setIntensity(i, kBrightness);
            setPower(i, false);
        }
    }

    /**
      * Add device to chain and return index. You must ensure that enough
      * devices were reserved for it to function.
      */
    virtual byte addDevice(byte count = 1) override
    {
        byte idx = fIndex;
        fIndex += count;
        return idx;
    }

    /**
      * \returns the number of devices in this chain
      */
    virtual int getDeviceCount() override
    {
        return numDevices;
    }

    /**
      * \returns true if the specified device is powered
      */
    virtual bool isPowered(byte device) override
    {
        return (device < numDevices) ? ((fPowerMask & (1<<device)) != 0) : false;
    }

    /**
      * Enable/disable power
      * Params :
      * device   The address of the display to control
      * status   If true the device goes into power-down mode. Set to false
      *      for normal operation.
      */
    virtual void setPower(byte device, bool onState, byte count = 1) override
    {
        while (count-- != 0 && device < numDevices)
        {
            bool shutdown = !onState;
            fPowerMask = (onState) ? (fPowerMask | (1<<device)) : (fPowerMask & ~(1<<device));
            spiTransfer(device, kOP_SHUTDOWN, !shutdown);
            device++;
        }
    }

    /** 
      * Set the number of digits (or rows) to be displayed.
      * See datasheet for sideeffects of the scanlimit on the brightness
      * of the display.
      * Params :
      * device   address of the display to control
      * limit    number of digits to be displayed (1..8)
      */
    virtual void setScanLimit(byte device, byte limit, byte count = 1) override
    {
        while (count-- != 0 && device < numDevices && limit < 8)
        {
            spiTransfer(device, kOP_SCANLIMIT, limit);
            device++;
        }
    }

    /** 
      * Set the brightness of the display.
      * Params:
      * device       the address of the display to control
      * intensity    the brightness of the display. (0..15)
      */
    virtual void setIntensity(byte device, byte intensity, byte count = 1) override
    {
        while (count-- != 0 && device < numDevices && intensity < 16)
        {
            spiTransfer(device, kOP_INTENSITY, intensity);
            device++;
        }
    }

    /**
      * Switch all Leds on the display off. 
      * Params:
      * device   address of the display to control
      */
    virtual void clearDisplay(byte device, byte count = 1) override
    {
        while (count-- != 0 && device < numDevices)
        {
            byte* status = &fBits[device * 8];
            for (int i = 1; i < 9; i++, status++)
            {
                if (*status != 0)
                {
                    *status = 0;
                    spiTransfer(device, i, 0);
                }
            }
            device++;
        }
    }

    /**
      * Set the status of a single Led.
      * Params :
      * device   address of the display 
      * row      the row of the Led (0..7)
      * col      the column of the Led (0..7)
      * state    If true the led is switched on, 
      *      if false it is switched off
      */
    virtual void setLed(byte device, int row, int column, boolean state) override
    {
        if (device < numDevices &&
            row >= 0 && row < 8 && column >= 0 && column < 8)
        {
            byte* status = &fBits[device * 8 + row];
            byte val = B10000000 >> column;
            if (state)
            {
                val |= *status;
            }
            else
            {
                val = ~val;
                val &= *status;
            }
            if (*status != val)
            {
                *status = val;
                spiTransfer(device, row + 1, val);
            }
        }
    }

    /**
      * Set all 8 Led's in a row to a new state
      * Params:
      * device   address of the display
      * row      row which is to be set (0..7)
      * value    each bit set to 1 will light up the
      *      corresponding Led.
      */
    virtual void setRow(byte device, int row, byte value) override
    {
        if (device < numDevices && row >= 0 && row < 8)
        {
            byte* status = &fBits[device * 8 + row];
            if (*status != value)
            {
                *status = value;
                spiTransfer(device, row + 1, value);
            }
        }
    }

    /**
      * Set all 8 Led's in a row to a new state (no cache)
      * Params:
      * device   address of the display
      * row      row which is to be set (0..7)
      * value    each bit set to 1 will light up the
      *      corresponding Led.
      */
    virtual void setRowNoCache(byte device, int row, byte value) override
    {
        if (device < numDevices && row >= 0 && row < 8)
        {
            byte* status = &fBits[device * 8 + row];
            *status = value;
            spiTransfer(device, row + 1, value);
        }
    }
    
    /**
      * Set all 8 Led's in a column to a new state
      * Params:
      * device   address of the display
      * column   column which is to be set (0..7)
      * value    each bit set to 1 will light up the
      *      corresponding Led.
      */
    virtual void setColumn(byte device, int column, byte value) override
    {
        byte val;
        if (device < numDevices && column >= 0 && column < 8)
        {
            for (int row = 0; row < 8; row++)
            {
                val = value >> (7 - row);
                val = val & 0x01;
                setLed(device, row, column, val);
            }
        }
    }

    /**
      * \returns the specified row of the specified device
      */
    virtual byte getRow(byte device, byte row) override
    {
        return (device < numDevices && row < 8) ? fBits[device * 8 + row] : 0;
    }

private:
    /* Send out a single command to the device */
    void spiTransfer(byte device, volatile byte opcode, volatile byte data)
    {
        //Create an array with the data to shift out
        int offset = device * 2;
        for (byte i = 0; i < sizeof(fSPIData); i++)
            fSPIData[i] = (byte)0;
        //put our device data into the array
        fSPIData[offset + 1] = opcode;
        fSPIData[offset] = data;
        //enable the line 
        digitalWrite(fSPI_CS, LOW);
        //Now shift out the data 
        for (byte i = sizeof(fSPIData); i > 0; i--)
            shiftOut(fSPI_MOSI, fSPI_CLK, MSBFIRST, fSPIData[i - 1]);
        //latch the data onto the display
        digitalWrite(fSPI_CS, HIGH);
    }

    byte fIndex;
    byte fPowerMask;
    byte fBits[8 * numDevices];
    byte fSPIData[2 * numDevices];
    byte fSPI_MOSI;
    byte fSPI_CLK;
    byte fSPI_CS;

    const int kBrightness = 15;
    enum {
        kOP_DECODEMODE  = 9,
        kOP_INTENSITY   = 10,
        kOP_SCANLIMIT   = 11,
        kOP_SHUTDOWN    = 12,
        kOP_DISPLAYTEST = 15
    };
};

#endif
