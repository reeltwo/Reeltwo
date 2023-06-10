#if !defined(ESP32)
#error Only supports ESP32
#endif

#ifndef PWMDecoder_h_
#define PWMDecoder_h_

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include "driver/rmt.h"
#include "esp32-hal.h"

#ifndef RMT_RX_MODE
#define RMT_RX_MODE false
#endif

class PWMDecoder: public AnimatedEvent
{
public:
	PWMDecoder(void (*changeNotify)(int pin, uint16_t pwm), int pin1, int pin2 = -1, int pin3 = -1, int pin4 = -1) :
		fChangeNotify(changeNotify)
	{
		fChannel[0].fGPIO = uint8_t(pin1);
		fChannel[1].fGPIO = uint8_t(pin2);
		fChannel[2].fGPIO = uint8_t(pin3);
		fChannel[3].fGPIO = uint8_t(pin4);
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
		return fChannel[i].fRawPulse;
	}

	inline uint16_t getValue(unsigned i = 0) const
	{
		return fChannel[i].fRawPulse;
	}

	inline bool hasChanged(unsigned i = 0)
	{
		return abs(int32_t(fChannel[i].fPulse) - int32_t(fChannel[i].fRawPulse)) > 20;
	}

	inline unsigned numChannels() const
	{
		return fNumChannels;
	}

	unsigned long getAge(unsigned i = 0)
	{
		return millis() - fChannel[i].fLastActive;
	}

	bool isActive(unsigned i = 0)
	{
		return millis() > 500 && (fChannel[i].fPulse != 0 && getAge(i) < 500);
	}

	bool becameActive(unsigned i = 0)
	{
		return (fChannel[i].fAliveStateChange && fChannel[i].fAlive);
	}

	bool becameInactive(unsigned i = 0)
	{
		return (fChannel[i].fAliveStateChange && !fChannel[i].fAlive);
	}

	virtual void animate() override
	{
		for (unsigned i = 0; i < fNumChannels; i++)
		{
		#ifndef RMT_RX_MODE
			checkActive(i);
		#endif
			fChannel[i].fAliveStateChange = false;
			if (isActive(i))
			{
				if (!fChannel[i].fAlive)
				{
					// DEBUG_PRINT("PWM Start Pin: "); DEBUG_PRINTLN(fChannel[i].fGPIO);
					fChannel[i].fAlive = true;
					fChannel[i].fAliveStateChange = true;
				}
				// if (fChangeNotify != nullptr)
				// 	fChangeNotify(fChannel[i].fGPIO, fChannel[i].fPulse);
			}
			else if (fChannel[i].fAlive)
			{
				// DEBUG_PRINT("PWM End Pin: "); DEBUG_PRINTLN(fChannel[i].fGPIO);
				fChannel[i].fAlive = false;
				fChannel[i].fAliveStateChange = true;
			}
			if (hasChanged(i))
			{
				fChannel[i].fPulse = fChannel[i].fRawPulse;
				if (fChangeNotify != nullptr)
					fChangeNotify(fChannel[i].fGPIO, fChannel[i].fPulse);
			}
		}
	}

#ifdef RMT_RX_MODE
	bool fStarted = false;
	void begin()
	{
		if (fStarted)
			return;
		for (unsigned i = 0; i < fNumChannels; i++)
		{
		    if ((fChannel[i].fReceiver = rmtInit(fChannel[i].fGPIO, RMT_RX_MODE, RMT_MEM_64)) != NULL)
		    {
				rmtSetTick(fChannel[i].fReceiver, 1000);
				rmtSetFilter(fChannel[i].fReceiver, true, 100);
			    rmtSetRxThreshold(fChannel[i].fReceiver, 5000);
			    rmtRead(fChannel[i].fReceiver, receive_data, &fChannel[i]);
		    }
		    else
		    {
		    	DEBUG_PRINT("FAILED TO INIT RMT on ");
		    	DEBUG_PRINTLN(fChannel[i].fGPIO);
		    }
		}
		fStarted = true;
	}

	void end()
	{
		if (!fStarted)
			return;
		for (unsigned i = 0; i < fNumChannels; i++)
		{
			if (fChannel[i].fReceiver != nullptr)
			{
				rmtEnd(fChannel[i].fReceiver);
				rmtDeinit(fChannel[i].fReceiver);
				fChannel[i].fReceiver = nullptr;
			}
		}
		fStarted = false;
	}
#else
	void begin();
	void end();
#endif

private:
	uint8_t fNumChannels = 1;
	static constexpr unsigned fMaxChannels = 8;
	struct Channel
	{
		uint8_t fGPIO;
		uint16_t fPulse;
		volatile uint16_t fRawPulse;
		bool fAlive;
		bool fAliveStateChange;
		uint32_t fLastActive;
		rmt_obj_t* fReceiver;
	} fChannel[fMaxChannels] = {};
	void (*fChangeNotify)(int pin, uint16_t pwm) = nullptr;

#ifdef RMT_RX_MODE
	static void receive_data(uint32_t* data, size_t len, void* arg)
	{
		Channel* channel = (Channel*)arg;
		rmt_data_t* it = (rmt_data_t*)data;
		if (len >= 1)
		{
			channel->fRawPulse = it[0].duration0;
			channel->fLastActive = millis();
		}
	}
#else
	rmt_isr_handle_t fISRHandle = nullptr;
	static void IRAM_ATTR rmt_isr_handler(void* arg);
	void checkActive(unsigned i);
#endif
};

#define ServoDecoder PWMDecoder

#endif
