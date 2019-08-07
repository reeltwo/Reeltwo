#ifndef SoftPot_h
#define SoftPot_h

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"

/**
  * \ingroup Core
  *
  * \class SoftPot
  *
  * \brief Encapsulate a soft potentiometer. Value is read once every cycle.
  *
  * Example usage:
  * \code
  *  SoftPot softPot(A0);
  *  Serial.println(softPot.getValue());
  * \endcode
  */
class SoftPot :
    public SetupEvent, AnimatedEvent
{
public:
    /**
      * \brief Default Constructor
      */
    SoftPot(const byte pin) :
        fPin(pin),
        fValue(0)
    {}

    /**
      * \returns last read value
      */
    virtual int getValue()
    {
        return fValue;
    }

    /**
      * Setup pin mode for analog read
      */
    virtual void setup() override
    {
        pinMode(fPin, INPUT);
    }

    /**
      * Read the pot once through the loop
      */
    virtual void animate() override
    {
        fValue = analogRead(fPin);
    }

protected:
    byte fPin;
    int fValue;
};

#endif
