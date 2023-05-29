#ifndef ServoDispatchPrivate_h
#define ServoDispatchPrivate_h

#include "ServoDispatch.h"
#include "ServoDispatchPrivate.h"

/************ static functions common to all instances ***********************/

#ifndef ARDUINO_ARCH_ESP32
/// \private
struct ServoDispatchISR
{
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
        volatile uint8_t isActive:1;
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
        auto priv = privates();
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
                case kNumTimers16:
                    /* Dummy */
                    break;
            }
        }
        auto priv = privates();
        priv->pwm[channel].isActive = true;
    }

    static boolean isTimerActive(Timer16Order timer)
    {
        auto priv = privates();
        // returns true if any servo is active on this timer
        for (uint8_t channel = 0; channel < SERVOS_PER_TIMER; channel++)
        {
            if (priv->pwm[SERVO_INDEX(timer,channel)].isActive == true)
                return true;
        }
        return false;
    }
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

#else /* ARDUINO_ARCH_ESP32 */

#include "esp32-hal-ledc.h"

/// \private
class ServoDispatchESP32
{
private:
#if CONFIG_IDF_TARGET_ESP32
    static constexpr unsigned kNumPWM = 16;
#elif CONFIG_IDF_TARGET_ESP32S2
    static constexpr unsigned kNumPWM = 8;
#elif CONFIG_IDF_TARGET_ESP32S3
    static constexpr unsigned kNumPWM = 8;
#elif CONFIG_IDF_TARGET_ESP32C3
    static constexpr unsigned kNumPWM = 6;
#else
    #error Unknown number of channels
#endif

    /// \private
    class Private
    {
    public:
        int PWMCount;
        int timerCount[4];
        ServoDispatchESP32* ChannelUsed[kNumPWM];
        long timerFreqSet[4] = { -1, -1, -1, -1 };
        bool explicateAllocationMode;
    };

    static Private* privates()
    {
        static Private priv;
        return &priv;
    }

public:
    ServoDispatchESP32()
    {
    }

    virtual ~ServoDispatchESP32()
    {
        if (attached())
            ledcDetachPin(fPin);
        deallocate();
    }

    void detachPin(int pin)
    {
        ledcDetachPin(pin);
        deallocate();
    }
    
    void attachPin(uint8_t pin, double freq, uint8_t resolution_bits = 10)
    {
        if (validPWM(pin)) {
            double pfreq = setup(freq, resolution_bits);
            if (pfreq == 0) {
                DEBUG_PRINT("PWM: FAILED TO ATTACH PIN: "); DEBUG_PRINTLN(pin);
            }
        }
        attachPin(pin);
    }

    inline bool attached()
    {
        return fAttached;
    }

    void write(uint32_t duty)
    {
        fDuty = duty;
        ledcWrite(getChannel(), duty);
    }

    void writeScaled(float duty)
    {
        write(mapf(duty, 0.0, 1.0, 0, (float) ((1 << fResolutionBits) - 1)));
    }

    void adjustFrequency(double freq, float dutyScaled = -1)
    {
        if (dutyScaled < 0)
            dutyScaled = getDutyScaled();
        writeScaled(dutyScaled);
        auto priv = privates();
        auto ChannelUsed = priv->ChannelUsed;
        for (int i = 0; i < priv->timerCount[getTimer()]; i++)
        {
            int pwm = timerAndIndexToChannel(getTimer(), i);
            if (ChannelUsed[pwm] != nullptr)
            {
                if (ChannelUsed[pwm]->fFreq != freq)
                {
                    ChannelUsed[pwm]->adjustFrequencyLocal(freq, ChannelUsed[pwm]->getDutyScaled());
                }
            }
        }
    }

    uint32_t read()
    {
        return ledcRead(getChannel());
    }

    double readFreq()
    {
        return fFreq;
    }

    float getDutyScaled()
    {
        return mapf((float)fDuty, 0, (float) ((1 << fResolutionBits) - 1), 0.0, 1.0);
    }

    static int timerAndIndexToChannel(int timerNum, int index)
    {
        int idx = 0;
        for (int j = 0; j < kNumPWM; j++)
        {
            if (((j / 2) % 4) == timerNum)
            {
                if (idx == index)
                    return j;
                idx++;
            }
        }
        return -1;
    }

    static void configureTimer(int timerNumber)
    {
        if (timerNumber >= 0 && timerNumber < 4)
        {
            auto priv = privates();
            if (priv->explicateAllocationMode == false)
            {
                priv->explicateAllocationMode = true;
                for (int i = 0; i < 4; i++)
                    priv->timerCount[i] = 4;
            }
            priv->timerCount[timerNumber] = 0;
        }
    }

    inline int getTimer()
    {
        return fTimerNum;
    }

    inline int getChannel()
    {
        return fPWMChannel;
    }

    inline int getPin()
    {
        return fPin;
    }

