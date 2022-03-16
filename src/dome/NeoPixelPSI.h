#ifndef NeoPixelPSI_h
#define NeoPixelPSI_h

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include <Adafruit_NeoPixel.h>

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
     * Pass the pin number for the PSI and optionally the size of the PSI (for denser pixel counts)
     */
    NeoPixelPSI(int psi_pin, int psi_size = 5)
    {
      leds.updateLength(num_leds[psi_size]);
      leds.updateType(NEO_GRB + NEO_KHZ800);
      leds.setPin(psi_pin);
      switch (psi_size) {
        case 8:
          memcpy(LEDmap, LEDmap8, sizeof(LEDmap8));
          break;
        case 7:
          for(int x = 0; x < psi_size ; x++ ){
            memcpy(LEDmap[x], LEDmap7[x], sizeof(LEDmap7[x]));
          }
          break;
        case 5:
          for(int x = 0; x < psi_size ; x++ ){
            memcpy(LEDmap[x], LEDmap5[x], sizeof(LEDmap5[x]));
          }
          break;
      }
      grid_size = psi_size;
      leds.begin();
      leds.setBrightness(brightness);
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

    virtual void animate() {
      unsigned long currentMillis = millis();
      if (currentMillis >= fSwipeSpeed)
      {
        swipe_main(swipe_position);
        if (swipe_direction == 0) {
          if (swipe_position >= grid_size-1) {
             swipe_direction = 1;
             fSwipeSpeed = currentMillis + random(sdelay, sdelay*4);
          } else {
             fSwipeSpeed = currentMillis + sspeed;
             swipe_position++;
          }
        } else {
          if (swipe_position <= 0) {
             swipe_direction = 0;
             fSwipeSpeed = currentMillis + random(sdelay, sdelay*4);
          } else {
             fSwipeSpeed = currentMillis + sspeed;
             swipe_position--;
          }
        }
        if (swipe_position == int((grid_size/2)+0.5)) {
             if (random(0,100) < stickiness) {
                     fSwipeSpeed  = currentMillis + random(sdelay, sdelay*4);
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

    /**
     * \brief Set the 'stickiness'
     *
     * To simulate the original PSI 'sticking' halfway through a swipe, you can set the percentage
     * chance of the swipe halting for a period (as set in set_delay()).
     *
     * Default: 0
     * Range: 0-100
     */
    void set_stickiness(int s) {
      stickiness = s;
    }


  private:
    int fPSI = 0;
    int swipe_direction = 0;
    int swipe_position = 0;
    int sspeed = SWIPE_SPEED;
    int sdelay = SWIPE_DELAY;
    int brightness = BRIGHTNESS;
    int stickiness = STICKINESS;
    int grid_size = 5;
    int num_leds[9] = {0, 0, 0, 0, 0, 21, 0, 37, 52};
    uint32_t color_one = leds.Color(255,0,0);
    uint32_t color_two = leds.Color(0,0, 255);
    unsigned long fSwipeSpeed;
    unsigned long fSwipeMillis;
    int LEDmap[8][8];

    const int LEDmap5[5][5] PROGMEM = {
       {99, 0, 1, 2, 99 },
       {3, 4, 5, 6, 7},
       {8, 9, 10, 11, 12},
       {13, 14, 15, 16, 17},
       {99, 18, 19, 20, 99},
    };

    const int LEDmap7[7][7] PROGMEM = {
      {99, 99, 2, 1, 0, 99, 99},
      {99, 7, 6, 5, 4, 3, 99},
      {14, 13, 12, 11, 10, 9, 8},
      {21, 20, 19, 18, 17, 16, 15},
      {28, 27, 26, 25, 24, 23, 22},
      {99, 33, 32, 31, 30, 29, 99 },
      {99, 99, 36, 35, 34, 99, 99}
    };

    const int LEDmap8[8][8] PROGMEM = {
      {99,99,3,2,1,0,99,99},
      {99,9,8,7,6,5,4,99},
      {17,16,15,14,13,12,11,10},
      {25,24,23,22,21,20,19,18},
      {33,32,31,30,29,28,27,26},
      {41,40,39,38,37,36,35,34},
      {99,47,46,45,44,43,42,99},
      {99,99,51,50,49,48,99,99}
    }; 

    void swipe_main(uint8_t pos)
    {
      uint32_t color;
      for(int row = 0; row <= grid_size-1 ; row++) {
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

