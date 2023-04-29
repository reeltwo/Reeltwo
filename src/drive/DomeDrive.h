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

#ifdef USE_VERBOSE_DOME_DEBUG
#if !defined(__AVR__)
static bool sVerboseDomeDebug;
#define VERBOSE_DOME_DEBUG_PRINT(s) { if (sVerboseDomeDebug) { DEBUG_PRINT(s); } }
#define VERBOSE_DOME_DEBUG_PRINTLN(s) { if (sVerboseDomeDebug) { DEBUG_PRINTLN(s); } }
#define VERBOSE_DOME_DEBUG_PRINT_HEX(s) DEBUG_PRINT_HEX(s)
#define VERBOSE_DOME_DEBUG_PRINTLN_HEX(s) DEBUG_PRINTLN_HEX(s)
#else
#define VERBOSE_DOME_DEBUG_PRINT(s) DEBUG_PRINT(s)
#define VERBOSE_DOME_DEBUG_PRINTLN(s) DEBUG_PRINTLN(s)
#define VERBOSE_DOME_DEBUG_PRINT_HEX(s) DEBUG_PRINT_HEX(s)
#define VERBOSE_DOME_DEBUG_PRINTLN_HEX(s) DEBUG_PRINTLN_HEX(s)
#endif
#else
#define VERBOSE_DOME_DEBUG_PRINT(s)
#define VERBOSE_DOME_DEBUG_PRINTLN(s)
#define VERBOSE_DOME_DEBUG_PRINT_HEX(s)
#define VERBOSE_DOME_DEBUG_PRINTLN_HEX(s)
#endif

#ifndef DOME_DIRECTION_CHANGE_THRESHOLD
#define DOME_DIRECTION_CHANGE_THRESHOLD 5
#endif

