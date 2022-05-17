#ifndef DomeSensorAnalogPosition_h
#define DomeSensorAnalogPosition_h

#include "ReelTwo.h"
#include "drive/DomePosition.h"
#include "core/AnalogMonitor.h"

class DomeSensorAnalogPosition : public DomePosition
{
public:
    DomeSensorAnalogPosition(uint8_t analogPin) :
        fDomePosition(analogPin)
    {
    }

    bool ready()
    {
        return true;
    }

    virtual Mode getDomeMode() override
    {
        if (!ready())
            return kOff;
        return fDomeMode;
    }

    virtual void setDomeMode(Mode mode) override
    {
        fDomeMode = mode;
    }

    virtual bool getDomeFlip() override
    {
        return fDomeFlip;
    }

    virtual float easingMethod(float completion)
    {
        // TODO
        return completion;
    }

    virtual float getDomeSpeedHome() override
    {
        return float(fDomeSpeedHome) / 100.0;
    }

    virtual unsigned getDomeFudge() override
    {
        return fDomeFudge;
    }

    virtual unsigned getDomeSeekLeft() override
    {
        return fDomeSeekLeft;
    }

    virtual unsigned getDomeSeekRight() override
    {
        return fDomeSeekRight;
    }

    virtual unsigned getDomeMinDelay() override
    {
        return fDomeMinDelay;
    }

    virtual unsigned getDomeMaxDelay() override
    {
        return fDomeMaxDelay;
    }

    virtual unsigned getDomeHome() override
    {
        return fDomeHome;
    }

    virtual unsigned getDomeTargetPosition() override
    {
        return fDomeTargetPos;
    }

    virtual unsigned getDomePosition() override
    {
        fDomePosition.animate();
        unsigned val = fDomePosition.getValue();
        // Serial.print("val : "); Serial.print(val); Serial.print(" : ");
        return map(val, 0, 1024, 0, 359);
        // val = min(max(val, fParams.domespmin), fParams.domespmax);
        // int pos = map(val, fParams.domespmin, fParams.domespmax, 0, 359);
        // return 0;
    }

protected:
    AnalogMonitor fDomePosition;
    Mode fDomeMode = kOff;
    bool fDomeFlip = false;
    uint16_t fDomeHome = 0;
    uint16_t fDomeTargetPos = fDomeHome;
    uint8_t fDomeMinDelay = 6;
    uint8_t fDomeMaxDelay = 8;
    uint8_t fDomeSeekRight = 80;
    uint8_t fDomeSeekLeft = 80;
    uint8_t fDomeFudge = 5;
    uint8_t fDomeSpeedHome = 40;
    uint8_t fDomeSpeedSeek = 30;
};

#endif
