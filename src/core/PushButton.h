
#ifndef PushButton_h
#define PushButton_h

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"

class PushButton : public SetupEvent, AnimatedEvent
{
public:
    typedef void (*CallbackFunction)(void);
    typedef void (*ParameterizedCallbackFunction)(void *);

    /** \brief Initialize the PushButton.
      * 
      * @param pin The pin to be used for input from a momentary button.
      * @param activeLow Set to true when the input level is LOW when the button is pressed, Default is true.
      * @param pullupActive Activate the internal pullup when available. Default is true.
      */
    PushButton(const int pin, const bool activeLow = true, const bool pullupActive = true) :
        fPin(pin),
        fButtonPressed(activeLow ? LOW : HIGH),
        fPullup(pullupActive)
    {
    }

    // ----- Set runtime parameters -----

    /** \brief set # millisec after safe click is assumed.
      *
     */
    void setDebounceTicks(const int ticks)
    {
        fDebounceTicks = ticks;
    }

    /** \brief set # millisec after single click is assumed.
      *
      */
    void setClickTicks(const int ticks)
    {
        fClickTicks = ticks;
    }

    /** \brief set # millisec after press is assumed.
      *
      */
    void setPressTicks(const int ticks)
    {
        fPressTicks = ticks;
    }

    /** \brief Attach an event to be called when a single click is detected.
      *
      * @param callback This function will be called when the event has been detected.
      */
    void attachClick(CallbackFunction callback)
    {
        fClickFunc = callback;
    }

    /** \brief Attach an event to be called when a single click is detected.
      *
      * @param callback This function will be called when the event has been detected.
      */
    void attachClick(ParameterizedCallbackFunction callback, void *param)
    {
        fParamClickFunc = callback;
        fClickFuncParam = param;
    }

    /** \brief Attach an event to be called after a double click is detected.
      *
      * @param callback This function will be called when the event has been detected.
      */
    void attachDoubleClick(CallbackFunction callback)
    {
        fDoubleClickFunc = callback;
        fMaxClicks = max(fMaxClicks, 2);
    }

    /** \brief Attach an event to be called after a double click is detected.
      *
      * @param callback This function will be called when the event has been detected.
      */
    void attachDoubleClick(ParameterizedCallbackFunction callback, void *param)
    {
        fParamDoubleClickFunc = callback;
        fDoubleClickFuncParam = param;
        fMaxClicks = max(fMaxClicks, 2);
    }

    /** \brief Attach an event to be called after a multi click is detected.
      *
      * @param callback This function will be called when the event has been detected.
      */
    void attachMultiClick(CallbackFunction callback)
    {
        fMultiClickFunc = callback;
        fMaxClicks = max(fMaxClicks, 100);
    }

    /** \brief Attach an event to be called after a multi click is detected.
      *
      * @param callback This function will be called when the event has been detected.
      */
    void attachMultiClick(ParameterizedCallbackFunction callback, void *param)
    {
        fParamMultiClickFunc = callback;
        fMultiClickFuncParam = param;
        fMaxClicks = max(fMaxClicks, 100);
    }

    /** \brief Attach an event to fire when the button is pressed and held down.
      *
      * @param callback
      */
    void attachLongPressStart(CallbackFunction callback)
    {
        fLongPressStartFunc = callback;
    }

    /** \brief Attach an event to fire when the button is pressed and held down.
      *
      * @param callback
      */
    void attachLongPressStart(ParameterizedCallbackFunction callback, void *param)
    {
        fParamLongPressStartFunc = callback;
        fLongPressStartFuncParam = param;
    }

    /** \brief Attach an event to fire as soon as the button is released after a long press.
      *
      * @param callback
      */
    void attachLongPressStop(CallbackFunction callback)
    {
        fLongPressStopFunc = callback;
    }

