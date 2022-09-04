#ifndef RotaryEncoder_h
#define RotaryEncoder_h

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include "core/PinInterruptHandler.h"

/**
  * \ingroup Core
  *
  * \class RotaryEncoder
  *
  * \brief
  *
  * Decodes the signals from a rotary encoder (quadrature pulses) and translates
  * them into a counter position.
  */
class RotaryEncoder: public SetupEvent, AnimatedEvent, PinInterruptHandler
{
public:
    enum class Direction
    {
        kNoRotation = 0,
        kClockwise = 1,
        kCounterClockwise = -1
    };

    enum class LatchMode
    {
        kFour3 = 1, // 4 steps, Latch at position 3 only (compatible to older versions)
        kFour0 = 2, // 4 steps, Latch at position 0 (reverse wirings)
        kTwo03 = 3  // 2 steps, Latch at position 0 and 3 
    };

    RotaryEncoder(byte pin1, byte pin2, LatchMode mode = LatchMode::kFour0, bool useInterrupt = true) :
        fPin1(pin1),
        fPin2(pin2),
        fMode(mode),
        fUseInterrupt(useInterrupt ? 2 : 0)
    {
        pinMode(fPin1, INPUT_PULLUP);
        pinMode(fPin2, INPUT_PULLUP);

        // when not started in motion, the current
        // state of the encoder should be 3
        int sig1 = digitalRead(fPin1);
        int sig2 = digitalRead(fPin2);
        fOldState = sig1 | (sig2 << 1);

        // start with position 0;
        fPosition = 0;
        fPositionExt = 0;
        fPositionExtPrev = 0;
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
        if (fUseInterrupt == 2)
            fUseInterrupt = (attachInterrupt(fPin1) && attachInterrupt(fPin2));
    }

    void end()
    {
        if (fUseInterrupt == 1)
        {
            detachInterrupt(fPin1);
            detachInterrupt(fPin2);
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
        return fPositionExt;
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
        RotaryEncoder::Direction ret = Direction::kNoRotation;

        if (fPositionExtPrev > fPositionExt)
        {
            ret = Direction::kCounterClockwise;
            fPositionExtPrev = fPositionExt;
        }
        else if (fPositionExtPrev < fPositionExt)
        {
            ret = Direction::kClockwise;
            fPositionExtPrev = fPositionExt;
        }
        else
        {
            ret = Direction::kNoRotation;
            fPositionExtPrev = fPositionExt;
        }
        return ret;
    }

    /**
      * Sets the current value
      */
    void setValue(long newValue)
    {
        switch (fMode)
        {
            case LatchMode::kFour3:
            case LatchMode::kFour0:
                // only adjust the external part of the position.
                fPosition = ((newValue << 2) | (fPosition & 0x03L));
                fPositionExt = newValue;
                fPositionExtPrev = newValue;
                break;

            case LatchMode::kTwo03:
                // only adjust the external part of the position.
                fPosition = ((newValue << 1) | (fPosition & 0x01L));
                fPositionExt = newValue;
                fPositionExtPrev = newValue;
                break;
        }
    }

    /**
      * Returns time in milliseconds between the current observed
      */
    uint32_t getMillisBetweenRotations() const
    {
        return (fPositionExtTime - fPositionExtTimePrev);
    }

    /**
      * Returns the RPM
      */
    uint32_t getRPM()
    {
        // calculate max of difference in time between last position changes or last change and now.
        uint32_t timeBetweenLastPositions = fPositionExtTime - fPositionExtTimePrev;
        uint32_t timeToLastPosition = millis() - fPositionExtTime;
        uint32_t t = max(timeBetweenLastPositions, timeToLastPosition);
        return 60000.0 / ((float)(t * 20));
    }

protected:
    virtual void interrupt() override
    {
        enum {
            kLatch0 = 0, // input state at position 0
            kLatch3 = 3  // input state at position 3
        };
        static const int8_t KNOBDIR[] = {
            0, -1, 1, 0,
            1, 0, 0, -1,
            -1, 0, 0, 1,
            0, 1, -1, 0
        };
        int sig1 = digitalRead(fPin1);
        int sig2 = digitalRead(fPin2);
        int8_t thisState = sig1 | (sig2 << 1);
        if (fOldState != thisState)
        {
            fPosition += KNOBDIR[thisState | (fOldState << 2)];
            fOldState = thisState;

            switch (fMode)
            {
                case LatchMode::kFour3:
                    if (thisState == kLatch3)
                    {
                        // The hardware has 4 steps with a latch on the input state 3
                        fPositionExt = fPosition >> 2;
                        fPositionExtTimePrev = fPositionExtTime;
                        fPositionExtTime = millis();
                    }
                    break;

                case LatchMode::kFour0:
                    if (thisState == kLatch0)
                    {
                        // The hardware has 4 steps with a latch on the input state 0
                        fPositionExt = fPosition >> 2;
                        fPositionExtTimePrev = fPositionExtTime;
                        fPositionExtTime = millis();
                    }
                    break;

                case LatchMode::kTwo03:
                    if ((thisState == kLatch0) || (thisState == kLatch3))
                    {
                        // The hardware has 2 steps with a latch on the input state 0 and 3
                        fPositionExt = fPosition >> 1;
                        fPositionExtTimePrev = fPositionExtTime;
                        fPositionExtTime = millis();
                    }
                    break;
            }
        }
    }

private:
    byte fPin1;
    byte fPin2; // Arduino pins used for the encoder.
    LatchMode fMode; // Latch mode from initialization
    uint8_t fUseInterrupt;

    volatile int8_t fOldState;

    long fValue = 0;
    volatile long fPosition = 0;        // Internal position (4 times fPositionExt)
    volatile long fPositionExt = 0;     // External position
    volatile long fPositionExtPrev = 0; // External position (used only for direction checking)

    uint32_t fPositionExtTime = 0;     // The time the last position change was detected.
    uint32_t fPositionExtTimePrev = 0; // The time the previous position change was detected.
};
#endif
