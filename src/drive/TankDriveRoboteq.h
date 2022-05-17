#ifndef TankDriveRoboteq_h
#define TankDriveRoboteq_h

#include "ServoDispatch.h"
#include "drive/TankDrive.h"
#include "drive/TurtleDrive.h"

/**
  * \ingroup drive
  *
  * \class TankDrive
  *
  * \brief Base template of automatic forwarder from i2c to CommandEvent
  *
  *
  * \code
  * #include "drive/TankDrive.h"
  *
  * TankDrive tankDrive;
  * \endcode
  *
  */
class TankDriveRoboteq : public TankDrive, public TurtleDrive
{
public:
    /** \brief Constructor
      *
      * Only a single instance of WifiSerialBridge should be created per sketch.
      *
      * \param port the port number of this service
      */
    TankDriveRoboteq(HardwareSerial& serial, JoystickController& driveStick) :
        TankDrive(driveStick),
        fSerial(&serial)
    {
    }

    /** \brief Constructor
      *
      * Will drive PWM pins
      */
    TankDriveRoboteq(HardwareSerial& serial, ServoDispatch& dispatch, int leftNum, int rightNum, JoystickController& driveStick) :
        TankDriveRoboteq(serial, dispatch, leftNum, rightNum, -1, driveStick)
    {
    }

    /** \brief Constructor
      *
      * Will drive PWM pins
      */
    TankDriveRoboteq(HardwareSerial& serial, ServoDispatch& dispatch, int leftNum, int rightNum, int throttleNum, JoystickController& driveStick) :
        TankDrive(driveStick),
        fSerial(&serial),
        fDispatch(&dispatch),
        fLeft(leftNum),
        fRight(rightNum),
        fThrottle(throttleNum)
    {
        setMaxSpeed(1.0f);
    }

    /** \brief Constructor
      *
      * Will drive PWM pins
      */
    TankDriveRoboteq(ServoDispatch& dispatch, int leftNum, int rightNum, JoystickController& driveStick) :
        TankDriveRoboteq(dispatch, leftNum, rightNum, -1, driveStick)
    {
    }

    /** \brief Constructor
      *
      * Will drive PWM pins
      */
    TankDriveRoboteq(ServoDispatch& dispatch, int leftNum, int rightNum, int throttleNum, JoystickController& driveStick) :
        TankDrive(driveStick),
        fDispatch(&dispatch),
        fLeft(leftNum),
        fRight(rightNum),
        fThrottle(throttleNum)
    {
        setMaxSpeed(1.0f);
    }

    virtual void setup() override
    {
        // stop odometry
        write("#\r");

        // stop motors
        write("!G 1 0\r");
        write("!S 1 0\r");
        write("!G 2 0\r");
        write("!S 2 0\r");

        // disable echo
        write("^ECHOF 1\r");

        // enable watchdog timer (100 ms)
        write("^RWD 100\r");
    }

    virtual void stop() override
    {
        if (fCommandMode)
        {
            writeIntCmd("!S", 1, 0);
            writeIntCmd("!S", 2, 0);
            writeIntCmd("!MS", 1);
            writeIntCmd("!MS", 2);
        }
        TankDrive::stop();
    }

    virtual bool enterCommandMode()
    {
        if (!isCommandModeActive() && fSerial != nullptr)
        {
            JoystickController* stick = getActiveStick();
            if (stick == nullptr)
            {
                stop();
                // Closed Loop Count
                write("^MMOD 1 3\r");
                write("^MMOD 2 3\r");
                fCommandMode = true;
                DEBUG_PRINTLN("ENTER COMMAND MODE");
            }
        }
        return fCommandMode;
    }

    virtual void leaveCommandMode()
    {
        if (fCommandMode)
        {
            // The Roboteq script should switch modes automatically if PWM is enabled
            stop();
            write("^MMOD 1 6\r");
            write("^MMOD 2 6\r");
            fCommandMode = false;
            DEBUG_PRINTLN("LEAVE COMMAND MODE");
        }
    }

