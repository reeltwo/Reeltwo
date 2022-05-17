#ifndef DomeDrive_h
#define DomeDrive_h

#include "ReelTwo.h"
#include "core/AnimatedEvent.h"
#include "core/SetupEvent.h"
#include "JoystickController.h"
#include "ServoEasing.h"
#include "drive/DomePosition.h"

#ifdef USE_DOME_DEBUG
#define DOME_DEBUG_PRINT(s) DEBUG_PRINT(s)
#define DOME_DEBUG_PRINTLN(s) DEBUG_PRINTLN(s)
#define DOME_DEBUG_PRINT_HEX(s) DEBUG_PRINT_HEX(s)
#define DOME_DEBUG_PRINTLN_HEX(s) DEBUG_PRINTLN_HEX(s)
#else
#define DOME_DEBUG_PRINT(s)
#define DOME_DEBUG_PRINTLN(s)
#define DOME_DEBUG_PRINT_HEX(s)
#define DOME_DEBUG_PRINTLN_HEX(s)
#endif

/**
  * \ingroup drive
  *
  * \class DomeDrive
  *
  * \brief Base template of automatic forwarder from i2c to CommandEvent
  *
  *
  * \code
  * #include "drive/DomeDrive.h"
  *
  * DomeDrive domeDrive;
  * \endcode
  *
  */
class DomeDrive : public SetupEvent, public AnimatedEvent
{
public:
    /** \brief Constructor
      *
      * Only a single instance of WifiSerialBridge should be created per sketch.
      *
      * \param port the port number of this service
      */
    DomeDrive(JoystickController& domeStick) :
        fDomeStick(domeStick)
    {
        // Default enabled
        setEnable(true);
        // Default to half speed max
        setMaxSpeed(0.5);
        // Default 25ms
        setSerialLatency(25);
        setThrottleAccelerationScale(100);
        setThrottleDecelerationScale(20);
        setInverted(false);
        setScaling(true);
        setUseRightStick();
        setUseThrottle(true);

        fEnabled = true;
    }

    virtual void setup() override
    {
    }
    
    inline void setDomePosition(DomePosition* domePosition)
    {
        fDomePosition = domePosition;
    }

    bool getEnable() const
    {
        return fEnabled;
    }

    void setEnable(bool enable)
    {
        fEnabled = enable;
    }

    uint32_t getSerialLatency() const
    {
        return fSerialLatency;
    }

    void setSerialLatency(uint32_t ms)
    {
        fSerialLatency = ms;
    }

    bool getInverted() const
    {
        return fInverted;
    }

    void setInverted(bool invert)
    {
        fInverted = invert;
    }

    bool getScaling() const
    {
        return fScaling;
    }

    void setScaling(bool scaling)
    {
        fScaling = scaling;
    }

    float getMaxSpeed() const
    {
        return fSpeedModifier;
    }

    void setMaxSpeed(float modifier)
    {
        if (fDomeStick.isConnected())
            stop();
        fSpeedModifier = min(max(modifier, 0.0f), 1.0f);
    }

    unsigned getThrottleAccelerationScale() const
    {
        return fThrottleAccelerationScale;
    }

    void setThrottleAccelerationScale(unsigned scale)
    {
        fThrottleAccelerationScale = scale;
    }

    unsigned getThrottleDecelerationScale() const
    {
        return fThrottleDecelerationScale;
    }

    void setThrottleDecelerationScale(unsigned scale)
    {
        fThrottleDecelerationScale = scale;
    }

    void setEasingMethod(float (*easingMethod)(float completion))
    {
        fEasingMethod = easingMethod;
    }

    void setDomeStick(JoystickController &domeStick)
    {
        fDomeStick = domeStick;
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
        fMotorStopped = true;
        fDrive = 0;
        fAutoDrive = 0;
    }

    JoystickController* getActiveStick()
    {
        if (fDomeStick.isConnected())
        {
            return &fDomeStick;
        }
        return nullptr;
    }

    virtual void animate() override
    {
        if (fDomeStick.isConnected())
        {
            domeStick(&fDomeStick, fSpeedModifier);
        }
        else if (fWasConnected)
        {
            stop();
            DOME_DEBUG_PRINTLN("Waiting to reconnect");
            fWasConnected = false;
        }
    }

    virtual void driveDome(float m)
    {
        fDrive = getInverted() ? -m : m;
    }

    virtual void autonomousDriveDome(float m)
    {
        fAutoDrive = getInverted() ? -m : m;
    }

