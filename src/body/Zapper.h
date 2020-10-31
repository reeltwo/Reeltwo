#ifndef Zapper_h
#define Zapper_h

#include "core/AnimatedEvent.h"
#include "core/CommandEvent.h"

/**
  * \ingroup Dome
  *
  * \class Zapper
  *
  * \brief Controls a relay to a spark gap igniter to create some actual sparking
  *
  * Smoke will run for a maximum of 6.5 seconds with a minimum of 5 minute pause in between activations.
  *
  * \code
  * ANIMATION(zapper)
  * {
  *     DO_START()
  *     // Zapper arm
  *     DO_COMMAND("BZARM")
  *     // Zapper on
  *     DO_COMMAND("BZON")
  *     // Fake being dead for 8 seconds
  *     DO_WAIT_SEC(8)
  *     // Ok We are back!
  *     DO_COMMAND("ST$0109")
  *     // Close pies
  *     DO_ONCE({
  *         // Close pie panels
  *         servoDispatch.moveServosTo(PIE_PANELS_MASK, 150, 100, 2400);
  *     })
  *     DO_END()
  * }
  * \endcode
  */
class Zapper :
    public CommandEvent, AnimatedEvent
{
public:
    /**
      * \brief Constructor
      */
    Zapper(byte idPin, byte relayPin) :
        fID(idPin),
        fRelayPin(relayPin)
    {
        pinMode(fID, INPUT_PULLUP);
        pinMode(fRelayPin, OUTPUT);
        zapperDisarm();
    }

    bool isAttached()
    {
        return !digitalRead(fID);
    }

    /**
      * Arm zapper. Extra step to make sure zapper is not fired while inside the body.
      */
    void zapperArm()
    {
        fArmed = true;
    }

    /**
      * Disarm zapper. Extra step to make sure zapper is not fired while inside the body.
      */
    void zapperDisarm()
    {
        fArmed = false;
        zapperOff();
    }

    /**
      * Turn on the zapper.
      */
    void zapperOn()
    {
        if (fArmed && fCount == 0)
        {
            fCount = random(10, 20);
            fCount += !(fCount&1); // ensure count is odd
            fNextTime = millis();
        }
    }

    /**
      * Turn off the smoke machine
      */
    void zapperOff()
    {
        fCount = 0;
        fNextTime = 0;
        digitalWrite(fRelayPin, LOW);
    }

    /**
      * Zapper Commands start with 'BZ'
      */
    virtual void handleCommand(const char* cmd) override
    {
        if (*cmd++ == 'B' && *cmd++ == 'Z')
        {
            // Cannot arm/disarm zapper using commands. Zapper should be automatically
            // armed after opening door and raising arm. As soon as arm starts lowering
            // the zapper should be disarmed.
            if (cmd[0] == 'O' && cmd[1] == 'N' && cmd[2] == '\0')
            {
                zapperOn();
            }
            else if (cmd[0] == 'O' && cmd[1] == 'F' && cmd[2] == 'F' && cmd[3] == '\0')
            {
                zapperOff();
            }
        }
    }

    /**
      * Perform a single frame of zapper animation
      */
    virtual void animate() override
    {
        if (!fArmed || fCount == 0)
            return;
        uint32_t currentTime = millis();
        if (fNextTime < currentTime)
        {
            bool relayOn = (fCount-- & 1);
            if (relayOn)
            {
                digitalWrite(fRelayPin, LOW);
                fNextTime = currentTime + random(100,200);
            }
            else
            {
                digitalWrite(fRelayPin, HIGH);
                fNextTime = currentTime + random(200, 800);
            }
        }
    }

private:
    byte fID;
    byte fRelayPin;
    bool fArmed;
    uint8_t fCount;
    uint32_t fNextTime;
};

////////////////////////////////////////////////////////////////////////////////

#endif
