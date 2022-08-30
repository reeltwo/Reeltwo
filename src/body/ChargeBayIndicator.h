#ifndef ChargeBayIndicator_h
#define ChargeBayIndicator_h

#include "ReelTwo.h"
#include "core/LedControlMAX7221.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include "core/CommandEvent.h"

/**
  * \ingroup Body
  *
  * \class ChargeBayIndicator
  *
  * \brief ChargeBayIndicator (CBI) controller
  *
  * ChargeBayIndicator (CBI) consists of a single MAX7221 device operating a 5x4 grid of LEDs plus 3 individual VCC indicator LEDs.
  *
  * \code
  * The 5x4 grid is arranged as follows. Only the top 5 bits of each byte are significant.
  *
  * Column# 0 1 2 3 4
  *
  * Bit     8 7 6 5 4
  *         ----------
  *         O O O O O |  row #0
  *         O O O O O |  row #1
  *         O O O O O |  row #2
  *         O O O O O |  row #3
  *
  *    Red LED    row #4 col #5   Bit pattern B00000100  (B10000000>>5)
  *    Yellow LED row #5 col #5   Bit pattern B00000100  (B10000000>>5)
  *    Green LED  row #6 col #5   Bit pattern B00000100  (B10000000>>5)
  * \endcode
  */
