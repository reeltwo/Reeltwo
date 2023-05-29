#ifndef ServoDispatchDirect_h
#define ServoDispatchDirect_h

#ifdef USE_SERVO_DEBUG
#define SERVO_DEBUG_PRINT(s) DEBUG_PRINT(s)
#define SERVO_DEBUG_PRINTLN(s) DEBUG_PRINTLN(s)
#define SERVO_DEBUG_PRINT_HEX(s) DEBUG_PRINT_HEX(s)
#define SERVO_DEBUG_PRINTLN_HEX(s) DEBUG_PRINTLN_HEX(s)
#else
#define SERVO_DEBUG_PRINT(s)
#define SERVO_DEBUG_PRINTLN(s)
#define SERVO_DEBUG_PRINT_HEX(s)
#define SERVO_DEBUG_PRINTLN_HEX(s)
#endif

#ifdef USE_VERBOSE_SERVO_DEBUG
#define VERBOSE_SERVO_DEBUG_PRINT(s) DEBUG_PRINT(s)
#define VERBOSE_SERVO_DEBUG_PRINTLN(s) DEBUG_PRINTLN(s)
#define VERBOSE_SERVO_DEBUG_PRINT_HEX(s) DEBUG_PRINT_HEX(s)
#define VERBOSE_SERVO_DEBUG_PRINTLN_HEX(s) DEBUG_PRINTLN_HEX(s)
#else
#define VERBOSE_SERVO_DEBUG_PRINT(s)
#define VERBOSE_SERVO_DEBUG_PRINTLN(s)
#define VERBOSE_SERVO_DEBUG_PRINT_HEX(s)
#define VERBOSE_SERVO_DEBUG_PRINTLN_HEX(s)
#endif

#include "ServoDispatchPrivate.h"

/**
  * \ingroup Core
  *
  * \class ServoDispatchDirect
  *
  * \brief Implements ServoDispatch dirctly on PWM enabled outputs.
  */
template <uint8_t numServos>
class ServoDispatchDirect :
    public ServoDispatch, SetupEvent, AnimatedEvent
#ifndef ARDUINO_ARCH_ESP32
    ,private ServoDispatchISR