#ifndef DOME_RANDOM_MOVE_MIN_DEGREES
#define DOME_RANDOM_MOVE_MIN_DEGREES 5
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
        //setSerialLatency(25);
        setThrottleAccelerationScale(100);
        setThrottleDecelerationScale(20);
        setInverted(false);
        setScaling(true);
        setUseRightStick();
        setUseThrottle(true);
    }

    virtual void setup() override
    {
    }
    
    inline void setDomePosition(DomePosition* domePosition)
    {
        fDomePosition = domePosition;
    }

    bool checkError()
    {
        bool error = fError;
        fError = false;
        return error;
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
        fMoving = false;
    }

    inline bool isMoving()
    {
        return fMoving;
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

    float getSpeed(float percentage)
    {
        if (fDomePosition != nullptr)
            return max(getMaxSpeed() * percentage, fDomePosition->getDomeMinSpeed());
        return getMaxSpeed() * percentage;
    }

    bool moveDomeToTarget(int pos, int target, int fudge, float speed, float &m)
    {
        if (!withinArc(target - fudge, target + fudge, pos))
        {
            int dist = shortestDistance(pos, target);
            float decelerationScale = getThrottleDecelerationScale();
            if (abs(dist) <= decelerationScale)
                speed *= (abs(dist) / decelerationScale);
            speed = getSpeed(speed);
            VERBOSE_DOME_DEBUG_PRINT("POS: "); VERBOSE_DOME_DEBUG_PRINT(pos);
            VERBOSE_DOME_DEBUG_PRINT(" DST: "); VERBOSE_DOME_DEBUG_PRINT(dist);
            VERBOSE_DOME_DEBUG_PRINT(" SPD: "); VERBOSE_DOME_DEBUG_PRINTLN(speed);
            if (dist > 0)
            {
                m = -speed;
            }
            else
            {
                m = speed;
            }
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

                if (abs(m) != 0.0 || fAutoDrive != 0)
                {
                    if (fIdle)
                    {
                        if (fDomePosition != nullptr)
                        {
                            fDomePosition->reachedTarget();
                            fDomePosition->resetDefaultMode();
                        }
                        fLastDomeMovement = currentMillis;
                        fDomeMovementStarted = false;
                        DOME_DEBUG_PRINTLN("DOME NO LONGER IDLE");
                    }
                    fIdle = false;
                }

                bool scaling = fScaling;
                DomePosition::Mode domeMode = (fDomePosition != nullptr) ?
                                                    fDomePosition->getDomeMode() : DomePosition::kOff;
                if (domeMode != DomePosition::kOff && abs(m) == 0.0)
                {
                    // No joystick movement - check auto dome
                    uint32_t minDelay = uint32_t(fDomePosition->getDomeMinDelay()) * 1000L;
                    uint32_t maxDelay = uint32_t(fDomePosition->getDomeMaxDelay()) * 1000L;
                    if (fLastDomeMovement + minDelay < currentMillis)
                    {
                        if (!fIdle)
                        {
                            DOME_DEBUG_PRINTLN("DOME IDLE.");
                        }
                        fIdle = true;
                        int pos = fDomePosition->getDomePosition();
                        int home = fDomePosition->getDomeHome();
                        int targetpos = fDomePosition->getDomeTargetPosition();
                        int fudge = fDomePosition->getDomeFudge();
                        bool newMode = (fLastDomeMode != domeMode);
                        if (!newMode)
                        {
                            if (!fDomeMovementStarted)
                            {
                                fDomePosition->resetWatchdog();
                                fDomeMovementStarted = true;
                            }
                            else if (fDomePosition->isTimeout())
                            {
                                DOME_DEBUG_PRINTLN("TIMEOUT: NO DOME MOVEMENT DETECTED");
                                fDomePosition->setDomeMode(domeMode = DomePosition::kOff);
                                fDomeMovementStarted = false;
                                fError = true;
                            }
                        }
                        else
                        {
                            fDomeMovementStarted = false;
                            DEBUG_PRINTLN("NEW DOME MODE");
                        }
                        switch (fLastDomeMode = domeMode)
                        {
                            case DomePosition::kOff:
                                m = 0;
                                break;
                            case DomePosition::kHome:
                            {
                                float speed = fDomePosition->getDomeSpeed();
                                if (moveDomeToTarget(pos, home, fudge, speed, m))
                                {
                                    // Reached target set mode back to default
                                    fDomePosition->reachedHomeTarget();
                                    fDomePosition->resetDefaultMode();
                                    fLastDomeMovement = currentMillis;
                                    fDomeMovementStarted = false;
                                }
                                break;
                            }
                            case DomePosition::kRandom:
                            {
                                float speed = fDomePosition->getDomeSpeed();
                                if (newMode)
                                {
                                    uint32_t r = random(minDelay, maxDelay);
                                    DOME_DEBUG_PRINT("RANDOM START IN "); DOME_DEBUG_PRINTLN(r);
                                    fNextAutoDomeMovement = millis() + r;
                                    fAutoDomeTargetPos = -1;
                                    fAutoDomeLeft = random(1);
                                }
                                if (fAutoDomeTargetPos == -1 && fNextAutoDomeMovement < millis())
                                {
                                    if (fAutoDomeGoHome)
                                    {
                                        DOME_DEBUG_PRINTLN("RANDOM GO HOME: "+String(home));
                                        fAutoDomeTargetPos = home;
                                        fAutoDomeGoHome = false;
                                        fAutoDomeLeft = random(1);
                                    }
                                    else if (random(100) < 10)
                                    {
                                        uint32_t r = random(minDelay, maxDelay);
                                        DOME_DEBUG_PRINTLN("RANDOM DO NOTHING NEXT: "+String(r));
                                        fNextAutoDomeMovement = millis() + r;
                                    }
                                    else if (fAutoDomeLeft)
                                    {
                                        int distance = random(fDomePosition->getDomeAutoLeft());
                                        distance = max(DOME_RANDOM_MOVE_MIN_DEGREES, distance);
                                        fAutoDomeTargetPos = normalize(home - distance);
                                        fAutoDomeGoHome = (random(100) < 10);
                                        DOME_DEBUG_PRINTLN("RANDOM TURN LEFT: "+String(distance)+" newpos: "+String(fAutoDomeTargetPos));
                                        fAutoDomeLeft = (random(100) < 10);
                                    }
                                    else
                                    {
                                        int distance = random(fDomePosition->getDomeAutoRight());
                                        distance = max(DOME_RANDOM_MOVE_MIN_DEGREES, distance);
                                        fAutoDomeTargetPos = normalize(home + distance);
                                        fAutoDomeGoHome = (random(100) < 10);
                                        DOME_DEBUG_PRINTLN("RANDOM TURN RIGHT: "+String(distance)+" newpos: "+String(fAutoDomeTargetPos));
                                        fAutoDomeLeft = !(random(100) < 10);
                                    }
                                    if (fAutoDomeTargetPos != -1)
                                    {
                                        fDomePosition->resetWatchdog();
                                    }
                                }
                                if (fAutoDomeTargetPos != -1)
                                {
                                    if (moveDomeToTarget(pos, fAutoDomeTargetPos, fudge, speed, m))
                                    {
                                        // Set next autodome movement time
                                        uint32_t r = random(minDelay, maxDelay);
                                        DOME_DEBUG_PRINTLN("RANDOM ARRIVED NEXT: "+String(r));
                                        fDomePosition->reachedAutoTarget();
                                        fNextAutoDomeMovement = millis() + r;
                                        fAutoDomeTargetPos = -1;
                                        fDomeMovementStarted = false;
                                    }
                                }
                                else
                                {
                                    fDomePosition->resetWatchdog();
                                }
                                break;
                            }
                            case DomePosition::kTarget:
                            {
                                float speed = fDomePosition->getDomeSpeed();
                                long relativeDegrees = fDomePosition->getDomeRelativeTargetPosition();
                                if (relativeDegrees != 0)
                                {
                                    int dist = abs(relativeDegrees - fDomePosition->getRelativeDegrees());
                                    if (abs(fDomePosition->getRelativeDegrees()) < abs(relativeDegrees))
                                    {
                                        float decelerationScale = getThrottleDecelerationScale();
                                        if (dist <= decelerationScale)
                                        {
                                            speed *= (dist / decelerationScale);
                                        }
                                        speed = getSpeed(speed);
                                        if (relativeDegrees > 0)
                                        {
                                            m = -speed;
                                        }
                                        else
                                        {
                                            m = speed;
                                        }
                                    }
                                    else
                                    {
                                        // Reached target set mode back to default
                                        fDomePosition->reachedTarget();
                                        fDomePosition->resetDefaultMode();
                                        fLastDomeMovement = currentMillis;
                                        fDomeMovementStarted = false;
                                    }
                                }
                                else if (moveDomeToTarget(pos, targetpos, fudge, speed, m))
                                {
                                    // Reached target set mode back to default
                                    fDomePosition->reachedTarget();
                                    fDomePosition->resetDefaultMode();
                                    fLastDomeMovement = currentMillis;
                                    fDomeMovementStarted = false;
                                }
                                break;
                            }
                        }
                    }
                    // Always scale auto dome
                    scaling = true;
                }
                else
                {
                    fLastDomeMovement = currentMillis;
                }
                if (scaling)
                {
                    if (m > fDomeThrottle)
                    {
                        float scale = fThrottleAccelerationScale;
                        if (fDomeThrottle < 0)
                        {
                            VERBOSE_DOME_DEBUG_PRINT("DECELERATING ");
                            scale = fThrottleDecelerationScale;
                            fDomeThrottle = m;
                        }
                        else
                        {
                            VERBOSE_DOME_DEBUG_PRINT("ACCELERATING REVERSE ");
                        }
                        float val = max(abs(m - fDomeThrottle) / scale, 0.01f);
                        VERBOSE_DOME_DEBUG_PRINT(val);
                        VERBOSE_DOME_DEBUG_PRINT(" m: ");
                        VERBOSE_DOME_DEBUG_PRINT(m);
                        VERBOSE_DOME_DEBUG_PRINT(" drive : ");
                        VERBOSE_DOME_DEBUG_PRINT(fDomeThrottle );
                        VERBOSE_DOME_DEBUG_PRINT(" => ");
                        fDomeThrottle = ((int)round(min(fDomeThrottle + val, m)*100))/100.0f;
                        VERBOSE_DOME_DEBUG_PRINTLN(fDomeThrottle);
                    }
                    else if (m < fDomeThrottle)
                    {
                        float scale = fThrottleAccelerationScale;
                        if (fDomeThrottle > 0)
                        {
                            VERBOSE_DOME_DEBUG_PRINT("DECELERATING REVERSE ");
                            scale = fThrottleDecelerationScale;
                            fDomeThrottle = m;
                        }
                        else
                        {
                            VERBOSE_DOME_DEBUG_PRINT("ACCELERATING ");
                        }
                        float val = abs(m - fDomeThrottle) / scale;
                        VERBOSE_DOME_DEBUG_PRINT(val);
                        VERBOSE_DOME_DEBUG_PRINT(" m: ");
                        VERBOSE_DOME_DEBUG_PRINT(m);
                        VERBOSE_DOME_DEBUG_PRINT(" drive : ");
                        VERBOSE_DOME_DEBUG_PRINT(fDomeThrottle );
                        VERBOSE_DOME_DEBUG_PRINT(" => ");
                        fDomeThrottle = ((int)floor(max(fDomeThrottle - val, m)*100))/100.0f;
                        VERBOSE_DOME_DEBUG_PRINTLN(fDomeThrottle);
                    }
                    m = fDomeThrottle;
                    if (fDomePosition != nullptr)
                    {
                        float minspeed = fDomePosition->getDomeMinSpeed();
                        if (abs(m) < minspeed)
                        {
                            m = 0;
                        }
                    }
                }
                motor(getInverted() ? -m : m);
                fLastCommand = currentMillis;
                fMoving = (abs(m) != 0.0);
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
    bool fError = false;
    bool fEnabled = false;
    bool fWasConnected = false;
    bool fMotorStopped = false;
    bool fIdle = true;
    bool fScaling = false;
    bool fUseLeftStick = false;
    bool fUseThrottle = true;
    bool fUseHardStop = false;
    bool fInverted = false;
    bool fMoving = false;
    float fSpeedModifier;
    float fDrive = 0;
    float fAutoDrive = 0;
    int fLastDomePosition = -1;
    uint32_t fSerialLatency = 0;
    uint32_t fLastCommand = 0;
    uint32_t fLastDomeMovement = 0;
    bool fDomeMovementStarted = false;
    unsigned fThrottleAccelerationScale = 0;
    unsigned fThrottleDecelerationScale = 0;
    float fDomeThrottle = 0;
    int fLastDomeMode = -1;
    int fAutoDomeTargetPos = 0;
    bool fAutoDomeGoHome = false;
    bool fAutoDomeLeft = random(1);
    uint32_t fNextAutoDomeMovement = 0;
    DomePosition* fDomePosition = nullptr;
    uint32_t fMovementStartTime = 0;
    uint32_t fMovementFinishTime = 0;
    void (*fComplete)() = nullptr;
};
#endif