    static bool validPWM(int pin)
    {
    #ifdef CONFIG_IDF_TARGET_ESP32
        // Datasheet https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf,
        // Pinout    https://docs.espressif.com/projects/esp-idf/en/latest/esp32/_images/esp32-devkitC-v4-pinout.jpg
        return (pin == 2 || pin == 4 || pin == 5) ||
               (pin >= 12 && pin <= 19) || (pin >= 21 && pin <= 23) ||
               (pin >= 25 && pin <= 27) || (pin == 32 || pin == 33);
    #elif CONFIG_IDF_TARGET_ESP32S2
        // Datasheet https://www.espressif.com/sites/default/files/documentation/esp32-s2_datasheet_en.pdf,
        // Pinout    https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/_images/esp32-s2_saola1-pinout.jpg
        return (pin >= 1 && pin <= 21) || (pin >= 33 && pin <= 44);
    #elif CONFIG_IDF_TARGET_ESP32S3
        // Datasheet https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/gpio.html
        // Pinout    https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/_images/ESP32-S3_DevKitC-1_pinlayout_v1.1.jpg
        if (pin >= 0 && pin <= 48)
        {
            // Strapping pin: GPIO0, GPIO3, GPIO45 and GPIO46 are strapping pins
            // if (pin == 0 || pin == 3 || pin == 45 || pin == 46)
            //     return false;
            // SPI0/1: GPIO26-32 are usually used for SPI flash and PSRAM and not recommended for other uses.
            if (pin >= 26 && pin <= 32)
                return false;
            // When using Octal Flash or Octal PSRAM or both, GPIO33~37 are connected to SPIIO4 ~ SPIIO7 and SPIDQS.
            if (pin >= 33 && pin <= 37)
                return false;
            // USB-JTAG: GPIO 19 and 20 are used by USB-JTAG by default.
            return true;
        }
        return false;
    #else
        #error Unsupported ESP32 platform
        return false;
    #endif
    }

    static int channelsRemaining()
    {
        return kNumPWM - privates()->PWMCount;
    }

private:
    int fPin = -1;
    double fFreq = -1;
    int fTimerNum = -1;
    uint32_t fDuty = 0;
    int fPWMChannel = -1;
    bool fAttached = false;
    uint8_t fResolutionBits = 8;

    static inline float mapf(float x, float in_min, float in_max, float out_min, float out_max)
    {
        return (x > in_max) ? out_max : (x < in_min) ? out_min :
                (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    static double _ledcSetupTimerFreq(uint8_t chan, double freq, uint8_t bit_num)
    {
        return ledcSetup(chan, freq, bit_num);
    }

    void attach(int pin)
    {
        fPin = pin;
        fAttached = true;
    }

    void adjustFrequencyLocal(double freq, float dutyScaled)
    {
        privates()->timerFreqSet[getTimer()] = (long) freq;
        fFreq = freq;
        if (attached())
        {
            ledcDetachPin(fPin);
            // Remove the PWM during frequency adjust
            _ledcSetupTimerFreq(getChannel(), freq, fResolutionBits);
            writeScaled(dutyScaled);
            ledcAttachPin(fPin, getChannel()); // re-attach the pin after frequency adjust
        }
        else
        {
            _ledcSetupTimerFreq(getChannel(), freq, fResolutionBits);
            writeScaled(dutyScaled);
        }
    }

    double setup(double freq, uint8_t resolution_bits = 10)
    {
        auto priv = privates();
        auto timerCount = priv->timerCount;
        auto timerFreqSet = priv->timerFreqSet;
        auto ChannelUsed = priv->ChannelUsed;

        long freqlocal = (long)freq;
        if (fPWMChannel < 0)
        {
            bool found = false;
            for (int i = 0; i < 4; i++)
            {
                bool freqAllocated = (timerFreqSet[i] == freqlocal || timerFreqSet[i] == -1);
                if (freqAllocated && timerCount[i] < 4)
                {
                    if (timerFreqSet[i] == -1)
                        timerFreqSet[i] = freqlocal;

                    fTimerNum = i;
                    for (int index = 0; index < 4; index++)
                    {
                        int myTimerNumber = timerAndIndexToChannel(fTimerNum, index);
                        if (myTimerNumber >= 0 && ChannelUsed[myTimerNumber] == nullptr)
                        {
                            fPWMChannel = myTimerNumber;
                            ChannelUsed[fPWMChannel] = this;
                            timerCount[fTimerNum]++;
                            priv->PWMCount++;
                            fFreq = freq;
                            found = true;
                            break;
                        }
                    }
                }
            }
            if (!found)
                return 0;
        }
        for (int i = 0; i < priv->timerCount[getTimer()]; i++)
        {
            int pwm = timerAndIndexToChannel(getTimer(), i);

            if (pwm == fPWMChannel || ChannelUsed[pwm] == nullptr ||
                ChannelUsed[pwm]->getTimer() != getTimer())
            {
                continue;
            }
            double diff = abs(ChannelUsed[pwm]->fFreq - freq);
            if (abs(diff) > 0.1)
            {
                DEBUG_PRINTLN("WARNING: PWM channel conflict");
                ChannelUsed[pwm]->fFreq = freq;
            }
        }

        fResolutionBits = resolution_bits;
        if (attached())
        {
            ledcDetachPin(fPin);
            double val = ledcSetup(getChannel(), freq, resolution_bits);
            attachPin(fPin);
            return val;
        }
        return ledcSetup(getChannel(), freq, resolution_bits);
    }

    void attachPin(uint8_t pin)
    {
        if (!validPWM(pin))
        {
            DEBUG_PRINTLN("PWM pin not valid!");
            return;
        }
        attach(pin);
        ledcAttachPin(pin, getChannel());
    }

    void deallocate()
    {
        if (fPWMChannel < 0)
           return;
        auto priv = privates();
        if (--priv->timerCount[getTimer()] == 0)
        {
            priv->timerFreqSet[getTimer()] = -1; // last pwn closed out
        }
        fTimerNum = -1;
        fAttached = false;
        priv->ChannelUsed[fPWMChannel] = nullptr;
        fPWMChannel = -1;
        priv->PWMCount--;
    }
};

#endif
#endif
