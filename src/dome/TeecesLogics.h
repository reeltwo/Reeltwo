
#ifndef TeecesLogics_h
#define TeecesLogics_h

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/CommandEvent.h"
#include "core/AnimatedEvent.h"
#include "core/LedControlMAX7221.h"

/**
  * \ingroup Dome
  *
  * \class TeecesRearLogics
  *
  * \brief Teeces Rear Logics Device
  *
  * Example sketch:
  *
  * \include teecesLogics.ino
  *
  * For further information on the hardware:
  *
  * https://astromech.net/forums/showthread.php?28369-TEECE-s-Light-Kits-(Assembled)-BC-Approved-255-(May-2016)-Open
  */
class TeecesRearLogics : public AnimatedEvent, SetupEvent, CommandEvent
{
public:
    TeecesRearLogics(LedControl& ledControl) :
        fLC(ledControl),
        fID(ledControl.addDevice(3)),
        fDisplayEffect(kNormal),
        fPreviousEffect(~fDisplayEffect),
        fFlipFlop(false),
        fFlipFlopLast(false),
        fStatusDelay(1000),
        fStatusMillis(0),
        fEffectSeqCount(0),
        fEffectSeqDir(0),
        fPrevEffectSeqCount(0),
        fEffectLengthMillis(0),
        fEffectStartMillis(0),
        fDisplayEffectVal(0)
    {
    }

    int numRows() const
    {
        return SizeOfArray(LEDgrid);
    }

    int numColumns() const
    {
        return 27;
    }

    virtual void setup() override
    {
        fLC.setPower(fID, true, NUMDEVICES);
        fLC.setIntensity(fID, 5, NUMDEVICES);

        selectEffect(10000);
        fStatusMillis = millis();
    }

    void selectEffect(long inputNum)
    {
        fDisplayEffectVal = inputNum;
    }

    virtual void handleCommand(const char* cmd) override
    {
        if (*cmd++ == 'R' && *cmd++ == 'L')
        {
            long int cmdvalue = 0;
            const char* c = cmd;
            while (*c >= '0' && *c <= '9')
            {
                cmdvalue = cmdvalue * 10 + (*c++ - '0');
            }
            selectEffect(cmdvalue);
        }
    }

    virtual void animate() override
    {
        bool timerExpired = false;
        unsigned long currentMillis = millis();
        if (currentMillis - fStatusMillis >= fStatusDelay)
        {
            timerExpired = true;
            fStatusMillis = currentMillis;
            fFlipFlop = !fFlipFlop;
            fEffectSeqCount += fEffectSeqDir;
            randomSeed(analogRead(A0));
        }

        int selectSequence = (fDisplayEffectVal % 1000000) / 10000;
        // 100ms - 9s
        int selectSpeed = (fDisplayEffectVal % 10000) / 100;
        int selectLength = (fDisplayEffectVal % 100);

        switch (selectSequence)
        {
            case kNormal:
            case kSolid:
            case kToggle:
            case kFlash:
            case kAlert:
            case kHorizontalScan:
            case kVerticalScan:
            case kSequence:
            case kLife:
                fDisplayEffect = selectSequence;
                break;
            default:
                fDisplayEffect = kNormal;
                break;
        }
        if (fPreviousEffect != fDisplayEffect)
        {
            timerExpired = true;
            fEffectSeqDir = +1;
            fEffectSeqCount = 0;
            fPrevEffectSeqCount = 0;
            fStatusDelay = (selectSpeed) ? 100 * (selectSpeed) : 75;
            fEffectStartMillis = currentMillis;
            fEffectLengthMillis = selectLength * 1000;
            fLC.clearDisplay(fID, NUMDEVICES);
        }
        unsigned int effectMillis = currentMillis - fEffectStartMillis;
        if (timerExpired)
        {
            clear();
            switch (fDisplayEffect)
            {
                case kNormal:
                    /* Do nothing */
                    break;
                case kSolid:
                {
                    for (int dev = 0; dev < 3; dev++)
                    {
                       for (int row = 0; row < 6; row++)
                            fLC.setRowNoCache(fID+dev, row, randomRow(2));
                    }
                    break;
                }
                // case kToggle:
                // {
                //     int i;
                //     for (i = 0; i < 4; i++)
                //         setRow(i, (!fFlipFlop) ? B11111111 : B00000000);
                //     for (; i < 8; i++)
                //         setRow(i, (!fFlipFlop) ? B00000000 : B11111111);
                //     break;
                // }
                // case kFlash:
                // {
                //     if (!fFlipFlop)
                //     {
                //         for (int i = 0; i < 8; i++)
                //             setRow(i, B11111111);
                //     }
                //     else
                //     {
                //         for (int i = 0; i < 8; i++)
                //             setRow(i, B00000000);
                //     }
                //     break;
                // }
                // case kHorizontalScan:
                // {
                //     if (fPrevEffectSeqCount != fEffectSeqCount)
                //         setCol(fPrevEffectSeqCount, 0);
                //     if (fEffectSeqCount > 7)
                //     {
                //         fEffectSeqDir = -1;
                //         fEffectSeqCount += fEffectSeqDir;
                //     }
                //     else if (fEffectSeqCount < 0)
                //     {
                //         fEffectSeqDir = +1;
                //         fEffectSeqCount += fEffectSeqDir;
                //     }
                //     setCol(fEffectSeqCount, 1);
                //     break;
                // }
                // case kVerticalScan:
                // {
                //     if (fPrevEffectSeqCount != fEffectSeqCount)
                //         setRow(fPrevEffectSeqCount, B00000000);
                //     if (fEffectSeqCount > 7)
                //     {
                //         fEffectSeqDir = -1;
                //         fEffectSeqCount += fEffectSeqDir;
                //     }
                //     else if (fEffectSeqCount < 0)
                //     {
                //         fEffectSeqDir = +1;
                //         fEffectSeqCount += fEffectSeqDir;
                //     }
                //     setRow(fEffectSeqCount, B11111111);
                //     break;
                // }
                // case kSequence:
                // {
                //     int frameCount;
                //     const byte* ptr = getFrame_Q(fEffectSeqCount, frameCount);
                //     for (int i = 0; i < 8; i++, ptr++)
                //     {
                //         setRow(i, pgm_read_byte(ptr));
                //     }
                //     break;
                // }
                // case kLife:
                // {
                //     if (fPreviousEffect != fDisplayEffect)
                //     {
                //         for (int i = 0; i < 8; i++)
                //             setRow(i, random(255));
                //     }
                //     play();
                //     bool empty = true;
                //     for (int y = 0; y < 8; y++)
                //     {
                //         if (getRow(y) != 0)
                //         {
                //             empty = false;
                //             break;
                //         }
                //     }
                //     if (empty)
                //     {
                //         selectEffect(30000 + 100 + 2);
                //     }
                //     else
                //     {
                //         setPixel(random(8), random(8), 1);
                //     }
                //     break;
                // }
            }
            fPrevEffectSeqCount = fEffectSeqCount;
        }
        if (fEffectLengthMillis > 0 && fEffectLengthMillis < effectMillis)
        {
            selectEffect(kNormalVal); //go back to normal operation if its time
        }
        fPreviousEffect = fDisplayEffect;
        // show();
    }

    enum
    {
        kNormalVal = 0
    };

    enum
    {
        kNormal = 0,
        kSolid = 1,
        kToggle = 2,
        kFlash = 3,
        kAlert = 4,
        kHorizontalScan = 5,
        kVerticalScan = 6,
        kSequence = 7,
        kLife = 8
    };

private:
    static const byte NUMDEVICES = 3;
    LedControl& fLC;
    byte fID;
    unsigned long fDisplayEffect;
    unsigned long fPreviousEffect;
    bool fFlipFlop; 
    bool fFlipFlopLast;
    unsigned long fStatusDelay;
    unsigned long fStatusMillis;
    int fEffectSeqCount;
    int fEffectSeqDir;
    int fPrevEffectSeqCount;
    unsigned int fEffectLengthMillis;
    unsigned long fEffectStartMillis;
    unsigned long fDisplayEffectVal;
    unsigned long LEDgrid[5]; 

    void setRow(byte row, uint32_t bits)
    {
        if (row < SizeOfArray(LEDgrid))
            LEDgrid[row] = bits;
    }

    void clear()
    {
        for (byte row = 0; row < SizeOfArray(LEDgrid); row++)
        {
            LEDgrid[row] = 0L;
        }
    }

    void setCol(byte col, uint8_t data)
    {
        for (byte row = 0; row < SizeOfArray(LEDgrid); row++)
        {
           // test if LED is on
            byte LEDon = (data & 1<<row);
            if (LEDon)
                LEDgrid[4-row] |= (1L << col);  // set column bit
            else
                LEDgrid[4-row] &= ~(1L << col); // reset column bit
        }
    }

    static uint8_t rev(uint8_t n)
    {
        // byte reversal fast RAM lookup table
        static uint8_t revlookup[] = {
            0x0, 0x8, 0x4, 0xC,
            0x2, 0xA, 0x6, 0xE,
            0x1, 0x9, 0x5, 0xD,
            0x3, 0xB, 0x7, 0xF
        };
        return (revlookup[n & 0x0F] << 4) | revlookup[n >> 4];
    }

    byte randomRow(int loop)
    {
        loop += random(2);
        byte val = random(256);
        while (loop-- > 0)
            val |= random(256);
        return (random(2) == 1) ? rev(val) : val;
    }

    void show()
    {
        // Every 9th column of the displays maps to the 6th row of the Maxim chip
        unsigned char col8 = 0;  // 9th column of FLDs and RLD, maps to 6th row of device 0
        unsigned char col17 = 0; // 18th column of RLD, goes to 6th row of RLD device 1
        unsigned char col26 = 0; // 27th column of RLD, goes to 6th row of RLD device 2 

        // Colums 0-7 map with a byte reversal
        for (byte row = 0; row < SizeOfArray(LEDgrid); row++)
        {
            for (int dev = 0; dev < 3; dev++) // RLD has 3 Maxim chip devices
            {
                // extract byte at column 0, 9 and 18, reverse byte order, and send to device
                fLC.setRowNoCache(fID+dev, row, rev((LEDgrid[row] & 255L << (9 * dev)) >> (9 * dev)));
            }
            // If the LED at column 8, 17 or 26 is on, add it to the extra row (starting "left" at MSB)
            if ((LEDgrid[row] & 1L<<8) == 1L<<8)   col8 += 128 >> row;
            if ((LEDgrid[row] & 1L<<17) == 1L<<17) col17 += 128 >> row;
            if ((LEDgrid[row] & 1L<<26) == 1L<<26) col26 += 128 >> row;
        }
        // send the extra columns as a 6th row or the Maxim (logical row 5)
        fLC.setRowNoCache(fID+0, 5, col8);
        fLC.setRowNoCache(fID+1, 5, col17);
        fLC.setRowNoCache(fID+2, 5, col26);
    }
};

/**
  * \ingroup Dome
  *
  * \class TeecesFrontLogics
  *
  * \brief Teeces Front Logics Device
  *
  * Controller class for Teeces style Front Logics.
  *
  * Example sketch:
  *
  * \include teecesLogics.ino
  *
  * For further information on the hardware:
  *
  * https://astromech.net/forums/showthread.php?28369-TEECE-s-Light-Kits-(Assembled)-BC-Approved-255-(May-2016)-Open
  */
class TeecesFrontLogics : public AnimatedEvent, SetupEvent, CommandEvent
{
public:
    TeecesFrontLogics(LedControl& ledControl) :
        fLC(ledControl),
        fID(ledControl.addDevice()),
        fDisplayEffect(kNormal),
        fPreviousEffect(~fDisplayEffect),
        fFlipFlop(false),
        fFlipFlopLast(false),
        fStatusDelay(1000),
        fStatusMillis(0),
        fEffectSeqCount(0),
        fEffectSeqDir(0),
        fPrevEffectSeqCount(0),
        fEffectLengthMillis(0),
        fEffectStartMillis(0),
        fDisplayEffectVal(0)
    {
    }

