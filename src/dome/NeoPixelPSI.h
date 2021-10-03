#ifndef NeoPixelPSI_h
#define NeoPixelPSI_h

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include <Adafruit_NeoPixel.h>

// How many leds are in the strip?
#define NUM_LEDS 21

#define SWIPE_SPEED 75 
#define SWIPE_DELAY 1000 
#define STICKINESS 0
#define BRIGHTNESS 28

#ifdef USE_DEBUG
 #define PSI_DEBUG
#endif

/**
  * \ingroup Dome
  *
  * \class NeoPixelPSI
  *
  * \brief NeoPixelPSI by Darren Poulson <daz@r2djp.co.uk>
  *
  * NeoPixelPSI are very simple and cheap boards, in reality just a string of neopixels on a disk. This class
  * will configure them to act as standard PSIs for R2.
  *
  * For more info:
  *
  * https://r2djp.co.uk/category/electronics/neopixel-psi/
  *
  * Example Code:
  * \code
  *  NeoPixelPSI rearPSI(4);
  *  NeoPixelPSI frontPSI(3);
  * \endcode
  *
  */

class NeoPixelPSI :
	public AnimatedEvent, SetupEvent, CommandEvent
{
  public: 
    Adafruit_NeoPixel leds;
    
    enum EffectValue
    {
        kNormalVal = 0
    };

    enum Sequence
    {
        kNormal = 0,
        kSolid = 1,
        kHeart = 2,
        kMalf = 3,
    };

    enum PSIID
    {
        /** Front PSI ID */
        kFrontPSI = 1,
        /** Rear PSI ID */
        kRearPSI = 2,
    };

    virtual void setup() override
    {
    }

    /** 
     * \brief Constructor
     *
     * Pass the pin number for the PSI
     */
    NeoPixelPSI(int psi_pin) :
      leds(NUM_LEDS, psi_pin, NEO_GRB + NEO_KHZ800)
    {
      leds.begin();
      leds.setBrightness(BRIGHTNESS);
    }

    /**
      * Select the specified effect using a 32-bit integer.
      *
      *  \li Sequence (0-99) * 10000
      *  \li Speed (0-9) * 100
      *  \li Duration (0-99)
      */
    void selectEffect(long inputNum)
    {
    }

    /**
     * Command Prefix: PS
     *
     */
    virtual void handleCommand(const char* cmd) override
    {
        if (*cmd++ == 'P' && *cmd++ == 'S')
        {
            long int cmdvalue = 0;
            const char* c = cmd;
            while (*c >= '0' && *c <= '9')
            {
                cmdvalue = cmdvalue * 10 + (*c++ - '0');
            }
            selectEffect(cmdvalue);
        }
    }
    
    /**
      * Select the specified effect sequence.
      */
    inline void setSequence(Sequence seq = kNormal, uint8_t speedScale = 0, uint8_t numSeconds = 0)
    {
        selectEffect((long int)seq * 10000L + (long int)speedScale * 100 + numSeconds);
    }

    virtual void animate() override {
      unsigned long currentMillis = millis();
      if (currentMillis >= fSwipeSpeed)
      {
        swipe_main(swipe_position);
        if (swipe_direction == 0) {
          if (swipe_position > 3) {
             swipe_direction = 1;
             fSwipeSpeed = currentMillis + random(sdelay, sdelay*4);
             //swipe_position--;
          } else {
             fSwipeSpeed = currentMillis + sspeed;
             swipe_position++;
          }
        } else {
          if (swipe_position <= 0) {
             swipe_direction = 0;
             fSwipeSpeed = currentMillis + random(sdelay, sdelay*4);
             //swipe_position++;
          } else {
             fSwipeSpeed = currentMillis + sspeed;
             swipe_position--;
          }
        }
      }
    }


    /**
     * \brief Set the brightness of the PSI
     *
     * Send a value from 0-255 to set the brightness, the higher the number the brighter it is
     *
     * Don't set 255 straight away, these can be bright. Typical value is between 20-50
     */
    void set_brightness(int bright) {
      brightness = bright;
      leds.setBrightness(brightness);
      leds.show();
    }

    /**
     * \brief set the colors of the PSI
     *
     * \li color number (1=first, 2=second)
     * \li red
     * \li green
     * \li blue
     */
    virtual void set_color(int c, int r, int g, int b) {
      if (c == 1)
        color_one = leds.Color(r,g,b);
      else
        color_two = leds.Color(r,g,b);

    }

    /**
     * \brief Set the speed the swipe effect moves at
     *
     * Lower the number, the faster the swipe. This is in ms
     * Typical value is around 75
     */
    void set_speed(int s)  {
      sspeed = s;
    }

    /**
     * \brief Set the delay between swipes
     *
     * Set how many ms between swipes. The actual value will be between <value> and <value>*4
     */
    void set_delay(int d) {
      sdelay = d;
    }



  private:
    int fPSI = 0;
    int swipe_direction = 0;
    int swipe_position = 0;
    int sspeed = SWIPE_SPEED;
    int sdelay = SWIPE_DELAY;
    int brightness = BRIGHTNESS;
    uint32_t color_one = leds.Color(255,0,0);
    uint32_t color_two = leds.Color(0,0, 255);
    unsigned long fSwipeSpeed;
    unsigned long fSwipeMillis; 
    byte LEDmap[5][5]  = {
       {99, 0, 1, 2, 99 },
       {3, 4, 5, 6, 7},
       {8, 9, 10, 11, 12},
       {13, 14, 15, 16, 17},
       {99, 18, 19, 20, 99},
    };
    

    void swipe_main(uint8_t pos)
    {
      uint32_t color;
      for(int row = 0; row <= 4 ; row++) {
        if(swipe_direction == 0)
            color = color_one;
        else
            color = color_two;
        int led = LEDmap[pos][row];
        if(led != 99)
          leds.setPixelColor(led, color);
      }
      leds.show();
    }
};

#endif