    virtual bool idle()
    {
        return fIdle;
    }

protected:
    virtual void motor(float m) = 0;

    virtual float getThrottle()
    {
        if (useThrottle() && fDomeStick.isConnected())
        {
            if (useLeftStick())
                return float(fDomeStick.state.analog.button.l2)/255.0f;
            if (useRightStick())
                return float(fDomeStick.state.analog.button.r2)/255.0f;
        }
        return 0.0f;
    }

    virtual float throttleSpeed(float speedModifier)
    {
        if (useThrottle())
        {
            if (fDomeStick.isConnected())
            {
                speedModifier += getThrottle() * ((1.0f-speedModifier));
            }
            return min(max(speedModifier,0.0f),1.0f) * -1.0f;
        }
        return 0.0f;
    }

    static bool withinArc(double p1, double p2, double p3)
    {
        return fmod(p2 - p1 + 2*360, 360) >= fmod(p3 - p1 + 2*360, 360);
    }

    int normalize(int degrees)
    {
        degrees = fmod(degrees, 360);
        if (degrees < 0)
            degrees += 360;
        return degrees;
    }

    int shortestDistance(int origin, int target)
    {
        int result = 0.0;
        int diff = fmod(fmod(abs(origin - target), 360), 360);

        if (diff > 180)
        {
            //There is a shorter path in opposite direction
            result = (360 - diff);
            if (target > origin)
                result *= -1;
        }
        else
        {
            result = diff;
            if (origin > target)
                result *= -1;
        }
        return result;
    }

    bool moveDomeToTarget(int pos, int target, int fudge, float speed, float minspeed, float &m)
    {
        if (!withinArc(target - fudge, target + fudge, pos))
        {
            int dist = shortestDistance(pos, target);
            if (abs(dist) > fudge*2)
                speed += (1.0 - speed) * float(abs(dist)) / 180;
            else
                speed = minspeed;
            speed = max(speed, minspeed);
            if (dist > 0)
            {
                m = -speed;
            }
            else
            {
                m = speed;
            }
            if (fDomePosition->getDomeFlip())
                m = -m;
            return false;
        }
        return true;
    }