    virtual bool isCommandModeActive()
    {
        JoystickController* stick = getActiveStick();
        if (fCommandMode && stick != nullptr)
        {
            // Disable command mode if the stick becomes active
            leaveCommandMode();
        }
        return fCommandMode;
    }

    virtual void moveMillimeters(double mm, float speed = 0.1)
    {
        if (isCommandModeActive())
        {
            int count = getMoveDistanceCount(mm);
            speed = max(min(speed, 1.0f), 0.0f);
            writeIntCmd("!S", 1, 1000 * speed);
            writeIntCmd("!S", 2, 1000 * speed);
            writeIntCmd("!PR", 1, count);
            writeIntCmd("!PR", 2, count);
        }
    }

    virtual void turnDegrees(double degrees, float speed = 0.1)
    {
        if (isCommandModeActive())
        {
            writeIntCmd("!S", 1, 1000 * speed);
            writeIntCmd("!S", 2, 1000 * speed);
            speed = max(min(speed, 1.0f), 0.0f);
            if (degrees < 0)
            {
                int count = getTurnDistanceCount(-degrees);
                writeIntCmd("!PR", 1, count);
                writeIntCmd("!PR", 2, -count);
            }
            else
            {
                int count = getTurnDistanceCount(degrees);
                writeIntCmd("!PR", 1, -count);
                writeIntCmd("!PR", 2, count);
            }
        }
    }

protected:
    HardwareSerial* fSerial = NULL;
    ServoDispatch* fDispatch = NULL;
    bool fCommandMode = false;
    int fLeft = -1;
    int fRight = -1;
    int fThrottle = -1;

    virtual void motor(float left, float right, float throttle) override
    {
        if (fDispatch != NULL && fLeft != -1 && fRight != -1)
        {
            left = map(left, -1.0f, 1.0f, 0.0f, 1.0f);
            right = map(right, -1.0f, 1.0f, 0.0f, 1.0f);

            // Serial.print("M "); Serial.print(left); Serial.print(", "); Serial.print(right);Serial.print(", "); Serial.println(throttle);
            if (fThrottle != -1)
            {
                fDispatch->moveTo(fLeft, left);
                fDispatch->moveTo(fRight, right);
                fDispatch->moveTo(fThrottle, throttle);
            }
            else
            {
                left *= throttle;
                right *= throttle;
                fDispatch->moveTo(fLeft, left);
                fDispatch->moveTo(fRight, right);
            }
        }
        else if (fSerial != NULL && !isCommandModeActive())
        {
            left *= throttle;
            right *= throttle;
            writeIntCmd("!S", 1, left * 1000);
            writeIntCmd("!S", 2, right * 1000);
        }
    }

    virtual bool hasThrottle() override
    {
        return (fDispatch != NULL && fThrottle != -1);
    }

    void writeIntCmd(const char* cmd, int arg1)
    {
        char buf[100];
        snprintf(buf, sizeof(buf), "%s %d\r", cmd, arg1);
        write(buf);
    #ifdef USE_MOTOR_DEBUG
        {
            // remove carriage return
            buf[strlen(buf)-1] = 0;
            MOTOR_DEBUG_PRINT("[ROBOTEQ] : ");
            MOTOR_DEBUG_PRINTLN(buf);
        }
    #endif
    }

    void writeIntCmd(const char* cmd, int arg1, int arg2)
    {
        char buf[100];
        snprintf(buf, sizeof(buf), "%s %d %d\r", cmd, arg1, arg2);
        write(buf);
    #ifdef USE_MOTOR_DEBUG
        {
            // remove carriage return
            buf[strlen(buf)-1] = 0;
            MOTOR_DEBUG_PRINT("[ROBOTEQ] : ");
            MOTOR_DEBUG_PRINTLN(buf);
        }
    #endif
    }

    void write(const char* cmd)
    {
        if (fSerial != NULL)
        {
            fSerial->write(cmd);
            fSerial->flush();
        }
    }

    static float map(float x, float in_min, float in_max, float out_min, float out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }
};
#endif
