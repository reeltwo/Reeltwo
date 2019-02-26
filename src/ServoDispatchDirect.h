#ifndef ServoDispatchDirect_h
#define ServoDispatchDirect_h

#include "ServoDispatch.h"

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
//#define SERVO_MIN() (MIN_PULSE_WIDTH - this->min * 4)
//#define SERVO_MAX() (MAX_PULSE_WIDTH - this->max * 4)

/************ static functions common to all instances ***********************/

/// \private
class ServoDispatchISR
{
public:
#if defined(__AVR_ATmega1280__)  || defined(__AVR_ATmega2560__)
#define USE_TIMER5
#define USE_TIMER1
#define USE_TIMER3
#define USE_TIMER4
enum Timer16Order { kTimer16_5, kTimer16_1, kTimer16_3, kTimer16_4, kNumTimers16 };
#elif defined(__AVR_ATmega32U4__) || defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__) || \
      defined(__AVR_ATmega128__) ||defined(__AVR_ATmega1281__)||defined(__AVR_ATmega2561__)
#define USE_TIMER3
#define USE_TIMER1
enum Timer16Order { kTimer16_3, kTimer16_1, kNumTimers16 };
#else  // everything else
#define USE_TIMER1
enum Timer16Order { kTimer16_1, kNumTimers16 };
#endif
#define SERVOS_PER_TIMER                    12      // the maximum number of servos controlled by one timer 
#define MAX_SERVOS                          (kNumTimers16 * SERVOS_PER_TIMER)
#define SERVO_INDEX(_timer,_channel)        ((_timer*SERVOS_PER_TIMER) + _channel)

    /// \private
    struct PWMChannel
    {
        uint8_t pin:6;
        uint8_t isActive:1;
        unsigned int ticks;
        unsigned int value;
        unsigned int target;
        uint8_t speed;
    };
    /// \private
    struct Private
    {
        uint8_t ServoCount;                             // the total number of attached servos
        volatile int8_t channel[kNumTimers16];          // counter for the servo being pulsed for each timer (or -1 if refresh interval)
        PWMChannel pwm[MAX_SERVOS];                     // static array of servo structures
    };

    static Private* privates()
    {
        static Private priv;
        return &priv;
    }

    static unsigned int convertMicrosecToTicks(unsigned int microsec)
    {
        return (clockCyclesPerMicrosecond() * microsec) / 8;
    }

    static unsigned int convertTicksToMicrosec(unsigned int ticks)
    {
        return ((unsigned)ticks * 8) / clockCyclesPerMicrosecond();
    }

    static inline void handle_interrupts(Timer16Order timer, volatile uint16_t *TCNTn, volatile uint16_t* OCRnA)
    {
        Private* priv = privates();
        volatile int8_t* timerChannel = &priv->channel[timer];
        PWMChannel* servo = &priv->pwm[SERVO_INDEX(timer, *timerChannel)];
        if (*timerChannel < 0)
        {
            *TCNTn = 0; // channel set to -1 indicated that refresh interval completed so reset the timer
        }
        else
        {
            if (SERVO_INDEX(timer,*timerChannel) < priv->ServoCount && servo->isActive == true)
                digitalWrite(servo->pin,LOW); // pulse this channel low if activated
        }

        *timerChannel += 1;    // increment to the next channel
        servo = &priv->pwm[SERVO_INDEX(timer, *timerChannel)];
        if (SERVO_INDEX(timer, *timerChannel) < priv->ServoCount && *timerChannel < SERVOS_PER_TIMER)
        {
            // Extension for slowmove
            if (servo->speed)
            {
                // Increment ticks by speed until we reach the target.
                // When the target is reached, speed is set to 0 to disable that code.
                if (servo->target > servo->ticks)
                {
                    servo->ticks += servo->speed;
                    if (servo->target <= servo->ticks)
                    {
                        servo->ticks = servo->target;
                        servo->speed = 0;
                    }
                }
                else
                {
                    servo->ticks -= servo->speed;
                    if (servo->target >= servo->ticks)
                    {
                        servo->ticks = servo->target;
                        servo->speed = 0;
                    }
                }
            }
            // End of Extension for slowmove

            // Todo
            *OCRnA = *TCNTn + servo->ticks;
            if (servo->isActive == true)     // check if activated
                digitalWrite(servo->pin,HIGH); // its an active channel so pulse it high
        }
        else
        {
            const unsigned kRefreshInterval = 20000; // time to refresh servos in microseconds
            // finished all channels so wait for the refresh period to expire before starting over
            unsigned long ticks = convertMicrosecToTicks(kRefreshInterval);
            if ((unsigned)*TCNTn <  ticks + 4)  // allow a few ticks to ensure the next OCR1A not missed
                *OCRnA = (unsigned int)ticks;
            else
                *OCRnA = *TCNTn + 4;  // at least REFRESH_INTERVAL has elapsed
            *timerChannel = -1; // this will get incremented at the end of the refresh period to start again at the first channel
        }
    }

