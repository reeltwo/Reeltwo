#ifndef DomePosition_h
#define DomePosition_h

#include "ReelTwo.h"
#include "drive/DomePositionProvider.h"

class DomePosition
{
public:
    enum Mode
    {
        kOff = 1,
        kHome,
        kRandom,
        kTarget
    };

    DomePosition(DomePositionProvider& provider) :
        fProvider(provider)
    {
    }

    inline bool ready()
    {
        return fProvider.ready();
    }

    Mode getDomeMode()
    {
        if (!ready())
            return kOff;
        return fDomeMode;
    }

    Mode getDomeDefaultMode()
    {
        return fDomeDefaultMode;
    }

    void setDomeMode(Mode mode)
    {
        fDomeMode = mode;
        fLastChangeMS = millis();
    }

    void setDomeDefaultMode(Mode mode)
    {
        fDomeDefaultMode = mode;
        setDomeMode(mode);
    }

    bool getDomeFlip()
    {
        return fDomeFlip;
    }

    float getDomeSpeed()
    {
        switch (getDomeMode())
        {
            case kHome:
                return getDomeSpeedHome();
            case kRandom:
                return getDomeAutoSpeed();
            case kTarget:
                return getDomeSpeedTarget();
            case kOff:
            default:
                return getDomeMinSpeed();
        }
    }

    float getDomeSpeedHome()
    {
        return float(fDomeSpeedHome) / 100.0;
    }

    float getDomeSpeedTarget()
    {
        return float(fDomeSpeedTarget) / 100.0;
    }

    float getDomeMinSpeed()
    {
        return float(fDomeSpeedMin) / 100.0;
    }

    float getDomeAutoSpeed()
    {
        return float(fDomeSpeedAuto) / 100.0;
    }

    unsigned getDomeFudge()
    {
        return fDomeFudge;
    }

    unsigned getDomeAutoLeft()
    {
        return fDomeAutoLeft;
    }

    unsigned getDomeAutoRight()
    {
        return fDomeAutoRight;
    }

    unsigned getDomeAutoMinDelay()
    {
        return fDomeAutoMinDelay;
    }

    unsigned getDomeAutoMaxDelay()
    {
        return fDomeAutoMaxDelay;
    }

    unsigned getDomeHomeMinDelay()
    {
        return fDomeHomeMinDelay;
    }

    unsigned getDomeHomeMaxDelay()
    {
        return fDomeHomeMaxDelay;
    }

    unsigned getDomeTargetMinDelay()
    {
        return fDomeTargetMinDelay;
    }

    unsigned getDomeTargetMaxDelay()
    {
        return fDomeTargetMaxDelay;
    }

    unsigned getDomeMinDelay()
    {
        switch (getDomeMode())
        {
            case kHome:
                return getDomeHomeMinDelay();
            case kRandom:
                return getDomeAutoMinDelay();
            case kTarget:
                return getDomeTargetMinDelay();
            case kOff:
            default:
                return 0;
        }
    }

    unsigned getDomeMaxDelay()
    {
        switch (getDomeMode())
        {
            case kHome:
                return getDomeHomeMaxDelay();
            case kRandom:
                return getDomeAutoMaxDelay();
            case kTarget:
                return getDomeTargetMaxDelay();
            case kOff:
            default:
                return 0;
        }
    }

    unsigned getDomeHome()
    {
        return fDomeHome;
    }

    unsigned getDomeTargetPosition()
    {
        return fDomeTargetPos;
    }

    long getDomeRelativeTargetPosition()
    {
        return fDomeRelativeTargetPos;
    }

    int shortestDistance(int origin, int target)
    {
        int result = 0.0;
        int diff = fmod(fmod(abs(origin - target), 360), 360);

        if (diff > 180)
        {
            //There is a shorter path in opposite direction
            result = (360 - diff);
            if (target > origin)
                result *= -1;
        }
        else
        {
            result = diff;
            if (origin > target)
                result *= -1;
        }
        return result;
    }

    unsigned getDomePosition()
    {
        unsigned angle = fProvider.getAngle();
        if (angle != fLastAngle)
        {
            if (fLastAngle < angle)
                fRelativeDegrees += shortestDistance(fLastAngle, angle);
            else
                fRelativeDegrees -= shortestDistance(angle, fLastAngle);
            fLastChangeMS = millis();
            fLastAngle = angle;
        }
        return angle;
    }

    int getRelativeDegrees()
    {
        return fRelativeDegrees;
    }

    void resetDefaultMode()
    {
        setDomeMode(getDomeDefaultMode());
        fDomeRelativeTargetPos = 0;
        fRelativeDegrees = 0;
    }

