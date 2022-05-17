#ifndef TankDrive_h
#define TankDrive_h

#include "ReelTwo.h"
#include "core/AnimatedEvent.h"
#include "core/SetupEvent.h"
#include "JoystickController.h"
#include "TargetSteering.h"

#ifdef USE_MOTOR_DEBUG
#define MOTOR_DEBUG_PRINT(s) DEBUG_PRINT(s)
#define MOTOR_DEBUG_PRINTLN(s) DEBUG_PRINTLN(s)
#define MOTOR_DEBUG_PRINT_HEX(s) DEBUG_PRINT_HEX(s)
#define MOTOR_DEBUG_PRINTLN_HEX(s) DEBUG_PRINTLN_HEX(s)
#else
#define MOTOR_DEBUG_PRINT(s)
#define MOTOR_DEBUG_PRINTLN(s)
#define MOTOR_DEBUG_PRINT_HEX(s)
#define MOTOR_DEBUG_PRINTLN_HEX(s)
#endif

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
class TankDrive : public SetupEvent, public AnimatedEvent
{
public:
    /** \brief Constructor
      *
      * Only a single instance of WifiSerialBridge should be created per sketch.
      *
      * \param port the port number of this service
      */
    TankDrive(JoystickController& driveStick) :
        fDriveStick(driveStick)
    {
        // Default enabled
        setEnable(true);
        // Default to half speed max
        setMaxSpeed(0.5);
        // Default 25ms
        setSerialLatency(25);
        setScaling(true);
        setThrottleAccelerationScale(100);
        setThrottleDecelerationScale(20);
        setTurnAccelerationScale(200);
        setTurnDecelerationScale(20);
        setChannelMixing(true);
        setThrottleInverted(false);
        setTurnInverted(false);
        setUseLeftStick();
        setUseThrottle(true);
        setUseHardStop(true);
    }

    virtual void setup() override
    {
    }

    bool getEnable()
    {
        return fEnabled;
    }

    void setEnable(bool enable)
    {
        fEnabled = enable;
    }

    uint32_t getSerialLatency()
    {
        return fSerialLatency;
    }

    void setSerialLatency(uint32_t ms)
    {
        fSerialLatency = ms;
    }

    bool getChannelMixing()
    {
        return fChannelMixing;
    }

    void setChannelMixing(bool mixing)
    {
        fChannelMixing = mixing;
    }

    bool getThrottleInverted()
    {
        return fThrottleInverted;
    }

    void setThrottleInverted(bool invert)
    {
        fThrottleInverted = invert;
    }

    bool getTurnInverted()
    {
        return fTurnInverted;
    }

    void setTurnInverted(bool invert)
    {
        fTurnInverted = invert;
    }

    bool getScaling()
    {
        return fScaling;
    }

    void setScaling(bool scaling)
    {
        fScaling = scaling;
    }

    float getMaxSpeed()
    {
        return fSpeedModifier;
    }

    void setMaxSpeed(float modifier)
    {
        stop();
        fSpeedModifier = min(max(modifier, 0.0f), 1.0f);
        fGuestSpeedModifier = fSpeedModifier / 2;
    }

    unsigned getThrottleAccelerationScale()
    {
        return fThrottleAccelerationScale;
    }

    void setThrottleAccelerationScale(unsigned scale)
    {
        fThrottleAccelerationScale = scale;
    }

    unsigned getThrottleDecelerationScale()
    {
        return fThrottleDecelerationScale;
    }

    void setThrottleDecelerationScale(unsigned scale)
    {
        fThrottleDecelerationScale = scale;
    }

    unsigned getTurnAccelerationScale()
    {
        return fTurnAccelerationScale;
    }

    void setTurnAccelerationScale(unsigned scale)
    {
        fTurnAccelerationScale = scale;
    }

    unsigned getTurnDecelerationScale()
    {
        return fTurnDecelerationScale;
    }

    void setTurnDecelerationScale(unsigned scale)
    {
        fTurnDecelerationScale = scale;
    }

    void setAccelerationScale(unsigned scale)
    {
        setThrottleAccelerationScale(scale);
        setTurnAccelerationScale(scale);
    }

