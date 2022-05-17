#ifndef DomeDriveSerial_h
#define DomeDriveSerial_h

#include "ServoDispatch.h"
#include "drive/DomeDrive.h"

/**
  * \ingroup drive
  *
  * \class DomeDriveSerial
  *
  * \brief Base template of automatic forwarder from i2c to CommandEvent
  *
  *
  * \code
  * #include "drive/DomeDriveSerial.h"
  *
  * DomeDriveSerial domeDrive;
  * \endcode
  *
  */
class DomeDriveSerial : public DomeDrive
{
public:
    /** \brief Constructor
      *
      * Will drive PWM pin
      */
    DomeDriveSerial(Stream& stream, JoystickController& domeStick) :
        DomeDrive(domeStick),
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
               fSerial->println("~DOMESTOP");
           fStopped = true;
        }
        DomeDrive::stop();
    }

protected:
    Stream* fSerial;
    bool fStopped;

    virtual void motor(float m) override
    {
        if (fSerial != nullptr)
        {
            fSerial->print("~DOME");
            printNumZeroPad(unsigned(100+m*100));
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
