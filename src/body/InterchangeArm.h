#ifndef InterchangeArm_h
#define InterchangeArm_h

#include "core/AnimatedEvent.h"
#include "core/CommandEvent.h"

/**
  * \ingroup Body
  *
  * \class InterchangeArm
  *
  * \brief Base class for Zapper, Gripper, and Welder
  *
  */
class InterchangeArm :
    public CommandEvent, AnimatedEvent
{
public:
    InterchangeArm(byte idPin) :
        fID(idPin),
        fArmed(false)
    {
        disarm();
    }

    inline bool isArmed()
    {
        return fArmed;
    }

    /**
      * Arm. Extra step to make sure arm is not fired while inside the body.
      */
    void arm()
    {
        fArmed = true;
    }

    /**
      * Disarm. Extra step to make sure arm is not fired while inside the body.
      */
    void disarm()
    {
        fArmed = false;
        off();
    }

    virtual void on() = NULL;
    virtual void off() = NULL;

    /**
      * Interchangable Arm Commands start with 'XC'
      */
    virtual void handleCommand(const char* cmd) override
    {
        if (*cmd++ == 'X' && *cmd++ == 'C')
        {
            // Cannot arm/disarm zapper using commands. Zapper should be automatically
            // armed after opening door and raising arm. As soon as arm starts lowering
            // the zapper should be disarmed.
            if (cmd[0] == 'O' && cmd[1] == 'N' && cmd[2] == '\0')
            {
                on();
            }
            else if (cmd[0] == 'O' && cmd[1] == 'F' && cmd[2] == 'F' && cmd[3] == '\0')
            {
                off();
            }
        }
    }

protected:
    bool fArmed;
};

////////////////////////////////////////////////////////////////////////////////

#endif