    int numRows() const
    {
        return SizeOfArray(LEDgrid);
    }

    int numColumns() const
    {
        return 27;
    }

    virtual void setup() override
    {
        fLC.setPower(fID, true);
        fLC.setIntensity(fID, 5);

        selectEffect(10000);
        fStatusMillis = millis();
    }

    void selectEffect(long inputNum)
    {
        fDisplayEffectVal = inputNum;
    }

    /**
      * Command Prefix: FL (top and bottom logics)
      * Command Prefix: TL (top logics)
      * Command Prefix: BL (bottom logics)
      *
      */
    virtual void handleCommand(const char* cmd) override
    {
        if ((cmd[0] == 'F' && cmd[1] == 'L') ||
            (cmd[0] == 'T' && cmd[1] == 'L') ||
            (cmd[0] == 'B' && cmd[1] == 'L'))
        {
            long int cmdvalue = 0;
            const char* c = &cmd[2];
            while (*c >= '0' && *c <= '9')
            {
                cmdvalue = cmdvalue * 10 + (*c++ - '0');
            }
            selectEffect(cmdvalue);
        }
    }

    virtual void animate() override
    {
        bool timerExpired = false;
        unsigned long currentMillis = millis();
        if (currentMillis - fStatusMillis >= fStatusDelay)
        {
            timerExpired = true;
            fStatusMillis = currentMillis;
            fFlipFlop = !fFlipFlop;
            fEffectSeqCount += fEffectSeqDir;
            randomSeed(analogRead(A0));
        }

        int selectSequence = (fDisplayEffectVal % 1000000) / 10000;
        // 100ms - 9s
        int selectSpeed = (fDisplayEffectVal % 10000) / 100;
        int selectLength = (fDisplayEffectVal % 100);

        switch (selectSequence)
        {
            case kNormal:
            case kSolid:
            case kToggle:
            case kFlash:
            case kAlert:
            case kHorizontalScan:
            case kVerticalScan:
            case kSequence:
            case kLife:
                fDisplayEffect = selectSequence;
                break;
            default:
                fDisplayEffect = kNormal;
                break;
        }
        if (fPreviousEffect != fDisplayEffect)
        {
            timerExpired = true;
            fEffectSeqDir = +1;
            fEffectSeqCount = 0;
            fPrevEffectSeqCount = 0;
            fStatusDelay = (selectSpeed) ? 100 * (selectSpeed) : 75;
            fEffectStartMillis = currentMillis;
            fEffectLengthMillis = selectLength * 1000;
            fLC.clearDisplay(fID);
        }
        unsigned int effectMillis = currentMillis - fEffectStartMillis;
        if (timerExpired)
        {
            clear();
            switch (fDisplayEffect)
            {
                case kNormal:
                    /* Do nothing */
                    break;
                case kSolid:
                {
                    for (int dev = 0; dev < 3; dev++)
                    {
                       for (int row = 0; row < 6; row++)
                            fLC.setRowNoCache(fID+dev, row, randomRow(2));
                    }
                    break;
                }
                // case kToggle:
                // {
                //     int i;
                //     for (i = 0; i < 4; i++)
                //         setRow(i, (!fFlipFlop) ? B11111111 : B00000000);
                //     for (; i < 8; i++)
                //         setRow(i, (!fFlipFlop) ? B00000000 : B11111111);
                //     break;
                // }
                // case kFlash:
                // {
                //     if (!fFlipFlop)
                //     {
                //         for (int i = 0; i < 8; i++)
                //             setRow(i, B11111111);
                //     }
                //     else
                //     {
                //         for (int i = 0; i < 8; i++)
                //             setRow(i, B00000000);
                //     }
                //     break;
                // }
                // case kHorizontalScan:
                // {
                //     if (fPrevEffectSeqCount != fEffectSeqCount)
                //         setCol(fPrevEffectSeqCount, 0);
                //     if (fEffectSeqCount > 7)
                //     {
                //         fEffectSeqDir = -1;
                //         fEffectSeqCount += fEffectSeqDir;
                //     }
                //     else if (fEffectSeqCount < 0)
                //     {
                //         fEffectSeqDir = +1;
                //         fEffectSeqCount += fEffectSeqDir;
                //     }
                //     setCol(fEffectSeqCount, 1);
                //     break;
                // }
                // case kVerticalScan:
                // {
                //     if (fPrevEffectSeqCount != fEffectSeqCount)
                //         setRow(fPrevEffectSeqCount, B00000000);
                //     if (fEffectSeqCount > 7)
                //     {
                //         fEffectSeqDir = -1;
                //         fEffectSeqCount += fEffectSeqDir;
                //     }
                //     else if (fEffectSeqCount < 0)
                //     {
                //         fEffectSeqDir = +1;
                //         fEffectSeqCount += fEffectSeqDir;
                //     }
                //     setRow(fEffectSeqCount, B11111111);
                //     break;
                // }
                // case kSequence:
                // {
                //     int frameCount;
                //     const byte* ptr = getFrame_Q(fEffectSeqCount, frameCount);
                //     for (int i = 0; i < 8; i++, ptr++)
                //     {
                //         setRow(i, pgm_read_byte(ptr));
                //     }
                //     break;
                // }
                // case kLife:
                // {
                //     if (fPreviousEffect != fDisplayEffect)
                //     {
                //         for (int i = 0; i < 8; i++)
                //             setRow(i, random(255));
                //     }
                //     play();
                //     bool empty = true;
                //     for (int y = 0; y < 8; y++)
                //     {
                //         if (getRow(y) != 0)
                //         {
                //             empty = false;
                //             break;
                //         }
                //     }
                //     if (empty)
                //     {
                //         selectEffect(30000 + 100 + 2);
                //     }
                //     else
                //     {
                //         setPixel(random(8), random(8), 1);
                //     }
                //     break;
                // }
            }
            fPrevEffectSeqCount = fEffectSeqCount;
        }
        if (fEffectLengthMillis > 0 && fEffectLengthMillis < effectMillis)
        {
            selectEffect(kNormalVal); //go back to normal operation if its time
        }
        fPreviousEffect = fDisplayEffect;
        // show();
    }

    enum
    {
        kNormalVal = 0
    };

    enum
    {
        kNormal = 0,
        kSolid = 1,
        kToggle = 2,
        kFlash = 3,
        kAlert = 4,
        kHorizontalScan = 5,
        kVerticalScan = 6,
        kSequence = 7,
        kLife = 8
    };

private:
    LedControl& fLC;
    byte fID;
    unsigned long fDisplayEffect;
    unsigned long fPreviousEffect;
    bool fFlipFlop; 
    bool fFlipFlopLast;
    unsigned long fStatusDelay;
    unsigned long fStatusMillis;
    int fEffectSeqCount;
    int fEffectSeqDir;
    int fPrevEffectSeqCount;
    unsigned int fEffectLengthMillis;
    unsigned long fEffectStartMillis;
    unsigned long fDisplayEffectVal;
    unsigned long LEDgrid[5]; 

    void setRow(byte row, uint32_t bits)
    {
        if (row < SizeOfArray(LEDgrid))
            LEDgrid[row] = bits;
    }

    void clear()
    {
        for (byte row = 0; row < SizeOfArray(LEDgrid); row++)
        {
            LEDgrid[row] = 0L;
        }
    }

    void setCol(byte col, uint8_t data)
    {
        for (byte row = 0; row < SizeOfArray(LEDgrid); row++)
        {
           // test if LED is on
            byte LEDon = (data & 1<<row);
            if (LEDon)
                LEDgrid[4-row] |= (1L << col);  // set column bit
            else
                LEDgrid[4-row] &= ~(1L << col); // reset column bit
        }
    }

    static uint8_t rev(uint8_t n)
    {
        // byte reversal fast RAM lookup table
        static uint8_t revlookup[] = {
            0x0, 0x8, 0x4, 0xC,
            0x2, 0xA, 0x6, 0xE,
            0x1, 0x9, 0x5, 0xD,
            0x3, 0xB, 0x7, 0xF
        };
        return (revlookup[n & 0x0F] << 4) | revlookup[n >> 4];
    }

    byte randomRow(int loop)
    {
        loop += random(2);
        byte val = random(256);
        while (loop-- > 0)
            val |= random(256);
        return (random(2) == 1) ? rev(val) : val;
    }

    void show()
    {
        // Every 9th column of the displays maps to the 6th row of the Maxim chip
        unsigned char col8 = 0;  // 9th column of FLDs and RLD, maps to 6th row of device 0
        unsigned char col17 = 0; // 18th column of RLD, goes to 6th row of RLD device 1
        unsigned char col26 = 0; // 27th column of RLD, goes to 6th row of RLD device 2 

        // Colums 0-7 map with a byte reversal
        for (byte row = 0; row < SizeOfArray(LEDgrid); row++)
        {
            for (int dev = 0; dev < 3; dev++) // RLD has 3 Maxim chip devices
            {
                // extract byte at column 0, 9 and 18, reverse byte order, and send to device
                fLC.setRowNoCache(fID+dev, row, rev((LEDgrid[row] & 255L << (9 * dev)) >> (9 * dev)));
            }
            // If the LED at column 8, 17 or 26 is on, add it to the extra row (starting "left" at MSB)
            if ((LEDgrid[row] & 1L<<8) == 1L<<8)   col8 += 128 >> row;
            if ((LEDgrid[row] & 1L<<17) == 1L<<17) col17 += 128 >> row;
            if ((LEDgrid[row] & 1L<<26) == 1L<<26) col26 += 128 >> row;
        }
        // send the extra columns as a 6th row or the Maxim (logical row 5)
        fLC.setRowNoCache(fID+0, 5, col8);
        fLC.setRowNoCache(fID+1, 5, col17);
        fLC.setRowNoCache(fID+2, 5, col26);
    }
};

/**
  * \ingroup Dome
  *
  * \class TeecesRLD
  *
  * \brief Alias for TeecesRearLogics
  */
typedef TeecesRearLogics TeecesRLD;
/**
  * \ingroup Dome
  *
  * \class TeecesTFLD
  *
  * \brief Alias for TeecesFrontLogics
  */
typedef TeecesFrontLogics TeecesTFLD;
/**
  * \ingroup Dome
  *
  * \class TeecesBFLD
  *
  * \brief Alias for TeecesFrontLogics
  */
typedef TeecesFrontLogics TeecesBFLD;

#endif

#if 0

#include <avr/pgmspace.h>
#include <TeecesControl.h>

// =======================================================================================
//  CuriousMarc Teeces Code
//  v1.3
//  Corrected numbers 4 to 9 in Latin Alphabet. Have not done the Aurabesh yet.
//  v1.2
//   - Updated to "const int PROGMEM" (added const) definitions to satisfy new compiler error
//   - Changed timing measures to unsigned long to correct compiler warning
//   - Changed setText(char*, char*) to (const char*, const char*) to satisfy compiler deprecation warning
//  v1.1
//  11/28/2013
//  Teeces code that runs with R2 Touch and MarcDuino's, as replacement for the JEDI
//  Works also better as regular Teeces code:
//  - more realistic random RLD animation, adjustable
//  - independent text on all logics
//  - bottom FLD text flipped right side up
//  - two alphabets: English and Aurabesh
//  - the two PSI are completely independent (they used to switch at the same time)
//  - Randomly "stuck" PSI effect
//  - flash, leia, march, star wars and march effects
//  - implements extended JawaLite command set for external control by an Arduino
//  - uses optimized TeecesControl library (instead of LedControl) to save on memory
// =======================================================================================
//                                 
// Some of this code inspired from the following sources
// John Vannoy, DanF, Paul Murphy, BigHappyDude
//
// Required Libraries: 
//   TeecesControl
//
// Logic Display and PSI Boards should be wired up in two chains.
// Teeces board V3.1 has two headers to facilitate this.
//   OUT1 should be connected to the Rear PSI (which adds it after the RLD to the "rear" chain)
//   OUT2 should be connected to the Top FLD, bottom FLD, then Front PDI (this is the "front" chain)
// 
// If you're using the older V3 RLD you don't have the OUT2 pins, connect it like this:
//   RLD OUT should be connected to the Rear PSI (which adds it after the RLD to the front chain)
//   Use Arduino Pins 9,8,7 to connect to top FLD IN D,C,L, then Bottom FLD then Front PSI
//   (you will also need to supply +5V and GND to the front chain; it can go to any pins
//   labeled +5V and GND on any of the FLD or front PSI boards)
//
// BOARDtype sets which Arduino we're using
// 1 = Arduino Pro Mini or Uno or Duemilanove ( http://arduino.cc/en/Main/ArduinoBoardProMini )
// 2 = Sparkfun Pro Micro ( https://www.sparkfun.com/products/11098 )
// 3 = Arduino Micro ( http://arduino.cc/en/Main/ArduinoBoardMicro )
//
//  This program is free software: you can redistribute it and/or modify it .
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.


