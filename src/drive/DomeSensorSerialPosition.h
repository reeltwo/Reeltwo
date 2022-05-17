#ifndef DomeSensorSerialPosition_h
#define DomeSensorSerialPosition_h

#include "ReelTwo.h"
#include "drive/DomePosition.h"
#include "drive/DomeSensorRingSerialListener.h"

class DomeSensorSerialPosition : public DomePosition, protected DomeSensorRingSerialListener
{
public:
    DomeSensorSerialPosition(Stream& serial) :
        DomeSensorRingSerialListener(serial)
    {
    }

    inline bool ready()
    {
        return DomeSensorRingSerialListener::ready();
    }

    virtual Mode getDomeMode() override
    {
        if (!ready())
            return kOff;
        return fDomeMode;
    }

    virtual Mode getDomeDefaultMode() override
    {
        return fDomeDefaultMode;
    }

    virtual void setDomeMode(Mode mode) override
    {
        fDomeMode = mode;
    }

    virtual void setDomeDefaultMode(Mode mode) override
    {
        fDomeDefaultMode = mode;
        setDomeMode(mode);
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

    virtual float getDomeMinSpeed() override
    {
        return float(fDomeSpeedMin) / 100.0;
    }

    virtual float getDomeSeekSpeed() override
    {
        return float(fDomeSpeedSeek) / 100.0;
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
        return getAngle();
    }

    unsigned getHomeRelativeDomePosition()
    {
        return normalize(long(getDomePosition()) - long(getDomeHome()));
    }

    long normalize(long degrees)
    {
        degrees = (long)fmod(degrees, 360);
        if (degrees < 0)
            degrees += 360;
        return degrees;
    }

    bool isAtPosition(long degrees)
    {
        long fudge = getDomeFudge();
        degrees = normalize(degrees);
        return withinArc(degrees - fudge, degrees + fudge, getDomePosition());

    }

    void setDomeHomePosition(long degrees)
    {
        degrees = (long)fmod(degrees, 360);
        if (degrees < 0)
            degrees += 360;
        fDomeHome = normalize(degrees);
    }

    void setDomeTargetPosition(long degrees)
    {
        degrees = (long)fmod(degrees, 360);
        if (degrees < 0)
            degrees += 360;
        fDomeTargetPos = normalize(degrees);
    }

    void setDomeHomeRelativeTargetPosition(long degrees)
    {
        degrees = (long)fmod(degrees + getDomeHome(), 360);
        if (degrees < 0)
            degrees += 360;
        fDomeTargetPos = normalize(degrees);
    }

    void setDomeHomeRelativeHomePosition(long degrees)
    {
        degrees = (long)fmod(degrees + getDomeHome(), 360);
        if (degrees < 0)
            degrees += 360;
        fDomeHome = normalize(degrees);
    }

    inline void setDomeHomeSpeed(uint8_t speed)
    {
        fDomeSpeedHome = speed;
    }

    inline void setDomeMinSpeed(uint8_t speed)
    {
        fDomeSpeedMin = speed;
    }

    inline void setDomeSeekSpeed(uint8_t speed)
    {
        fDomeSpeedSeek = speed;
    }

    inline void setDomeMinDelay(uint8_t sec)
    {
        fDomeMinDelay = sec;
    }

    inline void setDomeMaxDelay(uint8_t sec)
    {
        fDomeMaxDelay = sec;
    }

    inline void setDomeFudgeFactor(uint8_t fudge)
    {
        fDomeFudge = fudge;
    }

    inline void setDomeSeekLeftDegrees(uint8_t degrees)
    {
        fDomeSeekLeft = degrees;
    }

    inline void setDomeSeekRightDegrees(uint8_t degrees)
    {
        fDomeSeekRight = degrees;
    }

private:
    Mode fDomeMode = kOff;
    Mode fDomeDefaultMode = kOff;
    bool fDomeFlip = false;
    uint16_t fDomeHome = 0;
    uint16_t fDomeTargetPos = fDomeHome;
    uint8_t fDomeMinDelay = 6;
    uint8_t fDomeMaxDelay = 8;
    uint8_t fDomeSeekRight = 80;
    uint8_t fDomeSeekLeft = 80;
    uint8_t fDomeFudge = 5;
    uint8_t fDomeSpeedHome = 40;
    uint8_t fDomeSpeedMin = 15;
    uint8_t fDomeSpeedSeek = 30;

    static bool withinArc(double p1, double p2, double p3)
    {
        return fmod(p2 - p1 + 2*360, 360) >= fmod(p3 - p1 + 2*360, 360);
    }
};

#endif
