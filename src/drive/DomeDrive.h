#ifndef DomeDrive_h
#define DomeDrive_h

#include "ReelTwo.h"
#include "core/AnimatedEvent.h"
#include "core/SetupEvent.h"
#include "JoystickController.h"

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

class DomePosition
{
public:
    enum Mode
    {
        kOff = 1,
        kHome,
        kRandom,
        kTarget,
        kCalibrate
    };
    virtual Mode getDomeMode() = 0;
    virtual void setDomeMode(Mode mode) = 0;
    virtual bool getDomeFlip() = 0;
    virtual float getDomeSpeedHome() = 0;
    virtual unsigned getDomeFudge() = 0;
    virtual unsigned getDomeSeekLeft() = 0;
    virtual unsigned getDomeSeekRight() = 0;
    virtual unsigned getDomeMinDelay() = 0;
    virtual unsigned getDomeMaxDelay() = 0;
    virtual unsigned getDomeHome() = 0;
    virtual unsigned getDomeTargetPosition() = 0;
    virtual unsigned getDomePosition() = 0;
};

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
        fDomeStick(domeStick),
        fWasConnected(false),
        fMotorStopped(false),
        fLastCommand(0),
        fLastDomeMovement(0)
    {
        // Default enabled
        setEnable(true);
        // Default to half speed max
        setMaxSpeed(0.5);
        // Default 25ms
        setSerialLatency(0);

        fEnabled = true;
    }

    virtual void setup() override
    {
    }
    
    inline void setDomePosition(DomePosition* domePosition)
    {
        fDomePosition = domePosition;
    }

    // void setDomePosition(int degrees)
    // {
    //     fDegrees = (int)fmod(degrees, 360);
    //     if (fDegrees < 0)
    //         fDegrees += 360;
    //     DEBUG_PRINTLN("TARGET: "+String(fDegrees));
    // }

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

    float getMaxSpeed()
    {
        return fSpeedModifier;
    }

    void setMaxSpeed(float modifier)
    {
        if (fDomeStick.isConnected())
            stop();
        fSpeedModifier = min(max(modifier, 0.0f), 1.0f);
    }

    virtual void stop()
    {
        fMotorStopped = true;
    }

    JoystickController* getActiveStick()
    {
        if (fDomeStick.isConnected())
        {
            return &fDomeStick;
        }
        return nullptr;
    }

    /**
      * Dispatch any received i2c event to CommandEvent
      */
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