    /** \brief Attach an event to fire as soon as the button is released after a long press.
      *
      * @param callback
      */
    void attachLongPressStop(ParameterizedCallbackFunction callback, void *param)
    {
        fParamLongPressStopFunc = callback;
        fLongPressStopFuncParam = param;
    }

    /** \brief Attach an event to fire periodically while the button is held down.
      *
      * @param callback
      */
    void attachDuringLongPress(CallbackFunction callback)
    {
        fDuringLongPressFunc = callback;
    }

    /** \brief Attach an event to fire periodically while the button is held down.
      *
      * @param callback
      */
    void attachDuringLongPress(ParameterizedCallbackFunction callback, void *param)
    {
        fParamDuringLongPressFunc = callback;
        fDuringLongPressFuncParam = param;
    }

    virtual void setup() override
    {
        if (fPullup)
        {
            // use the given pin as input and activate internal PULLUP resistor.
            pinMode(fPin, INPUT_PULLUP);
        }
        else
        {
            // use the given pin as input
            pinMode(fPin, INPUT);
        }
    }

    /** \brief Call this function every some milliseconds for checking the input
      * level at the initialized digital pin.
      */
    virtual void animate() override
    {
        unsigned long now = millis(); // current (relative) time in msecs.
        unsigned long waitTime = (now - fStartTime);

        bool activeLevel = (digitalRead(fPin) == fButtonPressed);
        switch (fState)
        {
            case kPB_INIT:
                // waiting for level to become active.
                if (activeLevel)
                {
                    newState(kPB_DOWN);
                    fStartTime = now; // remember starting time
                    fNClicks = 0;
                }
                break;

            case kPB_DOWN:
                // waiting for level to become inactive.

                if ((!activeLevel) && (waitTime < fDebounceTicks))
                {
                    // button was released to quickly so I assume some bouncing.
                    newState(fLastState);
                }
                else if (!activeLevel)
                {
                    newState(kPB_UP);
                    fStartTime = now; // remember starting time
                }
                else if ((activeLevel) && (waitTime > fPressTicks))
                {
                    if (fLongPressStartFunc)
                        fLongPressStartFunc();
                    if (fParamLongPressStartFunc)
                        fParamLongPressStartFunc(fLongPressStartFuncParam);
                    newState(kPB_PRESS);
                }
                break;

            case kPB_UP:
                // level is inactive
                if ((activeLevel) && (waitTime < fDebounceTicks))
                {
                    // button was pressed to quickly so I assume some bouncing.
                    newState(fLastState); // go back
                }
                else if (waitTime >= fDebounceTicks)
                {
                    // count as a short button down
                    fNClicks++;
                    newState(kPB_COUNT);
                } // if
                break;

            case kPB_COUNT:
                // dobounce time is over, count clicks
                if (activeLevel)
                {
                    // button is down again
                    newState(kPB_DOWN);
                    fStartTime = now; // remember starting time
                }
                else if ((waitTime > fClickTicks) || (fNClicks == fMaxClicks))
                {
                    // now we know how many clicks have been made.
                    if (fNClicks == 1)
                    {
                        // this was 1 click only.
                        if (fClickFunc)
                            fClickFunc();
                        if (fParamClickFunc)
                            fParamClickFunc(fClickFuncParam);
                    }
                    else if (fNClicks == 2)
                    {
                        // this was a 2 click sequence.
                        if (fDoubleClickFunc)
                            fDoubleClickFunc();
                        if (fParamDoubleClickFunc)
                            fParamDoubleClickFunc(fDoubleClickFuncParam);
                    }
                    else
                    {
                        // this was a multi click sequence.
                        if (fMultiClickFunc)
                            fMultiClickFunc();
                        if (fParamMultiClickFunc)
                            fParamMultiClickFunc(fMultiClickFuncParam);
                    }
                    reset();
                }
                break;

            case kPB_PRESS:
                // waiting for menu pin being release after long press.
                if (!activeLevel)
                {
                    newState(kPB_PRESSEND);
                    fStartTime = now;
                }
                else
                {
                    // still the button is pressed
                    if (fDuringLongPressFunc)
                        fDuringLongPressFunc();
                    if (fParamDuringLongPressFunc)
                        fParamDuringLongPressFunc(fDuringLongPressFuncParam);
                }
                break;

            case kPB_PRESSEND:
                // button was released.
                if ((activeLevel) && (waitTime < fDebounceTicks))
                {
                    // button was released to quickly so I assume some bouncing.
                    newState(fLastState); // go back
                }
                else if (waitTime >= fDebounceTicks)
                {
                    if (fLongPressStopFunc)
                        fLongPressStopFunc();
                    if (fParamLongPressStopFunc)
                        fParamLongPressStopFunc(fLongPressStopFuncParam);
                    reset();
                }
                break;

            default:
                // unknown state detected -> reset state machine
                newState(kPB_INIT);
                break;
        }
    }


