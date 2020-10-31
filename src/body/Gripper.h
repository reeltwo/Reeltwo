#ifndef Gripper_h
#define Gripper_h

#include "core/AnimatedEvent.h"
#include "core/CommandEvent.h"

/**
  * \ingroup Dome
  *
  * \class Gripper
  *
  * \brief Controls a TB662 motor controller to open/close gripper hand
  *
  * Smoke will run for a maximum of 6.5 seconds with a minimum of 5 minute pause in between activations.
  *
  * \code
  * ANIMATION(gripper)
  * {
  *     DO_START()
  *     // Gripper arm
  *     DO_COMMAND("GPARM")
  *     // Gripper on
  *     DO_COMMAND("GPON")
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
class Gripper :
    public CommandEvent, AnimatedEvent
{
public:
    /**
      * \brief Constructor
      */
    Gripper(byte idPin, byte in1Pin, byte in2Pin, byte pwmPin, byte standbyPin,
            byte gripperOpenPin, byte gripperClosePin) :
        fID(idPin),
        fIn1Pin(in1Pin),
        fIn2Pin(in2Pin),
        fPWMPin(pwmPin),
        fStandbyPin(standbyPin),
        fGripperOpenPin(gripperOpenPin),
        fGripperClosePin(gripperClosePin)
    {
        pinMode(fID, INPUT_PULLUP);
        pinMode(fGripperOpenPin, INPUT_PULLUP);
        pinMode(fGripperClosePin, INPUT_PULLUP);
        pinMode(fIn1Pin, OUTPUT);
        pinMode(fIn2Pin, OUTPUT);
        pinMode(fPWMPin, OUTPUT);
        pinMode(fStandbyPin, OUTPUT);
        gripperDisarm();
    }

    bool isAttached()
    {
        return !digitalRead(fID);
    }

    bool isGripperOpen()
    {
      return !digitalRead(fGripperOpenPin);
    }

    bool isGripperClosed()
    {
        return !digitalRead(fGripperClosePin);
    }

    /**
      * Arm gripper.
      */
    void gripperArm()
    {
        fArmed = true;
    }

    /**
      * Disarm gripper.
      */
    void gripperDisarm()
    {
        fArmed = false;
        gripperOff();
    }

    /**
      * Turn on the gripper.
      */
    void gripperOn()
    {
        if (isGripperOpen())
        {
            DEBUG_PRINTLN("OPEN");
        }
        if (isGripperClosed())
        {
            DEBUG_PRINTLN("CLOSED");
        }
        if (fArmed && fCount == 0)
        {
            if (isGripperClosed())
            {
                fCount = random(20, 40);
                fDirection = 1000;
                fNextTime = millis();
            }
            else
            {
                DEBUG_PRINTLN("GRIPPER NOT CLOSED");
            }
        }
    }

    /**
      * Turn off the smoke machine
      */
    void gripperOff()
    {
        fCount = 0;
        fDirection = 0;
        fNextTime = 0;
        standby();
    }

    /**
      * Gripper Commands start with 'GP'
      */
    virtual void handleCommand(const char* cmd) override
    {
        if (*cmd++ == 'G' && *cmd++ == 'P')
        {
            // Cannot arm/disarm welder using commands. Gripper should be automatically
            // armed after opening door and raising arm. As soon as arm starts lowering
            // the gripper should be disarmed.
            if (cmd[0] == 'O' && cmd[1] == 'N' && cmd[2] == '\0')
            {
                gripperOn();
            }
            else if (cmd[0] == 'O' && cmd[1] == 'F' && cmd[2] == 'F' && cmd[3] == '\0')
            {
                gripperOff();
            }
        }
    }

    /**
      * Perform a single frame of welder animation
      */
    virtual void animate() override
    {
        if (!fArmed || fDirection == 0)
            return;
        uint32_t currentTime = millis();
        if (fNextTime < currentTime)
        {
            fNextTime = currentTime + random(150,350);
            if (fCount > 0)
            {
                fDirection = -fDirection;
                fCount--;
            }
            else
            {
                // Closing
                fDirection = -1000;
            }
            DEBUG_PRINT("fDirection: ");
            DEBUG_PRINTLN(fDirection);
        }
        if (fDirection > 0 && isGripperOpen())
        {
            // Fully open reverse direction
            fDirection = -fDirection;
            DEBUG_PRINTLN("Fully open reverse");
        }
        else if (fDirection < 0 && isGripperClosed())
        {
            // Fully closed reverse direction unless count is zero
            fDirection = (fCount > 0) ? -fDirection : 0;
            DEBUG_PRINTLN("Fully closed reverse");
        }

        if (fDirection == 0)
        {
            DEBUG_PRINTLN("******STOP");
            stop();
        }
        else
        {
            // DEBUG_PRINT("******DRIVE");
            // DEBUG_PRINTLN(fDirection);
            drive(fDirection);
        }
    }

private:
    byte fID;
    byte fIn1Pin;
    byte fIn2Pin;
    byte fPWMPin;
    byte fStandbyPin;
    byte fGripperOpenPin;
    byte fGripperClosePin;
    bool fArmed;
    int fDirection;
    uint8_t fCount;
    uint32_t fNextTime;

    void drive(int speed)
    {
        digitalWrite(fStandbyPin, HIGH);
        if (speed >= 0)
        {
            digitalWrite(fIn1Pin, HIGH);
            digitalWrite(fIn2Pin, LOW);
            analogWrite(fPWMPin, speed);
        }
        else
        {
            digitalWrite(fIn1Pin, LOW);
            digitalWrite(fIn2Pin, HIGH);
            analogWrite(fPWMPin, -speed);
        }
    }

    void stop()
    {
        digitalWrite(fIn1Pin, HIGH);
        digitalWrite(fIn2Pin, HIGH);
        analogWrite(fPWMPin, 0);
    }

    void standby()
    {
        digitalWrite(fStandbyPin, LOW);
    }
};

////////////////////////////////////////////////////////////////////////////////

#endif