// Type of Arduino you are using
// 1 = Arduino Pro Mini or Uno or Duemilanove (the cleanest option, but you need an FTDI adapter)
// 2 = Sparkfun Pro Micro (but be ready to deal with USB and upload issues)
// 3 = Arduino Micro (but be ready to deal with a baord that's too big)
#define BOARDtype 1

// PSItype sets the type of our front and rear PSI's
// 1 = Teeces original (6 LEDs of each color, arranged side by side)
// 2 = Teeces original checkerboard (6 LEDs of each color arranged in a checkerboard pattern)
// 3 = Teeces V3.2 PSI by John V (13 LEDs of each color, arranged side by side)
// 4 = Teeces V3.2 PSI checkerboard by John V  (13 LEDs of each color, in a checkerboard pattern)
#define PSItype 4

// Baud Rate sets the baud rate of the serial connection. Current MarcDuino HP Firmware (v1.5)
// uses the JEDI default rate which is very slow at 2400. 
// If you control it from something else, you probably want to use 9600.
// The serial input connects to the pin marked RXI or Rx on the Arduino (and optionally TXO or Tx for answer messages)
#define BAUDRATE 2400

// Startup Text
char TFLDtext[] =  "R2-D2  "; //PUT YOUR TOP FRONT STARTUP TEXT HERE.
char BFLDtext[] =  "     ASTROMECH"; //PUT YOUR BOTTOM FRONT STARTUP TEXT HERE.
char RLDtext[] = "CURIOUSMARC SKETCH V1.2  "; //PUT YOUR REAR STARTUP TEXT HERE.

//#define TESTLOGICS //uncomment to turns on all logic LEDs at once, useful for troubleshooting

// Most builders shouldn't have to edit anything below here. 
// =======================================================================================

/************
////////////////////////////////////////////////////////
// Command language reference - JEDI JawaLite emulation
///////////////////////////////////////////////////////

JawaLite command syntax uses plain text format, followed by carriage return. Format is address - command letter - value:
xxYzzz<cr>
where xx= address (one or two chars), Y=command letter, zzz=optional argument, <cr>= carriage return character ('\r'=0x13)

Address field is interpreted as follows:
0 - global address, all displays that support the command are set
1 - TFLD (Top Front Logic Dislay)
2 - BFLD (Bottom Front Logic Display)
3 - RLD  (Rear Logic Display)
4 - Front PSI
5 - Rear PSI
6 - Front Holo (not implemented here)
7 - Rear Holo  (not implemented here)
8 - Top Holo   (not implemented here)

Implemented commands (indicated durations are the sketch defaults, they are adjustable, see below):

Command T
0-5T0    - test (switches all logics to on)
0-5T1    - display to normal random
0T2  - flah, same as alarm
0T3  - alarm for 4 seconds (flashes displays on and off)
0T4  - short circuit (10 seconds sequence)
0T5  - scream, same as alarm
0T6  - leia, 34 seconds (moving horizontal bar)
0T10     - Star Wars. Displays "Star Wars" on RLD, "STARS" on TFLD, "WARS" on BFLD
0T11     - March (alternating halfs of logics, 47 seconds)
0-3T92   - spectrum, bargraph displays. Runs forever, reset by calling to 0T1
0-3T100  - Text displays. Displays text set by the M command below
6T1, 7T1, 8T1 -> holos on  - not implemented, HP lights controlled by MarcDuino HP.
0D, 6D, 7D, 8D -> holos off  - not implemented, HP lights controlled by MarcDuino HP.

Command M, set message for the text. 0= all logics, 1=TFLD, 2=BFLD, 3=RLD
0-3Mmessage - where "message" is an ASCII string to be displayed. Only uppercase implemented.

Command P, character set
0-3P61  - swtich display to aurabesh characters
0-3P60  - switch display to latin (e.g.english) characters

Extended "CuriousMarc Teeces" commands:

Extra command T to turn displays off
0-5T20

Command R, adjusts random style:
0-3Rx where x=0 to 6.

Command S, PSI state
0,4-5S0  - PSI all on (same as 4-5T0)
0,4-5S1  - PSI normal mode (same as 4-5T1)
0,4-5S2  - PSI first color
0,4-5S3  - PSI second color
0,4-5S4  - PSI off


********************/

// =======================================================================================
// You can tweak the effects by changing the numbers below
// =======================================================================================

// set brightness levels here (a value of 0-15)
#define RLDbrightness    5   //rear Logic
#define RPSIbrightness   15  //rear PSI
#define FLDbrightness    5   //front Logics
#define FPSIbrightness   15  //front PSI

// time in ms between logic display updates (lower = blink faster)
#define LOGICupdateDelay 75

// default random blinking styles of the logics. 
// Styles 0 to 2 are for special effects (I used them in Failure)
// Style 3 is legacy Teeces ("blocky random", about 50% off, most LED changing at a time)
// Styles 4 and 5 are recommended, more organic random styles (not all LEDs change at the same time, more LEDs on)
// Stlye 6 is most LEDs on (special effect). 
// 0= almost all off, 1= most off, 2= some off, 3= legacy random, 4= stage 1 organic, 5= stage 2 organic, 6 = stage 3 organic
// This is the default starting style. It can be adjusted on the fly by sending the R JawaLite command.
#define LOGICRandomStyle 4

// update speed of bargraph display
#define BARGRAPHupdateDelay 50

// Delay (in ms) between color wipe steps when PSI changes color (lower= wipe faster)
#define PSIwipeDelay  70

// You can make the front or rear PSI get stuck inbetween colors from time to time by setting these parameters to 1 (0 for normal operation
#define FPSIgetsStuck 0
#define RPSIgetsStuck 0

// parameters for how long and how often it gets stuck
#define PSIstuckhowlong 10000  // stuck time in ms
#define PSIstuckhowoften 5     // how often. 5 means 1 out of 5 swipes on average. So the higher the less often.

// text scroll speed (in ms, lower= scroll faster)
#define SCROLLspeed 48

// Leia effect parameters
#define LEIAduration  34000 // in ms
#define LEIAspeed  100 //2200         // in no particular units, lower=faster

// Alarm effect parameters
#define ALARMduration 4000 // in ms
#define ALARMspeed   100  // in ms

// March effect parameters
#define MARCHduration 47000 // in ms
#define MARCHspeed   555  // in ms. Adjusted to the tempo of 108 of my March recording (60/108=555ms). 

// Failure effect parameters
#define FAILUREduration 10000
#define FAILUREloops 5
#define FAILUREspeed 75

// =========================================================================
// Only make changes below here if you know what you are doing

/*
Different Arduino's have different pin numbers that are used for the rear chains.
 Arduino Pro Mini uses pins 12,11,10 for rear D,C,L
 Sparkfun Pro Micro uses pins 14,16,10 for rear D,C,L
 Arduino Micro uses pins A2,A1,A0 for rear D,C,L
*/

#if (BOARDtype==1) 
 #define DVAL 12
 #define CVAL 11
 #define LVAL 10
#elif (BOARDtype==2) 
 #define DVAL 14
 #define CVAL 16
 #define LVAL 10
#elif (BOARDtype==3) 
 #define DVAL A2
 #define CVAL A1
 #define LVAL A0
#endif

#define FDEV 3    //3 devices for front chain (top FLD, bottom FLD, front PSI
#define FPSIDEV 2 //front PSI is device #2 in the front chain
#define RPSIDEV 3 //rear PSI is device #3 in the rear chain

#define LETTERWIDTH 5 // letter with in pixels
#define MAXSTRINGSIZE 64  // maximum number of letters in a logic display message
#define CMD_MAX_LENGTH 64 // maximum number of characters in a command (63 chars since we need the null termination)

///////global variables////////////////////////////

// This is the LED grid for the 3 logic displays. 
// Since an "unsigned long" is 32 bits wide in gcc, this gives
// 3 grids of 5 rows x 32 bits (which could control 5 rows of 32 LEDs)
// Grid 0 if used for the top FLD, uses only the first 5x9 LEDs/bits
// Grid 1 is used for the bottom FLD, uses only the first 5x9 LEDs/bits
// Grid 3 is used for the RLD, uses only 5x27 LEDs/bits
// This LED grid is sent to the actual display in the function showGrid(display)
unsigned long LEDgrid[3][5]; 

// starting values for text scrolling positions on each display.
int scrollPositions[]={9,9,27};

// textScrollCount
// 0,1, and 2 are used in scrollText for each display, counts how many times the string has completely scrolled
long textScrollCount[3];

// alphabet 0= english, 1= aurabesh
#define LATIN 0
#define AURABESH 1
byte alphabetType[3];

// display mode
#define NORM 0
#define ALARM 1
#define MARCH 2
#define LEIA 3
#define FAILURE 4
byte displayEffect; // 0 = normal, 1 = alarm, 2 = march, 3 = leia, 4 = failure

// display state for the logics, 0=normal random, 1=text display, 2=bargraph, 3=test, 4=off 
#define RANDOM 0
#define TEXT 1
#define BARGRAPH 2
#define TEST 3
#define OFF 4
byte displayState[3];

// display state for the PSI, 0=normal random, 1=color1, 2=color2, 3=test, 4=off (0, 3 and 4 are shared with displays above)
#define COLOR1 1
#define COLOR2 2
byte psiState[2];

#define BLUE         COLOR1
#define RED          COLOR2
#define YELLOW       COLOR1
#define GREEN        COLOR2

// start with the default display random mode. This can be altered via extended JawaLite commands.
byte randomStyle[3]={LOGICRandomStyle, LOGICRandomStyle, LOGICRandomStyle};

// tracks if we are in the midst of running an effect
byte effectRunning;

// memory for the text strings to be displayed, add one for the NULL character at the end
char logicText[3][MAXSTRINGSIZE+1];

// memory for command string processing
char cmdString[CMD_MAX_LENGTH];

// Create Maxim devices SPI driver objects for front and rear chains
LedControl lcRear=LedControl(DVAL,CVAL,LVAL,4);  //rear chain, 3 devices for RLD, one for the rear PSI
LedControl lcFront=LedControl(9,8,7,FDEV);       //front chain, 3 devices: TFLD, BFLD, front PSI

// handle to the Serial object, since it's a different one Micro than Minis
HardwareSerial* serialPort;

// the array psiPatterns is used to program the psi wipe
// The first five elements are the LED values for the first color for each row.
// The second five elements are the LED values for the second color for each row.
#if (PSItype==4)  
#define HPROW 5
#define HPPAT1 B10101010
#define HPPAT2 B01010101
int psiPatterns[]={HPPAT1,HPPAT2,HPPAT1,HPPAT2,HPPAT1,HPPAT2,HPPAT1,HPPAT2,HPPAT1,HPPAT2};
#elif (PSItype==3)  
#define HPROW 5
#define HPPAT1 B11100000
#define HPPAT2 B00011111
int psiPatterns[]={HPPAT1,HPPAT1,HPPAT1,HPPAT1,HPPAT1,HPPAT2,HPPAT2,HPPAT2,HPPAT2,HPPAT2};
#elif (PSItype==2) 
#define HPROW 4
#define HPPAT1 B10101000
#define HPPAT2 B01010100
int psiPatterns[]={HPPAT1,HPPAT2,HPPAT1,HPPAT2,HPPAT1,HPPAT2,HPPAT1,HPPAT2,HPPAT1,HPPAT2};
#elif (PSItype==1)
#define HPROW 4
#define HPPAT1 B11000000
#define HPPAT2 B00110000
int psiPatterns[]={HPPAT1,HPPAT1,HPPAT1,HPPAT1,HPPAT1,HPPAT2,HPPAT2,HPPAT2,HPPAT2,HPPAT2};
#endif