    void setDecelerationScale(unsigned scale)
    {
        setThrottleDecelerationScale(scale);
        setTurnDecelerationScale(scale);
    }

    void setDriveStick(JoystickController &driveStick)
    {
        fDriveStick = driveStick;
    }

    void setGuestStick(JoystickController &guestStick)
    {
        fGuestStick = &guestStick;
    }

    float getGuestSpeedModifier()
    {
        return fGuestSpeedModifier;
    }

    void setGuestSpeedModifier(float maxGuestSpeed)
    {
        fGuestSpeedModifier = min(max(maxGuestSpeed, 0.0f), 1.0f);
    }

    void setTargetSteering(TargetSteering* target)
    {
        fTargetSteering = target;
    }

    bool useThrottle()
    {
        return fUseThrottle;
    }

    bool useHardStop()
    {
        return fUseHardStop;
    }

    bool useLeftStick()
    {
        return fUseLeftStick;
    }

    bool useRightStick()
    {
        return !fUseLeftStick;
    }

    void setUseThrottle(bool use)
    {
        fUseThrottle = use;
    }

    void setUseHardStop(bool use)
    {
        fUseHardStop = use;
    }

    void setUseLeftStick()
    {
        fUseLeftStick = true;
    }

    void setUseRightStick()
    {
        fUseLeftStick = false;
    }

    virtual void stop()
    {
        fMotorsStopped = true;
        fDriveThrottle = 0;
        fDriveTurning = 0;
    }

    JoystickController* getActiveStick()
    {
        if (fDriveStick.isConnected())
        {
            return &fDriveStick;
        }
        else if (fGuestStick != nullptr && fGuestStick->isConnected())
        {
            return fGuestStick;
        }
        return nullptr;
    }

    /**
      * Dispatch any received i2c event to CommandEvent
      */
    virtual void animate() override
    {
        if (fDriveStick.isConnected())
        {
            driveStick(&fDriveStick, fSpeedModifier);
        }
        else if (fGuestStick != nullptr && fGuestStick->isConnected())
        {
            driveStick(fGuestStick, fGuestSpeedModifier);
        }
        else if (fWasConnected)
        {
            stop();
            MOTOR_DEBUG_PRINTLN("Waiting to reconnect");
            fWasConnected = false;
        }
    }

protected:
    virtual void motor(float left, float right, float throttle) = 0;

    virtual float getThrottle()
    {
        if (useThrottle() && fDriveStick.isConnected())
        {
            if (useLeftStick())
                return float(fDriveStick.state.analog.button.l2)/255.0f;
            if (useRightStick())
                return float(fDriveStick.state.analog.button.r2)/255.0f;
        }
        return 0.0f;
    }

    virtual bool hasThrottle()
    {
        return false;
    }

    virtual float throttleSpeed(float speedModifier)
    {
        if (useThrottle())
        {
            if (!hasThrottle())
            {
                // We are going to simulate the throttle by increasing the speed modifier
                if (fDriveStick.isConnected())
                {
                    speedModifier += getThrottle() * ((1.0f-speedModifier));
                }
                return min(max(speedModifier,0.0f),1.0f) * -1.0f;
            }
            else
            {
                if (fDriveStick.isConnected())
                {
                    speedModifier = getThrottle();
                }
            }
        }
        return speedModifier;
    }

