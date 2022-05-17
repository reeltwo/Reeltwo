#if !defined(ESP32)
#error Only supports ESP32
#endif

#ifndef PWMDecoder_h_
#define PWMDecoder_h_

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"

#define RECEIVER_CH_MIN 8000
#define RECEIVER_CH_CENTER 12000
#define RECEIVER_CH_MAX 16000
// how many ticks +/- RECEIVER_CENTER is part of the deadzone
#define RECEIVER_CH_DEADZONE 10

class PWMDecoder: public SetupEvent, AnimatedEvent
{
public:
	PWMDecoder(int pin1, int pin2 = -1, int pin3 = -1, int pin4 = -1)
	{
		for (unsigned i = 0; i < fMaxChannels; i++)
			fChannels[i] = i+1;
		fGPIO[0] = uint8_t(pin1);
		fGPIO[1] = uint8_t(pin2);
		fGPIO[2] = uint8_t(pin3);
		fGPIO[3] = uint8_t(pin4);
		fNumChannels = 4;
		if (pin4 == -1)
			fNumChannels = 3;
		if (pin3 == -1)
			fNumChannels = 2;
		if (pin2 == -1)
			fNumChannels = 1;
	}

	inline uint16_t channel(unsigned i) const
	{
		return fRawPulse[i];
	}

    inline bool hasChanged(unsigned i)
    {
        return abs(int32_t(fPulse[i]) - int32_t(fRawPulse[i])) > 20;
    }

	inline unsigned numChannels() const
	{
		return fNumChannels;
	}

    virtual void setup() override
    {
        begin();
    }

    virtual void animate() override
    {
		for (unsigned i = 0; i < fNumChannels; i++)
		{
        	if (hasChanged(i))
        	{
            	fPulse[i] = fRawPulse[i];
        	}
        }
    }

private:
	uint8_t fNumChannels = 1;
	static constexpr unsigned fMaxChannels = 8;
	uint8_t fChannels[fMaxChannels] = {};
	uint8_t fGPIO[fMaxChannels] = {};
	uint16_t fPulse[fMaxChannels] = {0};
	volatile uint16_t fRawPulse[fMaxChannels] = {0};
	static PWMDecoder* sActive;

	static void IRAM_ATTR rmt_isr_handler(void* arg);

	void begin(void);
};

#endif
