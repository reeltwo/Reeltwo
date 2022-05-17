#ifndef DomeDriveSabertooh_h
#define DomeDriveSabertooh_h

#include "drive/DomeDrive.h"
#include "motor/SabertoothDriver.h"

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
class DomeDriveSabertooth : public DomeDrive, protected SabertoothDriver
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
        SabertoothDriver(id, serial)
    {
    }

    virtual void setup() override
    {
        setBaudRate(9600);
        setRamping(80);
    }

    virtual void stop() override
    {
        SabertoothDriver::stop();
        DomeDrive::stop();
    }

protected:
    virtual void motor(float m) override
    {
        DOME_DEBUG_PRINT("ST: ");
        DOME_DEBUG_PRINTLN((int)(m * 127));
        SabertoothDriver::motor(1, m * 127);
    }
};
#endif
