#ifndef TankDriveSerial_h
#define TankDriveSerial_h

#include "ServoDispatch.h"
#include "drive/TankDrive.h"

/**
  * \ingroup drive
  *
  * \class TankDriveSerial
  *
  * \brief Base template of automatic forwarder from i2c to CommandEvent
  *
  *
  * \code
  * #include "drive/TankDrivePWM.h"
  *
  * TankDrivePWM tankDrive;
  * \endcode
  *
  */
class TankDriveSerial : public TankDrive
{
public:
    /** \brief Constructor
      *
      * Will drive PWM pins
      */
    TankDriveSerial(Stream& stream, JoystickController& driveStick) :
        TankDrive(driveStick),
        fSerial(&stream),
        fStopped(false)
    {
        setMaxSpeed(1.0f);
    }

    virtual void setup() override
    {
    }

    virtual void stop() override
    {
        if (!fStopped)
        {
           if (fSerial != nullptr)
               fSerial->println("~DRIVESTOP");
           fStopped = true;
        }
        TankDrive::stop();
    }

protected:
    Stream* fSerial;
    bool fStopped;

    virtual void motor(float left, float right, float throttle) override
    {
        if (fSerial != nullptr)
        {
            fSerial->print("~DRIVE");
            printNumZeroPad(unsigned(100+left*100));
            fSerial->print(",");
            printNumZeroPad(unsigned(100+right*100));
            fSerial->print(",");
            printNumZeroPad(unsigned(100+throttle*100));
            fSerial->println();
        }
        fStopped = false;
    }

    void printNumZeroPad(unsigned num)
    {
        if (num < 100)
            fSerial->print(0);
        if (num < 10)
            fSerial->print(0);
        fSerial->print(num);
    }
};
#endif
