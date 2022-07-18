#ifndef TankDriveSabertooh_h
#define TankDriveSabertooh_h

#include "drive/TankDrive.h"
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
class TankDriveSabertooth : public TankDrive, public SabertoothDriver
{
public:
    /** \brief Constructor
      *
      * Only a single instance of WifiSerialBridge should be created per sketch.
      *
      * \param port the port number of this service
      */
    TankDriveSabertooth(int id, HardwareSerial& serial, JoystickController& driveStick) :
        TankDrive(driveStick),
        SabertoothDriver(id, serial)
    {
    }

    virtual void setup() override
    {
        setBaudRate(9600);
        setRamping(80);
        setTimeout(950);
    }

    virtual void stop() override
    {
        SabertoothDriver::stop();
        TankDrive::stop();
    }

protected:
    virtual void motor(float left, float right, float throttle) override
    {
        left *= throttle;
        right *= throttle;
        MOTOR_DEBUG_PRINT("ST1: ");
        MOTOR_DEBUG_PRINT((int)(left * 127));
        MOTOR_DEBUG_PRINT(" ST2: ");
        MOTOR_DEBUG_PRINTLN((int)(right * 127));
        SabertoothDriver::motor(1, left * 127);
        SabertoothDriver::motor(2, right * 127);
    }
};
#endif
