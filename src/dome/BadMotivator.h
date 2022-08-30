#ifndef BadMotivator_h
#define BadMotivator_h

#include "core/RelaySwitch.h"
#include "core/CommandEvent.h"

/**
  * \ingroup Dome
  *
  * \class BadMotivator
  *
  * \brief Controls a relay to a smoke machine and some LEDs
  *
  * Smoke will run for a maximum of 6.5 seconds with a minimum of 5 minute pause in between activations.
  *
  * \code
  * ANIMATION(badMotivator)
  * {
  *     DO_START()
  *     // Temp max volume
  *     DO_COMMAND_AND_WAIT("STtmpvol=100,15", 100)
  *     // Temp stop random sounds on main controller
  *     DO_COMMAND_AND_WAIT("STtmprnd=60", 100)
  *     // Short Circuit MP3 - play sound bank 8
  *     DO_COMMAND_AND_WAIT("ST$08", 500)
  *     // Smoke on and allow smoke to build up in dome
  *     DO_COMMAND_AND_WAIT("BMON", 3000)
  *     // Open pie panels
  *     DO_ONCE_AND_WAIT({
  *         // Open pie panels
  *         servoDispatch.moveServosTo(PIE_PANELS_MASK, 150, 100, 700);
  *     }, 500)
  *     // Spark fire strip
  *     DO_ONCE({ fireStrip.spark(500); })
  *     // Electrical Crackle MP3 -  sound bank 14
  *     DO_ONCE_AND_WAIT({ StealthCommand("ST$14"); }, 500)
  *     // Smoke off
  *     DO_COMMAND("BMOFF")
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
class BadMotivator :
    public CommandEvent, protected RelaySwitch
{
public:
    /**
      * \brief Constructor
      */
    BadMotivator(byte smokeRelayPin) :
    	RelaySwitch(smokeRelayPin, PAUSE_TIME)
    {
    }

    /**
      * Turn on the smoke machine. A minimum of 5 minutes have to pass in between activations
      * and the smoke will be automatically stopped after 6.5 seconds.
      */
    void smokeOn()
    {
        relayOn(MAXIMUM_TIME);
    }

    /**
      * Turn off the smoke machine
      */
    void smokeOff()
    {
        relayOff();
    }

    /**
      * BadMotivator Commands start with 'BM'
      */
    virtual void handleCommand(const char* cmd) override
    {
        if (*cmd++ == 'B' && *cmd++ == 'M')
        {
            if (cmd[0] == 'O' && cmd[1] == 'N' && cmd[2] == '\0')
            {
                smokeOn();
            }
            else if (cmd[0] == 'O' && cmd[1] == 'F' && cmd[2] == 'F' && cmd[3] == '\0')
            {
                smokeOff();
            }
        }
    }

private:
    static const uint32_t MAXIMUM_TIME = 6500L; /* 6.5 seconds */
    static const uint32_t PAUSE_TIME = 1*60*1000L; /* 1 minute */
};

////////////////////////////////////////////////////////////////////////////////

#endif
