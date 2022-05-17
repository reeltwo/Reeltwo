#ifndef PPMDecoder_h_
#define PPMDecoder_h_

#include "ReelTwo.h"
#include "core/SetupEvent.h"

class PPMDecoder: public SetupEvent
{
public:
    PPMDecoder(uint8_t pin, unsigned channelCount = 6) :
        fPin(pin),
        fChannelCount(channelCount),
        fChannel(new uint16_t[fChannelCount]),
        fLastChannel(new uint16_t[fChannelCount])
    {
        init();
    }

    ~PPMDecoder()
    {
        delete[] fChannel;
        delete[] fLastChannel;
    }

    void init()
    {
        fPass = 0;
        fPinState = LOW;
        fCurrent = fChannelCount;
        memset(fChannel, '\0', sizeof(fChannelCount)*sizeof(uint16_t));
        memset(fLastChannel, '\0', sizeof(fChannelCount)*sizeof(uint16_t));
    }

    virtual void setup() override
    {
        pinMode(fPin, INPUT);
    }

    bool decode()
    {
        uint32_t pulse = readPulse(fPin);
        if (!pulse)
            return false;
        if (fCurrent == fChannelCount)
        {
            if (pulse > 3000)
                fCurrent = 0;
        }
        else
        {
            fChannel[fCurrent++] = pulse;
            if (fCurrent == fChannelCount)
            {
                for (unsigned i = 0; i < fChannelCount; i++)
                {
                    if (fChannel[i] > 2000 || fChannel[i] < 100)
                    {
                        fChannel[i] = fLastChannel[i]; 
                    }
                    else
                    {
                        fChannel[i] = (fLastChannel[i] + fChannel[i]) / 2;
                        fPass++;
                    }
                }
                if (fPass > 10)
                {
                    for (unsigned i = 0; i < fChannelCount; i++)
                    {
                    #ifdef USE_PPM_DEBUG
                        DEBUG_PRINT("CH");
                        DEBUG_PRINT(i+1);
                        DEBUG_PRINT.print(": ");
                        DEBUG_PRINT(fChannel[i]);
                        DEBUG_PRINT(" ");
                    #endif
                        fLastChannel[i] = fChannel[i];
                    }
                #ifdef USE_PPM_DEBUG
                    DEBUG_PRINT('\r');
                #endif
                    fPass = 0;
                    return true;
                }
            }
        }
        return false;
    }

    uint16_t channel(unsigned ch, unsigned minvalue, unsigned maxvalue, unsigned neutralvalue)
    {
        unsigned pulse = (ch < fChannelCount) ? fChannel[ch] : 0;
        if (pulse != 0)
            return map(max(min(pulse, 1600u), 600u), 600u, 1600u, minvalue, maxvalue);
        return neutralvalue;
    }

private:
    uint8_t fPin;
    int fPinState;
    unsigned fPass;
    unsigned fCurrent;
    unsigned fChannelCount;
    uint32_t fRisingTime;
    uint16_t* fChannel;
    uint16_t* fLastChannel;

    uint32_t readPulse(uint8_t pin)
    {
        uint8_t state = digitalRead(pin);
        uint32_t pulseLength = 0;

        // On rising edge: record current time.
        if (fPinState == LOW && state == HIGH)
        {
            fRisingTime = micros();
        }

        // On falling edge: report pulse length.
        if (fPinState == HIGH && state == LOW)
        {
            unsigned long fallingTime = micros();
            pulseLength = fallingTime - fRisingTime;
        }

        fPinState = state;
        return pulseLength;
    }
};

#endif
