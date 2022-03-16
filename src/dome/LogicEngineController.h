#ifndef LOGICENGINECONTROLLER_H
#define LOGICENGINECONTROLLER_H

#include "ReelTwo.h"
#ifndef LENGINE_DELAY_PIN
 #if defined(REELTWO_TEENSY)
  #define LENGINE_DELAY_PIN A1 /* analog pin to read keyPause value */
  #define LENGINE_FADE_PIN  A2 /* analog pin to read tweenPause value */
  #define LENGINE_BRI_PIN   A3 /* analog pin to read tweenPause value */
  #define LENGINE_HUE_PIN   A6 /* analog pin to read tweenPause value */
  #define LENGINE_PAL_PIN   9  /* pin to switch palettes in ADJ mode */
  #define LENGINE_FJUMP_PIN 0  /* front jumper */
  #define LENGINE_RJUMP_PIN 1  /* rear jumper */
 #elif defined(REELTWO_ZERO)
  #define LENGINE_DELAY_PIN A0 /* analog pin to read keyPause value */
  #define LENGINE_FADE_PIN  A1 /* analog pin to read tweenPause value */
  #define LENGINE_BRI_PIN   A2 /* analog pin to read tweenPause value */
  #define LENGINE_HUE_PIN   A3 /* analog pin to read tweenPause value */
  #define LENGINE_PAL_PIN   9  /* pin to switch palettes in ADJ mode */
  #define LENGINE_FJUMP_PIN 2  /* front jumper */
  #define LENGINE_RJUMP_PIN 4  /* rear jumper */
 #elif defined(REELTWO_AVR_MEGA)
  #define LENGINE_DELAY_PIN A0 /* analog pin to read keyPause value */
  #define LENGINE_FADE_PIN  A1 /* analog pin to read tweenPause value */
  #define LENGINE_BRI_PIN   A2 /* analog pin to read Brightness value */
  #define LENGINE_HUE_PIN   A3 /* analog pin to read Color/Hue shift value */
  #define LENGINE_PAL_PIN   9  /* pin to switch palettes in ADJ mode */
  #define LENGINE_FJUMP_PIN 2  /* front jumper */
  #define LENGINE_RJUMP_PIN 4  /* rear jumper */
 #elif defined(REELTWO_AVR)
  #define LENGINE_DELAY_PIN A0 /* analog pin to read keyPause value */
  #define LENGINE_FADE_PIN  A1 /* analog pin to read tweenPause value */
  #define LENGINE_BRI_PIN   A2 /* analog pin to read Brightness value */
  #define LENGINE_HUE_PIN   A3 /* analog pin to read Color/Hue shift value */
  #define LENGINE_PAL_PIN   9  /* pin to switch palettes in ADJ mode */
  #define LENGINE_FJUMP_PIN 2  /* front jumper */
  #define LENGINE_RJUMP_PIN 4  /* rear jumper */
 #endif
#endif

#include "dome/LogicEngine.h"
#include "core/AnalogMonitor.h"
#include "core/PersistentStorage.h"

#if defined(ESP32)
#include "LogicEngineController32.h"
#else
/**
  * \ingroup Dome
  *
  * \class LogicEngineController
  *
  * \brief Settings adjust for LogicEngine logics
  *
  * \brief Adjust the settings for a set of LogicEngine logics using PCB trimpots and jumpers.
  */
template <byte kDelayPin = LENGINE_DELAY_PIN, byte kFadePin = LENGINE_FADE_PIN,
  byte kBriPin = LENGINE_BRI_PIN, byte kHuePin = LENGINE_HUE_PIN, byte kPalPin = LENGINE_PAL_PIN,
  byte kFrontJumperPin = LENGINE_FJUMP_PIN, byte kRearJumperPin = LENGINE_RJUMP_PIN>
