#ifndef FireStrip_h
#define FireStrip_h

#include <Adafruit_NeoPixel.h>
#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include "core/CommandEvent.h"

/**
  * \ingroup Dome
  *
  * \class FireStrip
  *
  * \brief Animates electrical sparks and then fire
  *
  * Animates electrical sparks and then fire.
  *
  * Example usage:
  * \code
  *  FireStrip fireStrip;
  *  fireStrip.spark(500);
  *  fireStrip.burn(500);
  * \endcode
  */
class FireStrip :
    public Adafruit_NeoPixel, SetupEvent, AnimatedEvent, CommandEvent
{
public:
    enum PixelType
    {
        kRGBW = NEO_GRBW + NEO_KHZ800,
        kRGB = NEO_GRB + NEO_KHZ800
    };

    FireStrip(const byte pin, PixelType type = kRGBW, const int id = 0) :
        Adafruit_NeoPixel(NUM_LEDS, pin, type),
        fID(id)
    {
    }

    /**
      * \returns FireStrip Jawa ID
      */
    inline int getID()
    {
        return fID;
    }

    /**
      * Spark animation for the specified millisecond duration
      */
    void spark(uint16_t duration)
    {
        fEffectType = kSparkEffect;
        fEffectEnd = millis() + duration;
        fNextTime = 0;
    }

    /**
      * Fire animation for the specified millisecond duration
      */
    void burn(uint16_t duration)
    {
        fEffectType = kFireEffect;
        fEffectEnd = millis() + duration;
        fNextTime = 0;
    }

    /**
      * Turn of all LEDs
      */
    void off()
    {
        fEffectEnd = 0;
        fFlipFlop = false;
        fEffectType = kNoEffect;
        for (int i = 0; i < numLEDs; i++)
            setPixelColor(i, 0);
        show();
    }

    /**
      * Configures the NeoPixel ring and centers the holoprojector if servos have been assigned
      */
    virtual void setup() override
    {
        begin(); 
        setBrightness(BRIGHT);
        show();
    }

    /**
      * Runs through one frame of animation for this FireStrip instance.
      */
    virtual void animate() override
    {
        uint32_t currentTime = millis();
        switch (fEffectType)
        {
           case kNoEffect:
                return;
            case kSparkEffect:
                if (currentTime > fNextTime)
                {
                    if (!fFlipFlop)
                    {
                        setAll(0x33+random(0x77), 0x33+random(0x77), 0xFF);
                    }
                    else
                    {
                        setAll(0, 0, 0);
                    }
                    fNextTime = currentTime + 10+(10*random(2,5));
                    fFlipFlop = !fFlipFlop;
                }
                break;
            case kFireEffect:
                if (currentTime > fNextTime)
                {
                    fire(55, 120);
                    fNextTime = currentTime + 10+(10*random(2,5));
                    fFlipFlop = !fFlipFlop;
                }
                break;
        }
        if (currentTime > fEffectEnd)
            off();
    }

    /**
      * FireStrip Commands start with 'FS'
      */
    virtual void handleCommand(const char* cmd) override
    {
        if (*cmd++ == 'F' && *cmd++ == 'S')
        {
            // TODO
            if (cmd[0] == 'O' && cmd[1] == 'F' && cmd[2] == 'F' && cmd[3] == '\0')
            {
                off();
            }
        }
    }

private:
    constexpr static int NUM_LEDS = 8;
    enum
    {
        BRIGHT = 100
    };
    enum
    {
        kNoEffect,
        kSparkEffect,
        kFireEffect,
    };
    int fID;
    int fEffectType = kNoEffect;
    bool fFlipFlop;
    uint32_t fEffectEnd;
    uint32_t fNextTime;

    void setAll(byte red, byte green, byte blue)
    {
        for (int i = 0; i < numLEDs; i++ )
        {
            setPixelColor(i, Color(red, green, blue));
        }
        show();
    }

    void fire(int Cooling, int Sparking)
    {
        static byte heat[NUM_LEDS];
        int cooldown;
  
        // Step 1.  Cool down every cell a little
        for (int i = 0; i < NUM_LEDS; i++)
        {
            cooldown = random(0, ((Cooling * 10) / NUM_LEDS) + 2);
    
            if (cooldown>heat[i])
            {
                heat[i] = 0;
            }
            else
            {
                heat[i] = heat[i] - cooldown;
            }
        }
  
        // Step 2.  Heat from each cell drifts 'up' and diffuses a little
        for (int k= NUM_LEDS - 1; k >= 2; k--)
        {
            heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
        }
    
        // Step 3.  Randomly ignite new 'sparks' near the bottom
        if (random(255) < Sparking)
        {
            int y = random(7);
            heat[y] = heat[y] + random(160,255);
        }

        // Step 4.  Convert heat to LED colors
        for (int j = 0; j < NUM_LEDS; j++)
        {
            setPixelHeatColor(j, heat[j]);
        }

        show();
    }

    void setPixelHeatColor(int pixel, byte temperature)
    {
        // Scale 'heat' down from 0-255 to 0-191
        byte t192 = round((temperature/255.0)*191);
 
        // calculate ramp up from
        byte heatramp = t192 & 0x3F; // 0..63
        heatramp <<= 2; // scale up to 0..252
 
        // figure out which third of the spectrum we're in:
        if (t192 > 0x80)
        {
            // hottest
            setPixelColor(pixel, Color(255, 255, heatramp));
        }
        else if (t192 > 0x40)
        {
            // middle
            setPixelColor(pixel, Color(255, heatramp, 0));
        }
        else
        {
            // coolest
            setPixelColor(pixel, Color(heatramp, 0, 0));
        }
    }
};

#endif