    static Timer16Order channelToTimer(byte channel)
    {
        return ((Timer16Order)(channel / SERVOS_PER_TIMER));
    }

    static void initISR(int channel)
    {
        Timer16Order timer = channelToTimer(channel);
        if (!isTimerActive(timer))
        {
            switch (timer)
            {
            #if defined (USE_TIMER1)
                case kTimer16_1:
                {
                    TCCR1A = 0;             // normal counting mode
                    TCCR1B = _BV(CS11);     // set prescaler of 8
                    TCNT1 = 0;              // clear the timer count
                #if defined(__AVR_ATmega8__)|| defined(__AVR_ATmega128__)
                    TIFR |= _BV(OCF1A);      // clear any pending interrupts;
                    TIMSK |=  _BV(OCIE1A) ;  // enable the output compare interrupt
                #else
                    // here if not ATmega8 or ATmega128
                    TIFR1 |= _BV(OCF1A);     // clear any pending interrupts;
                    TIMSK1 |=  _BV(OCIE1A) ; // enable the output compare interrupt
                #endif
                // #if defined(WIRING)
                //     timerAttach(TIMER1OUTCOMPAREA_INT, Timer1Service);
                // #endif
                    break;
                }
            #endif

            #if defined (USE_TIMER3)
                case kTimer16_3:
                {
                    TCCR3A = 0;             // normal counting mode
                    TCCR3B = _BV(CS31);     // set prescaler of 8
                    TCNT3 = 0;              // clear the timer count
                #if defined(__AVR_ATmega128__)
                    TIFR |= _BV(OCF3A);     // clear any pending interrupts;
                    ETIMSK |= _BV(OCIE3A);  // enable the output compare interrupt
                #else
                    TIFR3 = _BV(OCF3A);     // clear any pending interrupts;
                    TIMSK3 =  _BV(OCIE3A) ; // enable the output compare interrupt
                #endif
                // #if defined(WIRING)
                //     timerAttach(TIMER3OUTCOMPAREA_INT, Timer3Service);  // for Wiring platform only
                // #endif
                    break;
                }
            #endif

            #if defined (USE_TIMER4)
                case kTimer16_4:
                {
                    TCCR4A = 0;             // normal counting mode
                    TCCR4B = _BV(CS41);     // set prescaler of 8
                    TCNT4 = 0;              // clear the timer count
                    TIFR4 = _BV(OCF4A);     // clear any pending interrupts;
                    TIMSK4 =  _BV(OCIE4A) ; // enable the output compare interrupt
                    break;
                }
            #endif

            #if defined (USE_TIMER5)
                case kTimer16_5:
                {
                    TCCR5A = 0;             // normal counting mode
                    TCCR5B = _BV(CS51);     // set prescaler of 8
                    TCNT5 = 0;              // clear the timer count
                    TIFR5 = _BV(OCF5A);     // clear any pending interrupts;
                    TIMSK5 =  _BV(OCIE5A) ; // enable the output compare interrupt
                    break;
                }
            #endif
            }
        }
        Private* priv = privates();
        priv->pwm[channel].isActive = true;
    }

    static boolean isTimerActive(Timer16Order timer)
    {
        Private* priv = privates();
        // returns true if any servo is active on this timer
        for (uint8_t channel = 0; channel < SERVOS_PER_TIMER; channel++)
        {
            if (priv->pwm[SERVO_INDEX(timer,channel)].isActive == true)
                return true;
        }
        return false;
    }
};

/**
  * \ingroup Core
  *
  * \class ServoDispatchDirect
  *
  * \brief Implements ServoDispatch dirctly on PWM enabled outputs.
  */