    void driveStick(JoystickController* stick, float speedModifier)
    {
        fWasConnected = true;
        if (!fEnabled)
        {
            stop();
        }
        else if (useHardStop() &&
                ((useLeftStick() && stick->state.button.l1) ||
                 (useRightStick() && stick->state.button.r1)))
        {
            if (!fMotorsStopped)
            {
                MOTOR_DEBUG_PRINTLN("STOP");
                stop();
            }
            /* Disable Target Steering */
            setTargetSteering(nullptr);
        }
        else
        {
            if (millis() - fLastCommand > fSerialLatency)
            {
                auto stickx = useLeftStick() ? stick->state.analog.stick.lx : stick->state.analog.stick.rx;
                auto sticky = useLeftStick() ? stick->state.analog.stick.ly : stick->state.analog.stick.ry;
                // float drive_mod = speedModifier * -1.0f;
                auto drive_mod = throttleSpeed(speedModifier);
                auto turning = (float)(stickx + 128) / 127.5f - 1.0f;
                auto throttle = (float)(sticky + 128) / 127.5f - 1.0f;
                if (fThrottleInverted)
                    throttle = -throttle;
                if (fTurnInverted)
                    turning = -turning;

                if (abs(turning) < 0.2)
                    turning = 0;
                else
                    turning = pow(abs(turning)-0.2, 1.4) * ((turning < 0) ? -1 : 1);
                if (abs(throttle) < 0.2)
                    throttle = 0;

                if (fTargetSteering)
                {
                    auto targetThrottle = fTargetSteering->getThrottle();
                    auto targetTurning = fTargetSteering->getTurning();
                    throttle = (throttle != 0) ? throttle : targetThrottle;
                    turning = (turning != 0) ? turning : targetTurning;
                }

                // clamp turning if throttle is greater than turning
                // theory being that if you are at top speed turning should
                // be less responsive. if you are full on turning throttle
                // should remain responsive.
                // if (abs(turning) <= abs(fDriveThrottle))
                // {
                //     if (abs(fDriveThrottle) >= 0.8)
                //         turning /= 8;
                //     else if (abs(fDriveThrottle) >= 0.6)
                //         turning /= 6;
                //     else if (abs(fDriveThrottle) >= 0.4)
                //         turning /= 4;
                //     else if (abs(fDriveThrottle) >= 0.2)
                //         turning /= 2;
                // }
                if (turning != 0 || throttle != 0)
                {
                    MOTOR_DEBUG_PRINT("TURNING "); MOTOR_DEBUG_PRINT(turning);
                    MOTOR_DEBUG_PRINT(" THROTTLE "); MOTOR_DEBUG_PRINTLN(throttle);
                }
                if (fScaling)
                {
                    if (throttle > fDriveThrottle)
                    {
                        float scale = fThrottleAccelerationScale;
                        if (fDriveThrottle < 0)
                        {
                            MOTOR_DEBUG_PRINT("DECELERATING ");
                            scale = fThrottleDecelerationScale;
                        }
                        else
                        {
                            MOTOR_DEBUG_PRINT("ACCELERATING REVERSE ");
                        }
                        float val = max(abs(throttle - fDriveThrottle) / scale, 0.01f);
                        MOTOR_DEBUG_PRINT(val);
                        MOTOR_DEBUG_PRINT(" throttle: ");
                        MOTOR_DEBUG_PRINT(throttle);
                        MOTOR_DEBUG_PRINT(" drive : ");
                        MOTOR_DEBUG_PRINT(fDriveThrottle );
                        MOTOR_DEBUG_PRINT(" => ");
                        fDriveThrottle = ((int)round(min(fDriveThrottle + val, throttle)*100))/100.0f;
                        MOTOR_DEBUG_PRINTLN(fDriveThrottle );
                    }
                    else if (throttle < fDriveThrottle)
                    {
                        float scale = fThrottleAccelerationScale;
                        if (fDriveThrottle > 0)
                        {
                            MOTOR_DEBUG_PRINT("DECELERATING REVERSE ");
                            scale = fThrottleDecelerationScale;
                        }
                        else
                        {
                            MOTOR_DEBUG_PRINT("ACCELERATING ");
                        }
                        float val = abs(throttle - fDriveThrottle) / scale;
                        MOTOR_DEBUG_PRINT(val);
                        MOTOR_DEBUG_PRINT(" throttle: ");
                        MOTOR_DEBUG_PRINT(throttle);
                        MOTOR_DEBUG_PRINT(" drive : ");
                        MOTOR_DEBUG_PRINT(fDriveThrottle );
                        MOTOR_DEBUG_PRINT(" => ");
                        fDriveThrottle = ((int)floor(max(fDriveThrottle - val, throttle)*100))/100.0f;
                        MOTOR_DEBUG_PRINTLN(fDriveThrottle );
                    }
                    // Scale turning by fDriveThrottle
                    turning *= (1.0f - abs(fDriveThrottle) * 0.1);
                    if (turning > fDriveTurning)
                    {
                        float scale = fTurnAccelerationScale;
                        if (fDriveTurning < 0)
                        {
                            MOTOR_DEBUG_PRINT("DECELERATING LEFT ");
                            scale = fTurnDecelerationScale;
                        }
                        else
                        {
                            MOTOR_DEBUG_PRINT("ACCELERATING RIGHT ");
                        }
                        float val = max(abs(turning - fDriveTurning) / scale, 0.01f);
                        MOTOR_DEBUG_PRINT(val);
                        MOTOR_DEBUG_PRINT(" turning: ");
                        MOTOR_DEBUG_PRINT(turning);
                        MOTOR_DEBUG_PRINT(" drive : ");
                        MOTOR_DEBUG_PRINT(fDriveTurning );
                        MOTOR_DEBUG_PRINT(" => ");
                        fDriveTurning = ((int)round(min(fDriveTurning + val, turning)*100))/100.0f;
                        MOTOR_DEBUG_PRINTLN(fDriveTurning );
                        MOTOR_DEBUG_PRINT(" ");
                    }
                    else if (turning < fDriveTurning)
                    {
                        float scale = fTurnAccelerationScale;
                        if (fDriveTurning > 0)
                        {
                            MOTOR_DEBUG_PRINT("DECELERATING RIGHT ");
                            scale = fTurnDecelerationScale;
                        }
                        else
                        {
                            MOTOR_DEBUG_PRINT("ACCELERATING LEFT ");
                        }
                        float val = abs(turning - fDriveTurning) / scale;
                        MOTOR_DEBUG_PRINT(val);
                        MOTOR_DEBUG_PRINT(" turning: ");
                        MOTOR_DEBUG_PRINT(turning);
                        MOTOR_DEBUG_PRINT(" drive : ");
                        MOTOR_DEBUG_PRINT(fDriveTurning );
                        MOTOR_DEBUG_PRINT(" => ");
                        fDriveTurning = ((int)floor(max(fDriveTurning - val, turning)*100))/100.0f;
                        MOTOR_DEBUG_PRINTLN(fDriveTurning );
                    }
                }
                else
                {
                    fDriveThrottle = throttle;
                    fDriveTurning = turning;
                }

                auto x = fDriveThrottle;
                auto y = fDriveTurning;
    
                auto target_left = x;
                auto target_right = y;
                if (fChannelMixing)
                {
                    auto r = hypot(x, y);
                    auto t = atan2(y, x);
                    // rotate 45 degrees
                    t += M_PI / 4;
        
                    target_left = r * cos(t);
                    target_right = r * sin(t);
        
                    // rescale the new coords
                    target_left *= sqrt(2);
                    target_right *= sqrt(2);
                }    
                // clamp to -1/+1 and apply max speed limit
                target_left = max(-1.0f, min(target_left, 1.0f));
                target_right = max(-1.0f, min(target_right, 1.0f));

                motor(target_left, target_right, drive_mod);
                fLastCommand = millis();
                fMotorsStopped = false;
            }
        }
    }

protected:
    JoystickController &fDriveStick;
    JoystickController* fGuestStick;
    TargetSteering* fTargetSteering;
    bool fEnabled = false;
    bool fWasConnected = false;
    bool fMotorsStopped = false;
    bool fChannelMixing = false;
    bool fScaling = false;
    bool fUseLeftStick = true;
    bool fUseThrottle = true;
    bool fUseHardStop = true;
    bool fThrottleInverted = false;
    bool fTurnInverted = false;
    float fSpeedModifier = 0;
    float fGuestSpeedModifier = 0;
    uint32_t fSerialLatency = 0;
    uint32_t fLastCommand = 0;
    unsigned fThrottleAccelerationScale = 0;
    unsigned fThrottleDecelerationScale = 0;
    unsigned fTurnAccelerationScale = 0;
    unsigned fTurnDecelerationScale = 0;
    float fDriveThrottle = 0;
    float fDriveTurning = 0;
};
#endif