    void resetWatchdog()
    {
        fLastChangeMS = millis();
    }

    void setTimeout(uint8_t timeout)
    {
        fTimeout = timeout;
    }

    bool isTimeout()
    {
        return (ready() && fLastAngle != ~0u) ? uint32_t(fTimeout)*1000 < millis() - fLastChangeMS : true;
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
        fDomeRelativeTargetPos = 0;
    }

    void setDomeRelativeTargetPosition(long degrees)
    {
        // Save the starting position in the absolute target position
        fDomeTargetPos = getDomePosition();
        fDomeRelativeTargetPos = degrees;
        fRelativeDegrees = 0;
    }

    void setDomeHomeRelativeTargetPosition(long degrees)
    {
        setDomeTargetPosition(degrees + getDomeHome());
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

    inline void setDomeTargetSpeed(uint8_t speed)
    {
        fDomeSpeedTarget = speed;
    }

    inline void setDomeMinSpeed(uint8_t speed)
    {
        fDomeSpeedMin = speed;
    }

    inline void setDomeAutoSpeed(uint8_t speed)
    {
        fDomeSpeedAuto = speed;
    }

    inline void setDomeAutoMinDelay(uint8_t sec)
    {
        fDomeAutoMinDelay = sec;
    }

    inline void setDomeAutoMaxDelay(uint8_t sec)
    {
        fDomeAutoMaxDelay = sec;
    }

    inline void setDomeHomeMinDelay(uint8_t sec)
    {
        fDomeHomeMinDelay = sec;
    }

    inline void setDomeHomeMaxDelay(uint8_t sec)
    {
        fDomeHomeMaxDelay = sec;
    }

    inline void setDomeTargetMinDelay(uint8_t sec)
    {
        fDomeTargetMinDelay = sec;
    }

    inline void setDomeTargetMaxDelay(uint8_t sec)
    {
        fDomeTargetMaxDelay = sec;
    }

    inline void setDomeFudgeFactor(uint8_t fudge)
    {
        fDomeFudge = fudge;
    }

    inline void setDomeAutoLeftDegrees(uint8_t degrees)
    {
        fDomeAutoLeft = degrees;
    }

    inline void setDomeAutoRightDegrees(uint8_t degrees)
    {
        fDomeAutoRight = degrees;
    }

    inline void setTargetReached(void (*reached)())
    {
        fTargetReached = reached;
    }

    inline void setHomeTargetReached(void (*reached)())
    {
        fHomeTargetReached = reached;
    }

    inline void setAutoTargetReached(void (*reached)())
    {
        fAutoTargetReached = reached;
    }

    void reachedTarget()
    {
        if (fTargetReached != nullptr)
        {
            fTargetReached();
            fTargetReached = nullptr;
        }
    }

    void reachedHomeTarget()
    {
        if (fHomeTargetReached != nullptr)
            fHomeTargetReached();
    }

    void reachedAutoTarget()
    {
        if (fAutoTargetReached != nullptr)
            fAutoTargetReached();
    }

private:
    DomePositionProvider &fProvider;
    Mode fDomeMode = kOff;
    Mode fDomeDefaultMode = kOff;
    bool fDomeFlip = false;
    uint16_t fDomeHome = 0;
    uint16_t fDomeTargetPos = fDomeHome;
    long fDomeRelativeTargetPos = 0;
    uint8_t fDomeAutoMinDelay = 6;
    uint8_t fDomeAutoMaxDelay = 8;
    uint8_t fDomeHomeMinDelay = 6;
    uint8_t fDomeHomeMaxDelay = 8;
    uint8_t fDomeTargetMinDelay = 6;
    uint8_t fDomeTargetMaxDelay = 8;
    uint8_t fDomeAutoRight = 80;
    uint8_t fDomeAutoLeft = 80;
    uint8_t fDomeFudge = 5;
    uint8_t fDomeSpeedHome = 40;
    uint8_t fDomeSpeedTarget = 40;
    uint8_t fDomeSpeedMin = 15;
    uint8_t fDomeSpeedAuto = 30;
    uint8_t fTimeout = 5;
    unsigned fLastAngle = ~0;
    uint32_t fLastChangeMS = 0;
    unsigned fRelativeDegrees = 0;
    void (*fTargetReached)() = nullptr;
    void (*fHomeTargetReached)() = nullptr;
    void (*fAutoTargetReached)() = nullptr;

    static bool withinArc(double p1, double p2, double p3)
    {
        return fmod(p2 - p1 + 2*360, 360) >= fmod(p3 - p1 + 2*360, 360);
    }
};

#endif
