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
                return getDomeSeekSpeed();
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

    float getDomeSeekSpeed()
    {
        return float(fDomeSpeedSeek) / 100.0;
    }

    unsigned getDomeFudge()
    {
        return fDomeFudge;
    }

    unsigned getDomeSeekLeft()
    {
        return fDomeSeekLeft;
    }

    unsigned getDomeSeekRight()
    {
        return fDomeSeekRight;
    }

    unsigned getDomeSeekMinDelay()
    {
        return fDomeSeekMinDelay;
    }

    unsigned getDomeSeekMaxDelay()
    {
        return fDomeSeekMaxDelay;
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
                return getDomeSeekMinDelay();
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
                return getDomeSeekMaxDelay();
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
            fLastAngle = angle;
            fLastChangeMS = millis();
        }
        return angle;
    }

    int getRelativeDegrees()
    {
        return fRelativeDegrees;
    }

    void resetRelativeDegrees()
    {
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

    inline void setDomeSeekSpeed(uint8_t speed)
    {
        fDomeSpeedSeek = speed;
    }

    inline void setDomeSeekMinDelay(uint8_t sec)
    {
        fDomeSeekMinDelay = sec;
    }

    inline void setDomeSeekMaxDelay(uint8_t sec)
    {
        fDomeSeekMaxDelay = sec;
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

    inline void setDomeSeekLeftDegrees(uint8_t degrees)
    {
        fDomeSeekLeft = degrees;
    }

    inline void setDomeSeekRightDegrees(uint8_t degrees)
    {
        fDomeSeekRight = degrees;
    }

    inline void setTargetReached(void (*reached)())
    {
        fTargetReached = reached;
    }

    inline void setHomeTargetReached(void (*reached)())
    {
        fHomeTargetReached = reached;
    }

    inline void setSeekTargetReached(void (*reached)())
    {
        fSeekTargetReached = reached;
    }

    void reachedTarget()
    {
        if (fTargetReached != nullptr)
            fTargetReached();
    }

    void reachedHomeTarget()
    {
        if (fHomeTargetReached != nullptr)
            fHomeTargetReached();
    }

    void reachedSeekTarget()
    {
        if (fSeekTargetReached != nullptr)
            fSeekTargetReached();
    }

private:
    DomePositionProvider &fProvider;
    Mode fDomeMode = kOff;
    Mode fDomeDefaultMode = kOff;
    bool fDomeFlip = false;
    uint16_t fDomeHome = 0;
    uint16_t fDomeTargetPos = fDomeHome;
    long fDomeRelativeTargetPos = 0;
    uint8_t fDomeSeekMinDelay = 6;
    uint8_t fDomeSeekMaxDelay = 8;
    uint8_t fDomeHomeMinDelay = 6;
    uint8_t fDomeHomeMaxDelay = 8;
    uint8_t fDomeTargetMinDelay = 6;
    uint8_t fDomeTargetMaxDelay = 8;
    uint8_t fDomeSeekRight = 80;
    uint8_t fDomeSeekLeft = 80;
    uint8_t fDomeFudge = 5;
    uint8_t fDomeSpeedHome = 40;
    uint8_t fDomeSpeedTarget = 40;
    uint8_t fDomeSpeedMin = 15;
    uint8_t fDomeSpeedSeek = 30;
    uint8_t fTimeout = 5;
    unsigned fLastAngle = ~0;
    uint32_t fLastChangeMS = 0;
    unsigned fRelativeDegrees = 0;
    void (*fTargetReached)() = nullptr;
    void (*fHomeTargetReached)() = nullptr;
    void (*fSeekTargetReached)() = nullptr;

    static bool withinArc(double p1, double p2, double p3)
    {
        return fmod(p2 - p1 + 2*360, 360) >= fmod(p3 - p1 + 2*360, 360);
    }
};

#endif
