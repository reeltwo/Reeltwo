#ifndef ServoDecoder_h
#define ServoDecoder_h

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"

class ServoDecoder : public SetupEvent, AnimatedEvent
{
public:
    ServoDecoder(byte pin)
    {
		Private* _priv = priv();
		fISR = _priv->fISRCount++;

		_priv->fISRPin[fISR] = pin;
		pinMode(_priv->fISRPin[fISR], INPUT);
    }

    virtual void setup() override
    {
    	begin(true);
    }

    virtual void animate() override
    {
    	uint16_t val = getValue();
    	if (val != 0)
    		DEBUG_PRINTLN(val);
    }

    bool begin(bool measure_pulse_high)
    {
		Private* _priv = priv();
		uint8_t pin = _priv->fISRPin[fISR];
	    _priv->fISRLastState[fISR] = digitalRead(pin);
	    _priv->fISRTriggerState[fISR] = measure_pulse_high;
	    uint8_t inum =  digitalPinToInterrupt(pin);
	    switch (fISR) {
	    	#define ATTACH_INTTERUPT(num) \
	        	case num: \
	            	attachInterrupt(inum, ISR_##num, CHANGE); \
	            	break;
	        ATTACH_INTTERUPT(0)
	        ATTACH_INTTERUPT(1)
	        ATTACH_INTTERUPT(2)
	        ATTACH_INTTERUPT(3)
	        ATTACH_INTTERUPT(4)
	        ATTACH_INTTERUPT(5)
	        ATTACH_INTTERUPT(6)
	        ATTACH_INTTERUPT(7)
	        ATTACH_INTTERUPT(8)
	        ATTACH_INTTERUPT(9)
	        ATTACH_INTTERUPT(10)
	        ATTACH_INTTERUPT(11)
	        ATTACH_INTTERUPT(12)
	        ATTACH_INTTERUPT(13)
	        ATTACH_INTTERUPT(14)
	        ATTACH_INTTERUPT(15)
	        ATTACH_INTTERUPT(16)
	        ATTACH_INTTERUPT(17)
	        ATTACH_INTTERUPT(18)
	        ATTACH_INTTERUPT(19)

	        #undef ATTACH_INTTERUPT
	        default:
	            return false;
	    }
	    return true;
   	}
    
    unsigned int getValue()
    {
		Private* _priv = priv();
	    return _priv->fISRValue[fISR];
    }
    
    unsigned long getAge()
    {
		Private* _priv = priv();
	    return (micros() - _priv->fISRAge[fISR]);
    }
    
    void end()
    {
		Private* _priv = priv();
	    detachInterrupt(digitalPinToInterrupt(_priv->fISRPin[fISR]));
	}

private:
	static constexpr byte MAX_ISR_COUNT = 20;
	struct Private
	{
		byte fISRCount;
		byte fISRPin[MAX_ISR_COUNT];
		unsigned int fISRValue[MAX_ISR_COUNT];
		bool fISRLastState[MAX_ISR_COUNT];
		bool fISRTriggerState[MAX_ISR_COUNT];
		unsigned long fISRTiemr[MAX_ISR_COUNT];
		unsigned long fISRAge[MAX_ISR_COUNT];
	};
    byte fISR;

	static Private* priv()
	{
		static Private _priv;
		return &_priv;
	}

	static void ISR_generic(byte isr)
	{
		Private* _priv = priv();
	    unsigned long now = micros();
	    bool state_now = digitalRead(_priv->fISRPin[isr]);
	    if (state_now != _priv->fISRLastState[isr])
	    {
	        if (state_now == _priv->fISRTriggerState[isr])
	        {
	            _priv->fISRTiemr[isr] = now;
	        }
	        else
	        {
	            _priv->fISRValue[isr] = (unsigned int)(now - _priv->fISRTiemr[isr]);
	            _priv->fISRAge[isr] = now;
	        }
	        _priv->fISRLastState[isr] = state_now;
	    }
	}

	#define DEFINE_ISR_HANDLER(n) \
		static void ISR_##n() \
		{ \
		    ISR_generic(n); \
		}

	DEFINE_ISR_HANDLER(0)
	DEFINE_ISR_HANDLER(1)
	DEFINE_ISR_HANDLER(2)
	DEFINE_ISR_HANDLER(3)
	DEFINE_ISR_HANDLER(4)
	DEFINE_ISR_HANDLER(5)
	DEFINE_ISR_HANDLER(6)
	DEFINE_ISR_HANDLER(7)
	DEFINE_ISR_HANDLER(8)
	DEFINE_ISR_HANDLER(9)
	DEFINE_ISR_HANDLER(10)
	DEFINE_ISR_HANDLER(11)
	DEFINE_ISR_HANDLER(12)
	DEFINE_ISR_HANDLER(13)
	DEFINE_ISR_HANDLER(14)
	DEFINE_ISR_HANDLER(15)
	DEFINE_ISR_HANDLER(16)
	DEFINE_ISR_HANDLER(17)
	DEFINE_ISR_HANDLER(18)
	DEFINE_ISR_HANDLER(19)

	#undef DEFINE_ISR_HANDLER
};

#endif