#endif
{
public:
    ServoDispatchDirect() :
        fLastTime(0)
    {
    #ifndef ARDUINO_ARCH_ESP32
        const unsigned kDefaultPulseWidth = 1500;
        auto priv = privates();
    #endif
        memset(fServos, '\0', sizeof(fServos));
        for (uint16_t i = 0; i < numServos; i++)
        {
            ServoState* state = &fServos[i];
            state->channel = i;
        #ifndef ARDUINO_ARCH_ESP32
            priv->pwm[i].ticks = convertMicrosecToTicks(kDefaultPulseWidth);
        #endif
        }
    #ifndef ARDUINO_ARCH_ESP32
        priv->ServoCount = numServos;
    #endif
    }

    ServoDispatchDirect(const ServoSettings* settings) :
        fLastTime(0)
    {
    #ifndef ARDUINO_ARCH_ESP32
        const unsigned kDefaultPulseWidth = 1500;
        auto priv = privates();
    #endif
        memset(fServos, '\0', sizeof(fServos));
        for (uint16_t i = 0; i < numServos; i++)
        {
            ServoState* state = &fServos[i];
            uint8_t pin = pgm_read_uint16(&settings[i].pinNum);
        #ifndef ARDUINO_ARCH_ESP32
            pinMode(pin, OUTPUT);
        #endif
            state->channel = i;
            state->startPulse = pgm_read_uint16(&settings[i].startPulse);
            state->endPulse = pgm_read_uint16(&settings[i].endPulse);
            /* netural defaults to start position */
            state->neutralPulse = state->startPulse;
            state->group = pgm_read_uint32(&settings[i].group);
            state->posNow = state->neutralPulse;
        #ifndef ARDUINO_ARCH_ESP32
            priv->pwm[i].pin = pin;
            priv->pwm[i].ticks = convertMicrosecToTicks(kDefaultPulseWidth);
        #else
            state->pin = (ServoDispatchESP32::validPWM(pin)) ? pin : 0;
        #endif
        }
    #ifndef ARDUINO_ARCH_ESP32
        priv->ServoCount = numServos;
    #endif
    }

    virtual uint16_t getNumServos() override
    {
        return numServos;
    }

    virtual uint8_t getPin(uint16_t num) override
    {
        if (num < numServos)
        {
        #ifndef ARDUINO_ARCH_ESP32
            auto priv = privates();
            return priv->pwm[num].pin;
        #else
            ServoState* state = &fServos[num];
            return state->pin;
        #endif
        }
        return 0;
    }

    virtual uint16_t getStart(uint16_t num) override
    {
        return (num < numServos) ? fServos[num].startPulse : 0;
    }

    virtual uint16_t getEnd(uint16_t num) override
    {
        return (num < numServos) ? fServos[num].endPulse : 0;
    }

    virtual uint16_t getMinimum(uint16_t num) override
    {
        return (num < numServos) ? fServos[num].getMinimum() : 0;
    }

    virtual uint16_t getNeutral(uint16_t num) override
    {
        return (num < numServos) ? fServos[num].getNeutral() : 0;
    }

    virtual uint16_t getMaximum(uint16_t num) override
    {
        return (num < numServos) ? fServos[num].getMaximum() : 0;
    }

    virtual uint32_t getGroup(uint16_t num) override
    {
        return (num < numServos) ? fServos[num].group : 0;
    }

    virtual uint16_t currentPos(uint16_t num) override
    {
        return (num < numServos) ? fServos[num].currentPos() : 0;
    }

    virtual void setServo(uint16_t num, uint8_t pin, uint16_t startPulse, uint16_t endPulse, uint16_t neutralPulse, uint32_t group) override
    {
    #ifndef ARDUINO_ARCH_ESP32
        const unsigned kDefaultPulseWidth = 1500;
        auto priv = privates();
    #endif
        if (num < numServos)
        {
            ServoState* state = &fServos[num];
        #ifndef ARDUINO_ARCH_ESP32
            pinMode(pin, OUTPUT);
        #endif
            state->channel = num;
            state->startPulse = startPulse;
            state->endPulse = endPulse;
            state->neutralPulse = neutralPulse;
            state->group = group;
            state->posNow = state->neutralPulse;
        #ifndef ARDUINO_ARCH_ESP32
            priv->pwm[num].pin = pin;
            priv->pwm[num].ticks = convertMicrosecToTicks(kDefaultPulseWidth);
        #else
            state->pin = (ServoDispatchESP32::validPWM(pin)) ? pin : 0;
        #endif
        }
    }

    virtual void stop() override
    {
        // Stop all servo movement
        for (uint16_t i = 0; i < numServos; i++)
        {
            disable(i);
        }
    }

    virtual uint16_t scaleToPos(uint16_t num, float scale) override
    {
        uint16_t pos = 0;
        if (num < numServos)
        {
            scale = min(max(0.0f, scale), 1.0f);
            uint16_t startPulse = fServos[num].startPulse;
            uint16_t endPulse = fServos[num].endPulse;
            if (startPulse < endPulse)
            {
                pos = startPulse + (endPulse - startPulse) * scale;
            }
            else
            {
                pos = startPulse - (startPulse - endPulse) * scale;
            }
        }
        return pos;
    }

    virtual void setup() override
    {
    #ifdef ARDUINO_ARCH_ESP32
        // TODO only configure necessary number of timers
        ServoDispatchESP32::configureTimer(0);
        ServoDispatchESP32::configureTimer(1);
        ServoDispatchESP32::configureTimer(2);
        ServoDispatchESP32::configureTimer(3);
        // Attach and detach PWM pin to clear out any signals
        for (int i = 0; i < numServos; i++)
        {
            ServoState* state = &fServos[i];
            fPWM[i].attachPin(state->pin, REFRESH_CPS, DEFAULT_TIMER_WIDTH);
            fPWM[i].detachPin(state->pin);
        }
    #endif
    }

    virtual bool isActive(uint16_t num) override
    {
        if (num < numServos)
        {
        #ifndef ARDUINO_ARCH_ESP32
            auto priv = privates();
            return priv->pwm[num].isActive;
        #else
            return fPWM[num].attached();
        #endif
        }
        return false;
    }

    virtual void disable(uint16_t num) override
    {
        if (num < numServos)
        {
            ServoState* state = &fServos[num];
            state->init();
        #ifndef ARDUINO_ARCH_ESP32
            auto priv = privates();
            priv->pwm[num].isActive = false;
            digitalWrite(priv->pwm[num].pin, LOW);
        #else
            if (fPWM[num].attached())
            {
                fPWM[num].detachPin(state->pin);
            }
        #endif
        }
    }

    virtual void animate() override
    {
        uint32_t now = millis();
        if (fLastTime + 1 < now)
        {
            for (int i = 0; i < numServos; i++)
            {
                ServoState* state = &fServos[i];
                if (state->finishTime != 0)
                {
                    state->move(this, now);
                }
                else if (state->detachTime > 0 && state->detachTime < millis())
                {
                    disable(i);
                }
            }
            fLastTime = now = millis();
        }
    }

    virtual void setPWM(uint16_t channel, uint16_t targetLength) override
    {
    #ifndef ARDUINO_ARCH_ESP32
        auto priv = privates();
        if (channel < MAX_SERVOS)   // ensure channel is valid
        {
            if (!priv->pwm[channel].isActive)
            {
                initISR(fServos[channel].channel);
            }

            priv->pwm[channel].value = targetLength;
            // if (value < SERVO_MIN())                // ensure pulse width is valid
            //     value = SERVO_MIN();
            // else if (value > SERVO_MAX())
            //     value = SERVO_MAX();

            targetLength -= kTrimDuration;
            targetLength = convertMicrosecToTicks(targetLength);

            uint8_t oldSREG = SREG;
            // uint16_t oldticks;
            cli();
            // oldticks = priv->pwm[channel].ticks;
            priv->pwm[channel].ticks = targetLength;
            SREG = oldSREG;
            priv->pwm[channel].speed = 0;
            sei();
            // if (oldticks != targetLength)
            // {
            //     DEBUG_PRINT("setPWM ");
            //     DEBUG_PRINT(channel);
            //     DEBUG_PRINT(" target: ");
            //     DEBUG_PRINTLN(targetLength);
            // }
        }
    #else
        if (fPWM[channel].attached())
        {
            uint32_t ticks = convertMicrosecToTicks(targetLength);
            fPWM[channel].write(ticks);
            // Serial.print("PWM pin="); Serial.print(fServos[channel].pin); Serial.print(" = "); Serial.println(targetLength);
        }
    #endif
    }

private:
    enum
    {
        kMinPulseWidth = 544,
        kMaxPulseWidth = 2400,
        kTrimDuration  = 2
    };

    struct ServoState
    {
        uint16_t currentPos()
        {
            return posNow;
        }

        uint16_t getMinimum()
        {
            return min(startPulse, endPulse);
        }

        uint16_t getNeutral()
        {
            return neutralPulse;
        }

        uint16_t getMaximum()
        {
            return max(startPulse, endPulse);
        }

        void move(ServoDispatchDirect<numServos>* dispatch, uint32_t timeNow)
        {
            if (finishTime != 0)
            {
                if (timeNow < startTime)
                {
                    /* wait */
                }
                else if (timeNow >= finishTime)
                {
                    posNow = finishPos;
                    doMove(dispatch, timeNow);
                    init();
                    detachTime = timeNow + 500;
                }
                else if (lastMoveTime != timeNow)
                {
                    uint32_t timeSinceLastMove = timeNow - startTime;
                    uint32_t denominator = finishTime - startTime;
                    float (*useMethod)(float) = easingMethod;
                    if (useMethod == nullptr)
                        useMethod = Easing::LinearInterpolation;
                    float fractionChange = useMethod(float(timeSinceLastMove)/float(denominator));
                    int distanceToMove = float(deltaPos) * fractionChange;
                    uint16_t newPos = startPosition + distanceToMove;
                    if (newPos != posNow)
                    {
                        posNow = startPosition + distanceToMove;
                        doMove(dispatch, timeNow);
                    }
                }
            }
        }

        void moveToPulse(ServoDispatchDirect<numServos>* dispatch, uint32_t startDelay, uint32_t moveTime, uint16_t startPos, uint16_t pos)
        {
            uint32_t timeNow = millis();

            startTime = startDelay + timeNow;
            finishTime = moveTime + startTime;
            finishPos = min(getMaximum(), max(getMinimum(), pos));
            posNow = startPosition = startPos;
            deltaPos = finishPos - posNow;
            doMove(dispatch, timeNow);
        }
        uint8_t channel;
        uint32_t group;
        uint16_t startPulse;
        uint16_t endPulse;
        uint16_t neutralPulse;
        uint32_t startTime;
        uint32_t finishTime;
        uint32_t lastMoveTime;
        uint16_t finishPos;
        uint16_t startPosition;
        uint16_t posNow;
        uint32_t detachTime;
        int deltaPos;
        float (*easingMethod)(float completion) = nullptr;
    #ifdef ARDUINO_ARCH_ESP32
        uint8_t pin;
        uint32_t ticks;
    #endif

        void doMove(ServoDispatchDirect<numServos>* dispatch, uint32_t timeNow)
        {
        #ifdef USE_SERVO_DEBUG
            SERVO_DEBUG_PRINT("PWM ");
            SERVO_DEBUG_PRINT(channel);
            SERVO_DEBUG_PRINT(" ");
            SERVO_DEBUG_PRINTLN(posNow);
        #endif
            dispatch->setPWM(channel, posNow);
        #if 0//def USE_SMQ
            if (SMQ::sendTopic("PWM"))
            {
                SMQ::send_uint8(F("num"), channel);
                SMQ::send_float(F("len"), posNow);
                SMQ::send_end();
            }
        #endif
            lastMoveTime = timeNow;
        }

        void init()
        {
            finishTime = 0;
            lastMoveTime = 0;
            finishPos = 0;
            detachTime = 0;
        }
    };

    /////////////////////////////////////////////////////////////////////////////////

    virtual void _moveServoToPulse(uint16_t num, uint32_t startDelay, uint32_t moveTime, uint16_t startPos, uint16_t pos) override
    {
        if (num < numServos)
        {
        #ifndef ARDUINO_ARCH_ESP32
            initISR(fServos[num].channel);
        #else
            if (fServos[num].pin && !fPWM[num].attached())
            {
                fPWM[num].attachPin(fServos[num].pin, REFRESH_CPS, DEFAULT_TIMER_WIDTH);
                SERVO_DEBUG_PRINTLN("Attaching servo : "+String(fServos[num].pin)+" on PWM "+String(fPWM[num].getChannel())+" PULSE "+pos);
            }
        #endif
            fServos[num].moveToPulse(this, startDelay, moveTime, startPos, pos);
            fLastTime = 0;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////

    // Move all servos matching servoGroupMask starting at startDelay in moveTimeMin-moveTimeMax to position pos
    virtual void _moveServosToPulse(uint32_t servoGroupMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, uint16_t pos) override
    {
        for (uint16_t i = 0; i < numServos; i++)
        {
            uint32_t moveTime = (moveTimeMin != moveTimeMax) ? random(moveTimeMin, moveTimeMax) : moveTimeMax;
            if ((fServos[i].group & servoGroupMask) != 0)
            {
                moveToPulse(i, startDelay, moveTime, fServos[i].currentPos(), pos);
            }
        }
    }

    virtual void _moveServosByPulse(uint32_t servoGroupMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, int16_t pos) override
    {
        for (uint16_t i = 0; i < numServos; i++)        
        {
            uint32_t moveTime = (moveTimeMin != moveTimeMax) ? random(moveTimeMin, moveTimeMax) : moveTimeMax;
            if ((fServos[i].group & servoGroupMask) != 0)
            {
                int16_t curpos = fServos[i].currentPos();
                moveToPulse(i, startDelay, moveTime, curpos, curpos + pos);
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////

    virtual void _moveServoSetToPulse(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, uint16_t onPos, uint16_t offPos) override
    {
        byte bitShift = 31;
        for (uint16_t i = 0; i < numServos; i++)
        {
            if ((fServos[i].group & servoGroupMask) == 0)
                continue;
            uint32_t moveTime = (moveTimeMin != moveTimeMax) ? random(moveTimeMin, moveTimeMax) : moveTimeMax;
            bool on = ((servoSetMask & (1L<<bitShift)) != 0);
            moveToPulse(i, startDelay, moveTime, fServos[i].currentPos(), (on) ? onPos : offPos);
            if (bitShift-- == 0)
                break;
        }
    }

    virtual void _moveServoSetByPulse(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, int16_t onPos, int16_t offPos) override
    {
        byte bitShift = 31;
        for (uint16_t i = 0; i < numServos; i++)        
        {
            if ((fServos[i].group & servoGroupMask) == 0)
                continue;
            uint32_t moveTime = (moveTimeMin != moveTimeMax) ? random(moveTimeMin, moveTimeMax) : moveTimeMax;
            bool on = ((servoSetMask & (1L<<bitShift)) != 0);
            int16_t curpos = fServos[i].currentPos();
            moveToPulse(i, startDelay, moveTime, curpos, curpos + ((on) ? onPos : offPos));
            if (bitShift-- == 0)
                break;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////

    virtual void _moveServosTo(uint32_t servoGroupMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, float pos) override
    {
        for (uint16_t i = 0; i < numServos; i++)
        {
            uint32_t moveTime = (moveTimeMin != moveTimeMax) ? random(moveTimeMin, moveTimeMax) : moveTimeMax;
            if ((fServos[i].group & servoGroupMask) != 0)
            {
                moveToPulse(i, startDelay, moveTime, fServos[i].currentPos(), scaleToPos(i, pos));
            }
        }
    }

    virtual void _moveServoSetTo(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, float onPos, float offPos, float (*onEasingMethod)(float), float (*offEasingMethod)(float)) override
    {
        byte bitShift = 31;
        for (uint16_t i = 0; i < numServos; i++)
        {
            if ((fServos[i].group & servoGroupMask) == 0)
                continue;
            uint32_t moveTime = (moveTimeMin != moveTimeMax) ? random(moveTimeMin, moveTimeMax) : moveTimeMax;
            bool on = ((servoSetMask & (1L<<bitShift)) != 0);
            VERBOSE_SERVO_DEBUG_PRINT("moveToPulse on=");
            VERBOSE_SERVO_DEBUG_PRINT(on);
            VERBOSE_SERVO_DEBUG_PRINT(" onPos=");
            VERBOSE_SERVO_DEBUG_PRINT(onPos);
            VERBOSE_SERVO_DEBUG_PRINT(" offPos=");
            VERBOSE_SERVO_DEBUG_PRINT(offPos);
            VERBOSE_SERVO_DEBUG_PRINT(" pos=");
            VERBOSE_SERVO_DEBUG_PRINTLN(scaleToPos(i, (on) ? onPos : offPos));
            if (on)
            {
                if (onEasingMethod != nullptr)
                    setServoEasingMethod(i, onEasingMethod);
            }
            else if (offEasingMethod != nullptr)
            {
                setServoEasingMethod(i, offEasingMethod);
            }
            moveToPulse(i, startDelay, moveTime, fServos[i].currentPos(), scaleToPos(i, (on) ? onPos : offPos));
            if (bitShift-- == 0)
                break;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////

    virtual void _setServoEasingMethod(uint16_t num, float (*easingMethod)(float completion))
    {
        if (num < numServos && fServos[num].channel != 0)
        {
            fServos[num].easingMethod = easingMethod;
        }
    }

    virtual void _setServosEasingMethod(uint32_t servoGroupMask, float (*easingMethod)(float completion))
    {
        for (uint16_t i = 0; i < numServos; i++)
        {
            if ((fServos[i].group & servoGroupMask) != 0)
            {
                fServos[i].easingMethod = easingMethod;
            }
        }
    }

    virtual void _setEasingMethod(float (*easingMethod)(float completion))
    {
        for (uint16_t i = 0; i < numServos; i++)
        {
            fServos[i].easingMethod = easingMethod;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////

    uint32_t fLastTime;
    ServoState fServos[numServos];

private:
    inline uint16_t pgm_read_uint16(const uint16_t* p)
    {
    #if defined(__AVR_ATmega1280__)  || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega32U4__) || \
        defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__) || \
        defined(__AVR_ATmega128__) ||defined(__AVR_ATmega1281__)||defined(__AVR_ATmega2561__)
        return pgm_read_word(p);
    #else
        return *p;
    #endif
    }

    inline uint32_t pgm_read_uint32(const uint32_t* p)
    {
    #if defined(__AVR_ATmega1280__)  || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega32U4__) || \
        defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__) || \
        defined(__AVR_ATmega128__) ||defined(__AVR_ATmega1281__)||defined(__AVR_ATmega2561__)
        return pgm_read_dword(p);
    #else
        return *p;
    #endif
    }

#ifdef ARDUINO_ARCH_ESP32
    const int REFRESH_CPS = 50;
    const int REFRESH_USEC = 20000;
#ifdef CONFIG_IDF_TARGET_ESP32
    const int DEFAULT_TIMER_WIDTH = 16;
#else
    const int DEFAULT_TIMER_WIDTH = 14;
#endif
    const int DEFAULT_TIMER_WIDTH_TICKS = (1<<DEFAULT_TIMER_WIDTH);
    ServoDispatchESP32 fPWM[numServos];
    int convertMicrosecToTicks(int usec)
    {
        return (int)((float)usec / ((float)REFRESH_USEC / (float)DEFAULT_TIMER_WIDTH_TICKS)*(((float)REFRESH_CPS)/50.0));
    }

    int convertTicksToMicrosec(int ticks)
    {
        return (int)((float)ticks * ((float)REFRESH_USEC / (float)DEFAULT_TIMER_WIDTH_TICKS)/(((float)REFRESH_CPS)/50.0));
    }
#endif
};

#endif
