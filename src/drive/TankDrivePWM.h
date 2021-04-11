#ifndef TankDrivePWM_h
#define TankDrivePWM_h

#include "ServoDispatch.h"
#include "drive/TankDrive.h"

/**
  * \ingroup drive
  *
  * \class TankDrivePWM
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
class TankDrivePWM : public TankDrive
{
public:
    /** \brief Constructor
      *
      * Will drive PWM pins
      */
    TankDrivePWM(ServoDispatch& dispatch, uint8_t leftNum, uint8_t rightNum, JoystickController& driveStick) :
        TankDrivePWM(dispatch, leftNum, rightNum, 0, driveStick)
    {
    }

    /** \brief Constructor
      *
      * Will drive PWM pins
      */
    TankDrivePWM(ServoDispatch& dispatch, uint8_t leftNum, uint8_t rightNum, uint8_t throttleNum, JoystickController& driveStick) :
        TankDrive(driveStick),
        fDispatch(dispatch),
        fLeft(leftNum),
        fRight(rightNum),
        fThrottle(throttleNum)
    {
        setMaxSpeed(1.0f);
    }

    virtual void setup() override
    {
    }

    virtual void stop() override
    {
        fDispatch.moveTo(fLeft, 0.5);
        fDispatch.moveTo(fRight, 0.5);
        TankDrive::stop();
    }

protected:
    ServoDispatch& fDispatch;
    uint8_t fLeft;
    uint8_t fRight;
    uint8_t fThrottle;

    virtual float throttleSpeed(float speedModifier)
    {
        return speedModifier;
    }

    virtual void motor(float left, float right) override
    {
        left = map(left, -1.0f, 1.0f, 0.0f, 1.0f);
        right = map(right, -1.0f, 1.0f, 0.0f, 1.0f);
        JoystickController* stick = getActiveStick();
        float throttle = (stick != nullptr) ? (float)stick->state.analog.button.l2/255.0f : 0;

        // Serial.print("M "); Serial.print(left); Serial.print(", "); Serial.print(right);Serial.print(", "); Serial.println(throttle);
        fDispatch.moveTo(fLeft, left);
        fDispatch.moveTo(fRight, right);
        if (fThrottle != 0)
            fDispatch.moveTo(fThrottle, throttle);
    }

    static float map(float x, float in_min, float in_max, float out_min, float out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }
};
#endif
