#ifndef _DATAPANEL_H_
#define _DATAPANEL_H_

#include "ReelTwo.h"
#include "core/LedControlMAX7221.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include "core/CommandEvent.h"

/**
  * \ingroup Body
  *
  * \class DataPanel
  *
  * \brief DataPanel controller
  *
  * \code
  * DataPanel consists of a single MAX7221 device.
  *
  *  ----------------------------------
  *  Yellow Block (top row of LEDs) ROW #4
  *
  *  Column# 5 4 3 
  *
  *  Bit     3 4 5
  *          -----
  *          O O O
  *
  *  Solid top row pattern B00011100
  *
  *  Yellow Block (bottom row of LEDs)
  *
  *  Column# 0 1 2 
  *
  *  Bit     8 7 6
  *          -----
  *          O O O
  *
  *  Solid top row pattern B11100000
  *  ----------------------------------
  *  Green Block (top row of LEDs) ROW #5
  *
  *  Column# 5 4 3 
  *
  *  Bit     3 4 5
  *          -----
  *          O O O
  *
  *  Solid top row pattern B00011100
  *
  *  Green Block (bottom row of LEDs)
  *
  *  Column# 0 1 2 
  *
  *  Bit     8 7 6
  *          -----
  *          O O O
  *
  *  Solid top row pattern B11100000
  *  ----------------------------------
  *  Right side Red Yellow Green Bargraph ROW #2 and ROW #3
  *
  *  Vertical grid starting from top:
  *      B11111100 fills left side of LEDs  (ROW #2)
  *      B11111100 fills right side of LEDs (ROW #3)
  *
  *  ----------------------------------
  *  Blue Bargraph ROW #0
  *
  *  Vertical row starting from top:
  *      B11111100 fills LEDs  (ROW #0)
  *
  *  ----------------------------------
  *  Bottom LEDs ROW #1
  *
  *  Horizontal grid starting from left:
  *      B11100000 fills top row     (ROW #1)
  *      B00011100 fills bottom row  (ROW #1)
  * \endcode
  */
class DataPanel : public AnimatedEvent, SetupEvent, CommandEvent
{
public:
    /**
      * \brief Constructor
      */
    DataPanel(LedControl& ledControl) :
        fLC(ledControl),
        fID(ledControl.addDevice()),
        fDisplayEffectVal(kNormalVal),
        fPreviousEffectVal(~kNormalVal)
    {
    }

    enum EffectValue
    {
        kNormalVal = 0
    };
    enum Sequence
    {
        kNormal = 0,
        kDisabled = 1,
        kFlicker = 2
    };

    /**
      * Select the specified effect using a 32-bit integer.
      *
      *  \li Sequence (0-99) * 10000
      *  \li Speed (0-9) * 100
      *  \li Duration (0-99)
      */
    void selectEffect(long inputNum)
    {
        fDisplayEffectVal = inputNum;
        fPreviousEffectVal = ~fDisplayEffectVal;
    }

    /**
      * Select the specified effect sequence.
      */
    inline void setSequence(Sequence seq = kNormal, uint8_t speedScale = 0, uint8_t numSeconds = 0)
    {
        selectEffect((long int)seq * 10000L + (long int)speedScale * 100 + numSeconds);
    }

    /**
      * Perform any initialzation not possible in the constructor
      */
    virtual void setup() override
    {
        fLC.clearDisplay(fID);
        fLC.setIntensity(fID, 15);
        fLC.setPower(fID, true);
    }

    /**
      * Perform a single frame of LED animation based on the selected sequence.
      */
    virtual void animate() override
    {
        unsigned long now = millis();
        if (fDisplayEffectVal != fPreviousEffectVal)
        {
            fLC.setIntensity(fID, 15);
            fLC.clearDisplay(fID);
            fSeqStep = -1;
            fDelayTime = 50;
            fNextStepTimeMS = 0;
            fEffectStartMillis = millis();
            fPreviousEffectVal = fDisplayEffectVal;
            fLastTimeBar = now;
            fLastTimeBlue = now;
            fLastTimeRed = now;
            fLastTimeBottom = now;
            fLastTimeTop = now;
            fBarValue = 0;
            fBarDirection = 1;
        }
        else if (now < fNextStepTimeMS)
        {
            return;
        }

        int selectSequence = (fDisplayEffectVal % 1000000) / 10000;
        int selectSpeed = (fDisplayEffectVal % 1000) / 100;
        int selectLength = (fDisplayEffectVal % 100);
        UNUSED(selectSpeed);

        switch (selectSequence)
        {
            case kDisabled:
                break;
            case kFlicker:
                fLC.setIntensity(fID, random(15));
                // Fall through
            case kNormal:
                if (fLastTimeBar + BARGRAPHSPEED < now)
                {
                    byte chance = random(100);
                    byte displayValue = getBargraph();
                    /* 10% chance of changing direction */
                    if (displayValue == fBarValue && chance < 10)
                    {
                        fBarDirection = -fBarDirection;
                    }
                    /* 40% chance of moving */
                    if (displayValue == fBarValue && chance < 40)
                    {
                        fBarValue = min(max(0, fBarValue + fBarDirection), 6);
                        setBargraph(fBarValue);
                    }
                    /* 90% chance of blinking */
                    else if (chance < 90 && fBarValue > 0)
                    {
                        displayValue = (displayValue == fBarValue) ? displayValue - 1 : fBarValue;
                        setBargraph(displayValue);
                    }
                    fLastTimeBar = now;
                }
                if (fLastTimeBlue + BLUELEDSPEED < now)
                {
                    setBlueLed(fLC.randomRow(4));
                    fLastTimeBlue = now;
                }
                if (fLastTimeRed + REDLEDSPEED < now)
                {
                    setRed1Led(random(2));
                    setRed2Led(random(2));
                    fLastTimeRed = now;
                }
                if (fLastTimeBottom + BOTTOMLEDSPEED < now)
                {
                    setBottomLed(fLC.randomRow(4), fLC.randomRow(4));
                    fLastTimeBottom = now;
                }
                if (fLastTimeTop + TOPBLOCKSPEED < now)
                {
                    setYellowBlock(fLC.randomRow(4), fLC.randomRow(4));
                    setGreenBlock(fLC.randomRow(4), fLC.randomRow(4));
                    fLastTimeTop = TOPBLOCKSPEED;
                }
                fDelayTime = min(TOPBLOCKSPEED, min(BOTTOMLEDSPEED, min(REDLEDSPEED, min(BLUELEDSPEED, BARGRAPHSPEED))));
                break;
            default:
                selectEffect(kNormalVal);       //unknown effecct go back to normal
                break;
        }
        fNextStepTimeMS = now + fDelayTime;
        if (selectLength > 0 && millis() - fEffectStartMillis >= unsigned(selectLength) * 1000L)
        {
            selectEffect(kNormalVal); //go back to normal operation if its time
        }
    }

