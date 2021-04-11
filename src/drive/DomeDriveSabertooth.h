#ifndef DomeDriveSabertooh_h
#define DomeDriveSabertooh_h

#include "drive/DomeDrive.h"
#include <Sabertooth.h>

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
class DomeDriveSabertooth : public DomeDrive, public Sabertooth
{
public:
    /** \brief Constructor
      *
      * Only a single instance of WifiSerialBridge should be created per sketch.
      *
      * \param port the port number of this service
      */
    DomeDriveSabertooth(int id, HardwareSerial& serial, JoystickController& domeStick) :
        DomeDrive(domeStick),
        Sabertooth(id, serial)
    {
    }

    virtual void setup() override
    {
        setBaudRate(9600);
        setRamping(80);
    }

    virtual void stop() override
    {
        Sabertooth::stop();
        DomeDrive::stop();
    }

protected:
    virtual float throttleSpeed(float speedModifier) override
    {
        if (fDomeStick.isConnected())
        {
            speedModifier += (float)fDomeStick.state.analog.button.l2/255.0 * ((1.0f-speedModifier));
        }
        return min(max(speedModifier,0.0f),1.0f) * -1.0f;
    }

    virtual void motor(float m) override
    {
        DOME_DEBUG_PRINT("ST: ");
        DOME_DEBUG_PRINTLN((int)(m * 127));
        Sabertooth::motor(1, m * 127);
    }
};
#endif
