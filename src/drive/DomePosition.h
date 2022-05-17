#ifndef DomePosition_h
#define DomePosition_h

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
    virtual Mode getDomeMode() = 0;
    virtual Mode getDomeDefaultMode()
    {
        return getDomeMode();
    }
    virtual void setDomeMode(Mode mode) = 0;
    virtual void setDomeDefaultMode(Mode mode)
    {
        setDomeMode(mode);
    }
    virtual bool getDomeFlip() = 0;
    virtual float getDomeSpeedHome() = 0;
    virtual float getDomeMinSpeed()
    {
        return getDomeSpeedHome();
    }
    virtual float getDomeSeekSpeed()
    {
        return getDomeSpeedHome();
    }
    virtual unsigned getDomeFudge() = 0;
    virtual unsigned getDomeSeekLeft() = 0;
    virtual unsigned getDomeSeekRight() = 0;
    virtual unsigned getDomeMinDelay() = 0;
    virtual unsigned getDomeMaxDelay() = 0;
    virtual unsigned getDomeHome() = 0;
    virtual unsigned getDomeTargetPosition() = 0;
    virtual unsigned getDomePosition() = 0;
};

#endif
