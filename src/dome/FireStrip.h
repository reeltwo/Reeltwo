#ifndef FireStrip_h
#define FireStrip_h

#include "ReelTwo.h"
#include "core/LEDPixelEngine.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include "core/CommandEvent.h"

#if USE_LEDLIB == 0
template<uint8_t DATA_PIN, uint32_t RGB_ORDER, uint16_t NUM_LEDS>
class FireStripPCB : public FastLED_NeoPixel<NUM_LEDS, DATA_PIN, RGB_ORDER>
{
public:
    FireStripPCB() {}
};
#define USE_FIRESTRIP_TEMPLATE 1
#elif USE_LEDLIB == 1
template<uint8_t DATA_PIN, uint32_t RGB_ORDER, uint16_t NUM_LEDS>
class FireStripPCB : public Adafruit_NeoPixel
{
public:
    FireStripPCB() :
        Adafruit_NeoPixel(NUM_LEDS, DATA_PIN, RGB_ORDER)
    {
    }
};
#else
 #error Unsupported
#endif

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
#if USE_FIRESTRIP_TEMPLATE
template<uint8_t DATA_PIN, uint32_t RGB_ORDER = GRB, uint16_t NUM_LEDS = 8>
class FireStrip :
    private FireStripPCB<DATA_PIN, RGB_ORDER, NUM_LEDS>,
    public SetupEvent, AnimatedEvent, CommandEvent
#else
class FireStrip :
    private Adafruit_NeoPixel,
    public SetupEvent, AnimatedEvent, CommandEvent
#endif
{
public:
#if USE_FIRESTRIP_TEMPLATE
    FireStrip(const int id = 0) :
        fID(id)
    {
    }
#else
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
#endif

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
        for (unsigned i = 0; i < NUM_LEDS; i++)
            setPixelColor(i, 0);
        dirty();
    }

    /**
      * Configures the NeoPixel ring and centers the holoprojector if servos have been assigned
      */
    virtual void setup() override
    {
        TEENSY_PROP_NEOPIXEL_SETUP()
        setBrightness(BRIGHT);
        dirty();
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

        if (fDirty)
        {
            TEENSY_PROP_NEOPIXEL_BEGIN();
            show();
            TEENSY_PROP_NEOPIXEL_END();
            fDirty = false;
        } 
    }

    /**
      * FireStrip Commands start with 'FS'
      */
    virtual void handleCommand(const char* cmd) override
    {
        if (*cmd++ == 'F' && *cmd++ == 'S')
        {
            switch (cmd[0])
            {
                case 'O':
                    if (cmd[1] == 'F' && cmd[2] == 'F' && cmd[3] == '\0')
                    {
                        off();
                    }
                    break;
                case '0':
                    off();
                    break;

                case '1':
                    spark(atoi(&cmd[1]));
                    break;
                case '2':
                    burn(atoi(&cmd[1]));
                    break;
            }
        }
    }

    inline void dirty()
    {
        fDirty = true;
    }

private:
#if USE_FIRESTRIP_TEMPLATE
    void begin()
    {
        FireStripPCB<DATA_PIN, RGB_ORDER, NUM_LEDS>::begin();
    }

    void show()
    {
        FireStripPCB<DATA_PIN, RGB_ORDER, NUM_LEDS>::show();
    }

    void setPixelColor(uint16_t n, uint32_t c)
    {
        FireStripPCB<DATA_PIN, RGB_ORDER, NUM_LEDS>::setPixelColor(n, c);
    }

    void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b)
    {
        FireStripPCB<DATA_PIN, RGB_ORDER, NUM_LEDS>::setPixelColor(n, r, g, b);
    }
#else
    constexpr static int NUM_LEDS = 8;
#endif
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
    bool fDirty;
    uint32_t fEffectEnd;
    uint32_t fNextTime;

    static long atoi(const char* s)
    {
        long int val = 0;
        while (*s >= '0' && *s <= '9')
        {
            val = val * 10 + (*s++ - '0');
        }
        return val;
    }

    void setAll(byte red, byte green, byte blue)
    {
        for (unsigned i = 0; i < NUM_LEDS; i++)
        {
            setPixelColor(i, red, green, blue);
        }
        dirty();
    }

    void fire(int Cooling, unsigned Sparking)
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

        dirty();
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
            setPixelColor(pixel, 255, 255, heatramp);
        }
        else if (t192 > 0x40)
        {
            // middle
            setPixelColor(pixel, 255, heatramp, 0);
        }
        else
        {
            // coolest
            setPixelColor(pixel, heatramp, 0, 0);
        }
    }
};

#endif