class LogicEngineController : public AnimatedEvent
{
public:
    /**
      * \brief Constructor
      */
    LogicEngineController(
            LogicEngineRenderer& FLD,
            LogicEngineRenderer& RLD)
    {
        fLogic[0] = &FLD;
        fLogic[1] = &RLD;
        fSettings[0] = FLD.getSettings();
        fSettings[1] = RLD.getSettings();
        fFactorySettings[0] = fSettings[0];
        fFactorySettings[1] = fSettings[1];
        if (fFLDSettingsStorage.isValid())
            FLD.changeDefaultSettings(fFLDSettingsStorage.get(fSettings[0]));
        if (fRLDSettingsStorage.isValid())
            RLD.changeDefaultSettings(fRLDSettingsStorage.get(fSettings[1]));

        // ensure button pin is an input
        pinMode(kDelayPin, INPUT);
        pinMode(kFadePin, INPUT);
        pinMode(kBriPin, INPUT);
        pinMode(kHuePin, INPUT);

        //jumper is used to set front or rear adjustment mode (pull low to adjust)
        pinMode(kFrontJumperPin, INPUT_PULLUP); //use internal pullup resistors of Teensy
        pinMode(kRearJumperPin, INPUT_PULLUP);
        pinMode(kPalPin, INPUT_PULLUP); 
    }

    /**
      * \brief Constructor
      *
      * For AVR ProMini boards with restricted memory we can only support a single logic display
      * if using a controller.
      */
    LogicEngineController(
            LogicEngineRenderer& LD)
    {
        fLogic[0] = &LD;
        fLogic[1] = NULL;
        fSettings[0] = LD.getSettings();
        fFactorySettings[0] = fSettings[0];
        if (fFLDSettingsStorage.isValid())
            LD.changeDefaultSettings(fFLDSettingsStorage.get(fSettings[0]));
        //jumper is used to set front or rear adjustment mode (pull low to adjust)
        pinMode(kFrontJumperPin, INPUT_PULLUP); //use internal pullup resistors of Teensy
        pinMode(kRearJumperPin, INPUT_PULLUP);
        pinMode(kPalPin, INPUT_PULLUP); 
    }

    /**
      * Check the adjustment switch and read trimpots and adjust settings.
      */
    virtual void animate() override
    {
        checkAdjSwitch(); //checks the switch and sets adjMode and flipFlopMillis
        //setStatusLED(); //blinks the status LED back and forth
        if (fAdjMode != 0)
        {
            fAdjLoops++;
            compareTrimpots(fAdjMode);
        }

        int palBut = checkPaletteButton();
        if (palBut >= 400)
        {
            for (int i = 0; i < SizeOfArray(fLogic); i++)
            {
                LogicEngineRenderer* logic = fLogic[i];
                if (logic != NULL)
                {
                    logic->changeDefaultSettings(fFactorySettings[i]);
                    logic->changeSettings(fFactorySettings[i]);
                    logic->calculateAllColors();
                    if (i == 0)
                        fFLDSettingsStorage.put(fFactorySettings[i]);
                    else if (i == 1)
                        fRLDSettingsStorage.put(fFactorySettings[i]);
                }
            }
        }
        else if (fAdjMode != 0 && palBut >= 3)
        {
            //change up the color palette used for this logic display
            if (fAdjMode == 1 && fLogic[0] != NULL)
                fLogic[0]->changePalette();
            else if (fAdjMode == 2 && fLogic[1] != NULL)
                fLogic[1]->changePalette();
        }
    }

private:
    byte fAdjMode = 0; // 0 for no adjustments, 1 for front, 2 for rear. if adjMode>0, then trimpots will be enabled
    byte fPrevAdjMode = 0;
    byte fStartAdjMode = 0;
    byte fPrevBrightness = 0;
    byte fPrevPalNum = 0;
    unsigned fPalPinLoops;
    bool fPalPinStatus = 1;
    bool fPrevPalPinStatus = 1;
    unsigned fAdjLoops;
    int fStartTrimpots[4]; //will hold trimpot values when adjustments start being made
    bool fTrimEnabled[4]; //during adjustment, if trimpot has moved beyond specified threshold it will be enabled here
    int fLoopTrimpots[4]; //will hold trimpot values when adjustments start being made
    bool fAdjEnabled[4]; //tells us if a trimpot has been adjusted beyond adj_threshold
    byte fAdjThreshold = 5;
    LogicEngineSettings fSettings[2];
    LogicEngineSettings fFactorySettings[2];
    LogicEngineRenderer* fLogic[2];

