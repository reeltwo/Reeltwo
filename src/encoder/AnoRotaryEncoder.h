#ifndef AnoRotaryEncoder_h
#define AnoRotaryEncoder_h

#include "core/PinManager.h"
#include "encoder/RotaryEncoder.h"

class AnoRotaryEncoder: public RotaryEncoder
{
public:
	AnoRotaryEncoder(
			PinManager& pinManager,
			byte encoderA,
			byte encoderB,
			byte buttonUp,
			byte buttonLeft,
			byte buttonDown,
			byte buttonRight,
			byte buttonIn,
			bool useInterrupt = true) :
		RotaryEncoder(encoderA, encoderB, LatchMode::kTwo03, useInterrupt),
		fPinManager(pinManager)
	{
		fButtonPin[0] = buttonUp;
		fButtonPin[1] = buttonLeft;
		fButtonPin[2] = buttonDown;
		fButtonPin[3] = buttonRight;
		fButtonPin[4] = buttonIn;
		memset(&fButtonState, '\0', sizeof(fButtonState));
		memset(&fButtonOldState, '\0', sizeof(fButtonOldState));
		for (unsigned i = 0; i < sizeof(fButtonPin); i++)
	    	fPinManager.pinMode(fButtonPin[i], INPUT_PULLUP);
	}

	void setButtonNotify(byte pin, void (*notify)(bool))
	{
		for (unsigned i = 0; i < sizeof(fButtonPin); i++)
		{
			if (fButtonPin[i] == pin)
			{
				fButtonNotify[i] = notify;
			}
		}
	}

	bool hasButtonStateChanged() const
	{
		return fButtonStateChanged;
	}

	uint8_t getButtonPressedMask() const
	{
		uint8_t mask = 0;
		for (unsigned i = 0; i < sizeof(fButtonPin); i++)
		{
			if (fButtonState[i])
				mask |= (1<<i);
		}
		return mask;
	}

	bool isButtonPressed(byte pin) const
	{
		for (unsigned i = 0; i < sizeof(fButtonPin); i++)
		{
			if (fButtonPin[i] == pin)
				return fButtonState[i];
		}
		return false;
	}

	bool isButtonReleased(byte pin) const
	{
		for (unsigned i = 0; i < sizeof(fButtonPin); i++)
		{
			if (fButtonPin[i] == pin)
			{
				return fButtonOldState[i] && !fButtonState[i];
			}
		}
		return false;
	}

    virtual void animate() override
    {
    	fButtonStateChanged = false;
    	RotaryEncoder::animate();
		for (unsigned i = 0; i < sizeof(fButtonPin); i++)
		{
			bool newState = !fPinManager.digitalRead(fButtonPin[i]);
			fButtonOldState[i] = fButtonState[i];
			if (newState != fButtonState[i])
			{
				fButtonState[i] = newState;
				fButtonStateChanged = true;
				if (fButtonNotify[i] != nullptr)
				{
					fButtonNotify[i](newState);
				}
			}
		}
    }

private:
	typedef void (*ButtonNotifyProc)(bool pressed);
	byte fButtonPin[5];
	bool fButtonState[sizeof(fButtonPin)];
	bool fButtonOldState[sizeof(fButtonPin)];
    ButtonNotifyProc fButtonNotify[sizeof(fButtonPin)];
    bool fButtonStateChanged = false;
    PinManager &fPinManager;
};

#endif