    /** \brief Reset the button state machine.
      */
    void reset(void)
    {
        fState = kPB_INIT;
        fLastState = kPB_INIT;
        fNClicks = 0;
        fStartTime = 0;
    }

    /** \brief Returns number of clicks in any case: single or multiple clicks
     * 
     * @return number of clicks in any case: single or multiple clicks
     */
    inline int getNumberClicks(void) const
    {
        return fNClicks;
    }

    /**
    * @return true if we are currently handling button press flow
    * (This allows power sensitive applications to know when it is safe to power down the main CPU)
    */
    inline bool isIdle() const
    {
        return fState == kPB_INIT;
    }

    /**
      * @return true when a long press is detected
      */
    inline bool isLongPressed() const
    {
        return fState == kPB_PRESS;
    }

private:
    int fPin;                         // hardware pin number.
    unsigned int fDebounceTicks = 50; // number of ticks for debounce times.
    unsigned int fClickTicks = 400;   // number of msecs before a click is detected.
    unsigned int fPressTicks = 800;   // number of msecs before a long button press is detected

    int fButtonPressed;
    bool fPullup;

    // These variables will hold functions acting as event source.
    CallbackFunction fClickFunc = NULL;
    ParameterizedCallbackFunction fParamClickFunc = NULL;
    void *fClickFuncParam = NULL;

    CallbackFunction fDoubleClickFunc = NULL;
    ParameterizedCallbackFunction fParamDoubleClickFunc = NULL;
    void *fDoubleClickFuncParam = NULL;

    CallbackFunction fMultiClickFunc = NULL;
    ParameterizedCallbackFunction fParamMultiClickFunc = NULL;
    void *fMultiClickFuncParam = NULL;

    CallbackFunction fLongPressStartFunc = NULL;
    ParameterizedCallbackFunction fParamLongPressStartFunc = NULL;
    void *fLongPressStartFuncParam = NULL;

    CallbackFunction fLongPressStopFunc = NULL;
    ParameterizedCallbackFunction fParamLongPressStopFunc = NULL;
    void *fLongPressStopFuncParam;

    CallbackFunction fDuringLongPressFunc = NULL;
    ParameterizedCallbackFunction fParamDuringLongPressFunc = NULL;
    void *fDuringLongPressFuncParam = NULL;

    enum State : int
    {
        kPB_INIT = 0,
        kPB_DOWN = 1,
        kPB_UP = 2,
        kPB_COUNT = 3,
        kPB_PRESS = 6,
        kPB_PRESSEND = 7,
        kUNKNOWN = 99
    };

    void newState(State nextState)
    {
        fLastState = fState;
        fState = nextState;
    }

    State fState = kPB_INIT;
    State fLastState = kPB_INIT; // used for debouncing

    unsigned long fStartTime; // start of current input change to checking debouncing
    int fNClicks;             // count the number of clicks with this variable
    int fMaxClicks = 1;       // max number (1, 2, multi=3) of clicks of interest by registration of event functions.
};

#endif