protected:
    virtual void motor(float m) = 0;
    virtual float throttleSpeed(float speedModifier)
    {
        return speedModifier;
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

    bool moveDomeToTarget(int pos, int target, int fudge, float speed, float &m)
    {
        if (!withinArc(target - fudge, target + fudge, pos))
        {
            int dist = shortestDistance(pos, target);
            if (abs(dist) > fudge*2)
                speed += (1.0 - speed) * float(abs(dist)) / 180;
            else
                speed = 0.3;
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
        else
        {
            uint32_t currentMillis = millis();
            if (currentMillis - fLastCommand > fSerialLatency)
            {
                float drive_mod = throttleSpeed(speedModifier);
                float m = (float)(stick->state.analog.stick.rx + 128) / 127.5 - 1.0;

                if (abs(m) < 0.2)
                    m = 0;
                else
                    m = pow(abs(m)-0.2, 1.4) * ((m < 0) ? -1 : 1);

                // clamp to -1/+1 and apply max speed limit
                m = max(-1.0f, min(m, 1.0f)) * drive_mod;

                DomePosition::Mode domeMode = fDomePosition->getDomeMode();
                if (fDomePosition != nullptr && abs(m) == 0.0 && domeMode != DomePosition::kOff)
                {
                    // No joystick movement - check auto dome
                    uint32_t minDelay = uint32_t(fDomePosition->getDomeMinDelay()) * 1000L;
                    uint32_t maxDelay = uint32_t(fDomePosition->getDomeMaxDelay()) * 1000L;
                    if (fLastDomeMovement + minDelay < currentMillis)
                    {
                        int pos = fDomePosition->getDomePosition();
                        int home = fDomePosition->getDomeHome();
                        int targetpos = fDomePosition->getDomeTargetPosition();
                        int fudge = fDomePosition->getDomeFudge();
                        float speed = fDomePosition->getDomeSpeedHome();
                        if (fLastDomePosition == -1)
                            fLastDomePosition = pos;
                        // Ignore dome readings that are not near the last dome reading
                        if (!withinArc(pos, fLastDomePosition-10, fLastDomePosition+10))
                        {
                            // DEBUG_PRINTLN("pos out of range: "+String(pos)+" last= "+String(fLastDomePosition));
                            pos = fLastDomePosition;
                        }
                        else
                        {
                            fLastDomePosition = pos;                            
                        }
                        bool newMode = (fLastDomeMode != domeMode);
                        switch (fLastDomeMode = domeMode)
                        {
                            case DomePosition::kOff:
                                break;
                            case DomePosition::kHome:
                                moveDomeToTarget(pos, home, fudge, speed, m);
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
                            case DomePosition::kRandom:
                                if (newMode)
                                {
                                    uint32_t r = random(minDelay, maxDelay);
                                    // DEBUG_PRINT("RANDOM START IN "); DEBUG_PRINTLN(r);
                                    fNextAutoDomeMovement = millis() + r;
                                    fAutoDomeTargetPos = -1;
                                }
                                if (fAutoDomeTargetPos == -1 && fNextAutoDomeMovement < millis())
                                {
                                    if (fAutoDomeGoHome)
                                    {
                                    // DEBUG_PRINTLN("RANDOM GO HOME: ");
                                        fAutoDomeTargetPos = home;
                                        fAutoDomeGoHome = false;
                                    }
                                    else if (random(100) < 50)
                                    {
                                        int distance = random(fDomePosition->getDomeSeekLeft());
                                        fAutoDomeTargetPos = normalize(home - distance);
                                        fAutoDomeGoHome = true;
                                    // DEBUG_PRINTLN("RANDOM TURN LEFT: "+String(distance));
                                    }
                                    else if (random(100) < 50)
                                    {
                                        int distance = random(fDomePosition->getDomeSeekRight());
                                        fAutoDomeTargetPos = normalize(home + distance);
                                        fAutoDomeGoHome = true;
                                    // DEBUG_PRINTLN("RANDOM TURN RIGHT: "+String(distance));
                                    }
                                    else
                                    {
                                        uint32_t r = random(minDelay, maxDelay);
                                        // DEBUG_PRINTLN("RANDOM DO NOTHING NEXT: "+String(r));
                                        fNextAutoDomeMovement = millis() + r;
                                    }
                                }
                                if (fAutoDomeTargetPos != -1)
                                {
                                    // DEBUG_PRINT("POS: "); DEBUG_PRINT(pos); DEBUG_PRINT(" TARGET: "); DEBUG_PRINTLN(fAutoDomeTargetPos);
                                    if (moveDomeToTarget(pos, fAutoDomeTargetPos, fudge, speed, m))
                                    {
                                        // Set next autodome movement time
                                        uint32_t r = random(minDelay, maxDelay);
                                        // DEBUG_PRINTLN("RANDOM ARRIVED NEXT: "+String(r));
                                        fNextAutoDomeMovement = millis() + r;
                                        fAutoDomeTargetPos = -1;
                                    }
                                }
                                break;
                            case DomePosition::kTarget:
                                moveDomeToTarget(pos, targetpos, fudge, speed, m);
                                break;
                            case DomePosition::kCalibrate:
                                if (newMode)
                                {
                                    DEBUG_PRINTLN("CALIBRATE START: "+String(pos));
                                    fCalibrateStartMS = millis();
                                    fCalibrateStartPos = pos;
                                    m = 1.0;
                                }
                                else if (currentMillis - fCalibrateStartMS > 2000)
                                {
                                    DEBUG_PRINTLN("POS: "+String(pos));
                                    if (pos == fCalibrateStartPos)
                                    {
                                        fDomePosition->setDomeMode(DomePosition::kOff);
                                        DEBUG_PRINTLN("TIME: "+String(currentMillis - fCalibrateStartMS));
                                    }
                                    else
                                    {
                                        m = 1.0;
                                    }
                                }
                                else
                                {
                                    m = 1.0;
                                }
                                break;
                        }
                    }
                }
                else
                {
                    fLastDomePosition = -1; 
                    fLastDomeMovement = currentMillis;
                }
                motor(m);
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
    bool fEnabled;
    bool fWasConnected;
    bool fMotorStopped;
    float fSpeedModifier;
    int fLastDomePosition = -1;
    uint32_t fSerialLatency;
    uint32_t fLastCommand;
    uint32_t fLastDomeMovement;
    int fLastDomeMode = -1;
    int fAutoDomeTargetPos = 0;
    bool fAutoDomeGoHome = false;
    uint32_t fNextAutoDomeMovement;
    uint32_t fCalibrateStartMS;
    int fCalibrateStartPos;
    DomePosition* fDomePosition = nullptr;
};
#endif