    PERSIST(LogicEngineSettings) fFLDSettingsStorage;
    PERSIST(LogicEngineSettings) fRLDSettingsStorage;

    enum
    {
        kMAX_FADE = 15,
        kMAX_DELAY = 500,
        kMIN_DELAY = 10,
        kMIN_BRI = 10,

        kMAX_ADJLOOP = 90000,
        kMIN_ADJLOOP = 500,
    };

    void checkTrimpots(bool startTrim = false)
    {
        //check the current trimpot values and put them into startTrimpots[] or loopTrimpots[]
        int* trimData = (startTrim) ? fStartTrimpots : fLoopTrimpots;

        trimData[0] = map(analogRead(kDelayPin), 0, 1023, kMIN_DELAY, kMAX_DELAY);
        trimData[1] = map(analogRead(kFadePin), 0, 1023, 0, kMAX_FADE);
        trimData[2] = map(analogRead(kBriPin), 0, 1023, kMIN_BRI, LogicEngineDefaults::MAX_BRIGHTNESS);
        trimData[3] = map(analogRead(kHuePin), 0, 1023, 0, 255);
    }

    void compareTrimpots(byte adjMode = 0)
    {
        boolean update = false;
        checkTrimpots();
        for (byte x = 0; x < 4; x++)
        {
            if (x > 1 && fAdjEnabled[x] == 0 &&
                ((fStartTrimpots[x] - fLoopTrimpots[x] > fAdjThreshold) || (fLoopTrimpots[x] - fStartTrimpots[x] > fAdjThreshold)))
            {
                //compare Brightness and Hue using adjThreshold, as changes there can be a lot of work
                fAdjEnabled[x] = 1;
            }
            else if (fAdjEnabled[x] == 0 && fStartTrimpots[x] != fLoopTrimpots[x])
            {
                fAdjEnabled[x] = 1;
            }
            else if (fAdjEnabled[x] == 1)
            {
                if ((x == 1 && fLoopTrimpots[x] != fStartTrimpots[x]) ||
                    (fLoopTrimpots[x] - fStartTrimpots[x] >= 2 || fStartTrimpots[x] - fLoopTrimpots[x] >= 2))
                {
                    //adjustment is enabled for this pot, if settings have changed see if we need to recalc colors and all that jazz
                    if (fAdjMode == 1 || fAdjMode == 2)
                    {
                        LogicEngineRenderer* logic = fLogic[fAdjMode-1];
                        LogicEngineSettings* settings = &fSettings[fAdjMode-1];
                        *settings = logic->getSettings();
                        if (x == 0)
                        {
                            settings->fDelay = fLoopTrimpots[x];
                        }
                        else if (x == 1)
                        {
                            settings->fFade = fLoopTrimpots[x];
                        }
                        else if (x == 2)
                        {
                            settings->fBri = fLoopTrimpots[x];
                        }
                        else if (x == 3)
                        {
                            settings->fHue = fLoopTrimpots[x];
                        }
                        update = true;
                    }
                }
                //save the values for the next loop
                fStartTrimpots[x] = fLoopTrimpots[x];
            }
        }
        if (update && (fAdjMode == 1 || fAdjMode == 2))
        {
            LogicEngineRenderer* logic = fLogic[fAdjMode-1];
            LogicEngineSettings* settings = &fSettings[fAdjMode-1];
            if (logic != NULL)
            {
                logic->changeSettings(*settings);
                logic->calculateAllColors();
            }
        }
    }