class ChargeBayIndicator : public AnimatedEvent, SetupEvent, CommandEvent
{
public:
    /**
      * \brief Constructor
      *
      * "analogInput" defaults to disabled
      */
    ChargeBayIndicator(LedControl& ledControl, int analogInput = -1) :
        fLC(ledControl),
        fID(ledControl.addDevice()),
        fDisplayEffectVal(kNormalVal),
        fPreviousEffectVal(~kNormalVal),
        fAnalogInput(analogInput)
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
        kFlicker = 2,
        kNaboo = 3,
        kCharging = 4,
        kBlink = 5,
        kHeart = 6,
        kVCCOnly = 7
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
        setVCCAnalogInputPin(fAnalogInput);
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
        }
        else if (now < fNextStepTimeMS)
        {
            return;
        }

        int selectSequence = (fDisplayEffectVal % 1000000) / 10000;
        int selectSpeed = (fDisplayEffectVal % 1000) / 100;
        int selectLength = (fDisplayEffectVal % 100);
        UNUSED(selectSpeed);

        getVCC();
        switch (selectSequence)
        {
            case kFlicker:
                fLC.setIntensity(fID, random(15));
                randomSEQ();
                break;
            case kNormal:
                randomSEQ();
                // displayVCC();
                // ESBoperatingSEQ();
                break;
            case kDisabled:
                fDelayTime = 300;
                break;
            case kNaboo:
                displayVCC();
                operatingSEQ();
                break;
            case kCharging:
                chargingSEQ();
                break;
            case kBlink:
                blinkSEQ();
                break;
            case kHeart:
                displayVCC();
                heartSEQ();
                break;
            case kVCCOnly:
                displayVCC();
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
      * ChargeBayIndicator Commands start with 'CB'
      */
    virtual void handleCommand(const char* cmd) override
    {
        if (*cmd++ == 'C' && *cmd++ == 'B')
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
      * This function is only for the grid pattern not the VCC LEDs.
      * Only the top 5 bits of each byte are significant. Shift up 3 to allow
      * our B-style source code constants to reflect what we see displayed.
      */
    inline void setRow(byte row, byte data)
    {
        fLC.setRow(fID, row, data << 3);
    }

    /**
      * Turn the green LED on or off.
      */
    inline void setGreenLight(bool on)
    {
        fLC.setLed(fID, 6, 5, on);
    }

    /**
      * Turn the yellow LED on or off.
      */
    inline void setYellowLight(bool on)
    {
        fLC.setLed(fID, 5, 5, on);
    }

    /**
      * Turn the red LED on or off.
      */
    inline void setRedLight(bool on)
    {
        fLC.setLed(fID, 4, 5, on);
    }

    /**
      * Set the green/yellow/red LEDs based on the voltage levels read from the voltage divider.
      */
    void displayVCC()
    {
        setGreenLight((fVCC >= GREEN_VCC));
        setYellowLight((fVCC >= YELLOW_VCC));
        setRedLight(true);
    }

    /**
      * Update LEDs randomly for one frame
      */
    void randomSEQ()
    {
        setRow(random(4), fLC.randomRow(random(4)));
        if (random(100) < 30)
            setGreenLight(random(2));
        if (random(100) < 20)
            setYellowLight(random(2));
        if (random(100) < 10)
            setRedLight(random(2));
        fDelayTime = 300;
    }

    /**
      * Test utility routine for blinking all LEDs every two seconds.
      */
    void blinkSEQ()
    {
        // Check if we are still on the same step
        switch (fSeqStep)
        {
            case 0:
                setRow(0, B00000);
                setRow(1, B00000);
                setRow(2, B00000);
                setRow(3, B00000);
                setGreenLight(false);
                setYellowLight(false);
                setRedLight(false);
                break;
            case 1:
                setRow(0, B11111);
                setRow(1, B11111);
                setRow(2, B11111);
                setRow(3, B11111);
                setGreenLight(true);
                setYellowLight(true);
                setRedLight(true);
                break;
        }
        fSeqStep = (++fSeqStep <= 1) ? fSeqStep : 0;
        fDelayTime = 2000;
    }

    /**
      * Pulse
      */
    void heartSEQ()
    {
        // used when monitorVCC == false
        switch (fSeqStep)
        {
            case 0:
                setRow(0, B01110);
                setRow(1, B00100);
                setRow(2, B00100);
                setRow(3, B01110);
                break;
            case 1:
                setRow(0, B01010);
                setRow(1, B11111);
                setRow(2, B01110);
                setRow(3, B00100);
                break;
            case 2:
                setRow(0, B01010);
                setRow(1, B01010);
                setRow(2, B01010);
                setRow(3, B01110);
                break;
        }
        fSeqStep = (++fSeqStep <= 2) ? fSeqStep : 0;
        fDelayTime = 1000;
    }

    /**
      * Animate the LEDs like ESB charging sequence
      */
    void chargingSEQ()
    {
        // used when monitorVCC == false
        switch (fSeqStep)
        {
            case 0:
                setRow(0, B11100);
                setRow(1, B11100);
                setRow(2, B11100);
                setRow(3, B00010);
                setGreenLight(true);
                break;
            case 1:
                setRow(1, B00000110);
                break;
            case 2:
                setRow(1, B11100);
                setRow(2, B00010);
                setRow(3, B11100);
                setYellowLight(true);
                break;
            case 3:
                setRow(0, B00010);
                setRow(1, B11100);
                setRow(2, B11100); 
                setRow(3, B00010);
                setRedLight(true);
                break;
            case 4:
                setRow(0, B11100);
                setRow(1, B11100); 
                setRow(2, B11100);
                setRow(3, B11100); 
                break;
            case 5:
                setRow(0, B11100);
                setRow(1, B00010);
                setRow(1, B00010);
                setRow(3, B00010);
                break;
            case 6:
                setRow(0, B00010);
                setRow(1, B11100); 
                setRow(2, B00010);
                setRow(3, B00010);
                setRedLight(false);
                break;
            case 7:
                setRow(0, B11100);
                setRow(1, B11100);
                setRow(2, B00010);
                setRow(3, B00010);
                setYellowLight(false);
                break;
            case 8:
                setRow(0, B00011);
                setRow(1, B11100);
                setRow(2, B00010);
                setRow(3, B11100);
                setGreenLight(false);
                break;
            case 9:
                setRow(0, B00010);
                setRow(1, B11100);
                setRow(2, B11100);
                setRow(3, B11100);
                break;
        }
        fSeqStep = (++fSeqStep <= 9) ? fSeqStep : 0;
        fDelayTime = 300;
    }

    /**
      * Operating sequence
      */
    void operatingSEQ()
    {
        switch (fSeqStep)
        {
            case 0:
                setRow(0, B11100);
                setRow(1, B11100);
                setRow(2, B11110);
                setRow(3, B00011);
                break;
            case 1:
                setRow(1, B00011);
                break;
            case 2:
                setRow(1, B11100);
                setRow(2, B00011);
                setRow(3, B11100);
                break;
            case 3:
                setRow(0, B00011);
                setRow(1, B11100);
                setRow(2, B11100); 
                setRow(3, B00011);
                break;
            case 4:
                setRow(0, B11100);
                setRow(1, B11100); 
                setRow(2, B11100);
                setRow(3, B11100); 
                break;
            case 5:
                setRow(0, B11100);
                setRow(1, B00011);
                setRow(1, B00011);
                setRow(3, B00011);
                break;
            case 6:
                setRow(0, B00011);
                setRow(1, B11100); 
                setRow(2, B00011);
                setRow(3, B00011);
                break;
            case 7:
                setRow(0, B11100);
                setRow(1, B11100);
                setRow(2, B00011);
                setRow(3, B00011);
                break;
            case 8:
                setRow(0, B00011);
                setRow(1, B11100);
                setRow(2, B00011);
                setRow(3, B11100);
                break;
            case 9:
                setRow(0, B00011);
                setRow(1, B11100);
                setRow(2, B11100);
                setRow(3, B11100);
                break;
            case 10:
                fLC.clearDisplay(fID);
                break;
        }
        fSeqStep = (++fSeqStep <= 10) ? fSeqStep : 0;
        fDelayTime = 300;
    }

    /**
      * Specify the analog pint used by the voltage divider
      */
    void setVCCAnalogInputPin(int analogInput)
    {
        fAnalogInput = analogInput;
        if (fAnalogInput != -1)
            pinMode(fAnalogInput, INPUT);
    }

    /**
      * Read the voltage level from the voltage divider.
      */
    float getVCC()
    {
        fVCC = (fAnalogInput != -1) ? ((analogRead(fAnalogInput) * 5.0) / 1024.0) / (R2/(R1+R2)) : 0;
        return fVCC;
    }

private:
    LedControl& fLC;
    byte fID;
    int fSeqStep;
    unsigned long fNextStepTimeMS;
    unsigned long fEffectStartMillis;

    long fDisplayEffectVal;
    long fPreviousEffectVal;
    int fAnalogInput;                               // Analog Pin for the voltage reading. -1 for disabled.

    static constexpr float GREEN_VCC = 12.5;        // Green LED l21 on if above this voltage
    static constexpr float YELLOW_VCC = 12.0;       // Yellow LED l22 on if above this voltage & below greenVCC... below turn on only RED l23

    // For 15volts: R1=47k, R2=24k
    // For 30volts: R1=47k, R2=9.4k
    static constexpr float R1 = 47000.0;            // >> resistance of R1 in ohms << the more accurate these values are
    static constexpr float R2 = 24000.0;            // >> resistance of R2 in ohms << the more accurate the measurement will be

    float fVCC;                         // voltage measured by voltage divider circuit

    /* we always wait a bit between updates of the display */
    unsigned long fDelayTime = 300;
};

#if 0
    void ESBoperatingSEQ()
    {
        switch (fSeqStep)
        {
            case 0:
                setRow(0,0,B01101101);
                setRow(0,1,B00000110);
                setRow(0,2,B11011011);
                setRow(0,3,B00010011);
                setRow(0,4,B11000011);
                setRow(0,5,B01000011);
                setRow(0,6,B00110011);
                break;

            case 1:
                setRow(0,0,B01100101);
                setRow(0,1,B00000110);
                setRow(0,2,B10010111);
                setRow(0,3,B00001011);
                setRow(0,4,B00100001);
                setRow(0,5,B11000001);
                setRow(0,6,B10101011);
                break;
            case 2:
                setRow(0,0,B00000110);
                setRow(0,1,B00010011);
                setRow(0,2,B10110001);
                setRow(0,3,B00001011);
                setRow(0,4,B11101001);
                setRow(0,5,B10100000);
                setRow(0,6,B11101001);
                break;
            case 3:
                setRow(0,0,B10000111);
                setRow(0,1,B00010011);
                setRow(0,2,B10110111); 
                setRow(0,3,B11000010);
                setRow(0,4,B00000011);
                setRow(0,5,B10010000);
                setRow(0,6,B10001000);
                break;
            case 4:
                setRow(0,0,B01010001);
                setRow(0,1,B00010011);
                setRow(0,2,B10010111); 
                setRow(0,3,B11001000);
                setRow(0,4,B11001000);
                setRow(0,5,B10101001);
                setRow(0,6,B00000001);
                break;
            case 5:
                setRow(0,0,B01010101);
                setRow(0,1,B01010011);
                setRow(0,2,B00010011); 
                setRow(0,3,B01001100);
                setRow(0,4,B00000001);
                setRow(0,5,B00010000);
                setRow(0,6,B11000001);
                break;
            case 6:
                setRow(0,0,B00010011);
                setRow(0,1,B00010011);
                setRow(0,2,B00010011);
                setRow(0,3,B10100110);
                setRow(0,4,B10000001);
                setRow(0,5,B00010000);
                setRow(0,6,B11000001);
                break;
            case 7:
                setRow(0,0,B00010011);
                setRow(0,1,B00000110);
                setRow(0,2,B10011011);
                setRow(0,3,B00010011);
                setRow(0,4,B11001010);
                setRow(0,5,B11001000);
                setRow(0,6,B10100000);
                break;
            case 8:
                setRow(0,0,B00010011);
                setRow(0,1,B10100101);
                setRow(0,2,B10000010);
                setRow(0,3,B00110101);
                setRow(0,4,B10110000);
                setRow(0,5,B00000011);
                setRow(0,6,B11001000);
                break;
            case 9:
                setRow(0,0,B00001110);
                setRow(0,1,B00010011);
                setRow(0,2,B01000010); 
                setRow(0,3,B00000110);
                setRow(0,4,B10101011);
                setRow(0,5,B00000001);
                setRow(0,6,B10101011);
                break;
            case 10:
                setRow(0,0,B01000011);
                setRow(0,1,B00000110);
                setRow(0,2,B00010011); 
                setRow(0,3,B00000110);
                setRow(0,4,B10101011);
                setRow(0,5,B11101001);
                setRow(0,6,B00000011);
                break;
            case 11:
                setRow(0,0,B01100101);
                setRow(0,1,B00110011);
                setRow(0,2,B00010111); 
                setRow(0,3,B00000110);
                setRow(0,4,B10100000);
                setRow(0,5,B11101001);
                setRow(0,6,B10101011);
                break;
            case 12:
                setRow(0,0,B00100000);
                setRow(0,1,B00010011);
                setRow(0,2,B10001000); 
                setRow(0,3,B00011000);
                setRow(0,4,B00000001);
                setRow(0,5,B00000011);
                setRow(0,6,B11001000);
                break;
            case 13:
                setRow(0,0,B10000000);
                setRow(0,1,B00010011);
                setRow(0,2,B10110101); 
                setRow(0,3,B10010110);
                setRow(0,4,B00000001);
                setRow(0,5,B11101001);
                setRow(0,6,B11101001);
                break;
            case 14:
                setRow(0,0,B10100110);
                setRow(0,1,B01010001);
                setRow(0,2,B01010011); 
                setRow(0,3,B10100000);
                setRow(0,4,B11001000);
                setRow(0,5,B10110000);
                setRow(0,6,B00000011);
                break;
            case 15:
                setRow(0,0,B10101000);
                setRow(0,1,B01010001);
                setRow(0,2,B00010001); 
                setRow(0,3,B10000000);
                setRow(0,4,B11101001);
                setRow(0,5,B11101001);
                setRow(0,6,B10101011);
                break;
            case 16:
                setRow(0,0,B10000011);
                setRow(0,1,B00100000);
                setRow(0,2,B10010011); 
                setRow(0,3,B11000010);
                setRow(0,4,B10101011);
                setRow(0,5,B00000001);
                setRow(0,6,B00000011);
                break;
            case 17:
                setRow(0,0,B10000010);
                setRow(0,1,B00010011);
                setRow(0,2,B00010010); 
                setRow(0,3,B11000010);
                setRow(0,4,B00110000);
                setRow(0,5,B11101001);
                setRow(0,6,B00110000);
                break;
            case 18:
                setRow(0,0,B01100101);
                setRow(0,1,B00010011);
                setRow(0,2,B00100000); 
                setRow(0,3,B00000110);
                setRow(0,4,B11001000);
                setRow(0,5,B00011001);
                setRow(0,6,B01000001);
                break;
            case 19:
                setRow(0,0,B10100011);
                setRow(0,1,B00010011);
                setRow(0,2,B01010101); 
                setRow(0,3,B01100101);
                setRow(0,4,B00110000);
                setRow(0,5,B01010000);
                setRow(0,6,B01100001);
                break;
            case 20:
                setRow(0,0,B10000000);
                setRow(0,1,B10000000);
                setRow(0,2,B01010001); 
                setRow(0,3,B00001011);
                setRow(0,4,B11101001);
                setRow(0,5,B11001000);
                setRow(0,6,B11001000);
                break;
            case 21:
                setRow(0,0,B00000110);
                setRow(0,1,B00100000);
                setRow(0,2,B11011000); 
                setRow(0,3,B01001100);
                setRow(0,4,B10101011);
                setRow(0,5,B01010000);
                setRow(0,6,B00000010);
                break;
            case 22:
                setRow(0,0,B01000011);
                setRow(0,1,B00011000);
                setRow(0,2,B10011000); 
                setRow(0,3,B01000011);
                setRow(0,5,B00000010);
                setRow(0,6,B11001010);
                break;
            case 23:
                setRow(0,0,B00101100);
                setRow(0,1,B00010011);
                setRow(0,2,B01011010); 
                setRow(0,3,B01000011);
                setRow(0,4,B01010000);
                setRow(0,5,B10101011);
                setRow(0,6,B11001010);
                break;
            case 24:
                setRow(0,0,B10101010);
                setRow(0,1,B00100000);
                setRow(0,2,B01001100); 
                setRow(0,3,B01000010);
                setRow(0,4,B00000011);
                setRow(0,5,B11001010);
                setRow(0,6,B01010000);
                break;
            case 25:
                setRow(0,0,B00000110);
                setRow(0,1,B00000110);
                setRow(0,2,B10001100); 
                setRow(0,3,B00000110);
                setRow(0,4,B01001000);
                setRow(0,5,B01010000);
                setRow(0,6,B00000010);
                break;
            case 26:
                setRow(0,0,B00000110);
                setRow(0,1,B00010011);
                setRow(0,2,B00111000); 
                setRow(0,3,B01001100);
                setRow(0,4,B11001010);
                setRow(0,5,B00000001);
                setRow(0,6,B10101011);
                break;
            case 27:
                setRow(0,0,B00000110);
                setRow(0,1,B00010011);
                setRow(0,2,B01001100); 
                setRow(0,3,B01000011);
                setRow(0,4,B00000011);
                setRow(0,5,B11001000);
                setRow(0,6,B11001000);
                break;
        }
        fSeqStep = (++fSeqStep <= 10) ? fSeqStep : 0;
        fDelayTime = 300;
    }
#endif

#endif
