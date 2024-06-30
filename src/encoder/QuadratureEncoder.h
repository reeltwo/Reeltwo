#ifndef QuadratureEncoder_h
#define QuadratureEncoder_h

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include "core/PinInterruptHandler.h"

/**
  * \ingroup Core
  *
  * \class QuadratureEncoder
  *
  * \brief
  *
  */
class QuadratureEncoder: public SetupEvent, AnimatedEvent, PinInterruptHandler
{
public:
    enum Direction
    {
        kNoRotation = 0,
        kClockwise = 1,
        kCounterClockwise = -1
    };

    QuadratureEncoder(byte pin1, byte pin2, bool invert = false, bool useInterrupt = true) :
        fPin1(pin1),
        fPin2(pin2),
        fDir(invert ? -1 : 1),
        fUseInterrupt(useInterrupt)
    {
        pinMode(fPin1, INPUT);
        pinMode(fPin2, INPUT);

        fPrevValue = digitalRead(fPin1);
        fPosition = 0;
    }

    virtual void setup() override
    {
        begin();
    }

    virtual void animate() override
    {
        int32_t val = getValue();
        int32_t curval = fValue;
        if (!fUseInterrupt)
            interrupt();
        if (val != curval)
        {
            // if (fChangeNotify != NULL)
            //     fChangeNotify(val);
            fValue = val;
        }
    }

    void begin()
    {
        if (fUseInterrupt) {
            fUseInterrupt = (attachInterrupt(digitalPinToInterrupt(fPin1)) &&
                                attachInterrupt(digitalPinToInterrupt(fPin2)));
        }
    }

    void end()
    {
        if (fUseInterrupt)
        {
            detachInterrupt(digitalPinToInterrupt(fPin1));
            detachInterrupt(digitalPinToInterrupt(fPin2));
        }
    }

    /**
      * Returns true if value has changed since last animated event
      */
    inline bool hasChanged()
    {
        return (getValue() != fValue);
    }

    /**
      * Returns current value
      */
    long getValue()
    {
        return fPosition;
    }

    /**
      * Returns true if recently used (<250ms)
      */
    bool isActive()
    {
        return getValue() != 0 && getMillisBetweenRotations() < 250;
    }

    /**
      * Return the direction the knob was rotated last time.
      */
    Direction getDirection()
    {
        Direction ret = Direction::kNoRotation;

        if (fPositionPrev > fPosition)
        {
            ret = Direction::kCounterClockwise;
            fPositionPrev = fPosition;
        }
        else if (fPositionPrev < fPosition)
        {
            ret = Direction::kClockwise;
            fPositionPrev = fPosition;
        }
        else
        {
            ret = Direction::kNoRotation;
            fPositionPrev = fPosition;
        }
        return ret;
    }

    /**
      * Sets the current value
      */
    void setValue(long newValue)
    {
        fPosition = newValue;
    }

    /**
      * Returns time in milliseconds between the current observed
      */
    uint32_t getMillisBetweenRotations() const
    {
        return (fPositionTime - fPositionTimePrev);
    }

    /**
      * Returns the RPM
      */
    uint32_t getRPM()
    {
        // calculate max of difference in time between last position changes or last change and now.
        uint32_t timeBetweenLastPositions = fPositionTime - fPositionTimePrev;
        uint32_t timeToLastPosition = millis() - fPositionTime;
        uint32_t t = max(timeBetweenLastPositions, timeToLastPosition);
        return 60000.0 / ((float)(t * 20));
    }

    void resetChangedState() {
        fChangedState = 0;
    }

    uint32_t getChangedState() {
        return fChangedState;
    }

protected:
    virtual void interrupt() override
    {
        int valPin1 = digitalRead(fPin1);
        if (fPrevValue == LOW && valPin1 == HIGH) {
            if (digitalRead(fPin2) == LOW) {
                fPosition = fPosition - fDir;
            } else {
                fPosition = fPosition + fDir;
            }
            fPositionTimePrev = fPositionTime;
            fPositionTime = millis();
            fChangedState++;
        }
        fPrevValue = valPin1;
    }

private:
    byte fPin1;
    byte fPin2; // Arduino pins used for the encoder.
    int fDir = 1;
    bool fUseInterrupt = false;
    int fPrevValue;

    long fValue = 0;
    volatile long fPosition = 0;        // Internal position (4 times fPositionExt)
    volatile long fPositionPrev = 0;        // Internal position (4 times fPositionExt)
    volatile uint32_t fChangedState = 0;

    volatile uint32_t fPositionTime = 0;     // The time the last position change was detected.
    volatile uint32_t fPositionTimePrev = 0; // The time the previous position change was detected.
};
#endif
