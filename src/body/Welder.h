#ifndef Welder_h
#define Welder_h

#include "core/AnimatedEvent.h"
#include "core/CommandEvent.h"

/**
  * \ingroup Dome
  *
  * \class Welder
  *
  * \brief Controls a relay to a spark gap igniter to create some actual sparking
  *
  * Smoke will run for a maximum of 6.5 seconds with a minimum of 5 minute pause in between activations.
  *
  * \code
  * ANIMATION(welder)
  * {
  *     DO_START()
  *     // Welder arm
  *     DO_COMMAND("WLARM")
  *     // Welder on
  *     DO_COMMAND("WLON")
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
class Welder :
    public CommandEvent, AnimatedEvent
{
public:
    /**
      * \brief Constructor
      */
    Welder(byte idPin, byte relayPin) :
        fID(idPin),
        fRelayPin(relayPin)
    {
        pinMode(fID, INPUT_PULLUP);
        pinMode(fRelayPin, OUTPUT);
        welderDisarm();
    }

    bool isAttached()
    {
        return !digitalRead(fID);
    }

    /**
      * Arm Welder.
      */
    void welderArm()
    {
        fArmed = true;
    }

    /**
      * Disarm welder.
      */
    void welderDisarm()
    {
        fArmed = false;
        welderOff();
    }

    /**
      * Turn on the welder.
      */
    void welderOn()
    {
        if (fArmed && fCount == 0)
        {
            fCount = random(80, 120);
            fCount += !(fCount&1); // ensure count is odd
            fNextTime = millis();
        }
    }

    /**
      * Turn off the smoke machine
      */
    void welderOff()
    {
        fCount = 0;
        fNextTime = 0;
        digitalWrite(fRelayPin, LOW);
    }

    /**
      * Welder Commands start with 'WL'
      */
    virtual void handleCommand(const char* cmd) override
    {
        if (*cmd++ == 'W' && *cmd++ == 'L')
        {
            // Cannot arm/disarm welder using commands. Welder should be automatically
            // armed after opening door and raising arm. As soon as arm starts lowering
            // the welder should be disarmed.
            if (cmd[0] == 'O' && cmd[1] == 'N' && cmd[2] == '\0')
            {
                welderOn();
            }
            else if (cmd[0] == 'O' && cmd[1] == 'F' && cmd[2] == 'F' && cmd[3] == '\0')
            {
                welderOff();
            }
        }
    }

    /**
      * Perform a single frame of welder animation
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
                fNextTime = currentTime + random(200);
            }
            else
            {
                digitalWrite(fRelayPin, HIGH);
                fNextTime = currentTime + random(20,120);
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
