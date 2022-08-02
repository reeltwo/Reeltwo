#if !defined(ESP32)
#error Only supports ESP32
#endif

#ifndef PWMDecoder_h_
#define PWMDecoder_h_

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include "driver/rmt.h"

#define RECEIVER_CH_MIN 1000
#define RECEIVER_CH_CENTER 1500
#define RECEIVER_CH_MAX 2000

class PWMDecoder: public SetupEvent, AnimatedEvent
{
public:
	PWMDecoder(void (*changeNotify)(int pin, uint16_t pwm), int pin1, int pin2 = -1, int pin3 = -1, int pin4 = -1) :
		fChangeNotify(changeNotify)
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

	PWMDecoder(int pin1, int pin2 = -1, int pin3 = -1, int pin4 = -1) :
		PWMDecoder(nullptr, pin1, pin2, pin3, pin4)
	{
	}

	inline uint16_t channel(unsigned i = 0) const
	{
		return fRawPulse[i];
	}

	inline uint16_t getValue(unsigned i = 0) const
	{
		return fRawPulse[i];
	}

	inline bool hasChanged(unsigned i = 0)
	{
		return abs(int32_t(fPulse[i]) - int32_t(fRawPulse[i])) > 20;
	}

	inline unsigned numChannels() const
	{
		return fNumChannels;
	}

	unsigned long getAge(unsigned i = 0)
	{
		return millis() - fLastActive[i];
	}

	bool isActive(unsigned i = 0)
	{
		return millis() > 500 && (fPulse[i] != 0 && getAge(i) < 500);
	}

	bool becameActive(unsigned i = 0)
	{
		return (fAliveStateChange[i] && fAlive[i]);
	}

	bool becameInactive(unsigned i = 0)
	{
		return (fAliveStateChange[i] && !fAlive[i]);
	}

	virtual void setup() override
	{
		begin();
	}

	virtual void animate() override
	{
		for (unsigned i = 0; i < fNumChannels; i++)
		{
			checkActive(i);
			if (hasChanged(i))
			{
				fPulse[i] = fRawPulse[i];
				if (fChangeNotify != nullptr)
					fChangeNotify(fGPIO[i], fPulse[i]);
			}
			fAliveStateChange[i] = false;
			if (isActive(i))
			{
				if (!fAlive[i])
				{
					DEBUG_PRINT("PWM Start Pin`: "); DEBUG_PRINTLN(fGPIO[i]);
					fAlive[i] = true;
					fAliveStateChange[i] = true;
				}
			}
			else if (fAlive[i])
			{
				DEBUG_PRINT("PWM End Pin: "); DEBUG_PRINTLN(fGPIO[i]);
				fAlive[i] = false;
				fAliveStateChange[i] = true;
			}
		}
	}

	void begin();
	void end();

private:
	uint8_t fNumChannels = 1;
	static constexpr unsigned fMaxChannels = 8;
	uint8_t fChannels[fMaxChannels] = {};
	uint8_t fGPIO[fMaxChannels] = {};
	uint16_t fPulse[fMaxChannels] = {};
	bool fAlive[fMaxChannels] = {};
	bool fAliveStateChange[fMaxChannels] = {};
	uint32_t fLastActive[fMaxChannels] = {};
	rmt_isr_handle_t fISRHandle = nullptr;
	void (*fChangeNotify)(int pin, uint16_t pwm) = nullptr;
	volatile uint16_t fRawPulse[fMaxChannels] = {};
	static PWMDecoder* sActive;

	static void IRAM_ATTR rmt_isr_handler(void* arg);
	void checkActive(unsigned i = 0);
};

#endif
