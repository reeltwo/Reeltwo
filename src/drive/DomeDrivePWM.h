#ifndef DomeDrivePWM_h
#define DomeDrivePWM_h

#include "ServoDispatch.h"
#include "drive/DomeDrive.h"

/**
  * \ingroup drive
  *
  * \class DomeDrivePWM
  *
  * \brief Base template of automatic forwarder from i2c to CommandEvent
  *
  *
  * \code
  * #include "drive/DomeDrivePWM.h"
  *
  * DomeDrivePWM domeDrive;
  * \endcode
  *
  */
class DomeDrivePWM : public DomeDrive
{
public:
    /** \brief Constructor
      *
      * Will drive PWM pin
      */
    DomeDrivePWM(ServoDispatch& dispatch, uint8_t pwmNum, JoystickController& domeStick) :
        DomeDrive(domeStick),
        fDispatch(dispatch),
        fPWM(pwmNum)
    {
        setMaxSpeed(1.0f);
    }

    virtual void setup() override
    {
    }

    virtual void stop() override
    {
        fDispatch.moveTo(fPWM, 0.5);
        DomeDrive::stop();
    }

protected:
    ServoDispatch& fDispatch;
    uint8_t fPWM;

    virtual void motor(float m) override
    {
        m = map(m, -1.0f, 1.0f, 0.0f, 1.0f);
        fDispatch.moveTo(fPWM, m);
    }

    static float map(float x, float in_min, float in_max, float out_min, float out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }
};
#endif