    /**
      * ChargeBayIndicator Commands start with 'DP'
      */
    virtual void handleCommand(const char* cmd) override
    {
        if (*cmd++ == 'D' && *cmd++ == 'P')
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

    /**
      * Set the yellow block LEDs
      */
    void setYellowBlock(byte top, byte bottom)
    {
        byte bits = fLC.getRow(fID, YELLOWBLOCK);
        fLC.setRow(fID, YELLOWBLOCK, (((top&7)<<5)|(bottom&7)<<2)|(bits&3));
    }

    /**
      * Set the green block LEDs
      */
    void setGreenBlock(byte top, byte bottom)
    {
        byte bits = fLC.getRow(fID, GREENBLOCK);
        fLC.setRow(fID, GREENBLOCK, ((top&7)<<5)|((bottom&7)<<2)|(bits&3));
    }

    /**
      * Set the Red #1 LED
      */
    void setRed1Led(bool on)
    {
        byte bits = fLC.getRow(fID, BARGRAPH_LEFT) & ~B10;
        if (on)
            bits |= B10;
        fLC.setRow(fID, BARGRAPH_LEFT, bits);
    }

    /**
      * Set the Red #2 LED
      */
    void setRed2Led(bool on)
    {
        byte bits = fLC.getRow(fID, BARGRAPH_RIGHT) & ~B10;
        if (on)
            bits |= B10;
        fLC.setRow(fID, BARGRAPH_RIGHT, bits);
    }

    /**
      * Set the bottom LEDs
      */
    void setBottomLed(byte top, byte bottom)
    {
        byte bits = fLC.getRow(fID, BOTTOMLED);
        fLC.setRow(fID, BOTTOMLED, (((top&7)<<5)|(bottom&7)<<2)|(bits&3));
    }

    /**
      * Set the blue LEDs
      */
    void setBlueLed(byte pattern)
    {
        fLC.setRow(fID, BLUELED, pattern<<2);
    }

    /**
      * Set the bargraph height
      *
      * 0 off
      * 1 green
      * 2 green, green
      * 3 green, green, yellow
      * 4 green, green, yellow, yellow
      * 5 green, green, yellow, yellow, red
      * 6 green, green, yellow, yellow, red, red
      */
    void setBargraph(byte value)
    {
        byte i = 0;
        byte bits = 0;
        value = min((int)value, 6);
        while (value > 0)
        {
            bits |= B100<<i;
            value--;
            i++;
        }
        fLC.setRow(fID, BARGRAPH_LEFT,  bits | (fLC.getRow(fID, BARGRAPH_LEFT) & 3));
        fLC.setRow(fID, BARGRAPH_RIGHT, bits | (fLC.getRow(fID, BARGRAPH_RIGHT) & 3));
    }

    /**
      * Get the current bargraph height
      */
    byte getBargraph()
    {
        byte i = 0;
        byte bits = fLC.getRow(fID, BARGRAPH_LEFT);
        if (bits == fLC.getRow(fID, BARGRAPH_RIGHT))
        {
            while (i < 6)
            {
                if (!(bits & (B100<<i)))
                    break;
                i++;
            }
        }
        return i;
    }

private:
    LedControl& fLC;
    byte fID;
    int fSeqStep;
    byte fBarPrevValue;
    signed char fBarValue;
    signed char fBarDirection;
    unsigned long fLastTimeBar;
    unsigned long fLastTimeBlue;
    unsigned long fLastTimeRed;
    unsigned long fLastTimeBottom;
    unsigned long fLastTimeTop;
    unsigned long fNextStepTimeMS;
    unsigned long fEffectStartMillis;

    long fDisplayEffectVal;
    long fPreviousEffectVal;

    /* we always wait a bit between updates of the display */
    unsigned long fDelayTime = 300;

    static const int YELLOWBLOCK = 4;
    static const int GREENBLOCK = 5;

    static const int BARGRAPH_LEFT = 2;
    static const int BARGRAPH_RIGHT = 3;

    static const int REDLED_LEFT = 2;
    static const int REDLED_RIGHT = 3;

    static const int BLUELED = 0;
    static const int BOTTOMLED = 1;

    static const int TOPBLOCKSPEED   = 70;
    static const int BOTTOMLEDSPEED  = 200;
    static const int REDLEDSPEED     = 500;
    static const int BLUELEDSPEED    = 500;
    static const int BARGRAPHSPEED   = 100;
};

#endif