// =======================================================================================

//////////////////
// Initialization
void setup() 
{
  // Since the serial names are different on different platforms, need to access through a universal pointer instead
  // Sprakfun Pro Micro and Arduino Micro are based on Atmega32u4 chips, hardware serial is Serial1
  #if (BOARDtype==2 || BOARDtype==3) 
  Serial1.begin(BAUDRATE);
  serialPort=&Serial1;
  #else
  // Arduino Pro Mini is based on an Atmega328, hardware serial is Serial
  Serial.begin(BAUDRATE);
  serialPort=&Serial;
  #endif
  
  // Serial port welcome string, using the universal serialPort pointer
  serialPort->println();
  serialPort->println();
  serialPort->println("--- CuriousMarc Teeces v1.0 ---");

  randomSeed(analogRead(0));
  
 // exit shutdown of each chip and clear displays
  for(int dev=0;dev<lcRear.getDeviceCount();dev++) 
  {
    lcRear.shutdown(dev, false); //take the device out of shutdown (power save) mode
    lcRear.clearDisplay(dev);
  }
  for(int dev=0;dev<lcFront.getDeviceCount();dev++) 
  {
    lcFront.shutdown(dev, false); //take the device out of shutdown (power save) mode
    lcFront.clearDisplay(dev);
  }
  
  //set intensity of devices in  rear chain...
  lcRear.setIntensity(0, RLDbrightness); //RLD
  lcRear.setIntensity(1, RLDbrightness); //RLD
  lcRear.setIntensity(2, RLDbrightness); //RLD
  lcRear.setIntensity(3, RPSIbrightness); //Rear PSI
  
  //set intensity of devices in front chain...
  for(int dev=0;dev<(lcFront.getDeviceCount()-1);dev++) 
  {
    lcFront.setIntensity(dev, FLDbrightness);  //front logics (all but the last dev in chain)
  }
  lcFront.setIntensity(FPSIDEV, FPSIbrightness); //Front PSI
  
  //HP lights on constantly. 
  //lcRear.setRow(3,4,255); //rear psi
  //lcFront.setRow(FPSIDEV,4,255); //front psi
  
  //startup scrolling text. This function blocks until all the strings have scrolled completely off-screen.
  while((textScrollCount[0]<1) || (textScrollCount[1]<1) || (textScrollCount[2]<1)) // keep scrolling until they have all scrolled once
  {
    if (textScrollCount[0]<1) scrollText(0,TFLDtext);  //top front text
    if (textScrollCount[1]<1) scrollText(1,BFLDtext);  //bottom front text
    if (textScrollCount[2]<1) scrollText(2,RLDtext);   //rear text
  }

  // Scrolling is done, switch to random mode
  resetDisplays();
  
  // Ready to accept commands
  serialPort->println("Ready for command.");
  serialPort->println();
  serialPort->print("> ");
  
  /***************************
  //------------------------------------------------------------
  // For code testing, throw the displays into a particular mode 
  //------------------------------------------------------------
  
  // test strings for logics
  setText(0,"TFLD TEST    ");
  setText(1,"BFLD TEST    ");
  setText(2,"RLD TEST    ");
  
  // displayEffect test
  // 0=normal
  // 5=leia
  // 11= march
  // 1= alarm
  displayEffect=0;
  
  // alphabet selection test
  alphabetType[0]=0;
  alphabetType[1]=0;
  alphabetType[2]=0;
  
  // text/random state test
  displayState[0]=1;
  displayState[1]=1;
  displayState[2]=1;
  
  //--------end test--------
  ******************************/

}

///////////////////
// Main Loop
void loop() 
{

  // listen to commands through the serial port
  if(serialPort->available())
  {
    char ch;
    byte command_available;
    
    ch=serialPort->read();  // get input
    serialPort->print(ch);  // echo back
    command_available=buildCommand(ch, cmdString);  // build command line
    if (command_available) 
    {
      parseCommand(cmdString);  // interpret the command
      serialPort->println();    // prompt again
      serialPort->print("> ");
    }
  }
  
  // PSIs are always random
  // randomFPSI();
  //setFPSI(0);
  //randomRPSI();
  //setRPSI(GREEN);
  
  // Logics can be 
  // - in a global effect mode (all displays involved)
  // - in normal individual, each display can be independtly in one  of the following state:
  //     - in random display
  //     - in text display
  //     - in bargraph display
  //     - all on (test)
  //     - all off

  switch(displayEffect)
  {
    // go to specific effect
    case 1:
      alarmDisplay(ALARMduration);
      break;
    case 2:
      marchDisplay(MARCHduration);
      break;
    case 3:
      leiaDisplay(LEIAduration);
      break;
    case 4:
      failureDisplay(FAILUREduration);
      break;
   default:   // default is random, text or test depending on each displayState
      // cycle for each display
      for(byte disp=0; disp<3; disp++)
      {
        switch(displayState[disp])
        {
          case RANDOM:
            randomDisplay(disp);
            break;
          case TEXT:
            textDisplay(disp);
            break;
          case TEST:
            testDisplay(disp);
            break;
          case OFF:
            offDisplay(disp);
            break;
          case BARGRAPH:
            bargraphDisplay(disp);
            break;            
          default: // unknown mode reverts to random display
            randomDisplay(disp);
            break;
        }  
      }
      switch(psiState[0])
      {
        case RANDOM:
          randomFPSI();
          break;
        case COLOR1: case COLOR2: case OFF: case TEST:
          setFPSI(psiState[0]);   
          break;
        default:
          randomFPSI();
          break;
      }
      switch(psiState[1])
      {
        case RANDOM:
          randomRPSI();
          break;
        case COLOR1: case COLOR2: case OFF: case TEST:
          setRPSI(psiState[1]);   
          break;
        default:
          randomRPSI();
          break;
      }      
  }
}

////////////////////////////////////////////////////////
// Command language - JawaLite emulation
///////////////////////////////////////////////////////


////////////////////////////////
// command line builder, makes a valid command line from the input
byte buildCommand(char ch, char* output_str)
{
  static uint8_t pos=0;
  switch(ch)
 {
    case '\r':                          // end character recognized
      output_str[pos]='\0';     // append the end of string character
      pos=0;                // reset buffer pointer
      return true;          // return and signal command ready
      break;
    default:                // regular character
      output_str[pos]=ch;       // append the  character to the command string
      if(pos<=CMD_MAX_LENGTH-1)pos++;   // too many characters, discard them.
      break;
  }
  return false;
}

///////////////////////////////////
// command parser and switcher, 
// breaks command line in pieces, 
// rejects invalid ones, 
// switches to the right command
void parseCommand(char* inputStr)
{
  byte hasArgument=false;
  int argument;
  int address;
  byte pos=0;
  byte length=strlen(inputStr);
  if(length<2) goto beep;   // not enough characters
  
  // get the adress, one or two digits
  char addrStr[3];
  if(!isdigit(inputStr[pos])) goto beep;  // invalid, first char not a digit
    addrStr[pos]=inputStr[pos];
    pos++;                            // pos=1
  if(isdigit(inputStr[pos]))          // add second digit address if it's there
  {  
    addrStr[pos]=inputStr[pos];
    pos++;                            // pos=2
  }
  addrStr[pos]='\0';                  // add null terminator
  address= atoi(addrStr);        // extract the address
  
  // check for more
  if(!length>pos) goto beep;            // invalid, no command after address
  
  // special case of M commands, which take a string argument
  if(inputStr[pos]=='M')
  {
    pos++;
    if(!length>pos) goto beep;     // no message argument
    doMcommand(address, inputStr+pos);   // pass rest of string as argument
    return;                     // exit
  }
  
  // other commands, get the numerical argument after the command character

  pos++;                             // need to increment in order to peek ahead of command char
  if(!length>pos) hasArgument=false; // end of string reached, no arguments
  else
  {
    for(byte i=pos; i<length; i++)
    {
      if(!isdigit(inputStr[i])) goto beep; // invalid, end of string contains non-numerial arguments
    } 
    argument=atoi(inputStr+pos);    // that's the numerical argument after the command character
    hasArgument=true;
  }
  
  // switch on command character
  switch(inputStr[pos-1])               // 2nd or third char, should be the command char
  {
    case 'T':
      if(!hasArgument) goto beep;       // invalid, no argument after command
      doTcommand(address, argument);      
      break;
    case 'D':                           // D command is weird, does not need an argument, ignore if it has one
      doDcommand(address);
      break;
    case 'P':    
      if(!hasArgument) goto beep;       // invalid, no argument after command
      doPcommand(address, argument);
      break;
    case 'R':    
      if(!hasArgument) goto beep;       // invalid, no argument after command
      doRcommand(address, argument);
      break;
    case 'S':    
      if(!hasArgument) goto beep;       // invalid, no argument after command
      doScommand(address, argument);
      break;
    default:
      goto beep;                        // unknown command
      break;
  }
  
  return;                               // normal exit
  
  beep:                                 // error exit
    serialPort->write(0x7);             // beep the terminal, if connected
    return;
}

//////////////////////
// command executers

// set text command
void doMcommand(int address, char* message)
{
  serialPort->println();
  serialPort->print("Command: M ");
  serialPort->print("Address: ");
  serialPort->print(address);
  serialPort->print(" Argument: ");
  serialPort->print(message);
  
  if(address==0) {setText(0, message); setText(1, message); setText(2, message); resetAllText();}
  if(address==1) {setText(0, message); resetText(0);}
  if(address==2) {setText(1, message); resetText(1);}
  if(address==3) {setText(2, message); resetText(2);}  
  
}

// various commands for states and effects
void doTcommand(int address, int argument)
{
  serialPort->println();
  serialPort->print("Command: T ");
  serialPort->print("Address: ");
  serialPort->print(address);
  serialPort->print(" Argument: ");
  serialPort->print(argument);  
  
  
  switch(argument)
  {
    case 0:    // test mode
      exitEffects();
      if(address==0) {displayState[0]=displayState[1]=displayState[2]=psiState[0]=psiState[1]=TEST; resetAllText();}
      if(address==1) {displayState[0]=TEST; resetText(0);}
      if(address==2) {displayState[1]=TEST; resetText(1);}
      if(address==3) {displayState[2]=TEST; resetText(2);}
      if(address==4) {psiState[0]=TEST;}
      if(address==5) {psiState[1]=TEST;}
      break;
    case 1:    // normal random mode, cancel effects too
      exitEffects();
      if(address==0) {displayState[0]=displayState[1]=displayState[2]=psiState[0]=psiState[1]=RANDOM; resetAllText();}
      if(address==1) {displayState[0]=RANDOM; resetText(0);}
      if(address==2) {displayState[1]=RANDOM; resetText(1);}
      if(address==3) {displayState[2]=RANDOM; resetText(2);}
      if(address==4) {psiState[0]=RANDOM;}
      if(address==5) {psiState[1]=RANDOM;}
      break;
    case 2: case 3: case 5:    // alarm
      exitEffects();
      displayEffect=ALARM;
      break;   
    case 4:   // short circuit
      exitEffects();
      displayEffect=FAILURE;
      break;
    case 6:   // leia
      exitEffects();
      displayEffect=LEIA;
      break;   
    case 10:  // star wars
      exitEffects();
        // reset text
      for(byte disp=0; disp<3; disp++)
      {
        resetText(disp);
        alphabetType[disp]=0;
        displayState[disp]=TEXT;

      }
      setText(0,"STAR    ");
      setText(1,"    WARS");
      setText(2,"STAR WARS   ");  
      break;
    case 11:   // March
      exitEffects();
      displayEffect=MARCH;
      break; 
    case 20:    // extra CuriousMarc command, to turn displays off.
      exitEffects();
      if(address==0) {displayState[0]=displayState[1]=displayState[2]=psiState[0]=psiState[1]=OFF; resetAllText();}
      if(address==1) {displayState[0]=OFF; resetText(0);}
      if(address==2) {displayState[1]=OFF; resetText(1);}
      if(address==3) {displayState[2]=OFF; resetText(2);}
      if(address==4) {psiState[0]=OFF;}
      if(address==5) {psiState[1]=OFF;}
      break;
    case 92:    // bargraph mode, does not cancel effects, but resets text
      exitEffects();
      if(address==0) {displayState[0]=displayState[1]=displayState[2]=BARGRAPH; resetAllText();}
      if(address==1) {displayState[0]=BARGRAPH; resetText(0);}
      if(address==2) {displayState[1]=BARGRAPH; resetText(1);}
      if(address==3) {displayState[2]=BARGRAPH; resetText(2);}
      break;
    case 100:    // text mode, cancel effects too
      exitEffects();
      if(address==0) {displayState[0]=displayState[1]=displayState[2]=TEXT; resetAllText();}
      if(address==1) {displayState[0]=TEXT; resetText(0);}
      if(address==2) {displayState[1]=TEXT; resetText(1);}
      if(address==3) {displayState[2]=TEXT; resetText(2);}
      break;
    default:
      exitEffects(); // default stops any running effect
      break;
  }
}

