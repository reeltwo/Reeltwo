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
    DomeDriveSabertooth(int id, Stream& serial, JoystickController& domeStick) :
        DomeDrive(domeStick),
        SabertoothDriver(id, serial),
        fBaudRate(9600)
    {
    }

    // Must be called before setup()
    void setBaudRate(unsigned baudRate)
    {
        fBaudRate = baudRate;
    }

    void setAddress(uint8_t addr)
    {
        SabertoothDriver::setAddress(addr);
    }

    virtual void setup() override
    {
        setBaudRate(fBaudRate);
        setRamping(80);
        setTimeout(950);
    }

    virtual void stop() override
    {
        SabertoothDriver::stop();
        DomeDrive::stop();
    }

protected:
    virtual void motor(float m) override
    {
        static bool sLastZero;
        if (!sLastZero || m != 0)
        {
            VERBOSE_DOME_DEBUG_PRINT("ST: ");
            VERBOSE_DOME_DEBUG_PRINTLN((int)(m * 127));
            sLastZero = (abs(m) == 0);
        }
        SabertoothDriver::motor(1, m * 127);
    }

    uint16_t fBaudRate;
};
#endif
