#ifndef LOGICENGINECONTROLLER32_H
#define LOGICENGINECONTROLLER32_H

#include "ReelTwo.h"
#ifndef LENGINE_DELAY_PIN
  #define LENGINE_BUTTON_PIN    36 /* 4-button resistor lattice pin */
#endif
#ifndef LENGINE_TRIMPOT_PIN
  #define LENGINE_TRIMPOT_PIN   39 /* analog pin to read trimpot */
#endif
#ifndef LENGINE_STATUS_PIN
  #define LENGINE_STATUS_PIN    14 /* LED status pin */
#endif

#include "dome/LogicEngine.h"
#include "core/AnalogMonitor.h"
#include "core/LEDPixelEngine.h"
#include <Preferences.h>

//buttons are connected via a resistor ladder to a single ADC pin
// 100K, 33K, 66K and 200K
// so pressing each button will give us a different analog reading
static constexpr unsigned int LogicEngineController_kButtonVals[4]={0,850,1900,2900};

/* LED preferences */
#define PREFERENCE_LOGICS       "logics"

/**
  * \ingroup Dome
  *
  * \class LogicEngineController
  *
  * \brief Settings adjust for LogicEngine logics
  *
  * \brief Adjust the settings for a set of LogicEngine logics using PCB trimpots and jumpers.
  */
template <byte kButtonPin = LENGINE_BUTTON_PIN, byte kTrimpotPin = LENGINE_TRIMPOT_PIN, byte kLEDStatusPin = LENGINE_STATUS_PIN>
class LogicEngineController : public AnimatedEvent, public SetupEvent
{
public:
    template <uint8_t DATA_PIN = FRONT_LOGIC_PIN>
    class StatusLED : public FastLEDPCB<WS2812B, kLEDStatusPin>
    {
    };

    /**
      * \brief Constructor
      */
    LogicEngineController(
            Preferences& preferences) :
        fPreferences(preferences),
        fTrimPot(kTrimpotPin)
    {
        fLogic[0] = nullptr;
        fLogic[1] = nullptr;
    }

    void setWifiReset(void (*resetFunction)())
    {
        fWifiResetFunction = resetFunction;
    }

    void setWifiToggle(void (*toggleFunction)())
    {
        fWifiToggleFunction = toggleFunction;
    }

    virtual void setup() override
    {
        // ensure button pin is an input
        pinMode(kButtonPin, INPUT);
        pinMode(kTrimpotPin, INPUT);
        fPrevButtonValue = analogRead(kButtonPin);

        fTrimPot.setActivityThreshold(40);

        fStatusLED.init();        
    }

    /**
      * Check the adjustment switch and read trimpots and adjust settings.
      */
    void configure(
        LogicEngineRenderer* FLD,
        LogicEngineRenderer* RLD)
    {
        fLogic[0] = FLD;
        fLogic[1] = RLD;
        if (FLD != nullptr)
        {
            fFactorySettings[0] = fSettings[0] = FLD->getSettings();
        }
        if (RLD != nullptr)
        {
            fFactorySettings[1] = fSettings[1] = RLD->getSettings();
        }
        if (fPreferences.isKey(PREFERENCE_LOGICS))
        {
            if (fPreferences.getBytes(PREFERENCE_LOGICS, fSettings, sizeof(fSettings)) == sizeof(fSettings))
            {
                if (FLD != nullptr)
                {
                    FLD->changeDefaultSettings(fSettings[0]);
                }
                if (RLD != nullptr)
                {
                    RLD->changeDefaultSettings(fSettings[1]);
                }
            }
        }
    }

    LogicEngineSettings& getSettings(unsigned logicNum)
    {
        logicNum = (logicNum & 1);
        return fSettings[logicNum];
    }

    // logicNum: 0==FLD 1==RLD
    void restoreFactoryDefaults(unsigned logicNum)
    {
        DEBUG_PRINT("RESTORE DEFAULTS logicNum="); DEBUG_PRINTLN(logicNum);
        logicNum = (logicNum & 1);
        LogicEngineRenderer* logic = fLogic[logicNum];
        LogicEngineSettings* settings = &fSettings[logicNum];
        *settings = fFactorySettings[logicNum];
        if (logic != nullptr)
        {
            commit();
            logic->changeDefaultSettings(*settings);
            logic->calculateAllColors();
        }
    }

