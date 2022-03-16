#ifndef ResistorLadderButtons_h
#define ResistorLadderButtons_h

#include "ReelTwo.h"

/**
  * \ingroup Core
  *
  * \class ResistorLadderButtons
  *
  * \brief Push buttons connected using a resistor ladder
  *
  * Reads a group of buttons through a single pin attached via a resistor ladder.
  *
  * \code
  * typedef ResistorLadderButtons<BUTTON_PIN, 0, 1016, 2058, 3075> BoardButtons;
  * BoardButtons    boardButtons;
  * \endcode
  */
template <byte kButtonPin, unsigned int... buttonVal>
class ResistorLadderButtons : public AnimatedEvent, public SetupEvent
{
public:
    virtual void setup() override
    {
        pinMode(kButtonPin, INPUT);
        fPrevButtonValue = analogRead(kButtonPin);
    }

    virtual void buttonPressed(unsigned buttonNum, unsigned buttonTime)
    {
        DEBUG_PRINT("BUTTON: "); DEBUG_PRINT(buttonNum); DEBUG_PRINT(" ms: "); DEBUG_PRINTLN(buttonTime);
    }

    virtual void animate() override
    {
        uint32_t timeNow = millis();
        unsigned buttonValue = analogRead(kButtonPin);
        unsigned buttonReleased = kNumberOfButtons+1;
        if (!inRange(buttonValue, fPrevButtonValue - kButtonVariance, fPrevButtonValue + kButtonVariance))
        {
            for (byte i = 0; i < kNumberOfButtons; i++)
            {
                if (inRange(buttonValue, getButtonValue(i) - kButtonVariance, getButtonValue(i) + kButtonVariance))
                {
                    if (!fButtonState[i])
                    {
                        fButtonState[i] = true;
                        fButtonTime[i] = timeNow;
                    }
                }
                else
                {
                    fButtonState[i] = false;
                    if (fPrevButtonState[i])
                    {
                        //okay, so a button was pressed and we're pretty sure which one it was
                        //but because of our ladder setup we could be seeing a false reading for a teeny moment
                        //so disregard any button presses that are shorter than 30ms  
                        if (timeNow - fButtonTime[i] > 30)
                        {
                            buttonReleased = i;
                        }
                    }
                }
                fPrevButtonState[i] = fButtonState[i];
            }
        }
        fPrevButtonValue = buttonValue;

        if (buttonReleased < kNumberOfButtons+1)
        {
            buttonPressed(buttonReleased, fButtonTime[buttonReleased]);
        }
    }

protected:
    static constexpr unsigned kButtonVariance = 200;
    static constexpr unsigned kNumberOfButtons = sizeof...(buttonVal);

    unsigned getButtonValue(unsigned i)
    {
        static constexpr unsigned int kButtonValues[] = { buttonVal... };
        return (i < kNumberOfButtons) ? kButtonValues[i] : 0;
    }

    unsigned fPrevButtonValue;
    bool fButtonState[kNumberOfButtons]; //state of each button
    bool fPrevButtonState[kNumberOfButtons];
    uint32_t fButtonTime[kNumberOfButtons+1]; //when did state of each button last change

    static bool inRange(int val, int minimum, int maximum)
    {
        return ((minimum <= val) && (val <= maximum));
    }
};

#endif