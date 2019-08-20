
#ifndef TeecesPSI_h
#define TeecesPSI_h

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include "core/JawaEvent.h"
#include "core/LedControlMAX7221.h"

/**
  * \ingroup Dome
  *
  * \class TeecesPSI
  *
  * \brief Process State Indicator
  *
  * For further information on the hardware:
  *
  * https://astromech.net/forums/showthread.php?28369-TEECE-s-Light-Kits-(Assembled)-BC-Approved-255-(May-2016)-Open
  */
class TeecesPSI : public AnimatedEvent, SetupEvent, JawaEvent
{
public:
    TeecesPSI(LedControl& ledControl) :
        fLC(ledControl),
        fID(ledControl.addDevice()),
        fState(0),
        fPauseTime(0),
        fPause(1000+2000*random(3)),
        fAnimate(true),
        fDelay(50 + random(3) * 25),
        fStuck(15)
    {
    }

    virtual void setup() override
    {
        fLC.clearDisplay(fID);
        fLC.setIntensity(fID, 15);
        fLC.setPower(fID, true);
        fMillis = millis();
    }

    void setPower(bool on)
    {
        fLC.setPower(fID, on);
    }

    inline void setPause(unsigned int pause)
    {
        fPause = pause;
    }

    inline void setDelay(byte delay)
    {
        fDelay = delay;
    }

    void setStuckChance(byte chance)
    {
        fStuck = chance;
    }

    void setState(byte state)
    {
        //set PSI (0 or 1) to a state between 0 (full red) and 6 (full blue)
        // states 7-11 are moving backwards
        if (state > 6)
            state = 12 - state;
        for (byte col = 0; col < 6; col++)
        {
            byte mask = (col < state) ? B10101010 : B01010101;
            fLC.setColumn(fID, col, ((col & 1) == 0) ? mask : ~mask);
        }
        fState = state;
    }

    void setAnimate(bool animate)
    {
        fAnimate = animate;
    }

    void setSolidState(byte mask)
    {
        setAnimate(false);
        for (byte col = 0; col < 6; col++)
            fLC.setColumn(fID, col, mask);
    }

    virtual void jawaCommand(char cmd, int arg, int value) override
    {
        UNUSED_ARG(value)
        switch (cmd)
        {
            case 'P':
                switch (arg)
                {
                    case 0:
                        setPower(true);
                        setAnimate(true);
                        break;
                    case 1:
                        // Color 1
                        setPower(true);
                        setState(0);
                        break;
                    case 2:
                        // Color 2
                        setPower(true);
                        setState(0);
                        break;
                    case 3:
                        // Color Both
                        setPower(true);
                        setSolidState(~0);
                        break;
                    case 4:
                        setPower(false);
                        break;
                }
                break;
        }
    }

    virtual void animate()
    {
        unsigned long currentMillis = millis();
        if (fLC.isPowered(fID) && fAnimate && fDelay > 0 &&
            currentMillis - fMillis >= fPauseTime)
        {
            //time's up, do something...
            if (++fState == 12)
                fState = 0;
            if (fState != 0 && fState != 6)
            {
                //we're swiping...
                fPauseTime = fDelay;
            }
            else
            {
                //we're pausing
                fPauseTime = random(fPause);
                //decide if we're going to get 'stuck'
                if (random(100) <= fStuck)
                {
                    fState = (fState == 0) ? random(1, 3) : random(3, 5);
                }
            }
            setState(fState);
            fMillis = currentMillis;
        }
    }

private:
    LedControl& fLC;
    byte fID;
    byte fState;
    unsigned long fMillis;
    unsigned int fPauseTime;
    int fPause;
    bool fAnimate;
    byte fDelay;
    byte fStuck;
};

#endif