// holos commands
void doDcommand(int address)
{
  serialPort->println();
  serialPort->print("Command: D ");
  serialPort->print("Address: ");
  serialPort->print(address); 

  // for turning off holos, not implemented
}

// alphabet switching
void doPcommand(int address, int argument)
{
  serialPort->println();
  serialPort->print("Command: P ");
  serialPort->print("Address: ");
  serialPort->print(address);
  serialPort->print(" Argument: ");
  serialPort->print(argument);  
  switch(argument)
  {
    case 60:    // latin
      if(address==0) {alphabetType[0]=alphabetType[1]=alphabetType[2]=LATIN;}
      if(address==1) {alphabetType[0]=LATIN;}
      if(address==2) {alphabetType[1]=LATIN;}
      if(address==3) {alphabetType[2]=LATIN;}
      break;
    case 61:    // Aurabesh
      if(address==0) {alphabetType[0]=alphabetType[1]=alphabetType[2]=AURABESH;}
      if(address==1) {alphabetType[0]=AURABESH;}
      if(address==2) {alphabetType[1]=AURABESH;}
      if(address==3) {alphabetType[2]=AURABESH;}
      break;
    default:
      // should I do back to latin on default argument?
      break;
  }  
}

// random styles for Logics
void doRcommand(int address, int argument)
{
  serialPort->println();
  serialPort->print("Command: R ");
  serialPort->print("Address: ");
  serialPort->print(address);
  serialPort->print(" Argument: ");
  serialPort->print(argument);  
  
  if(address==0) {randomStyle[0]=randomStyle[1]=randomStyle[2]=argument;}
  if(address==1) {randomStyle[0]=argument;}
  if(address==2) {randomStyle[1]=argument;}
  if(address==3) {randomStyle[2]=argument;} 
}

void doScommand(int address, int argument)
{
  serialPort->println();
  serialPort->print("Command: S ");
  serialPort->print("Address: ");
  serialPort->print(address);
  serialPort->print(" Argument: ");
  serialPort->print(argument);  
  switch(argument)
  {
    case 0:    // test, all PSI leds on
      if(address==0) {psiState[0]=psiState[1]=TEST;}
      if(address==4) {psiState[0]=TEST;}
      if(address==5) {psiState[1]=TEST;}
      break;
    case 1:    // normal, random mode
      if(address==0) {psiState[0]=psiState[1]=RANDOM;}
      if(address==4) {psiState[0]=RANDOM;}
      if(address==5) {psiState[1]=RANDOM;}
      break;
    case 2:    // color 1
      if(address==0) {psiState[0]=psiState[1]=COLOR1;}
      if(address==4) {psiState[0]=COLOR1;}
      if(address==5) {psiState[1]=COLOR1;}
      break;
    case 3:    // color 2
      if(address==0) {psiState[0]=psiState[1]=COLOR2;}
      if(address==4) {psiState[0]=COLOR2;}
      if(address==5) {psiState[1]=COLOR2;}
      break;
    case 4:    // off 
      if(address==0) {psiState[0]=psiState[1]=OFF;}
      if(address==4) {psiState[0]=OFF;}
      if(address==5) {psiState[1]=OFF;}
      break;      
    default:
      // should I do back to latin on default argument?
      break;
  }  
}

// =======================================================================================

/////////////////////////////////////////////////////////////////////////////////////
// Logic Display modes (random, text, test, bargraph, off) and random PSI functions
/////////////////////////////////////////////////////////////////////////////////////

// Utility Random
void randomDisplay(byte disp)
{
  switch(disp)
  {
    case 0:
      randomDisplayTFLD();
      break;
    case 1:
      randomDisplayBFLD();
      break;
    case 2:
      randomDisplayRLD();
      break;
    default:
      break;
  }
}

// Utility Scrolling Text
void textDisplay(byte disp)
{
  if(disp>2) return;
  scrollText(disp, logicText[disp]);
}

//////////////////
// Test (all on)
void testDisplay(byte disp)
{
  if(disp>2) return;
  for(byte i=0; i<5; i++)
  {
    LEDgrid[disp][i]=~0L;
  }
  showGrid(disp);
}

//////////////////
// Utility: Off
void offDisplay(byte disp)
{
  if(disp>2) return;
  for(byte i=0; i<5; i++)
  {
    LEDgrid[disp][i]=0L;
  }
  showGrid(disp);
}

//////////////////////
// bargraph
void bargraphDisplay(byte disp)
{ 
  static byte bargraphdata[3][27]; // status of bars
  
  if(disp>2) return;
  
  // speed control
  static long previousDisplayUpdate[3]={0,0,0};
  unsigned long currentMillis = millis();
  if(currentMillis - previousDisplayUpdate[disp] < BARGRAPHupdateDelay) return;
  previousDisplayUpdate[disp] = currentMillis;
  
  byte maxcol;
  if(disp==0 || disp==1) maxcol=9;
  else maxcol=27;
  
  // loop over each column
  for(byte column=0; column<maxcol; column++)
  {
    //byte value=random(0,5);
    byte value = updatebar(disp, column, bargraphdata[disp]);
    byte data=0;
    for(int i=0; i<=value; i++) 
    {
      data |= 0x01<<i;
    }
    //data=B00011111;
    fillColumn( disp, column, data);   
  }
  showGrid(disp);
}

// helper for updating bargraph values
byte updatebar(byte disp, byte column, byte* bargraphdata)
{
  // bargraph values go up or down one pixel at a time
  int variation = random(0,3);            // 0= move down, 1= stay, 2= move up
  int value=(int)bargraphdata[column];    // get the previous value
  if (value==5) value=3;                 // special case, staying stuck at maximum does not look realistic, knock it down
  else value += (variation-1);            // vary it
  if (value<=0) value=0;                  // can't belower than 0
  if (value>5) value=5;                   // can't be higher than 5
  bargraphdata[column]=(byte)value;       // store new value, use byte type to save RAM
  return (byte)value;                     // return new value
}

// helper for dealing with setting LEDgrid by column instead of by row
void fillColumn(byte disp, byte column, byte data)
{
  if (disp==2 && column>27) return;
  if (disp!=2 && column>9) return;
  for(byte row=0; row<5; row++)
  {
    // test if LED is on
    byte LEDon=(data & 1<<row);
    if(LEDon)
      LEDgrid[disp][4-row] |= (1L << column);    // set column bit
    else
      LEDgrid[disp][4-row] &= ~(1L << column); // reset column bit
  }
}

// helper for generating more interesting random patterns for the logics
long randomRow(byte randomMode)
{
  switch(randomMode)
  {
    case 0:  // stage -3
      return (random(256)&random(256)&random(256)&random(256));
      break;
    case 1:  // stage -2
      return (random(256)&random(256)&random(256));
      break;
    case 2:  // stage -1
      return (random(256)&random(256));
      break;
    case 3: // legacy "blocky" mode
      return random(256);
      break;
    case 4:  // stage 1
      return (random(256)|random(256));
      break;
    case 5:  // stage 2
      return (random(256)|random(256)|random(256));
      break;
    case 6:  // stage 3
      return (random(256)|random(256)|random(256)|random(256));
      break;
    default:
      return random(256);
      break;
  }


}


/////////////////////////
// Random Rear Logic
void randomDisplayRLD()
{
  // static parameter for each display
  static long previousDisplayUpdate=0;
  
  // wait until delay before randomizing again
  unsigned long currentMillis = millis();
  if(currentMillis - previousDisplayUpdate < LOGICupdateDelay) return;
  previousDisplayUpdate = currentMillis;
  
#if defined(TESTLOGICS) //turn on all logic LEDs to make sure they're all working

  for (int dev=0; dev<3; dev++) // loop on all devices, all rows, rear chain
  {
    for (int row=0; row<6; row++) 
    lcRear.setRow(dev,row,255);
  }

  
#else  // regular random code

  // loop on all devices, all rows for RLD
  for (int dev=0; dev<3; dev++)
  {
    for (int row=0; row<6; row++)
    lcRear.setRow(dev,row,randomRow(randomStyle[2]));   
    // Or-ing 3 times prevents too many LEDs from blinking off together
    // nice trick from BHD...
  }
  
#endif

}

//////////////////////
// random top FLD
void randomDisplayTFLD()
{
  // static parameter for each display
  static long previousDisplayUpdate=0;
  
  // wait until delay before randomizing again
  unsigned long currentMillis = millis();
  if(currentMillis - previousDisplayUpdate < LOGICupdateDelay) return;
  previousDisplayUpdate = currentMillis;
  
#if defined(TESTLOGICS) //turn on all logic LEDs to make sure they're all working

  int dev=0; // loop on FLD, all rows, front chain
  for (int row=0; row<6; row++)
  lcFront.setRow(dev,row,255); 
  
#else  // regular random code

  // loop on top FLD devices, all rows on front chain
  int dev=0;
  for (int row=0; row<6; row++)
  lcFront.setRow(dev,row,randomRow(randomStyle[0])); 
  
#endif

}


//////////////////////
// random bottom FLD
void randomDisplayBFLD()
{
  // static parameter for each display
  static long previousDisplayUpdate=0;
  
  // wait until delay before randomizing again
  unsigned long currentMillis = millis();
  if(currentMillis - previousDisplayUpdate < LOGICupdateDelay) return;
  previousDisplayUpdate = currentMillis;
  
#if defined(TESTLOGICS) //turn on all logic LEDs to make sure they're all working

  int dev=1; // loop on FLD, all rows, front chain
  for (int row=0; row<6; row++)
  lcFront.setRow(dev,row,255); 
  
#else  // regular random code

  // loop on top FLD devices, all rows on front chain
  int dev=1;
  for (int row=0; row<6; row++)
  lcFront.setRow(dev,row,randomRow(randomStyle[1])); 
  
#endif

}

/////////////////////////////////////////////////////////////////
// PSI Modes
/////////////////////////////////////////////////////////////////

////////////////////////////
// Front PSI static modes (on, off, color1, color2




void setFPSI(byte mode)
{
  switch(mode)
  {
    case OFF:
      for(byte row=0; row<HPROW; row++)
      {
        lcFront.setRow(2, row, 0x00);
      }
      break;
    case TEST:  // all on
      for(byte row=0; row<HPROW; row++)
      {
        lcFront.setRow(2, row, 0xFF);
      }
      break;
    case COLOR1:
      for(byte row=0; row<HPROW; row++)
      {
        lcFront.setRow(2, row, psiPatterns[row]);
      }
      break;
    case COLOR2:
      for(byte row=0; row<HPROW; row++)
      {
        lcFront.setRow(2, row, psiPatterns[row+5]);
      }
      break;
    default:
      break;
  }
}