template <uint8_t numServos>
class ServoDispatchDirect :
    public ServoDispatch, SetupEvent, AnimatedEvent,
    private ServoDispatchISR
{
public:
    ServoDispatchDirect(const ServoSettings* settings) :
        fLastTime(0),
        fOutputEnabled(true),
        fOutputExpireMillis(0)
    {
        const unsigned kDefaultPulseWidth = 1500;
        Private* priv = privates();
        memset(fServos, '\0', sizeof(fServos));
        for (uint16_t i = 0; i < numServos; i++)
        {
            ServoState* state = &fServos[i];
            uint8_t pin = pgm_read_byte(&settings[i].pinNum);
            pinMode(pin, OUTPUT);
            state->channel = i;
            state->group = pgm_read_dword(&settings[i].group);
            state->minPulse = pgm_read_word(&settings[i].minPulse);
            state->maxPulse = pgm_read_word(&settings[i].maxPulse);
            state->posNow = state->minPulse;
            priv->pwm[i].pin = pin;
            priv->pwm[i].ticks = convertMicrosecToTicks(kDefaultPulseWidth);
        }
        priv->ServoCount = numServos;
    }

    virtual uint16_t getNumServos() override
    {
        return numServos;
    }

    virtual uint16_t getMinimum(uint16_t num) override
    {
        return (num < numServos) ? fServos[num].minPulse : 0;
    }

    virtual uint16_t getMaximum(uint16_t num) override
    {
        return (num < numServos) ? fServos[num].maxPulse : 0;
    }

    virtual uint16_t currentPos(uint16_t num)
    {
        return (num < numServos) ? fServos[num].currentPos() : 0;
    }

    virtual void setup() override
    {
        for (uint16_t i = 0; i < numServos; i++)
        {
        }
    }

    virtual void animate() override
    {
        uint32_t now = millis();
        if (fLastTime + 1 < now)
        {
            for (int i = 0; i < numServos; i++)
            {
                if (fServos[i].finishTime != 0)
                    fServos[i].move(this, now);
            }
            fLastTime = now = millis();
        }
        if (fOutputEnabled && now > fOutputExpireMillis)
        {
            SERVO_DEBUG_PRINTFLN("POWER OFF");
            //digitalWrite(fOutputEnablePin, LOW);
            // setOutputAll(false);
            Private* priv = privates();
            for (int i = 0; i < numServos; i++)
            {
                priv->pwm[i].isActive = false;
            }
            fOutputEnabled = false;
        }
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
                }
                else if (lastMoveTime != timeNow)
                {
                    uint32_t timeSinceLastMove = timeNow - lastMoveTime;
                    uint32_t denominator = finishTime - lastMoveTime;
                    float fractionChange = float(timeSinceLastMove)/float(denominator);

                    int distanceToGo = finishPos - posNow;
                    float distanceToMove = float(distanceToGo) * fractionChange;
                    int distanceToMoveInt = int(distanceToMove);

                    if (abs(distanceToMoveInt) > 1)
                    {
                        posNow = posNow + distanceToMoveInt;
                        doMove(dispatch, timeNow);
                    }
                }
            }
        }

        uint16_t minPulseWidth()
        {
            return (minPulse - kMinPulseWidth) / 4;
        }

        uint16_t maxPulseWidth()
        {
            return (kMaxPulseWidth - maxPulse) / 4;
        }

        void moveTo(ServoDispatchDirect<numServos>* dispatch, uint32_t startDelay, uint32_t moveTime, uint16_t startPos, uint16_t pos)
        {
            uint32_t timeNow = millis();

            startTime = startDelay + timeNow;
            finishTime = moveTime + startTime;
            finishPos = min(maxPulse, max(minPulse, pos));
            posNow = startPos;
            doMove(dispatch, timeNow);
        }

        uint8_t channel;
        uint32_t group;
        uint16_t minPulse;
        uint16_t maxPulse;
        uint32_t startTime;
        uint32_t finishTime;
        uint32_t lastMoveTime;
        uint16_t finishPos;
        uint16_t posNow;

        void doMove(ServoDispatchDirect<numServos>* dispatch, uint32_t timeNow)
        {
        #ifdef SERVO_DEBUG
            SERVO_DEBUG_PRINTF("PWM ");
            SERVO_DEBUG_PRINT(channel);
            SERVO_DEBUG_PRINTF(" ");
            SERVO_DEBUG_PRINTLN(posNow);
        #endif

            dispatch->setPWM(channel, posNow);
        #ifdef USE_SMQ
            SMQ::send_start(F("PWM"));
            SMQ::send_uint8(F("num"), channel);
            SMQ::send_float(F("len"), posNow);
            SMQ::send_end();
        #endif
            lastMoveTime = timeNow;
        }

        void init()
        {
            finishTime = 0;
            lastMoveTime = 0;
            finishPos = 0;
        }
    };

    void setPWM(uint16_t channel, uint16_t targetLength)
    {
        Private* priv = privates();
        if (channel < MAX_SERVOS)   // ensure channel is valid
        {
            priv->pwm[channel].value = targetLength;
            // if (value < SERVO_MIN())                // ensure pulse width is valid
            //     value = SERVO_MIN();
            // else if (value > SERVO_MAX())
            //     value = SERVO_MAX();

            targetLength -= kTrimDuration;
            targetLength = convertMicrosecToTicks(targetLength);

            uint8_t oldSREG = SREG;
            cli();
            priv->pwm[channel].ticks = targetLength;
            SREG = oldSREG;
            priv->pwm[channel].speed = 0;
            sei();
        }
    }

    /////////////////////////////////////////////////////////////////////////////////

    virtual void _moveServoTo(uint16_t num, uint32_t startDelay, uint32_t moveTime, uint16_t startPos, uint16_t pos) override
    {
        if (num < numServos)
        {
            initISR(fServos[num].channel);
            fServos[num].moveTo(this, startDelay, moveTime, startPos, pos);
            fOutputEnabled = true;
            fOutputExpireMillis = millis() + startDelay + moveTime + 500;
            fLastTime = 0;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////

    virtual void _moveServosTo(uint32_t servoGroupMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, uint16_t pos) override
    {
        for (uint16_t i = 0; i < numServos; i++)        
        {
            uint32_t moveTime = (moveTimeMin != moveTimeMax) ? random(moveTimeMin, moveTimeMax) : moveTimeMax;
            if ((fServos[i].group & servoGroupMask) != 0)
            {
                moveTo(i, startDelay, moveTime, fServos[i].currentPos(), pos);
            }
        }
    }

    virtual void _moveServosBy(uint32_t servoGroupMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, int16_t pos) override
    {
        for (uint16_t i = 0; i < numServos; i++)        
        {
            uint32_t moveTime = (moveTimeMin != moveTimeMax) ? random(moveTimeMin, moveTimeMax) : moveTimeMax;
            if ((fServos[i].group & servoGroupMask) != 0)
            {
                int16_t curpos = fServos[i].currentPos();
                moveTo(i, startDelay, moveTime, curpos, curpos + pos);
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////

    virtual void _moveServoSetTo(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, uint16_t onPos, uint16_t offPos) override
    {
        byte bitShift = 31;
        for (uint16_t i = 0; i < numServos; i++)
        {
            if ((fServos[i].group & servoGroupMask) == 0)
                continue;
            uint32_t moveTime = (moveTimeMin != moveTimeMax) ? random(moveTimeMin, moveTimeMax) : moveTimeMax;
            bool on = ((servoSetMask & (1L<<bitShift)) != 0);
            moveTo(i, startDelay, moveTime, fServos[i].currentPos(), (on) ? onPos : offPos);
            if (bitShift-- == 0)
                break;
        }
    }

    virtual void _moveServoSetBy(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, int16_t onPos, int16_t offPos) override
    {
        byte bitShift = 31;
        for (uint16_t i = 0; i < numServos; i++)        
        {
            if ((fServos[i].group & servoGroupMask) == 0)
                continue;
            uint32_t moveTime = (moveTimeMin != moveTimeMax) ? random(moveTimeMin, moveTimeMax) : moveTimeMax;
            bool on = ((servoSetMask & (1L<<bitShift)) != 0);
            int16_t curpos = fServos[i].currentPos();
            moveTo(i, startDelay, moveTime, curpos, curpos + ((on) ? onPos : offPos));
            if (bitShift-- == 0)
                break;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////

    uint32_t fLastTime;
    bool fOutputEnabled;
    uint32_t fOutputExpireMillis;
    ServoState fServos[numServos];
};

#if defined(USE_TIMER1)
/* {Secret} */
ISR (TIMER1_COMPA_vect)
{
    ServoDispatchISR::handle_interrupts(ServoDispatchISR::kTimer16_1, &TCNT1, &OCR1A);
}
#endif

#if defined(USE_TIMER3)
/* {Secret} */
ISR (TIMER3_COMPA_vect)
{
    ServoDispatchISR::handle_interrupts(ServoDispatchISR::kTimer16_3, &TCNT3, &OCR3A);
}
#endif

#if defined(USE_TIMER4)
/* {Secret} */
ISR (TIMER4_COMPA_vect)
{
    ServoDispatchISR::handle_interrupts(ServoDispatchISR::kTimer16_4, &TCNT4, &OCR4A);
}
#endif

#if defined(USE_TIMER5)
/* {Secret} */
ISR (TIMER5_COMPA_vect)
{
    ServoDispatchISR::handle_interrupts(ServoDispatchISR::kTimer16_5, &TCNT5, &OCR5A);
}
#endif

#undef USE_TIMER5
#undef USE_TIMER1
#undef USE_TIMER3
#undef USE_TIMER4

#endif