    // logicNum: 0==FLD 1==RLD
    // settingNum: 1==brightness 2==hue 3==fade 4==delay 5==pal
    void changeSetting(unsigned logicNum, unsigned settingNum, unsigned val)
    {
        logicNum = (logicNum & 1);
        DEBUG_PRINT("CHANGE logicNum="); DEBUG_PRINT(logicNum); DEBUG_PRINT(" setting="); DEBUG_PRINT(settingNum); DEBUG_PRINT(" val="); DEBUG_PRINTLN(val);
        LogicEngineRenderer* logic = fLogic[logicNum];
        LogicEngineSettings* settings = &fSettings[logicNum];
        *settings = logic->getSettings();
        switch (settingNum)
        {
            case 1:
                settings->fBri = val;
                break;
            case 2:
                settings->fHue = val;
                break;
            case 3:
                settings->fFade = val;
                break;
            case 4:
                settings->fDelay = val;
                break;
            case 5:
                if (val >= logic->PAL_COUNT)
                    val = 0;
                settings->fPalNum = val;
                break;
        }
        if (logic != nullptr)
        {
            logic->changeDefaultSettings(*settings);
            logic->calculateAllColors();
        }
    }

    // logicNum: 0==FLD 1==RLD
    bool commit()
    {
        return (fPreferences.putBytes(PREFERENCE_LOGICS, fSettings, sizeof(fSettings)) == sizeof(fSettings));
    }

    /**
      * Check the adjustment switch and read trimpots and adjust settings.
      */
    virtual void animate() override
    {
        readButtons();

        doStatusLED();

        if ((fAdjMode == 1 || fAdjMode == 2) && fLogic[fAdjMode-1] != nullptr && fTrimPot.hasChanged())
        {
            uint8_t val = 255 - (uint8_t)(0xFF * (fTrimPot.getValue() / 4096.0f));
            changeSetting(fAdjMode-1, fTrimpotMode, val);
        }
    }

private:
    static constexpr unsigned kButtonVariance = 300; //changed from 200 for boards that wouldn't recognize buttons 2 & 3
    static constexpr uint32_t kStatusLED_LowDelay = 300;
    static constexpr uint32_t kStatusLED_NormalDelay = 1000;

    static constexpr unsigned numberOfButtons()
    {
        return sizeof(LogicEngineController_kButtonVals)/sizeof(LogicEngineController_kButtonVals[0]);
    }

    Preferences& fPreferences;
    StatusLED<> fStatusLED;
    AnalogMonitor fTrimPot;
    void (*fWifiResetFunction)() = nullptr;
    void (*fWifiToggleFunction)() = nullptr;

    byte fAdjMode = 0; // 0 for no adjustments, 1 for front, 2 for rear. if adjMode>0, then trimpots will be enabled
    byte fTrimpotMode;
    unsigned fPrevButtonValue;
    bool fButtonState[numberOfButtons()]; //state of each button
    bool fPrevButtonState[numberOfButtons()];
    uint32_t fButtonTime[numberOfButtons()+1]; //when did state of each button last change
    LogicEngineRenderer* fLogic[2];
    LogicEngineSettings fSettings[sizeof(fLogic)/sizeof(fLogic[0])];
    LogicEngineSettings fFactorySettings[sizeof(fLogic)/sizeof(fLogic[0])];
    bool fChanges[sizeof(fLogic)/sizeof(fLogic[0])] = {};

    byte fStatusColor = 0; //status LED will cycle between 4 colors depending on what mode we're in
    byte fPrevStatusColor = 0;
    uint32_t fPrevFlipFlopMillis = 0;
    uint32_t fStatusFlipFlopTime = kStatusLED_NormalDelay;

    static bool inRange(int val, int minimum, int maximum)
    {
        return ((minimum <= val) && (val <= maximum));
    }

    inline void fastStatusMode()
    {
        fStatusFlipFlopTime = kStatusLED_LowDelay;
    }

    inline void normalStatusMode()
    {
        fStatusFlipFlopTime = kStatusLED_NormalDelay;
    }

