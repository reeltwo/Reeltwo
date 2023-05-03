#ifndef SingleStatusLED_h
#define SingleStatusLED_h

#include "ReelTwo.h"
#include "dome/LogicEngine.h"

/**
  * \ingroup Core
  *
  * \class SingleStatusLED
  *
  * \brief LED status indicator
  *
  * Animate a single LED status indicator
  *
  * \code
  * typedef SingleStatusLED<LED_STATUS_PIN> StatusLED;
  * StatusLED       statusLED;
  * \endcode
  */
template <uint8_t DATA_PIN>
class SingleStatusLED : public AnimatedEvent, public SetupEvent
{
public:
    SingleStatusLED()
    {
    }

    SingleStatusLED(const uint8_t (*colors)[4][3], unsigned numModes = 1) :
        fNumModes(numModes),
        fColors(colors)
    {
        setDelay(1000);
    }

    virtual void setup() override
    {
        fStatus.init();
        pickColor();
    #if USE_LEDLIB == 1
        fStatus.show();
    #else
        FastLED.show();
    #endif
    }

    virtual void animate() override
    {
        uint32_t timeNow = millis();
        if (timeNow - fPrevFlipFlopMillis > fStatusFlipFlopTime)
        {
            fPrevFlipFlopMillis = timeNow;
            fStatusColor++;
            if (fStatusColor == 4) fStatusColor = 0;
            pickColor();
        #if USE_LEDLIB == 1
            fStatus.show();
        #else
            FastLED.show();
        #endif
        }
    }

    void setMode(unsigned mode)
    {
        if (mode < fNumModes)
        {
            if (fMode != mode)
            {
                fMode = mode;
                fPrevFlipFlopMillis = 0;
                fStatusColor = 0;
            }
            animate();
        }
    }

    void setDelay(uint32_t delay)
    {
        fStatusFlipFlopTime = delay;
    }

protected:
    FastLEDPCB<WS2812B, DATA_PIN> fStatus;
    byte fStatusColor = 0; //status LED will cycle between 4 colors depending on what mode we're in
    byte fPrevStatusColor = 0;
    unsigned fMode = 0;
    unsigned fNumModes = 4;
    const uint8_t (*fColors)[4][3] = nullptr;
    uint32_t fPrevFlipFlopMillis = 0;
    uint32_t fStatusFlipFlopTime = 1000;

    void pickColor()
    {
        if (fColors == nullptr)
        {
            static constexpr uint8_t kStatusColors[5][4][3] = {
                  { {  10,   0,   0} , {  0,   0,   10} , {  10,   0,   0} , {  0,   0,   10}  } , // red,blue,red,blue
                  { { 25,  25,  25} , { 16,  16,  16} , { 10,  10,  10} , {  10,   2,   2}  } , // brightness 
                  { {  2,   0,   0} , {  2,   2,   0} , {  0,   2,   0} , {  0,   0,   2}  } , // hue (red , yel, green, blue)
                  { {  2,   0,   2} , {  2,   0,   1} , {  2,   0,   0} , {  2,   0,   1}  } , // fade (purple, blue)
                  { {  0,   2,   0} , {  0,   2,   0} , {  0,   2,   0} , {  0,   2,   0}  }   // pause (all green)
            };
            fStatus.fLED[0].setRGB(
                kStatusColors[fMode][fStatusColor][0],
                kStatusColors[fMode][fStatusColor][1],
                kStatusColors[fMode][fStatusColor][2]);
        }
        else
        {
            fStatus.fLED[0].setRGB(
                fColors[fMode][fStatusColor][0],
                fColors[fMode][fStatusColor][1],
                fColors[fMode][fStatusColor][2]);
        }
    }
};

#endif
