#ifndef ServoFeedback_h
#define ServoFeedback_h

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"

/**
  * \ingroup Core
  *
  * \class ServoFeedback
  *
  * \brief Provides an interface to read analog feedback from a set of servos
  *
  * Implements ServoFeedback reads servo feedback from a given set of analog pins.
  *
  */
template <uint8_t NUM_SERVOS>
class ServoFeedback : public SetupEvent, AnimatedEvent
{
public:
    ServoFeedback(const uint8_t* analogPins /*PROGMEM*/) :
        fPins(analogPins),
        fCurrentServo(0),
        fLastRead(0)
    {
        for (uint8_t i = 0; i < NUM_SERVOS; i++)
            fPos[i] = 0;
    }

    uint8_t numServos() const
    {
        return NUM_SERVOS;
    }

    virtual void setup() override
    {
        for (uint8_t i = 0 ; i < NUM_SERVOS; i++)
        {
            uint8_t pin = pgm_read_byte(&fPins[i]);
            pinMode(pin, INPUT);
        }
    }

    virtual void animate() override
    {
        if (fLastRead + 15 < millis())
        {
            if (fCurrentServo < NUM_SERVOS)
            {
                uint8_t pin = pgm_read_byte(&fPins[fCurrentServo]);
                uint16_t val = analogRead(pin);
                fPos[fCurrentServo] = map(val, 100, 512, 0, 255);
                if (++fCurrentServo >= NUM_SERVOS)
                    fCurrentServo = 0;
            }
            fLastRead = millis();
        }
    }

    inline uint8_t getRaw(uint8_t servoNum)
    {
        return (servoNum <= NUM_SERVOS) ? fPos[servoNum] : 0;
    }

    uint8_t getDegrees(uint8_t servoNum)
    {
        return map(getRaw(servoNum), 0, 254, 0, 180);
    }

private:
    const uint8_t* fPins; /* PROGMEM */
    uint8_t fPos[NUM_SERVOS];
    uint8_t fCurrentServo;
    uint32_t fLastRead;
};

#endif