    void doStatusLED()
    {
        static constexpr uint8_t kStatusColors[5][4][3] = {
            { {  2,   0,   0} , {  0,   0,   2} , {  2,   0,   0} , {  0,   0,   2}  } , // red,blue,red,blue
            { { 25,  25,  25} , { 16,  16,  16} , { 10,  10,  10} , {  2,   2,   2}  } , // brightness 
            { {  2,   0,   0} , {  2,   2,   0} , {  0,   2,   0} , {  0,   0,   2}  } , // hue (red , yel, green, blue)
            { {  2,   0,   2} , {  2,   0,   1} , {  2,   0,   0} , {  2,   0,   1}  } , // fade (purple, blue)
            { {  0,   2,   0} , {  0,   2,   0} , {  0,   2,   0} , {  0,   2,   0}  }   // pause (all green)
        };
        uint32_t timeNow = millis();
        if (timeNow - fPrevFlipFlopMillis > fStatusFlipFlopTime)
        {
            fPrevFlipFlopMillis = timeNow;
            fStatusColor++;
            if (fStatusColor == 4) fStatusColor = 0;
            fStatusLED.fLED[0].r = kStatusColors[fTrimpotMode][fStatusColor][0];
            fStatusLED.fLED[0].g = kStatusColors[fTrimpotMode][fStatusColor][1];
            fStatusLED.fLED[0].b = kStatusColors[fTrimpotMode][fStatusColor][2];
        #if USE_LEDLIB == 1
            fStatusLED.show();
        #else
            FastLED.show();
        #endif
        }
    }

    void readButtons()
    {
        uint32_t timeNow = millis();
        unsigned buttonValue = analogRead(kButtonPin);
        unsigned buttonReleased = numberOfButtons()+1;
        if (!inRange(buttonValue, fPrevButtonValue - kButtonVariance, fPrevButtonValue + kButtonVariance))
        {
            for (byte i = 0; i < numberOfButtons(); i++)
            {
                if (inRange(buttonValue, LogicEngineController_kButtonVals[i] - kButtonVariance, LogicEngineController_kButtonVals[i] + kButtonVariance))
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

        if (buttonReleased < numberOfButtons()+1)
        {
            DEBUG_PRINT("BUTTON: "); DEBUG_PRINTLN(buttonReleased);
            if (buttonReleased == 0)
            {
                if (timeNow - fButtonTime[buttonReleased] > 5000 && fWifiResetFunction != nullptr)
                    fWifiResetFunction();
                else if (timeNow - fButtonTime[buttonReleased] > 2000 && fWifiToggleFunction != nullptr)
                    fWifiToggleFunction();
            }
            else if (fLogic[0] != nullptr && fAdjMode == 0 && buttonReleased == 1)
            {
                if (timeNow - fButtonTime[1] > 5000)
                {
                    DEBUG_PRINTLN("front reset defaults");
                    restoreFactoryDefaults(0);
                }
                else if (timeNow - fButtonTime[1] > 1000)
                {
                    DEBUG_PRINTLN("front adj mode");
                    fAdjMode = 1;
                    fTrimpotMode = 1; //brightness->hue->fade->pause
                    fastStatusMode();
                }
            }
            else if (fLogic[1] != nullptr && fAdjMode == 0 && buttonReleased == 2)
            {
                if (timeNow - fButtonTime[2] > 5000)  //changed from 1000, was preventing us going to rear adjust mode
                {
                    DEBUG_PRINTLN("rear reset defaults");
                    restoreFactoryDefaults(1);
                }
                else if (timeNow - fButtonTime[2] > 1000)
                {
                    DEBUG_PRINTLN("rear adj mode");
                    fAdjMode = 2;
                    fTrimpotMode = 1; //brightness->hue->fade->pause
                    fastStatusMode();
                }
            }
            else if ((fAdjMode==1 || fAdjMode==2) && (buttonReleased == 1 || buttonReleased == 2))
            {
                if (timeNow - fButtonTime[buttonReleased] > 1000)
                {
                    DEBUG_PRINTLN("save settings");
                    commit();
                    fAdjMode = 0;
                    normalStatusMode();
                    fTrimpotMode = 0;
                }
                else if (timeNow - fButtonTime[1] > 30)
                {
                    fTrimpotMode++;
                    if (fTrimpotMode > 4) fTrimpotMode = 1;
                #ifdef USE_DEBUG
                    if (fTrimpotMode==1) DEBUG_PRINTLN(" bri");
                    else if (fTrimpotMode==2) DEBUG_PRINTLN(" hue");
                    else if (fTrimpotMode==3) DEBUG_PRINTLN(" fade");
                    else if (fTrimpotMode==4) DEBUG_PRINTLN(" pause");
                #endif
                }
            }
            if ((fAdjMode == 1 || fAdjMode == 2) && buttonReleased == 3 && timeNow - fButtonTime[4] > 30)
            {
                //change up palette number
                if (fLogic[fAdjMode-1] != nullptr)
                {
                    changeSetting(fAdjMode-1, 5, fLogic[fAdjMode-1]->getCurrentPalette()+1);
                }
            }  
        }
    }
};

typedef LogicEngineController<> LogicEngineControllerDefault;

#endif