    void domeStick(JoystickController* stick, float speedModifier)
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
            if (!fMotorStopped)
            {
                DOME_DEBUG_PRINTLN("STOP");
                stop();
            }
        }
        else
        {
            uint32_t currentMillis = millis();
            if (currentMillis - fLastCommand > fSerialLatency)
            {
                auto stickx = useLeftStick() ? stick->state.analog.stick.lx : stick->state.analog.stick.rx;
                auto drive_mod = throttleSpeed(speedModifier);
                auto m = (float)(stickx + 128) / 127.5f - 1.0f;

                if (abs(m) < 0.2)
                    m = 0;
                else
                    m = pow(abs(m)-0.2, 1.4) * ((m < 0) ? -1 : 1);
                if (m != 0)
                {
                    // Any movement of joysticks will disable programmatic drive
                    fDrive = 0;
                    fAutoDrive = 0;
                }
                else if (fDrive != 0)
                {
                    // Use programmatic drive if active
                    m = fDrive;
                    fAutoDrive = 0;
                }
                else
                {
                    // Use lowest priorty programmatic drive if active
                    m = fAutoDrive;
                }

                // clamp to -1/+1 and apply max speed limit
                m = max(-1.0f, min(m, 1.0f)) * drive_mod;

                if (fScaling)
                {
                    if (m > fDomeThrottle)
                    {
                        float scale = fThrottleAccelerationScale;
                        if (fDomeThrottle < 0)
                        {
                            DOME_DEBUG_PRINT("DECELERATING ");
                            scale = fThrottleDecelerationScale;
                        }
                        else
                        {
                            DOME_DEBUG_PRINT("ACCELERATING REVERSE ");
                        }
                        float val = max(abs(m - fDomeThrottle) / scale, 0.01f);
                        DOME_DEBUG_PRINT(val);
                        DOME_DEBUG_PRINT(" m: ");
                        DOME_DEBUG_PRINT(m);
                        DOME_DEBUG_PRINT(" drive : ");
                        DOME_DEBUG_PRINT(fDomeThrottle );
                        DOME_DEBUG_PRINT(" => ");
                        fDomeThrottle = ((int)round(min(fDomeThrottle + val, m)*100))/100.0f;
                        DOME_DEBUG_PRINTLN(fDomeThrottle);
                    }
                    else if (m < fDomeThrottle)
                    {
                        float scale = fThrottleAccelerationScale;
                        if (fDomeThrottle > 0)
                        {
                            DOME_DEBUG_PRINT("DECELERATING REVERSE ");
                            scale = fThrottleDecelerationScale;
                        }
                        else
                        {
                            DOME_DEBUG_PRINT("ACCELERATING ");
                        }
                        float val = abs(m - fDomeThrottle) / scale;
                        DOME_DEBUG_PRINT(val);
                        DOME_DEBUG_PRINT(" m: ");
                        DOME_DEBUG_PRINT(m);
                        DOME_DEBUG_PRINT(" drive : ");
                        DOME_DEBUG_PRINT(fDomeThrottle );
                        DOME_DEBUG_PRINT(" => ");
                        fDomeThrottle = ((int)floor(max(fDomeThrottle - val, m)*100))/100.0f;
                        DOME_DEBUG_PRINTLN(fDomeThrottle);
                    }
                    m = fDomeThrottle;
                }

                // if (abs(m) == 0.0 || fAutoDrive != 0)
                //     fIdle = true;

                DomePosition::Mode domeMode = (fDomePosition != nullptr) ?
                                                    fDomePosition->getDomeMode() : DomePosition::kOff;
                if (domeMode != DomePosition::kOff && abs(m) == 0.0)
                {
                    // No joystick movement - check auto dome
                    uint32_t minDelay = uint32_t(fDomePosition->getDomeMinDelay()) * 1000L;
                    uint32_t maxDelay = uint32_t(fDomePosition->getDomeMaxDelay()) * 1000L;
                    if (domeMode == DomePosition::kTarget)
                        minDelay = 0;
                    if (fLastDomeMovement + minDelay < currentMillis)
                    {
                        int pos = fDomePosition->getDomePosition();
                        int home = fDomePosition->getDomeHome();
                        int targetpos = fDomePosition->getDomeTargetPosition();
                        int fudge = fDomePosition->getDomeFudge();
                        bool newMode = (fLastDomeMode != domeMode);
                        float minspeed = fDomePosition->getDomeMinSpeed();
                        switch (fLastDomeMode = domeMode)
                        {
                            case DomePosition::kOff:
                                break;
                            case DomePosition::kHome:
                            {
                                float speed = fDomePosition->getDomeSpeedHome();
                                if (moveDomeToTarget(pos, home, fudge, speed, minspeed, m))
                                {
                                    // Reached target set mode back to default
                                    fDomePosition->setDomeMode(fDomePosition->getDomeDefaultMode());
                                    fLastDomeMovement = currentMillis;
                                }
                                // uint32_t timeNow = millis();
                                // if (newMode || fMovementFinishTime < fLastDomeMovement)
                                // {
                                //     // DEBUG_PRINT("HOME START");
                                //     fMovementStartTime = timeNow;
                                //     fMovementFinishTime = timeNow + 10000;
                                // }
                                // else if (timeNow >= fMovementFinishTime)
                                // {
                                //     moveDomeToTarget(pos, home, fudge, speed, m);
                                // }
                                // else
                                // {
                                //     uint32_t timeSinceLastMove = timeNow - fMovementStartTime;
                                //     uint32_t denominator = fMovementFinishTime - fMovementStartTime;
                                //     float (*useMethod)(float) = fEasingMethod;
                                //     if (useMethod == NULL)
                                //         useMethod = Easing::LinearInterpolation;
                                //     float fractionChange = useMethod(float(timeSinceLastMove)/float(denominator));
                                //     int distanceToMove = float(normalize(home - pos)) * fractionChange;
                                //     int newPos = normalize(pos + distanceToMove);
                                //     if (newPos != pos)
                                //     {
                                //         DEBUG_PRINT("NEWPOS: "); DEBUG_PRINTLN(newPos);
                                //         moveDomeToTarget(pos, newPos, 0, speed, m);
                                //     }
                                // }
                                // if (!withinArc(home - fudge, home + fudge, pos))
                                // {
                                //     int dist = shortestDistance(pos, home);
                                //     // DEBUG_PRINTLN("pos: "+String(pos)+" home: "+String(home)+" shortest: "+String(shortestDistance(pos, home)));
                                //     if (abs(dist) > fudge*2)
                                //         speed += (1.0 - speed) * float(abs(dist)) / 180;
                                //     else
                                //         speed = 0.3;
                                //     if (dist > 0)
                                //     {
                                //         m = -speed;
                                //     }
                                //     else
                                //     {
                                //         m = speed;
                                //     }
                                //     if (fDomePosition->getDomeFlip())
                                //         m = -m;
                                // }
                                break;
                            }
                            case DomePosition::kRandom:
                            {
                                float speed = fDomePosition->getDomeSeekSpeed();
                                if (newMode)
                                {
                                    uint32_t r = random(minDelay, maxDelay);
                                    DEBUG_PRINT("RANDOM START IN "); DEBUG_PRINTLN(r);
                                    fNextAutoDomeMovement = millis() + r;
                                    fAutoDomeTargetPos = -1;
                                }
                                if (fAutoDomeTargetPos == -1 && fNextAutoDomeMovement < millis())
                                {
                                    if (fAutoDomeGoHome)
                                    {
                                    DEBUG_PRINTLN("RANDOM GO HOME: "+String(home));
                                        fAutoDomeTargetPos = home;
                                        fAutoDomeGoHome = false;
                                    }
                                    else if (random(100) < 50)
                                    {
                                        int distance = random(fDomePosition->getDomeSeekLeft());
                                        fAutoDomeTargetPos = normalize(home - distance);
                                        fAutoDomeGoHome = true;
                                    DEBUG_PRINTLN("RANDOM TURN LEFT: "+String(distance));
                                    }
                                    else if (random(100) < 50)
                                    {
                                        int distance = random(fDomePosition->getDomeSeekRight());
                                        fAutoDomeTargetPos = normalize(home + distance);
                                        fAutoDomeGoHome = true;
                                    DEBUG_PRINTLN("RANDOM TURN RIGHT: "+String(distance));
                                    }
                                    else
                                    {
                                        uint32_t r = random(minDelay, maxDelay);
                                        DEBUG_PRINTLN("RANDOM DO NOTHING NEXT: "+String(r));
                                        fNextAutoDomeMovement = millis() + r;
                                    }
                                }
                                if (fAutoDomeTargetPos != -1)
                                {
                                    DEBUG_PRINT("POS: "); DEBUG_PRINT(pos); DEBUG_PRINT(" TARGET: "); DEBUG_PRINTLN(fAutoDomeTargetPos);
                                    if (moveDomeToTarget(pos, fAutoDomeTargetPos, fudge, speed, minspeed, m))
                                    {
                                        // Set next autodome movement time
                                        uint32_t r = random(minDelay, maxDelay);
                                        DEBUG_PRINTLN("RANDOM ARRIVED NEXT: "+String(r));
                                        fNextAutoDomeMovement = millis() + r;
                                        fAutoDomeTargetPos = -1;
                                    }
                                }
                                break;
                            }
                            case DomePosition::kTarget:
                            {
                                float speed = fDomePosition->getDomeSeekSpeed();
                                if (moveDomeToTarget(pos, targetpos, fudge, speed, minspeed, m))
                                {
                                    // Reached target set mode back to default
                                    fDomePosition->setDomeMode(fDomePosition->getDomeDefaultMode());
                                    DEBUG_PRINTLN("REACHED TARGET: newMode="+String(fDomePosition->getDomeDefaultMode()));
                                    fLastDomeMovement = currentMillis;
                                }
                                break;
                            }
                        }
                    }
                }
                else
                {
                    fLastDomeMovement = currentMillis;
                }
                motor(getInverted() ? -m : m);
                fLastCommand = currentMillis;
                fMotorStopped = false;
            }
        }
    }

    enum DomeMode
    {
        kOff,
        kHome,
        kRandom
    };

    JoystickController &fDomeStick;
    bool fEnabled = false;
    bool fWasConnected = false;
    bool fMotorStopped = false;
    bool fIdle = true;
    bool fScaling = false;
    bool fUseLeftStick = false;
    bool fUseThrottle = true;
    bool fUseHardStop = false;
    bool fInverted = false;
    float fSpeedModifier;
    float fDrive = 0;
    float fAutoDrive = 0;
    int fLastDomePosition = -1;
    uint32_t fSerialLatency = 0;
    uint32_t fLastCommand = 0;
    uint32_t fLastDomeMovement = 0;
    unsigned fThrottleAccelerationScale = 0;
    unsigned fThrottleDecelerationScale = 0;
    float fDomeThrottle = 0;
    int fLastDomeMode = -1;
    int fAutoDomeTargetPos = 0;
    bool fAutoDomeGoHome = false;
    uint32_t fNextAutoDomeMovement = 0;
    DomePosition* fDomePosition = nullptr;
    uint32_t fMovementStartTime = 0;
    uint32_t fMovementFinishTime = 0;
    float (*fEasingMethod)(float completion) = nullptr;
};
#endif