void setRPSI(byte mode)
{
  switch(mode)
  {
    case OFF:
      for(byte row=0; row<HPROW; row++)
      {
        lcRear.setRow(3, row, 0x00);
      }
      break;
    case TEST:
      for(byte row=0; row<HPROW; row++)
      {
        lcRear.setRow(3, row, 0xFF);
      }
      break;
    case COLOR1:
      for(byte row=0; row<HPROW; row++)
      {
        lcRear.setRow(3, row, psiPatterns[row]);
      }
      break;
    case COLOR2:
      for(byte row=0; row<HPROW; row++)
      {
        lcRear.setRow(3, row, psiPatterns[row+5]);
      }
      break;
    default:
      break;
  }
}



///////////////////////////
// Front PSI random swipe
void randomFPSI() 
{   
  // Static variables per PSI function
  // PSI wipe timers
  static unsigned long psiMillisChangeDir=0;       // wait time between change of directions
  static unsigned long psiMillisSwipe=0;           // wait time between swipe tests
  // psi delay timer, currently only one, both PSI change at the same time
  static long unsigned psiChangeColorDelay=0;              // variable time for changing colors
  // psi color and row counter numbers
  static int psiColor=0;
  static int psiCurrentSwipeRow=0;
  // "stuck" flag
  static byte isStuck=0;
  
  unsigned long currentMillis = millis(); 
  if(isStuck==1)
  {
    // put direction changing on hold
    psiMillisChangeDir = currentMillis;
  }
  else
  {
    // if time has elapsed, reverse color and direction of swipe, chose another random time
    if(currentMillis - psiMillisChangeDir > psiChangeColorDelay*500) // delay between .5 to 5 seconds
    {
      // choose another random delay
      psiMillisChangeDir = currentMillis;
      psiChangeColorDelay = random(1,11);
      
      // reverse color and swipe direction
      if (psiColor ==0)
      {
        psiColor = 5;
        psiCurrentSwipeRow=0;
      }
      else
      {
        psiColor = 0;
        psiCurrentSwipeRow=HPROW-1;
      }
    }
  }
  
  // do next swipe step only if time has elapsed. 
  if(isStuck==1) // pause swipe with long time if PSI is stuck.
  {if (currentMillis - psiMillisSwipe < PSIstuckhowlong) return;}
  else          // regular swipe speed if not
  {if(currentMillis - psiMillisSwipe < PSIwipeDelay) return;}
  
  // get unstuck
  if(isStuck)  isStuck=0;  
    
  // if we are going to color 2 and haven't reach the end row, do next row
  if (psiCurrentSwipeRow<HPROW && psiColor == 5)
  {
    psiMillisSwipe = currentMillis;
    lcFront.setRow(2, psiCurrentSwipeRow, psiPatterns[psiCurrentSwipeRow+psiColor]);
    psiCurrentSwipeRow++; 
  }
  // if we are going to color 1 and haven't reached the first row, do next row
  else if (psiCurrentSwipeRow>=0 && psiColor == 0)
  {
    psiMillisSwipe = currentMillis;
    lcFront.setRow(2, psiCurrentSwipeRow, psiPatterns[psiCurrentSwipeRow+psiColor]);
    psiCurrentSwipeRow--;
  }
 
  // let's get stuck once in a while
  if(FPSIgetsStuck && psiCurrentSwipeRow==2 && psiColor==5)
  // && isStuck==0 && isStuck!=2)
  {
    byte onceinawhile=random(PSIstuckhowoften);  // one chance out of 20
    if(onceinawhile==1) isStuck=1;
  }
}