    void checkAdjSwitch()
    {
        if (digitalRead(kFrontJumperPin) == 0 && fPrevAdjMode != 1 && fStartAdjMode == 0)
        {
            fAdjMode = 1;
            checkTrimpots(true); //put initial trimpot values into startTrimpots[]
            fAdjLoops = 0;
//            flipFlopLoops = fastBlink;
        }
        else if (digitalRead(kRearJumperPin) == 0 && fPrevAdjMode != 2 && fStartAdjMode == 0)
        {
            fAdjMode = 2;
            checkTrimpots(true); //put initial trimpot values into startTrimpots[]
            fAdjLoops = 0;
//            flipFlopLoops = fastBlink;
        }
        else if ((fPrevAdjMode != 0 && digitalRead(kRearJumperPin) == 1 &&
            digitalRead(kFrontJumperPin) == 1 && fStartAdjMode == 0) || (fAdjLoops > kMAX_ADJLOOP))
        {
            if (fAdjLoops > kMAX_ADJLOOP)
            {
                DEBUG_PRINTLN("MAXED OUT");
            }
            //if we were in previous adjMode for way too long, save settings here  SAVE STUFF HERE and go back to regular mode!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            if (fAdjLoops > kMIN_ADJLOOP) 
            {
                if (fAdjMode == 1)
                {
                    fFLDSettingsStorage.put(fSettings[0]);
                    fLogic[0]->changeDefaultSettings(fSettings[0]);
                }
                else if (fAdjMode == 2)
                {
                    fRLDSettingsStorage.put(fSettings[1]);
                    fLogic[1]->changeDefaultSettings(fSettings[1]);
                }
                if (fAdjLoops > kMAX_ADJLOOP)
                {
                    fStartAdjMode = fAdjMode;
                    fAdjLoops = 0;
                }
                fAdjMode = 0;
                for (byte x = 0; x < 4; x++)
                    fAdjEnabled[x] = 0;
                // flipFlopLoops = slowBlink;  
            }
        }
        else if (digitalRead(kRearJumperPin) == 1 && digitalRead(kFrontJumperPin) == 1 && fStartAdjMode != 0)
        {
            //adjMode didn't start off centered, which could have messed us up.
            //now it is centered though, so let's get back to our normal state.
            fStartAdjMode = 0;
        }
        if (fAdjMode != fPrevAdjMode)
        {
            // statusBlink(2, 250, 1, 0, 2); //blink purple 2 times
        }
        fPrevAdjMode = fAdjMode;
    }

    int checkPaletteButton()
    {
        if (digitalRead(kPalPin) == 0)
        {
            //button is held
            fPalPinLoops++;
            if (fPalPinStatus == 1 && fPrevPalPinStatus == 1)
            {
                //we just started holding the button
                fPalPinStatus = 0;
                fPalPinLoops = 0;
            }
        }  
        else if (digitalRead(kPalPin) == 1 && fPalPinStatus == 0 && fPrevPalPinStatus == 0)
        {
            //button has just been released
            fPalPinLoops++;
            fPalPinStatus = 1;
            return fPalPinLoops;
        }
        fPrevPalPinStatus = fPalPinStatus;
        return 0;
    }
};

#if defined(REELTWO_TEENSY)
typedef LogicEngineController<> LogicEngineControllerTeensy;
typedef LogicEngineControllerTeensy LogicEngineControllerDefault;
#elif defined(REELTWO_ZERO)
typedef LogicEngineController<> LogicEngineControllerReactorZeroRedPCB;
typedef LogicEngineController<LENGINE_DELAY_PIN, LENGINE_FADE_PIN, LENGINE_BRI_PIN, LENGINE_HUE_PIN, LENGINE_PAL_PIN, LENGINE_RJUMP_PIN, LENGINE_FJUMP_PIN> LogicEngineControllerReactorZeroGreenPCB;
typedef LogicEngineControllerReactorZeroRedPCB LogicEngineControllerReactorZero;
typedef LogicEngineControllerReactorZeroRedPCB LogicEngineControllerDefault;
#elif defined(REELTWO_AVR_MEGA)
typedef LogicEngineController<> LogicEngineControllerAVR;
typedef LogicEngineControllerAVR LogicEngineControllerDefault;
#elif defined(REELTWO_AVR)
typedef LogicEngineController<> LogicEngineControllerAVR;
typedef LogicEngineControllerAVR LogicEngineControllerDefault;
#endif

#endif

#endif
