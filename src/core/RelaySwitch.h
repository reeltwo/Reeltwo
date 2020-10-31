#ifndef RelaySwitch_h
#define RelaySwitch_h

#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include "core/CommandEvent.h"

/**
  * \ingroup Core
  *
  * \class RelaySwitch
  *
  * \brief Controls a relay
  *
  * Control a relay with the ability to specify maximum pause times between activated as well as auto shut-off timer.
  *
  */
class RelaySwitch :
    public AnimatedEvent
{
public:
    /**
      * \brief Constructor
      */
    RelaySwitch(byte relayPin, uint32_t pauseTime = 0) :
        fRelayPin(relayPin),
        fNextTime(0),
        fCutOffTime(0),
        fPauseTime(pauseTime)
    {
        // Smoke Machine Relay
        pinMode(fRelayPin, OUTPUT);
        digitalWrite(fRelayPin, LOW);
    }

    /**
      * Turn on the smoke machine. A minimum of 5 minutes have to pass in between activations
      * and the smoke will be automatically stopped after 6.5 seconds.
      */
    void relayOn(uint32_t switchOffMS = 0)
    {
        uint32_t currentTime = millis();
        if (fNextTime < currentTime)
        {
            digitalWrite(fRelayPin, HIGH);
            fCutOffTime = (switchOffMS != 0) ? currentTime + switchOffMS : 0;
            fNextTime = (fPauseTime != 0) ? currentTime + fPauseTime : 0;
        }
        else
        {
            /* Ignore any activation that happens before the minimum pause time */
        }
    }

    /**
      * Turn off the smoke machine
      */
    void relayOff()
    {
        digitalWrite(fRelayPin, LOW);
        fCutOffTime = 0;
    }

    /**
      * Check if smoke machine timer has expired
      */
    virtual void animate() override
    {
        if (fCutOffTime == 0)
            return;
        if (fCutOffTime < millis())
        {
            relayOff();
        }
    }

private:
    static const uint32_t MAXIMUM_TIME = 6500L; /* 6.5 seconds */
    byte fRelayPin;
    uint32_t fNextTime;
    uint32_t fCutOffTime;
    uint32_t fPauseTime;
};

////////////////////////////////////////////////////////////////////////////////

#endif