////////////////////////
// Rear PSI random swipe
void randomRPSI() 
{   
  // Static variables, don't need to be globals
  // PSI wipe timers
  static unsigned long psiMillisChangeDir=0;       // wait time between change of directions
  static unsigned long psiMillisSwipe=0;           // wait time between swipe tests
  // psi delay timer, currently only one, both PSI change at the same time
  static unsigned long psiChangeColorDelay=0;              // variable time for changing colors
  // psi color and row counter numbers
  static int psiColor=0;
  static int psiCurrentSwipeRow=0;
  // "stuck" flag
  static byte isStuck=0;  
  
  
  unsigned long currentMillis = millis(); 
  if(isStuck==1)
  {
    // put direction changing on hold
    psiMillisChangeDir = currentMillis;
  }
  else
  {
    // if time has elapsed, reverse color and direction of swipe, chose another random time
    if(currentMillis - psiMillisChangeDir > psiChangeColorDelay*500) // delay between .5 to 5 seconds
    {
      // choose another random delay
      psiMillisChangeDir = currentMillis;
      psiChangeColorDelay = random(1,11);
      
      // reverse color and swipe direction
      if (psiColor ==0)
      {
        psiColor = 5;
        psiCurrentSwipeRow=0;
      }
      else
      {
        psiColor = 0;
        psiCurrentSwipeRow=HPROW-1;
      }
    }
  }
  
  // do next swipe step only if time has elapsed. 
  if(isStuck==1) // pause swipe with long time if PSI is stuck.
  {if (currentMillis - psiMillisSwipe < PSIstuckhowlong) return;}
  else          // regular swipe speed if not
  {if(currentMillis - psiMillisSwipe < PSIwipeDelay) return;}
  
  // get unstuck
  if(isStuck)  isStuck=0;  
  
  // if we are going to color 2 and haven't reach the end row, do next row
  if (psiCurrentSwipeRow<HPROW && psiColor == 5)
  {
    psiMillisSwipe = currentMillis;
    lcRear.setRow(3, psiCurrentSwipeRow, psiPatterns[psiCurrentSwipeRow+psiColor]);
    psiCurrentSwipeRow++; 
  }
  // if we are going to color 1 and haven't reached the first row, do next row
  else if (psiCurrentSwipeRow>=0 && psiColor == 0)
  {
    psiMillisSwipe = currentMillis;
    lcRear.setRow(3, psiCurrentSwipeRow, psiPatterns[psiCurrentSwipeRow+psiColor]);
    psiCurrentSwipeRow--;
  }
 
  // let's get stuck once in a while
  if(RPSIgetsStuck && psiCurrentSwipeRow==2 && psiColor==5)
  // && isStuck==0 && isStuck!=2)
  {
    byte onceinawhile=random(PSIstuckhowoften);  // one chance out of 20
    if(onceinawhile==1) isStuck=1;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Special Effect Routines
/////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////
// Reset Utilities

// resets text scrolling except alphabet
void resetText(byte display)
{
  // reset text
  scrollPositions[display]= (display==2? 27 : 9);
  textScrollCount[display]=0;  
}

// same for all the displays
void resetAllText()
{
  for(byte disp=0; disp<3; disp++) 
  {
    resetText(disp);
  } 
}

// forces exit from effects immediately
void exitEffects()
{
  displayEffect=NORM;
//  psiState[0]=NORM;
//  psiState[1]=NORM;
  effectRunning=0;
}

// exit effects, reset scrolling, alphabet back to latin, mode to random
void resetDisplays()
{
  resetAllText();
  exitEffects();
  for(byte disp=0; disp<3; disp++)
  {
    alphabetType[disp]=LATIN;
    displayState[disp]=NORM;
  }
}



////////////////
// Leia Display: horizontal moving line on all displays
void leiaDisplay(unsigned long playTime)
{
  static byte a = 0;     // row counter a for writes
  static byte b = 0;     // row counter b for erase
  //static byte color = 0; // counter for PSI color

  unsigned long currentMillis = millis();
  static unsigned long swtchMillis;
  static unsigned long enterMillis;
  
  // entry and exit timing
  if(effectRunning==0) 
  {
    enterMillis=currentMillis;
    effectRunning=1;
    // blank display out
    clearGrid(0); showGrid(0);
    clearGrid(1); showGrid(1);
    clearGrid(2); showGrid(2);
  }
  // exit on playTime elapsed. playTime=0 means run forever.
  if(playTime && (currentMillis - enterMillis > playTime))
  {
    effectRunning=0;
    displayEffect=0;
    return;
  }
  
  // PSI stay random during effect (see at the end of function for an alternate effect
  randomFPSI();
  randomRPSI();
  
  // move the line when it's time
  if (currentMillis - swtchMillis > LEIAspeed)
  {    
     swtchMillis=currentMillis;
     // draw moving line on RLD  
     for(int dev=0;dev<3;dev++) 
     {        
       // set line LEDs on
       lcRear.setRow(dev,a,255);
       lcRear.setLed(dev,5,a,true);
         
       // set previous line LEDs off 
       lcRear.setRow(dev,b,0);
       lcRear.setLed(dev,5,b,false);
     }
   
    // draw moving line on TFLD
    int dev=0; 
    {        
       // set line LEDs on
       lcFront.setRow(dev,a,255);
       lcFront.setLed(dev,5,a,true);
        
       // set line LEDs off
       lcFront.setRow(dev,b,0);
       lcFront.setLed(dev,5,b,false);
    } 
     
    // draw moving line inverted on BFLD
    dev=1;
    {        
       // set line LEDs on
       lcFront.setRow(dev,4-a,255);
       lcFront.setLed(dev,5,4-a,true);
       
       // set line LEDs off
       lcFront.setRow(dev,4-b,0);
       lcFront.setLed(dev,5,4-b,false);
    } 
     
    

    // update row count
    b=a; // remember last row
    a++; // go to next row
    if (a>4) 
    {
      a=0; // restart if done with the 5 rows
      /****** nah, don't like it
      // and switch PSI colors
      if(color)
      {
        setFPSI(BLUE);
        setRPSI(YELLOW);
        color=0;
      }
      else
      {
        setFPSI(RED);
        setRPSI(GREEN);
        color=1;
      }
      ************/
    }
  }
}

///////////////// 
//alarmDisplay, alternating full on/full off

void alarmDisplay(unsigned long playTime)
{ 
  static byte swtch = 0;
  
  
  static unsigned long swtchMillis;
  unsigned long currentMillis = millis();
 
  // entry and exit timing
  static unsigned long enterMillis;
  if(effectRunning==0) 
  {
    enterMillis=currentMillis;
    effectRunning=1;
  }
  // exit on playTime elapsed. playTime=0 means run forever.
  if(playTime && (currentMillis - enterMillis > playTime))
  {
    effectRunning=0;
    displayEffect=0;
    return;
  }
  
  // wait for the next period
  if (currentMillis - swtchMillis > ALARMspeed)
  {    
    if (swtch == 0) 
    {
      clearGrid(0);
      clearGrid(1);
      clearGrid(2);
      
      // everything on
      for(int row=0;row<5;row++)
      {
        LEDgrid[0][row]=0xFFFFFFFFL;      
        LEDgrid[1][row]=0xFFFFFFFFL;     
        LEDgrid[2][row]=0xFFFFFFFFL;   
      }
      
      showGrid(0);
      showGrid(1);
      showGrid(2);
      
      setFPSI(RED);
      setRPSI(YELLOW);
      
      swtchMillis = millis();
      swtch = 1;
    }
    else  if (swtch == 1) 
    {
      // everything off
      clearGrid(0);
      clearGrid(1);
      clearGrid(2); 
      
      showGrid(0);
      showGrid(1);
      showGrid(2);
      
      setFPSI(OFF);
      setRPSI(OFF);      
       
      swtchMillis = millis();
      swtch = 0;
    }
  }
}

///////////////////////
// March Effect, blocks alternating sideways 
void marchDisplay(unsigned long playTime)
{
  unsigned long currentMillis = millis();
  static unsigned long swtchMillis = millis();
  static byte swtch = 0;
 
  // entry and exit timing
  static unsigned long enterMillis;
  if(effectRunning==0) 
  {
    enterMillis=currentMillis;
    effectRunning=1;
  }
  // exit on playTime elapsed. playTime=0 means run forever.
  if(playTime && (currentMillis - enterMillis > playTime))
  {
  effectRunning=0;
  displayEffect=0;
  return;
  }
  
  if (currentMillis - swtchMillis > MARCHspeed)
  { 
    if (swtch == 0) 
    {
      clearGrid(0);
      clearGrid(1);
      clearGrid(2);
      
      for(int row=0;row<5;row++)
      {
        LEDgrid[0][row]=31L;      // first 5 column on
        LEDgrid[1][row]=31L;     // first 5 column on
        LEDgrid[2][row]=16383L;   // first 14 column on
      }
      
      showGrid(0);
      showGrid(1);
      showGrid(2);
      
      setFPSI(RED);
      setRPSI(YELLOW);      
      
      swtchMillis = millis();
      swtch = 1;
    }
    else  if (swtch == 1) 
    {
      clearGrid(0);
      clearGrid(1);
      clearGrid(2); 
      
      for(int row=0;row<5;row++)
      {
        LEDgrid[0][row]= ~15L;      // first 4 column off
        LEDgrid[1][row]= ~15L;     // first 4 column off
        LEDgrid[2][row]= ~8191L;   // first 13 column off
      }
      
      showGrid(0);
      showGrid(1);
      showGrid(2);

      setFPSI(BLUE);
      setRPSI(GREEN); 
      
      swtchMillis = millis();
      swtch = 0;
    }
  }
}


////////////////
// Failure: screen having less and less dots

// failure helper function
void showFailure(byte style)
{
    // TLFLD
    for (int row=0; row<6; row++)
      lcFront.setRow(0,row,randomRow(style));
    // BFLD
    for (int row=0; row<6; row++)
      lcFront.setRow(1,row,randomRow(style));
    // RLD
    for (int dev=0; dev<3; dev++)
    {
      for (int row=0; row<6; row++)
      lcRear.setRow(dev,row,randomRow(style));
    }
    // FPSI
    for (int row=0; row<HPROW; row++)
      lcFront.setRow(2,row,randomRow(style));
    // RPSI
    for (int row=0; row<HPROW; row++)
      lcRear.setRow(3,row,randomRow(style));
}

// failure main function
void failureDisplay(unsigned long playTime)
{
  static int loopCount=0;  // number of loops
  static unsigned long lastMillis;
  unsigned long currentMillis = millis();
  static unsigned long blinkSpeed = FAILUREspeed;
  
  // entry and exit timing
  static unsigned long enterMillis;
  if(effectRunning==0) 
  {
    blinkSpeed = FAILUREspeed;
    loopCount=0;
    enterMillis=currentMillis;
    effectRunning=1;
  }
  // exit on playTime elapsed. playTime=0 means run forever.
  if(playTime && (currentMillis - enterMillis > playTime))
  {
    effectRunning=0;
    displayEffect=0;
    return;
  }
  
  // speed control
  if (currentMillis - lastMillis < blinkSpeed) return;
  lastMillis = currentMillis;
  loopCount++;
  
  // every 2200 counts, move the line
  if (loopCount<FAILUREloops)
  {    
    blinkSpeed=FAILUREspeed;
    showFailure(4);
  }
  else if(loopCount<2*FAILUREloops)
  {
    blinkSpeed=2*FAILUREspeed;
    showFailure(3);
  }
  else if(loopCount<3*FAILUREloops)
  {
    blinkSpeed=3*FAILUREspeed;
    showFailure(2);
  }
  else if(loopCount<4*FAILUREloops)
  {
    blinkSpeed=4*FAILUREspeed;
    showFailure(1);
  }
  else if(loopCount<5*FAILUREloops)
  {
    showFailure(0);    
  }
   else
  {
    // just stay stuck at the end.
  }
}


  
//////////////////////////////////////////////////////
// showGrid: main function to display LED grid on Logics
/////////////////////////////////////////////////////

// byte reversal fast RAM lookup table
uint8_t revlookup[16] = {
   0x0, 0x8, 0x4, 0xC,
   0x2, 0xA, 0x6, 0xE,
   0x1, 0x9, 0x5, 0xD,
   0x3, 0xB, 0x7, 0xF };
// byte reversal function
uint8_t rev( uint8_t n )
{
   //This should be just as fast and it is easier to understand.
   //return (lookup[n%16] << 4) | lookup[n/16];
   return (revlookup[n&0x0F] << 4) | revlookup[n>>4];
}

// blank grid, to turn of all LEDs
void clearGrid(byte display)
{
  for (byte row=0; row<5; row++) LEDgrid[display][row]=0L;
}
  
// Sends LED Grid to actual LEDs on the logic displays
void showGrid(byte display)
{
  // didplay=0 Top FLD
  // display=1 Bottom FLD
  // display=2 RLD
  
  // Every 9th column of the displays maps to the 6th row of the Maxim chip
  unsigned char col8=0;  // 9th column of FLDs and RLD, maps to 6th row of device 0
  unsigned char col17=0; // 18th column of RLD, goes to 6th row of RLD device 1
  unsigned char col26=0; // 27th column of RLD, goes to 6th row of RLD device 2 
  
  // Colums 0-7 map with a byte reversal
  
  switch(display)
  {
    case 0:  // Top FLD
      
      for (byte row=0; row<5; row++) // loop on first 5 rows
      {
    // extract first 8 bits, reverse, send to device 0, front chain which is top FLD
        lcFront.setRow(0, row, rev(LEDgrid[display][row] & 255L));
        // If the LED at column 8 is on, add it to the extra row (starting "left" at MSB)
        if ( (LEDgrid[display][row] & 1L<<8) == 1L<<8)   col8 += 128>>row;
      }
      // send the 9th column (logical 8) as a 6th row or the Maxim (logical 5)
      lcFront.setRow(0, 5, col8);

    break;
    
    case 1:  // Bottom FLD
      // Bottom FLD is upside down. So rows are inverted. Top is bottom, left is right
      for (byte row=0; row<5; row++) // loop on first 5 rows
      {
    // extract bits 2-9, do not reverse, send to device 1, start with device row 4 (invert top and bottom)
        lcFront.setRow(1, 4-row, (LEDgrid[display][row] & 255L<<1) >> 1);
        // If the LED at first column is on, add it to the extra row (starting "left" at MSB)
        // we still call it col8, but with the inverted display it really is col 0
        // we fill in the forward direction starting from bit 3
        if ( (LEDgrid[display][row] & 1L) == 1L)   col8 += 8<<row;
      }
      // send the column 0 as a 6th row or the Maxim (logical row 5)
      lcFront.setRow(1, 5, col8);
    break;
    
    case 2:  // RLD
      for (byte row=0; row<5; row++) // loop on first 5 rows
      {
        int loops = 0;
        for (byte dev=0; dev < 3; dev++) // RLD has 3 Maxim chip devices
        {
      // extract byte at column 0, 9 and 18, reverse byte order, and send to device
          lcRear.setRow(dev, row, rev(  (LEDgrid[display][row] & 255L<<(9*loops)) >> (9*loops) ));
      loops++;
        }
        // If the LED at column 8, 17 or 26 is on, add it to the extra row (starting "left" at MSB)
        if ( (LEDgrid[display][row] & 1L<<8) == 1L<<8)   col8 += 128>>row;
        if ( (LEDgrid[display][row] & 1L<<17) == 1L<<17) col17 += 128>>row;
        if ( (LEDgrid[display][row] & 1L<<26) == 1L<<26) col26 += 128>>row;
      }
      // send the extra columns as a 6th row or the Maxim (logical row 5)
      lcRear.setRow(0, 5, col8);
      lcRear.setRow(1, 5, col17);
      lcRear.setRow(2, 5, col26);
    break;
    
    default:
    break;
  }
}
///////////////////////end showGrid routines/////////////////////////////////



//////////////////////////////////////
// Text Display Routines
/////////////////////////////////////

//////////////////////
// Set String
void setText(byte disp, const char* message)
{
  strncpy(logicText[disp], message, MAXSTRINGSIZE);
  logicText[disp][MAXSTRINGSIZE]=0; // just in case
}
 
///////////////////////////////
// Latin Alphabet, put in PROGMEM so save RAM
const int cA[] PROGMEM = { B00000110,
            B00001001,
            B00001111,
            B00001001,
            B00001001 };
           
const int cB[] PROGMEM = { B00000111,
            B00001001,
            B00000111,
            B00001001,
            B00000111 };

const int cC[] PROGMEM = { B00000110,
            B00001001,
            B00000001,
            B00001001,
            B00000110 };

const int cD[] PROGMEM = { B0000111,
            B0001001,
            B0001001,
            B0001001,
            B0000111 };

const int cE[] PROGMEM = { B00001111,
            B00000001,
            B00000111,
            B00000001,
            B00001111 };
            
const int cF[] PROGMEM = { B00001111,
            B00000001,
            B00000111,
            B00000001,
            B00000001 };
            
const int cG[] PROGMEM = { B00001110,
            B00000001,
            B00001101,
            B00001001,
            B00000110 };
            
const int cH[] PROGMEM = { B00001001,
            B00001001,
            B00001111,
            B00001001,
            B00001001 };
            
const int cI[] PROGMEM = { B00000111,
            B00000010,
            B00000010,
            B00000010,
            B00000111 };
            
const int cJ[] PROGMEM = { B00001000,
            B00001000,
            B00001000,
            B00001001,
            B00000110 };
            
const int cK[] PROGMEM = { B00001001,
            B00000101,
            B00000011,
            B00000101,
            B00001001 };
            
const int cL[] PROGMEM = { B00000001,
            B00000001,
            B00000001,
            B00000001,
            B00001111 };
            
const int cM[] PROGMEM = { B00010001,
            B00011011,
            B00010101,
            B00010001,
            B00010001 };
            
const int cN[] PROGMEM = { B00001001,
            B00001011,
            B00001101,
            B00001001,
            B00001001 };
            
const int cO[] PROGMEM = { B00000110,
            B00001001,
            B00001001,
            B00001001,
            B00000110 };
            
const int cP[] PROGMEM = { B00000111,
             B00001001,
             B00000111,
             B00000001,
             B00000001 };
            
const int cQ[] PROGMEM = { B00000110,
            B00001001,
            B00001101,
            B00001001,
            B00010110 };
            
const int cR[] PROGMEM = { B00000111,
            B00001001,
            B00000111,
            B00000101,
            B00001001 };
            
const int cS[] PROGMEM = { B00001110,
             B00000001,
             B00000110,
             B00001000,
             B00000111 };
const int cT[] PROGMEM = { B00001111,
             B00000110,
             B00000110,
             B00000110,
             B00000110 };
const int cU[] PROGMEM = { B00001001,
             B00001001,
             B00001001,
             B00001001,
             B00000110 };
const int cV[] PROGMEM = { B00001001,
             B00001001,
             B00001001,
             B00000110,
             B00000110 };
const int cW[] PROGMEM = { B00010001,
             B00010001,
             B00010101,
             B00011011,
             B00010001 };
const int cX[] PROGMEM = { B00001001,
             B00001001,
             B00000110,
             B00001001,
             B00001001 };
const int cY[] PROGMEM = { B00001001,
             B00001001,
             B00000110,
             B00000110,
             B00000110 };
const int cZ[] PROGMEM = { B00001111,
             B00000100,
             B00000010,
             B00000001,
             B00001111 };
const int c0[] PROGMEM = { 
             B00001100,
             B00010010,
             B00010010,
             B00010010,
             B00001100 };
//Non-letters
const int c1[] PROGMEM = { 
             B00001100,
             B00001010,
             B00001000,
             B00001000,
             B00011110 };
const int c2[] PROGMEM = { 
             B00011100,
             B00010010,
             B00001000,
             B00000100,
             B00011110 };  
const int c3[] PROGMEM = { 
             B00011110,
             B00010000,
             B00011100,
             B00010000,
             B00011110 };  
const int c4[] PROGMEM = { 
             B00000010,
             B00000010,
             B00001010,
             B00011110,
             B00001000 };  
const int c5[] PROGMEM = { 
             B00011110,
             B00000010,
             B00001110,
             B00010000,
             B00001110 };  
const int c6[] PROGMEM = { 
             B00011100,
             B00000010,
             B00001110,
             B00010010,
             B00001100 };  
const int c7[] PROGMEM = { 
             B00011110,
             B00010000,
             B00001000,
             B00000100,
             B00000010 };  
const int c8[] PROGMEM = { 
             B00001100,
             B00010010,
             B00001100,
             B00010010,
             B00001100 };      
const int c9[] PROGMEM = { 
             B00001100,
             B00010010,
             B00011100,
             B00010000,
             B00001110 };  
// Heart Symbol    
const int ch[] PROGMEM = { B00110110,
             B01001001,
             B01000001,
             B00100010,
             B00001000 };
// Tie Fighter Symbol    
const int ct[] PROGMEM = { B00100010,
             B00101010,
             B00110110,
             B00101010,
             B00100010 }; 
// R2D2 Symbol    
const int cr[] PROGMEM = { B00001110,
             B00011011,
             B00011111,
             B00010101,
             B00010001 } ; 
// dash - Symbol    
const int cd[] PROGMEM = { B00000000,
             B00000000,
             B00001110,
             B00000000,
             B00000000 };
// Film Bar Symbol for use with Leia message
const int cf[] PROGMEM = { B00000100,
             B00000100,
             B00000100,
             B00000100,
             B00000100 };

//Blank Symbol
const int cb[] PROGMEM = { B00000000,
             B00000000,
             B00000000,
             B00000000,
             B00000000 };  

//upSymbol
const int cu[] PROGMEM = { B00000001,
             B00000010,
             B00000100,
             B00001000,
             B00010000 }; 

//down Symbol
const int cn[] PROGMEM = { B00010000,
             B00001000,
             B00000100,
             B00000010,
             B00000001 }; 

//Dot Symbol
const int cdot[] PROGMEM = { B00000000,
             B00000000,
             B00000000,
             B00000000,
             B00000100 };  


// retrieve latin alphabet letter from progam memory
void getLatinLetter(int* letterbitmap, char let)
{
  // pLetter will be a pointer to program memory
  const int* pLetter;
  
  // get pointer to program memory from character
  switch (let)
  {
    case 'A': pLetter=cA; break;
    case 'B': pLetter=cB; break;
    case 'C': pLetter=cC; break;
    case 'D': pLetter=cD; break;
    case 'E': pLetter=cE; break;
    case 'F': pLetter=cF; break;
    case 'G': pLetter=cG; break;
    case 'H': pLetter=cH; break;
    case 'I': pLetter=cI; break;
    case 'J': pLetter=cJ; break;
    case 'K': pLetter=cK; break;
    case 'L': pLetter=cL; break;
    case 'M': pLetter=cM; break;
    case 'N': pLetter=cN; break;
    case 'O': pLetter=cO; break;
    case 'P': pLetter=cP; break;
    case 'Q': pLetter=cQ; break;
    case 'R': pLetter=cR; break;
    case 'S': pLetter=cS; break;
    case 'T': pLetter=cT; break;
    case 'U': pLetter=cU; break;
    case 'V': pLetter=cV; break;
    case 'W': pLetter=cW; break;
    case 'X': pLetter=cX; break;
    case 'Y': pLetter=cY; break;
    case 'Z': pLetter=cZ; break;
    //non-letters
    //numbers
    case '0': pLetter=c0; break;
    case '1': pLetter=c1; break;
    case '2': pLetter=c2; break;
    case '3': pLetter=c3; break;
    case '4': pLetter=c4; break;
    case '5': pLetter=c5; break;
    case '6': pLetter=c6; break;
    case '7': pLetter=c7; break;
    case '8': pLetter=c8; break;
    case '9': pLetter=c9; break;
    //special characters
    case '*': pLetter=ch; break;
    case '#': pLetter=ct; break;
    case '@': pLetter=cr; break;
    case '-': pLetter=cd; break;
    case '|': pLetter=cf; break;
    case '.': pLetter=cdot; break;
    //whitespace
    case ' ': pLetter=cb; break;
    case '<': pLetter=cu; break;
    case '>': pLetter=cn; break;
    default : pLetter=cb; break;
    break;
  }
  
  // move data back from program memory to RAM
  for(byte i=0; i<5; i++)
  {
      letterbitmap[i]=(int)pgm_read_word(&(pLetter[i]));
  }
}


// Aurabesh Alphabet, in PROGMEM
const int a2[] PROGMEM = {
    B00001111,
    B00001001,
    B00000100,
    B00001001,
    B00001111 };
const int aA[] PROGMEM = {
    B00010001,
    B00001111,
    B00000000,
    B00001111,
    B00010001 };
const int aB[] PROGMEM = {
    B00001110,
    B00010001,
    B00001110,
    B00010001,
    B00001110 };
const int aC[] PROGMEM = {
    B00000001,
    B00000001,
    B00000100,
    B00010000,
    B00010000 };
const int aD[] PROGMEM = {
    B00011111,
    B00001000,
    B00000111,
    B00000010,
    B00000001 };
const int aE[] PROGMEM = {
    B00011001,
    B00011001,
    B00011001,
    B00010110,
    B00010100 };
const int aF[] PROGMEM = {
    B00010000,
    B00001010,
    B00000111,
    B00000011,
    B00011111 };
const int aG[] PROGMEM = {
    B00011101,
    B00010101,
    B00010001,
    B00001001,
    B00000111 };
const int aH[] PROGMEM = {
    B00011111,
    B00000000,
    B00001110,
    B00000000,
    B00011111 };
const int aI[] PROGMEM = {
    B00000100,
    B00000110,
    B00000100,
    B00000100,
    B00000100 };
const int aJ[] PROGMEM = {
    B00010000,
    B00011000,
    B00001111,
    B00000100,
    B00000011 };
const int aK[] PROGMEM = {
    B00011111,
    B00010000,
    B00010000,
    B00010000,
    B00011111 };
const int aL[] PROGMEM = {
    B00010000,
    B00010001,
    B00010010,
    B00010100,
    B00011000 };
const int aM[] PROGMEM = {
    B00011100,
    B00010010,
    B00000001,
    B00010001,
    B00011111 };
const int aN[] PROGMEM = {
    B00001010,
    B00010101,
    B00010101,
    B00010011,
    B00010010 };
const int aO[] PROGMEM = {
    B00000000,
    B00001110,
    B00010001,
    B00010001,
    B00011111 };
const int aP[] PROGMEM = {
    B00010110,
    B00010101,
    B00010001,
    B00010001,
    B00011110 };
const int aQ[] PROGMEM = {
    B00011111,
    B00010001,
    B00000001,
    B00000001,
    B00000111 };
const int aR[] PROGMEM = {
    B00011111,
    B00001000,
    B00000100,
    B00000010,
    B00000001 };
const int aS[] PROGMEM = {
    B00010000,
    B00010010,
    B00010101,
    B00011010,
    B00010100 };
const int aT[] PROGMEM = {
    B00001111,
    B00000010,
    B00000010,
    B00000010,
    B00000010 };
const int aU[] PROGMEM = {
    B00000100,
    B00000100,
    B00010101,
    B00001110,
    B00000100 };
const int aV[] PROGMEM = {
    B00010001,
    B00001010,
    B00000100,
    B00000100,
    B00000100 };
const int aW[] PROGMEM = {
    B00011111,
    B00010001,
    B00010001,
    B00010001,
    B00011111 };
const int aX[] PROGMEM = {
    B00000100,
    B00001010,
    B00010001,
    B00010001,
    B00011111 };
const int aY[] PROGMEM = {
    B00010011,
    B00010101,
    B00001010,
    B00001010,
    B00000100 };
const int aZ[] PROGMEM = {
    B00010110,
    B00010101,
    B00010000,
    B00010001,
    B00011111 };
const int aZZ[] PROGMEM = {
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000 };

// retrieve latin alphabet letter from progam memory
void getAurabeshLetter(int* letterbitmap, char let)
{
  // pLetter will be a pointer to program memory
  const int* pLetter;
  
  // get pointer to program memory from character
  switch (let)
  {
    case '2': pLetter=a2; break;
    case 'A': pLetter=aA; break;
    case 'B': pLetter=aB; break;
    case 'C': pLetter=aC; break;
    case 'D': pLetter=aD; break;
    case 'E': pLetter=aE; break;
    case 'F': pLetter=aF; break;
    case 'G': pLetter=aG; break;
    case 'H': pLetter=aH; break;
    case 'I': pLetter=aI; break;
    case 'J': pLetter=aJ; break;
    case 'K': pLetter=aK; break;
    case 'L': pLetter=aL; break;
    case 'M': pLetter=aM; break;
    case 'N': pLetter=aN; break;
    case 'O': pLetter=aO; break;
    case 'P': pLetter=aP; break;
    case 'Q': pLetter=aQ; break;
    case 'R': pLetter=aR; break;
    case 'S': pLetter=aS; break;
    case 'T': pLetter=aT; break;
    case 'U': pLetter=aU; break;
    case 'V': pLetter=aV; break;
    case 'W': pLetter=aW; break;
    case 'X': pLetter=aX; break;
    case 'Y': pLetter=aY; break;
    case 'Z': pLetter=aZ; break;
    case ' ': pLetter=aZZ; break;
    default : pLetter=aZZ; break;
    break;
  }
  
  // move data back from program memory to RAM
  for(byte i=0; i<5; i++)
  {
      letterbitmap[i]=(int)pgm_read_word(&(pLetter[i]));
  }
}

// Draws in a letter on the LED grid     
// shift=0 draws starting from the left edge (column 0)
// shift>0 slide letter further towards the right
// shift<0 slide letter further towards the left (becomes only partly visible)
void drawLetter(byte display, char let, int shift)
{
  
  // return immediately if the letter won't show
  if(shift < -LETTERWIDTH ||  shift>27) return;
  
  // allocate RAM space for the bitmap
  int letterBitmap[5];
  
  // retrieve letter bitmap from program memory to RAM using either alphabet
  switch(alphabetType[display])
  {
    case 0:
      getLatinLetter(letterBitmap, let);
      break;
    case 1:
      getAurabeshLetter(letterBitmap, let);
      break;
    default:
      getLatinLetter(letterBitmap, let);
      break;
  }
  
  //loop thru rows of the letter
  // shift=0 draws starting from the left edge (column 0)
  // shift>0 slide letter further towards the right
  // shift<0 slide letter further towards the left (becomes only partly visible)
  for (byte i=0; i<5; i++) 
  {
    if (shift>0) //positive shift means letter is slid to the right on the display
      LEDgrid[display][i] |= ((long)letterBitmap[i]) << shift;
    else //negative shift means letter is slid to the left so that only part of it is visible
      LEDgrid[display][i] |= ((long)letterBitmap[i]) >> -shift;
  }
}


// Scrolls the given text string on the given display
// Call repeatedly to keep the text scrolling.
// A scrolling count global tells you how many times the string has scrolled entirely
// There is no reset of the original position, should be accomplished elsewhere
// (9, 9 and 27 respectively)
void scrollText(byte display, char text[])
{
  static unsigned long previousTextScroll[3];
  
  // wait until next update cycle
  unsigned long currentMillis = millis();
  if((currentMillis - previousTextScroll[display]) < SCROLLspeed) return;
  previousTextScroll[display] = currentMillis;

  // LED grid to all off
  clearGrid(display);
   
  // draw all letters in the grid, scrolled according to the global scrollPosition[display]
  // each letter is moved 5 pixels from the next letter.
  // Positive scroll means moves towards the right
  // So scrollPosition should start at the last column and be decremented
  for (unsigned int i=0; i<strlen(text); i++)
  {
    int shift=i*LETTERWIDTH + scrollPositions[display];
    //if(shift > -(LETTERWIDTH+1) &&  shift<28)
    {
      drawLetter(display, text[i], shift);
    }
  }
  
  // this moves the text one step to the left, it will eventually become negative
  // and some text will start to disappear
  scrollPositions[display]--;
  
  // if the whole text is off screen to the left
  if (scrollPositions[display] < -LETTERWIDTH*(int)strlen(text))
  {
    // resets the scroll to just off screen to the right
    if (display==2) scrollPositions[display]=27;
    else scrollPositions[display]=9;
    
    // increment global scroll count
    // Right now this is used once at startup to stop calling this in the loop 
    // once the startup text has scrolled once
    // warning there is no reset to this
    textScrollCount[display]++;
  }
  
  // show text on logics
  showGrid(display);
}


// =======================================================================================


#endif
