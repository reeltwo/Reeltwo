#ifndef LOGICENGINE_H
#define LOGICENGINE_H

#include "ReelTwo.h"
#include "core/LEDPixelEngine.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include "core/CommandEvent.h"
#include "core/JawaEvent.h"
#include "core/PeakValueProvider.h"
#include "core/Font.h"

#ifndef FRONT_LOGIC_PIN
 #if defined(REELTWO_TEENSY)
  #define FRONT_LOGIC_PIN 21
 #elif defined(REELTWO_ZERO)
  #define FRONT_LOGIC_PIN 5
 #elif defined(REELTWO_AVR_MEGA)
  #define FRONT_LOGIC_PIN 5
 #elif defined(REELTWO_AVR)
  /* Pro Mini only supports either front or rear logic */
  #define FRONT_LOGIC_PIN 6
 #elif defined(REELTWO_RP2040)
  #define FRONT_LOGIC_PIN 15
  #define FRONT_LOGIC_CLOCK_PIN 2
 #elif defined(ESP32)
  #define FRONT_LOGIC_PIN 15
  #define FRONT_LOGIC_CLOCK_PIN 2
 #else
  #error Unsupported platform
 #endif
#endif

#ifndef REAR_LOGIC_PIN
 #if defined(REELTWO_TEENSY)
  #define REAR_LOGIC_PIN 22
 #elif defined(REELTWO_ZERO)
  #define REAR_LOGIC_PIN 3
 #elif defined(REELTWO_AVR_MEGA)
  #define REAR_LOGIC_PIN 6
  #define REAR_LOGIC_CLOCK_PIN  7
 #elif defined(REELTWO_AVR)
  /* Pro Mini only supports either front or rear logic */
  #define REAR_LOGIC_PIN 6
 #elif defined(REELTWO_RP2040)
  #define REAR_LOGIC_PIN  33
  #define REAR_LOGIC_CLOCK_PIN  32
 #elif defined(ESP32)
  #define REAR_LOGIC_PIN  33
  #define REAR_LOGIC_CLOCK_PIN  32
 #else
  #error Unsupported platform
 #endif
#endif

/** \ingroup Dome
 *
 * \struct LEDStatus
 *
 * \brief Current color number and pause value for a single LED
 *
 * Holds the current color number and pause value of a single LED
 * (when pause value hits 0, the color number gets changed) to save SRAM,
 * we don't store the "direction" anymore, instead we pretend that instead of
 * having 16 colors, we've got 31 (16 that cross-fade up, and 15 "bizarro"
 * colors that cross-fade back in reverse)
 */
struct LEDStatus
{
    /**
      *  Color number
      */
    byte fColorNum;

    /**
      *  Pause value
      */
    byte fColorPause;
};

/** \ingroup Dome
 *
 * \class LogicEngineDefaults
 *
 * \brief Default settings for LogicEngine hardware
 */
class LogicEngineDefaults
{
public:
    static constexpr byte FRONT_FADE = 1;
    static constexpr byte FRONT_DELAY = 10;
    static constexpr byte FRONT_HUE = 0;

    static constexpr byte REAR_FADE = 3;
    static constexpr byte REAR_DELAY = 40;
    static constexpr byte REAR_HUE = 0;

    static constexpr byte FRONT_PAL = 0;
    static constexpr byte REAR_PAL = 1;

    static constexpr byte FRONT_PSI_PAL = 4;
    static constexpr byte REAR_PSI_PAL = 5;

    static constexpr byte FRONT_BRI = 160; //changed from 180 to bring down default current draw under 500mA
    static constexpr byte REAR_BRI = 140; //changed from 180 to bring down default current draw under 500mA

    static constexpr byte MAX_BRIGHTNESS = 225; //can go up to 255, but why? this limit keeps current and heat down, and not noticeably dimmer than 255
    static constexpr byte MIN_BRIGHTNESS = 1;   //minimum brightness for standard logic patterns that adjustment pots can go down to

    static constexpr uint32_t NORMVAL = 0;
    static constexpr byte PAL_COUNT = 6;

    /* Values should match array offsets in LogicEffectDefaultSelector() */
    static constexpr byte NORMAL = 0;
    static constexpr byte ALARM = 1;
    static constexpr byte FAILURE = 2;
    static constexpr byte LEIA = 3;
    static constexpr byte MARCH = 4;
    static constexpr byte SOLIDCOLOR = 5;
    static constexpr byte FLASHCOLOR = 6;
    static constexpr byte FLIPFLOPCOLOR = 7;
    static constexpr byte FLIPFLOPALTCOLOR = 8;
    static constexpr byte COLORSWAP = 9;
    static constexpr byte RAINBOW = 10;
    static constexpr byte REDALERT = 11;
    static constexpr byte MICBRIGHT = 12;
    static constexpr byte MICRAINBOW = 13;
    static constexpr byte LIGHTSOUT = 14;
    static constexpr byte TEXT = 15;
    static constexpr byte TEXTSCROLLLEFT = 16;
    static constexpr byte TEXTSCROLLRIGHT = 17;
    static constexpr byte TEXTSCROLLUP = 18;
    static constexpr byte ROAMINGPIXEL = 19;
    static constexpr byte HORIZONTALSCANLINE = 20;
    static constexpr byte VERTICALSCANLINE = 21;
    static constexpr byte FIRE = 22;
    static constexpr byte PSICOLORWIPE = 23;
    static constexpr byte PULSE = 24;
    static constexpr byte RANDOM = 99;
    // static constexpr byte TESTROW = 90;
    // static constexpr byte TESTCOL = 91;

    enum ColorVal
    {
        kRed                = 1,
        kOrange             = 2,
        kYellow             = 3,
        kGreen              = 4,
        kCyan               = 5,
        kBlue               = 6,
        kPurple             = 7,
        kMagenta            = 8,
        kPink               = 9,
        kDefault            = 0
    };

    /**
      * Calculate sequence value given four parameters
      */
    static long sequence(byte seq, ColorVal colorVal = kDefault, uint8_t speedScale = 0, uint8_t numSeconds = 0)
    {
        return(
            (long int)seq * 10000L +
            (long int)colorVal * 1000L +
            (long int)speedScale * 100 +
            numSeconds);
    }
};

/** \ingroup Dome
 *
 * \class LogicEngineSettings
 *
 * \brief Current settings for LogicEngine hardware
 */
class LogicEngineSettings
{
public:
    LogicEngineSettings() {}
    LogicEngineSettings(
        byte fade,
        byte hue,
        byte delay,
        byte palNum,
        byte bri,
        long defaultEffect = 0) :
            fFade(fade),
            fHue(hue),
            fDelay(delay),
            fPalNum(palNum),
            fBri(bri),
            fDefaultEffect(defaultEffect)
    {
    }

    byte fFade;
    byte fHue;
    byte fDelay;
    byte fPalNum;
    byte fBri;
    long fDefaultEffect;
};

///////////////////////////////////////////////////////////////////////////////////////////
//
// HARDWARE PCB DEFINITIONS
//
///////////////////////////////////////////////////////////////////////////////////////////

#if USE_LEDLIB == 0
/// \private
template <template<uint8_t DATA_PIN, EOrder RGB_ORDER> class CHIPSET, uint8_t DATA_PIN,
    unsigned _count = 1, unsigned _start = 0, unsigned _end = 1, unsigned _width = 1, unsigned _height = 1>
class FastLEDPCB
#elif USE_LEDLIB == 1
/// \private
template <LEDChipset CHIPSET, uint8_t DATA_PIN,
    unsigned _count = 1, unsigned _start = 0, unsigned _end = 1, unsigned _width = 1, unsigned _height = 1>
class FastLEDPCB : private Adafruit_NeoPixel
#else
 #error Unsupported
#endif
{
public:
    static const int count = _count;
    static const int start = _start;
    static const int end = _end;
    static const int width = _width;
    static const int height = _height;

    void init()
    {
    #if USE_LEDLIB == 0
        FastLED.addLeds<CHIPSET, DATA_PIN, GRB>(fLED, count);
        fill_solid(fLED, _count, CRGB(0,0,0));
    #elif USE_LEDLIB == 1
        // Avoid call to malloc()
        updateType(CHIPSET);
        numBytes = count * 3;
        pixels = (uint8_t*)&fLED;
        memset(fLED, '\0', sizeof(fLED));
        setPin(DATA_PIN);
        TEENSY_PROP_NEOPIXEL_SETUP()
    #else
        #error Not supported
    #endif
    }

#if USE_LEDLIB == 1
    void show()
    {
        TEENSY_PROP_NEOPIXEL_BEGIN()
        Adafruit_NeoPixel::show();
        TEENSY_PROP_NEOPIXEL_END()
    }
#endif

    CRGB fLED[count];
    LEDStatus fLEDStatus[count];
};

#if USE_LEDLIB == 0
/// \private
template <ESPIChipsets CHIPSET, uint8_t DATA_PIN, uint8_t CLOCK_PIN, EOrder RGB_ORDER,
    unsigned _count, unsigned _start, unsigned _end, unsigned _width, unsigned _height>
class FastLEDPCBClock
{
public:
    static const int count = _count;
    static const int start = _start;
    static const int end = _end;
    static const int width = _width;
    static const int height = _height;

    void init()
    {
        FastLED.addLeds<CHIPSET, DATA_PIN, CLOCK_PIN, RGB_ORDER>(fLED, count);
        fill_solid(fLED, _count, CRGB(0,0,0));
    }

    CRGB fLED[count];
    LEDStatus fLEDStatus[count];
};
#endif

/// \private
template <uint8_t DATA_PIN = FRONT_LOGIC_PIN>
class LogicEngineFLDPCB0 : public FastLEDPCB<WS2812B, DATA_PIN, 96, 8, 88, 8, 10>
{
public:
    static inline const byte* getLEDMap()
    {
        //Originals (with Naboo logo on backs of Front and Rear Logic)
        //mapping for FLD boards from first run (with 48 LEDs per PCB)
        static const byte sLEDmap[] PROGMEM =
        {
            15,14,13,12,11,10, 9, 8,
            16,17,18,19,20,21,22,23,
            31,30,29,28,27,26,25,24,
            32,33,34,35,36,37,38,39,
            47,46,45,44,43,42,41,40,
            88,89,90,91,92,93,94,95,
            87,86,85,84,83,82,81,80,
            72,73,74,75,76,77,78,79,
            71,70,69,68,67,66,65,64,
            56,57,58,59,60,61,62,63
        };
        return sLEDmap;
    }
};

/// \private
template <uint8_t DATA_PIN = FRONT_LOGIC_PIN>
class LogicEngineFLDPCB1 : public FastLEDPCB<WS2812B, DATA_PIN, 80, 0, 80, 8, 10>
{
public:
    static inline const byte* getLEDMap()
    {
        //2014 Version (with Kenny & McQuarry art on Rear, C3PO on Fronts)
        //mapping for newer FLD PCBs (40 LEDs per PCB, lower FLD upside-down)...
        static const byte sLEDmap[] PROGMEM =
        {
             0, 1, 2, 3, 4, 5, 6, 7,
            15,14,13,12,11,10, 9, 8,
            16,17,18,19,20,21,22,23,
            31,30,29,28,27,26,25,24,
            32,33,34,35,36,37,38,39,
            79,78,77,76,75,74,73,72,
            64,65,66,67,68,69,70,71,
            63,62,61,60,59,58,57,56,
            48,49,50,51,52,53,54,55,
            47,46,45,44,43,42,41,40
        };
        return sLEDmap;
    }
};

/// \private
template <uint8_t DATA_PIN = FRONT_LOGIC_PIN>
class LogicEngineFLDPCB2 : public FastLEDPCB<SK6812, DATA_PIN, 80, 0, 80, 8, 10>
{
public:
    static inline const byte* getLEDMap()
    {
        //2014 Version (with Kenny & McQuarry art on Rear, C3PO on Fronts)
        //mapping for newer FLD PCBs (40 LEDs per PCB, lower FLD upside-down)...
        static const byte sLEDmap[] PROGMEM =
        {
             0, 1, 2, 3, 4, 5, 6, 7,
            15,14,13,12,11,10, 9, 8,
            16,17,18,19,20,21,22,23,
            31,30,29,28,27,26,25,24,
            32,33,34,35,36,37,38,39,
            79,78,77,76,75,74,73,72,
            64,65,66,67,68,69,70,71,
            63,62,61,60,59,58,57,56,
            48,49,50,51,52,53,54,55,
            47,46,45,44,43,42,41,40
        };
        return sLEDmap;
    }
};

/// \private
template <uint8_t DATA_PIN = FRONT_LOGIC_PIN>
class LogicEngineFLDPCB2Inverted : public FastLEDPCB<SK6812/*SK6812CUSTOM*/, DATA_PIN, 80, 0, 80, 8, 10>
{
public:
    static inline const byte* getLEDMap()
    {
        //2014 Version (with Kenny & McQuarry art on Rear, C3PO on Fronts)
        //mapping for newer FLD PCBs (40 LEDs per PCB, lower FLD upside-down)...
        static const byte sLEDmap[] PROGMEM =
        {
            40,41,42,43,44,45,46,47,
            55,54,53,52,51,50,49,48,
            56,57,58,59,60,61,62,63,
            71,70,69,68,67,66,65,64,
            72,73,74,75,76,77,78,79,
            39,38,37,36,35,34,33,32,
            24,25,26,27,28,29,30,31,
            23,22,21,20,19,18,17,16,
             8, 9,10,11,12,13,14,15,
             7, 6, 5, 4, 3, 2, 1, 0
        };
        return sLEDmap;
    }
};

/// \private
template <uint8_t DATA_PIN = FRONT_LOGIC_PIN>
class AstroPixelFLDPCB0 : public FastLEDPCB<WS2812B, DATA_PIN, 90, 0, 90, 9, 10>
{
public:
    static inline const byte* getLEDMap()
    {
        // 2022 AstroPixel boards by Darren Poulson
        // Neopixel FLD boards, First Release. 9x5 LEDs per board
        static const byte sLEDmap[] PROGMEM =
        {
             0, 1, 2, 3, 4, 5, 6, 7, 8,
            17,16,15,14,13,12,11,10, 9,
            18,19,20,21,22,23,24,25,26,
            35,34,33,32,31,30,29,28,27,
            36,37,38,39,40,41,42,43,44,
            45,46,47,48,49,50,51,52,53,
            62,61,60,59,58,57,56,55,54,
            63,64,65,66,67,68,69,70,71,
            80,79,78,77,76,75,74,73,72,
            81,82,83,84,85,86,87,88,89
        };
        return sLEDmap;
    }
};
/// \private
template <uint8_t DATA_PIN = REAR_LOGIC_PIN>
class LogicEngineRLDPCB0 : public FastLEDPCB<WS2812B, DATA_PIN, 80, 0, 80, 16, 4>
{
public:
    static inline const byte* getLEDMap()
    {
        //Originals (with Naboo logo on backs of Front and Rear Logic)
        //mapping for first RLD (two PCBs soldered together)
        static const byte sLEDmap[] PROGMEM =
        {
             0, 1, 2, 3, 4, 5, 6, 7,  48,49,50,51,52,53,54,55,
            15,14,13,12,11,10, 9, 8,  63,62,61,60,59,58,57,56,
            16,17,18,19,20,21,22,23,  64,65,66,67,68,69,70,71,
            31,30,29,28,27,26,25,24,  79,78,77,76,75,74,73,72,
            32,33,34,35,36,37,38,39,  80,81,82,83,84,85,86,87,
            47,46,45,44,43,42,41,40,  95,94,93,92,91,90,89,88
        };
        return sLEDmap;
    }
};

/// \private
template <uint8_t DATA_PIN = REAR_LOGIC_PIN>
class LogicEngineRLDPCBSUPER : public FastLEDPCB<WS2812B, DATA_PIN, 256, 0, 255, 32, 8>
{
public:
    static inline const byte* getLEDMap()
    {
        static const byte sLEDmap[] PROGMEM =
        {
              0, 15, 16, 31, 32, 47, 48, 63, 64, 79, 80, 95, 96,111,112,127,128,143,144,159,160,175,176,191,192,207,208,223,224,239,240,255,
              1, 14, 17, 30, 33, 46, 49, 62, 65, 78, 81, 94, 97,110,113,126,129,142,145,158,161,174,177,190,193,206,209,222,225,238,241,254,
              2, 13, 18, 29, 34, 45, 50, 61, 66, 77, 82, 93, 98,109,114,125,130,141,146,157,162,173,178,189,194,205,210,221,226,237,242,253,
              3, 12, 19, 28, 35, 44, 51, 60, 67, 76, 83, 92, 99,108,115,124,131,140,147,156,163,172,179,188,195,204,211,220,227,236,243,252,
              4, 11, 20, 27, 36, 43, 52, 59, 68, 75, 84, 91,100,107,116,123,132,139,148,155,164,171,180,187,196,203,212,219,228,235,244,251,
              5, 10, 21, 26, 37, 42, 53, 58, 69, 74, 85, 90,101,106,117,122,133,138,149,154,165,170,181,186,197,202,213,218,229,234,245,250,
              6,  9, 22, 25, 38, 41, 54, 57, 70, 73, 86, 89,102,105,118,121,134,137,150,153,166,169,182,185,198,201,214,217,230,233,246,249,
              7,  8, 23, 24, 39, 40, 55, 56, 71, 72, 87, 88,103,104,119,120,135,136,151,152,167,168,183,184,199,200,215,216,231,232,247,248
        };
        return sLEDmap;
    }
};

/// \private
template <uint8_t DATA_PIN = REAR_LOGIC_PIN>
class LogicEngineRLDPCB1 : public FastLEDPCB<WS2812B, DATA_PIN, 96, 0, 96, 24, 4>
{
public:
    static inline const byte* getLEDMap()
    {
        //2014 Version (with Kenny & McQuarry art on Rear, C3PO on Fronts)
        static const byte sLEDmap[] PROGMEM =
        {
            0, 1,  2,28, 3,27, 4,26, 5,25, 6, 7, 8, 9,22,10,21,11,20,12,19,13,14,15,
            31,30,29,32,33,34,35,36,37,38,39,24,23,40,41,42,43,44,45,46,47,18,17,16,
            64,65,66,63,62,61,60,59,58,57,56,71,72,55,54,53,52,51,50,49,48,77,78,79,
            95,94,93,67,92,68,91,69,90,70,89,88,87,86,73,85,74,84,75,83,76,82,81,80
        };
        return sLEDmap;
    }
};

/// \private
template <uint8_t DATA_PIN = REAR_LOGIC_PIN>
class LogicEngineRLDPCB2 : public FastLEDPCB<SK6812, DATA_PIN, 96, 0, 96, 24, 4>
{
public:
    static inline const byte* getLEDMap()
    {
        // 2016 Version (with Deathstar plans on back of Rear Logic)
        static const byte sLEDmap[] PROGMEM =
        {
             0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,
            47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,
            48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,
            95,94,93,92,91,90,89,88,87,86,85,84,83,82,81,80,79,78,77,76,75,74,73,72
        };
        return sLEDmap;
    }
};

/// \private
template <uint8_t DATA_PIN = REAR_LOGIC_PIN>
class LogicEngineRLDPCB2Inverted : public FastLEDPCB<SK6812, DATA_PIN, 96, 0, 96, 24, 4>
{
public:
    static inline const byte* getLEDMap()
    {
        // 2016 Version (with Deathstar plans on back of Rear Logic)
        //oops installed upside down
        static const byte sLEDmap[] PROGMEM =
        {
            72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
            71,70,69,68,67,66,65,64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
            24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
            23,22,21,20,19,18,17,16,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
        };
        return sLEDmap;
    }
};

/// \private
template <uint8_t DATA_PIN = REAR_LOGIC_PIN>
class AstroPixelRLDPCB0 : public FastLEDPCB<WS2812B, DATA_PIN, 108, 0, 108, 27, 4>
{
public:
    static inline const byte* getLEDMap()
    {
        // 2022 AstroPixel boards by Darren Poulson
        static const byte sLEDmap[] PROGMEM =
        {
             0,  1,  2,  3,  4,  5,  6,  7,  8, 9, 10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,
             53, 52, 51, 50, 49, 48, 47, 46, 45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,
             54, 55, 56, 57, 58, 59, 60, 61, 62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,
             107,106,105,104,103,102,101,100,99,98,97,96,95,94,93,92,91,90,89,88,87,86,85,84,83,82,81	
        };
        return sLEDmap;
    }
};

#if USE_LEDLIB == 0
/// \private
template <uint8_t DATA_PIN = REAR_LOGIC_PIN, uint8_t CLOCK_PIN = REAR_LOGIC_CLOCK_PIN>
class LogicEngineRLDPCB3 : public FastLEDPCBClock<SK9822, DATA_PIN, CLOCK_PIN, BGR, 112, 0, 112, 28, 4>
{
public:
    static inline const byte* getLEDMap()
    {
        // 2020 Version (with Grant Imahara on back of Rear Logic)
        static const byte sLEDmap[] PROGMEM =
        {
              0,  1,  2,  3,  4,  5,  6,    7,  8,  9, 10, 11, 12, 13,   14, 15, 16, 17, 18, 19, 20,   21, 22, 23, 24, 25, 26, 27,
             55, 54, 53, 52, 51, 50, 49,   48, 47, 46, 45, 44, 43, 42,   41, 40, 39, 38, 37, 36, 35,   34, 33, 32, 31, 30, 29, 28,
             56, 57, 58, 59, 60, 61, 62,   63, 64, 65, 66, 67, 68, 69,   70, 71, 72, 73, 74, 75, 76,   77, 78, 79, 80, 81, 82, 83,
            111,110,109,108,107,106,105,  104,103,102,101,100, 99, 98,   97, 96, 95, 94, 93, 92, 91,   90, 89, 88, 87, 86, 85, 84
        };
        return sLEDmap;
    }
};

/// \private
template <uint8_t DATA_PIN = REAR_LOGIC_PIN, uint8_t CLOCK_PIN = REAR_LOGIC_CLOCK_PIN>
class LogicEngineRLDPCB3Inverted : public FastLEDPCBClock<SK9822, DATA_PIN, CLOCK_PIN, BGR, 112, 0, 112, 28, 4>
{
public:
    static inline const byte* getLEDMap()
    {
        // 2020 Version (with Grant Imahara on back of Rear Logic)
        //oops installed upside down
        static const byte sLEDmap[] PROGMEM =
        {
             84, 85, 86, 87, 88, 89, 90,   91, 92, 93, 94, 95, 96, 97,   98, 99,100,101,102,103,104,  105,106,107,108,109,110,111,
             83, 82, 81, 80, 79, 78, 77,   76, 75, 74, 73, 72, 71, 70,   69, 68, 67, 66, 65, 64, 63,   62, 61, 60, 59, 58, 57, 56,
             28, 29, 30, 31, 32, 33, 34,   35, 36, 37, 38, 39, 40, 41,   42, 43, 44, 45, 46, 47, 48,   49, 50, 51, 52, 53, 54, 55,
             27, 26, 25, 24, 23, 22, 21,   20, 19, 18, 17, 16, 15, 14,   13, 12, 11, 10,  9,  8,  7,    6,  5,  4,  3,  2,  1,  0
        };
        return sLEDmap;
    }
};
#endif

class LogicEffectObject
{
public:
    virtual ~LogicEffectObject() {}
};

/** \ingroup Dome
 *
 * \class LogicEngineRenderer
 *
 * \brief Base class renderer for both front and rear RSeries logics.
 *
 * Base class renderer for all animation on RSeries front and rear logics.
 *
 * For further information on the hardware:
 *
 *  https://www.youtube.com/watch?v=CbYfDwH_mig
 *
 *  https://astromech.net/forums/showthread.php?30271-RSeries-Logic-Engine-dome-lighting-kits-230-(Nov-2016)-Open
 */
class LogicEngineRenderer : public LogicEngineDefaults, SetupEvent, AnimatedEvent, CommandEvent, JawaEvent
{
public:
    typedef bool (*LogicEffect)(LogicEngineRenderer& renderer);
    typedef LogicEffect (*LogicEffectSelector)(unsigned effectVal);
    typedef const char* (*LogicMessageSelector)(unsigned index);
    typedef PROGMEMString (*LogicPMessageSelector)(unsigned index);
    typedef byte (*LogicRenderGlyph)(char ch, byte fontNum, const CRGB fontColors[], int x, int y, CRGB* leds, const byte* ledMap, int w, int h, byte* glyphHeight);

    ColorVal randomColor()
    {
        return ColorVal(random(10));
    }

    void selectEffect(long inputNum)
    {
        fDisplayEffectVal = inputNum;
    }

    inline void selectSequence(byte seq, ColorVal colorVal = kDefault, uint8_t speedScale = 0, uint8_t numSeconds = 0)
    {
        selectEffect(sequence(seq, colorVal, speedScale, numSeconds));
    }

    inline void selectTextCenter(const char* text, ColorVal colorVal = kDefault, uint8_t speedScale = 0, uint8_t numSeconds = 0)
    {
        setTextMessage(text);
        selectEffect(sequence(TEXT, colorVal, speedScale, numSeconds));
    }

    inline void selectScrollTextLeft(const char* text, ColorVal colorVal = kDefault, uint8_t speedScale = 0, uint8_t numSeconds = 0)
    {
        setTextMessage(text);
        selectEffect(sequence(TEXTSCROLLLEFT, colorVal, speedScale, numSeconds));
    }

    inline void selectScrollTextRight(const char* text, ColorVal colorVal = kDefault, uint8_t speedScale = 0, uint8_t numSeconds = 0)
    {
        setTextMessage(text);
        selectEffect(sequence(TEXTSCROLLRIGHT, colorVal, speedScale, numSeconds));
    }

    inline void selectScrollTextUp(const char* text, ColorVal colorVal = kDefault, uint8_t speedScale = 0, uint8_t numSeconds = 0)
    {
        setTextMessage(text);
        selectEffect(sequence(TEXTSCROLLUP, colorVal, speedScale, numSeconds));
    }

    virtual void handleCommand(const char* cmd)
    {
        int length = strlen(cmd);
        if (*cmd++ == 'L' && *cmd++ == 'E')
        {
            long int cmdvalue = 0;
            const char* c = cmd;
            if (length >= 9) 
            {
                // Command has id. 
                unsigned reqId = *c++ - '0';
                if (reqId != getID()) {
                    return;
                } 
            } 
            while (*c >= '0' && *c <= '9')
            {
                cmdvalue = cmdvalue * 10 + (*c++ - '0');
            }
            selectEffect(cmdvalue);
        }
    }

    virtual void setup() override
    {
        restoreSettings();
        calculateAllColors(fSettings.fPalNum, fSettings.fBri);
        for (unsigned i = 0; i < count(); i++)
        {
            unsigned index = mapLED(i);
            LEDStatus* ledStatus = &fLEDStatus[index];
            ledStatus->fColorNum = random8(fTotalColorsWBIZ);
            ledStatus->fColorPause = random8();
            updateMappedLED(index, fSettings.fHue);
        }
        fStatusMillis = millis();
        fDisplayEffectVal = fSettings.fDefaultEffect;
    }

    virtual void animate() override
    {
        uint32_t currentMillis = millis();
        if (fLastMillis + 10L > currentMillis)
            return;
        fLastMillis = currentMillis;
        if (currentMillis - fStatusMillis >= fStatusDelay)
        {
            fStatusMillis = currentMillis;
            fFlipFlop = !fFlipFlop;
        }
        int selectSequence = (fDisplayEffectVal % 100000000L) / 10000L;
        int selectLength = (fDisplayEffectVal % 100);

        // byte peakVal = fPeakSource->getPeakValue();
        if (hasEffectChanged())
        {
            setEffectObject(NULL);
            restoreSettings();
            calculateAllColors(fSettings.fPalNum, fSettings.fBri);
            fStatusDelay = getDefaultEffectDelay();
            fEffectStartMillis = currentMillis;
            if (fEffectSelector != NULL)
            {
                fLogicEffect = fEffectSelector(selectSequence);
            }
        }
        fEffectMillis = currentMillis - fEffectStartMillis;

        bool continueEffect = (fLogicEffect != NULL) ? fLogicEffect(*this) : false;
        fPreviousEffectVal = fDisplayEffectVal;
        if (!continueEffect || (selectLength > 0 && unsigned(selectLength) * 1000L < fEffectMillis))
        {
            if (selectSequence == RANDOM)
            {
                // Select next random sequence
                fPreviousEffectVal = ~fDisplayEffectVal;
            }
            else
            {
                selectEffect(fSettings.fDefaultEffect); //go back to normal operation if its time
            }
        }
        fPreviousEffect = fDisplayEffect;
    #if USE_LEDLIB == 0
        AnimatedEvent::setLoopDoneCallback([]() { FastLED.show(); });
    #elif USE_LEDLIB == 1
        show();
    #else
        #error Unsupported
    #endif
    }

    inline bool hasEffectChanged()
    {
        return (fPreviousEffectVal != fDisplayEffectVal);
    }

    inline void resetEffect() {
            fPreviousEffectVal = ~fDisplayEffectVal;
    }

    inline bool hasEffectChangedType()
    {
        return (fPreviousEffect != fDisplayEffect);
    }

    inline unsigned getID() const
    {
        return fID;
    }

    static int getNextID()
    {
        static int sID;
        return ++sID;
    }

    inline int getEffectColor()
    {
        int selectColor = (fDisplayEffectVal % 10000) / 1000;
        return selectColor;
    }

    inline int getEffectHue()
    {
        return mapSelectColorToHue(getEffectColor());
    }

    inline int getEffectSpeed()
    {
        int selectSpeed = (fDisplayEffectVal % 1000) / 100;
        return selectSpeed;
    }

    inline int getEffectLength()
    {
        int selectLength = (fDisplayEffectVal % 100);
        return selectLength;
    }

    inline int getEffectTextMsg()
    {
        int selectTextMsg = (fDisplayEffectVal % 100000000) / 10000000;
        return selectTextMsg;
    }

    inline unsigned getEffectDuration()
    {
        return fEffectMillis;
    }

    inline byte getHue()
    {
        return fSettings.fHue;
    }

    inline void setHue(byte hue)
    {
        fSettings.fHue = hue;
    }

    inline byte getFade()
    {
        return fSettings.fFade;
    }

    inline void setFade(byte fade)
    {
        fSettings.fFade = fade;
    }

    inline byte getDelay()
    {
        return fSettings.fDelay;
    }

    inline void setDelay(byte delay)
    {
        fSettings.fDelay = delay;
    }

    inline byte getBrightness()
    {
        return fSettings.fBri;
    }

    inline void setBrightness(byte bri)
    {
        fSettings.fBri = bri;
    }

    inline bool getEffectFlip()
    {
        return fFlipFlop;
    }

    inline void setEffectFlip(bool flip)
    {
        fFlipFlop = flip;
    }

    inline LogicEffectObject* getEffectObject()
    {
        return fEffectObject;
    }

    inline uint32_t getEffectData()
    {
        return fEffectData;
    }

    inline void setEffectObject(LogicEffectObject* obj)
    {
        if (fEffectObject != NULL)
            delete fEffectObject;
        fEffectObject = obj;
    }

    inline void setEffectData(uint32_t data)
    {
        fEffectData = data;
    }

    inline uint32_t getEffectData2()
    {
        return fEffectData2;
    }

    inline void setEffectData2(uint32_t data)
    {
        fEffectData2 = data;
    }

    inline void setEffectWidthRange(float percent)
    {
        fEffectRange = max(min(1.0f, percent), 0.0f);
    }

    inline int getEffectMsgWidth()
    {
        return fEffectMsgWidth;
    }

    inline int getEffectMsgHeight()
    {
        return fEffectMsgHeight;
    }

    inline byte getPeakValue()
    {
        return (fPeakSource != NULL) ? fPeakSource->getPeakValue() : 0;
    }

    static constexpr uint32_t getDefaultEffectDelay()
    {
        return 1500;
    }

    inline void setEffectDelay(uint32_t effectDelay)
    {
        fStatusDelay = effectDelay;
    }

    inline int width() const
    {
        return fWidth;
    }

    inline int height() const
    {
        return fHeight;
    }

    inline unsigned count() const
    {
        return fLEDCount;
    }

    inline unsigned mapLED(unsigned index)
    {
        return pgm_read_byte(&fLEDMap[index]);
    }

    void setLogicEffectSelector(LogicEffectSelector selector)
    {
        fEffectSelector = selector;
    }

    void setMessageSelector(LogicMessageSelector selector)
    {
        fMessageSelector = selector;
    }

    void setPMessageSelector(LogicPMessageSelector selector)
    {
        fPMessageSelector = selector;
    }

    void setPeakValueProvider(PeakValueProvider& provider)
    {
        fPeakSource = &provider;
    };

    void set(unsigned index, const struct CRGB& val)
    {
        fLED[pgm_read_byte(&fLEDMap[fLEDStart + index])] = val;
    }

    void setHSV(unsigned index, uint8_t hue, uint8_t sat, uint8_t val)
    {
        fLED[pgm_read_byte(&fLEDMap[fLEDStart + index])].setHSV(hue, sat, val);
    }

    void restoreSettings()
    {
        defaultSettings();
    }

    void changePalette()
    {
        if (++fSettings.fPalNum == PAL_COUNT)
            fSettings.fPalNum = 0;
        calculateAllColors(fSettings.fPalNum, fSettings.fBri);
    }

    unsigned getCurrentPalette()
    {
        return fSettings.fPalNum;
    }

    void setPaletteHue(byte palNum, byte hue)
    {
        fSettings.fPalNum = palNum;
        fSettings.fHue = hue;
        calculateAllColors(fSettings.fPalNum, fSettings.fBri);
    }

    void clearBlockedPortion()
    {
        int blackX = int(fEffectRange * width());
        if (blackX < width())
        {
            CRGB blackColor = { 0, 0, 0 };
            for (int x = blackX; x < width(); x++)
            {
                for (int y = 0; y < height(); y++)
                {
                    fLED[pgm_read_byte(&fLEDMap[y * width() + x])] = blackColor;
                }
            }
        }
    }

    void updateDisplay()
    {
        updateDisplay(fSettings.fBri);
    }

    void updateDisplayPeak()
    {
        updateDisplay((fPeakSource != NULL) ? fPeakSource->getPeakValue() : fSettings.fBri);
    }

    void updateDisplay(byte bri)
    {
        for (unsigned i = fLEDStart; i < fLEDEnd; i++)
            updateMappedLED(mapLED(i), fSettings.fHue, bri); 
        clearBlockedPortion();
    }

    void updateDisplaySplitHalf(byte topBri, byte bottomBri)
    {
        for (unsigned i = fLEDStart; i < count() / 2; i++)
            updateMappedLED(mapLED(i), fSettings.fHue, topBri);
        for (unsigned i = count() / 2; i < fLEDEnd; i++)
            updateMappedLED(mapLED(i), fSettings.fHue, bottomBri);
        clearBlockedPortion();
    }

    void updateDisplaySplitRowThirds(byte topBri, byte bottomBri)
    {
        if (width() % 3 == 0)
        {
            /* row splits evenly into thirds */
            bool flip = false;
            unsigned index = fLEDStart;
            unsigned third = width() / 3;
            for (byte row = 0; row < height() * 3; row++)
            {
                for (byte col = 0; col < third && index < fLEDEnd; col++, index++)
                    updateMappedLED(mapLED(index), fSettings.fHue, (!flip) ? topBri : bottomBri);
                flip = !flip;
            }
        }
        else
        {
            /* split half instead */
            for (unsigned i = fLEDStart; i < count() / 2; i++)
                updateMappedLED(mapLED(i), fSettings.fHue, topBri);
            for (unsigned i = count() / 2; i < fLEDEnd; i++)
                updateMappedLED(mapLED(i), fSettings.fHue, bottomBri);
        }
        clearBlockedPortion();
    }

    unsigned measureText(const char* txt, int &outWidth, int &outHeight)
    {
        char ch;
        int len = 0;
        int textWidth = 0;
        byte glyphHeight = 0;
        outWidth = 0;
        outHeight = 0;
        fRenderGlyph(' ', fEffectFontNum, NULL, 0, 0, NULL, NULL, 0, 0, &glyphHeight);
        while ((ch = *txt++) != '\0')
        {
            len++;
            if (ch == '\n')
            {
                outWidth = max(outWidth, textWidth);
                outHeight += glyphHeight;
                textWidth = 0;
                continue;
            }
            textWidth += fRenderGlyph(ch, fEffectFontNum, NULL, 0, 0, NULL, NULL, 0, 0, NULL);
        }
        outWidth = max(outWidth, textWidth);
        outHeight = (outWidth > 0) ? outHeight + glyphHeight : 0;
        return len;
    }

    unsigned measureText(PROGMEMString ptxt, int &outWidth, int &outHeight)
    {
        char ch;
        int len = 0;
        int textWidth = 0;
        byte glyphHeight = 0;
        const char* txt = (const char*)ptxt;
        outWidth = 0;
        outHeight = 0;
        fRenderGlyph(' ', fEffectFontNum, NULL, 0, 0, NULL, NULL, 0, 0, &glyphHeight);
        while ((ch = pgm_read_byte(txt++)) != '\0')
        {
            len++;
            if (ch == '\n')
            {
                outWidth = max(outWidth, textWidth);
                outHeight += glyphHeight;
                textWidth = 0;
                continue;
            }
            textWidth += fRenderGlyph(ch, fEffectFontNum, NULL, 0, 0, NULL, NULL, 0, 0, NULL);
        }
        outWidth = max(outWidth, textWidth);
        outHeight = (outWidth > 0) ? outHeight + glyphHeight : 0;
        return len;
    }

    inline byte getEffectFontNum()
    {
        return fEffectFontNum;
    }

    inline void setEffectFontNum(byte fontNum)
    {
        fEffectFontNum = fontNum;
        /* Recalculate lengths if text is set */
        if (fEffectMsgText != NULL)
        {
            fEffectMsgLen = measureText(fEffectMsgText, fEffectMsgWidth, fEffectMsgHeight);
        }
        else if (fEffectMsgTextP != NULL)
        {
            fEffectMsgLen = measureText(fEffectMsgTextP, fEffectMsgWidth, fEffectMsgHeight);
        }
    }

    inline void setTextMessage(const char* msg)
    {
        fEffectMsgLen = fEffectMsgWidth = 0;
        fEffectMsgText = msg;
        fEffectMsgTextP = NULL;
        if (fEffectMsgText != NULL)
            fEffectMsgLen = measureText(fEffectMsgText, fEffectMsgWidth, fEffectMsgHeight);
    }

    void setupTextMessage(int selectTextMsg)
    {
        fEffectMsgLen = fEffectMsgWidth = 0;
        if (selectTextMsg != 0)
        {
            /* Look for hard coded message */
            fEffectMsgTextP = (fPMessageSelector != NULL) ? fPMessageSelector(selectTextMsg) : NULL;
            if (fEffectMsgTextP == NULL)
                fEffectMsgText = (fMessageSelector != NULL) ? fMessageSelector(selectTextMsg) : NULL;
            if (fEffectMsgTextP == NULL && fEffectMsgText == NULL)
                fEffectMsgTextP = F("STAR WARS");
        }
        if (fEffectMsgText != NULL)
        {
            fEffectMsgLen = measureText(fEffectMsgText, fEffectMsgWidth, fEffectMsgHeight);
        }
        else if (fEffectMsgTextP != NULL)
        {
            fEffectMsgLen = measureText(fEffectMsgTextP, fEffectMsgWidth, fEffectMsgHeight);
        }
    }

    void clear()
    {
    #if USE_LEDLIB == 0
        fill_solid(fLED, count(), CRGB(0,0,0));
    #elif USE_LEDLIB == 1
        int numToFill = count();
        CRGB* leds = fLED;
        CRGB color = CRGB(0, 0, 0);
        for (int i = 0; i < numToFill; i++)
            *leds++ = color;
    #endif
    }

    void renderText(int x, int y, byte effectHue)
    {
        CRGB fontColors[3];

        byte hue = fAllColors[0].h + effectHue;
        byte sat = fAllColors[0].s;
        fontColors[0].setHSV(hue, sat, 1);  /* dimmest */
        fontColors[1].setHSV(hue, sat, 16);
        fontColors[2].setHSV(hue, sat, 64); /* brightest */
        clear();
        int startx = x;
        for (int i = 0; i < fEffectMsgLen; i++)
        {
            char ch = 0;
            if (fEffectMsgText != NULL)
                ch = fEffectMsgText[i];
            if (fEffectMsgTextP != NULL)
                ch = pgm_read_byte(&((const char*)fEffectMsgTextP)[i]);
            if (ch == '\n')
            {
                x = startx;
                y += 5;
            }
            else if (x < width())
            {
                int adv = fRenderGlyph(ch, fEffectFontNum, fontColors, x, y, fLED, fLEDMap, fWidth, fHeight, NULL);
                x += adv;
            }
            if (y >= height())
                break;
        }
    }

    inline LEDStatus* getUnmappedLEDStatus()
    {
        return fLEDStatus;
    }

    inline CRGB* getUnmappedLEDs()
    {
        return fLED;
    }

    void setPixel(int x, int y, byte effectHue, byte bri)
    {
        if (unsigned(x) < unsigned(fEffectRange * width()) && unsigned(y) < unsigned(height()))
        {
            fLED[pgm_read_byte(&fLEDMap[y * width() + x])].setHSV(
                fAllColors[0].h + effectHue, fAllColors[0].s, bri);
        }
    }

    void setPixelRGB(int x, int y, const struct CRGB& val)
    {
        if (unsigned(x) < unsigned(fEffectRange * width()) && unsigned(y) < unsigned(height()))
        {
            fLED[pgm_read_byte(&fLEDMap[y * width() + x])] = val;
        }
    }

    void setPixelRGB(int x, int y, uint8_t r, uint8_t g, uint8_t b)
    {
        CRGB color;
        color.r = (r / 255.0) * MAX_BRIGHTNESS / 2;
        color.g = (g / 255.0) * MAX_BRIGHTNESS / 2;
        color.b = (b / 255.0) * MAX_BRIGHTNESS / 2;
        setPixelRGB(x, y, color);
    }

    //function to calculate all colors based on the chosen colorPalNum
    //this will get called during setup(), and also if we've remotely commanded to change color palettes
    //NEW: added a "bright" parameter, to allow front and rear brightness to be adjusted independantly 
    void calculateAllColors(byte colorPalNum, byte brightVal)
    {
        //we define both all palettes, in case the user wants to try out rear colors on the front etc
        // note that these are not RGB colors, they're HSV
        // for help calculating HSV color values see http://joymonkey.com/logic/
        static const HSVColor keyColors[PAL_COUNT][4] PROGMEM = {
          { {170, 255,   0} , {170, 255,  85} , {170, 255, 170} , {170,   0, 170}  } , //front colors
          { { 90, 235,   0} , { 75, 255, 250} , { 30, 255, 184} , {  0, 255, 250}  } , //rear colors (hues: 87=bright green, 79=yellow green, 45=orangey yellow, 0=red)
          { {  0, 255,   0} , {  0, 255,   0} , {  0, 255, 100} , {  0, 255, 250}  } , //monotone (black to solid red)
          { {  0, 255,   0} , {  0, 255, 250} , { 40, 255,   0} , { 40, 255, 250}  } , //dual color red and yellow
          { {165, 50,  248} , {166, 181, 226} , {165, 223,  89} , {255, 255, 214}  } ,  //blue and red
          { {87, 206,  105} , { 79, 255, 214} , { 43, 255, 250} , { 25, 255, 214}  }    //yellow and green
        };
        //take a set of 4 key colors from keyColors[3][4][3] and generate 16 colors
        // 328P will only have one set of full colors, Teensy will have two
        for (byte kcol = 0; kcol < 4; kcol++)
        {
            //go through each Key color
            const HSVColor* wc = &keyColors[colorPalNum][kcol];
            HSVColor workColor = {
                pgm_read_byte(&wc->h),
                pgm_read_byte(&wc->s),
                pgm_read_byte(&wc->v)
            };
            byte tnum = kcol + fTweens * kcol;
            HSVColor* tc = &fAllColors[tnum];
            tc->h = workColor.h;
            tc->s = workColor.s;
            //Value (V) is adjusted down to whatever brightness setting we've specified
            brightVal = min(brightVal, (byte)MAX_BRIGHTNESS);
            tc->v = map8(workColor.v, MIN_BRIGHTNESS, brightVal);
            if (tnum + 1 != fTotalColors)
            {
                const HSVColor* hsvptr = &keyColors[colorPalNum][kcol + 1];
                for (byte el = 0; el < 3; el++)
                {
                    //loop through H, S and V from this key to the next
                    int perStep = int(pgm_read_byte(&hsvptr->c[el]) - workColor.c[el]) / (fTweens + 1);
                    if (perStep != 0)
                    {
                        for (byte tweenCount = 1; tweenCount <= fTweens; tweenCount++)
                        {
                            byte val = workColor.c[el] + tweenCount * perStep;
                            tc[tweenCount].c[el] = (el == 2) ?
                                map8(val, MIN_BRIGHTNESS, brightVal) : val;
                        }
                    }
                    else
                    {
                        //tweens for this element (h,s or v) don't change between this key and the next, fill em up
                        for (byte tweenCount = 1; tweenCount <= fTweens; tweenCount++)
                        {
                            tc[tweenCount].c[el] = (el == 2) ?
                                map8(workColor.c[el], MIN_BRIGHTNESS, brightVal) : workColor.c[el];
                        }
                    }              
                }
            }
        }
    }

    void calculateAllColors()
    {
        calculateAllColors(fSettings.fPalNum, fSettings.fBri);
    }

    void updateMappedLED(unsigned index, byte hueVal, byte briVal = 255)
    {
        LEDStatus* ledStatus = &fLEDStatus[index];
        if (ledStatus->fColorPause != 0)
        {
            ledStatus->fColorPause--; //reduce the LEDs pause number and check back next loop
        }
        else
        {
            ledStatus->fColorNum++;
            if (ledStatus->fColorNum >= fTotalColorsWBIZ)
                ledStatus->fColorNum = 0; //bring it back to color zero
            byte realColor = actualColorNum(ledStatus->fColorNum);
            ledStatus->fColorPause =
                (ledStatus->fColorNum % 5 == 0) ?
                    random8(fSettings.fDelay) : //color is a key, assign random pause
                    fSettings.fFade; //color is a tween, assign a quick pause

            HSVColor* myColor = &fAllColors[realColor];
            fLED[index].setHSV(myColor->h + hueVal, myColor->s,
                (briVal == 255) ? myColor->v : map8(briVal, 0, myColor->v));
        }
    }

    virtual void jawaCommand(char cmd, int arg, int value) override
    {
        UNUSED_ARG(value)
        switch (cmd)
        {
            case 'A':
            case 'D':
                selectEffect(NORMVAL);
                break;
            case 'C':
                // TODO Message scroll speed
                break;
            case 'F':
                fSettings.fFade = arg;
                break;
            case 'G':
                fSettings.fDelay = arg;
                break;
            case 'H':
                fSettings.fHue = arg;
                break;
            case 'J':
                fSettings.fBri = arg;
                calculateAllColors();
                break;
            case 'P':
                switch (arg)
                {
                    case 60:
                        // Latin
                        break;
                    case 61:
                        // Aurabesh
                        break;
                    case 70:
                        defaultSettings();
                        break;
                    case 71:
                        // Load from persistent
                        break;
                    case 72:
                        // Save to persistent
                        break;
                    case 73:
                        // Cycle to next color palette
                        if (++fSettings.fPalNum >= PAL_COUNT)
                            fSettings.fPalNum = 0;
                        calculateAllColors();
                        break;
                }
                break;
            case 'R':
                // Random sequence
                selectSequence(RANDOM);
                break;
            case 'W':
                // Wait number of seconds and then revert to default
                fDisplayEffectVal = (fDisplayEffectVal / 100) * 100 + arg;
                break;
            case 'Z':
                selectEffect(TEXTSCROLLLEFT);
                break;
        }
    }

    virtual void jawaCommand(char cmd, const char* arg) override
    {
        switch (cmd)
        {
            case 'M':
                setTextMessage(arg);
                break;
        }
    }

    inline LogicEngineSettings getSettings()
    {
        return fSettings;
    }

    inline void changeSettings(LogicEngineSettings& newSettings)
    {
        fSettings = newSettings;
    }

    virtual void changeDefaultSettings(LogicEngineSettings& settings) = 0;

    static inline int mapSelectColorToHue(unsigned selectColor)
    {
        static const byte sEffectHue[] PROGMEM = {
            0,0,26,42,85,128,170,202,213,228
        };
        return (selectColor < SizeOfArray(sEffectHue)) ? pgm_read_byte(&sEffectHue[selectColor]) : 0;
    }

protected:
    /// \private
    union HSVColor
    {
        struct
        {
            byte h;
            byte s;
            byte v;
        };
        byte c[3];
    };

    LogicEngineRenderer(
        byte id,
        byte tweens,
        byte totalColors,
        byte totalColorsWBIZ,
        unsigned width,
        unsigned height,
        unsigned count,
        unsigned start,
        unsigned end,
        CRGB* led,
        HSVColor* allColors,
        LEDStatus* ledStatus,
        const byte* ledMap,
        LogicRenderGlyph renderGlyph) :
            fID((id == 0) ? getNextID() : id),
            fWidth(width),
            fHeight(height),
            fLEDCount(count),
            fLEDStart(start),
            fLEDEnd(end),
            fTweens(tweens),
            fTotalColors(totalColors),
            fTotalColorsWBIZ(totalColorsWBIZ),
            fLED(led),
            fAllColors(allColors),
            fLEDStatus(ledStatus),
            fLEDMap(ledMap),
            fRenderGlyph(renderGlyph)
    {
        JawaID addr = kJawaOther;
        switch (fID)
        {
            case 1:
                addr = kJawaTFLD;
                break;
            case 2:
                addr = kJawaRFLD;
                break;
        }
        setJawaAddress(addr);
    }

#if USE_LEDLIB == 1
    virtual void show() = 0;
#endif

    virtual void defaultSettings() = 0;

    LogicEngineSettings fSettings;
    LogicEffectSelector fEffectSelector = NULL;
    LogicMessageSelector fMessageSelector = NULL;
    LogicPMessageSelector fPMessageSelector = NULL;

    static uint16_t sLastEventCount;

private:
    byte fID;
    int fWidth;
    int fHeight;
    unsigned fLEDCount;
    unsigned fLEDStart;
    unsigned fLEDEnd;
    byte fTweens;
    byte fTotalColors;
    byte fTotalColorsWBIZ;
    CRGB* fLED;
    HSVColor* fAllColors;
    LEDStatus* fLEDStatus;
    const byte* fLEDMap;
    byte fMaxBrightness = MAX_BRIGHTNESS;

    byte fDisplayEffect = 0;
    byte fPreviousEffect = ~0;

    uint32_t fLastMillis = 0;
    uint32_t fStatusMillis = 0;
    uint32_t fStatusDelay = 0;
    bool fFlipFlop = false;

    long fDisplayEffectVal = NORMVAL;
    long fPreviousEffectVal = ~fDisplayEffectVal;
    uint32_t fEffectStartMillis = 0;
    unsigned int fEffectMillis = 0;
    LogicEffect fLogicEffect;
    PeakValueProvider* fPeakSource = NULL;

    LogicEffectObject* fEffectObject = NULL;
    uint32_t fEffectData = 0;
    uint32_t fEffectData2 = 0;
    float fEffectRange = 1.0;
    int fEffectMsgStartX;
    int fEffectMsgLen;
    int fEffectMsgWidth;
    int fEffectMsgHeight;
    bool fEffectMsgTransition;
    byte fEffectFontNum = 0;
    const char* fEffectMsgText = NULL;
    PROGMEMString fEffectMsgTextP = NULL;
    LogicRenderGlyph fRenderGlyph;

    inline int actualColorNum(int x) const
    {
        return (x >= fTotalColors) ? (fTotalColors - 2) - (x - fTotalColors) : x;
    }
};

///////////////////////////////////////////////////////////////////////////////////////////

typedef LogicEngineRenderer::LogicEffect LogicEffect;
typedef LogicEngineRenderer::LogicEffectSelector LogicEffectSelector;
typedef LogicEngineRenderer::LogicRenderGlyph LogicRenderGlyph;

LogicEffect LogicEffectDefaultSelector(unsigned effectVal);

///////////////////////////////////////////////////////////////////////////////////////////

/// \private
template <typename PCB, LogicRenderGlyph renderGlyph, byte TWEENS = 14>
class LogicEngineDisplay : public LogicEngineRenderer
{
public:
    LogicEngineDisplay(LogicEngineSettings& defaults, byte id = 0, LogicEffectSelector selector = NULL) :
        LogicEngineRenderer(
            id,
            TWEENS,
            TOTALCOLORS,
            TOTALCOLORSWBIZ,
            fPCB.width,
            fPCB.height,
            fPCB.count,
            fPCB.start,
            fPCB.end,
            fPCB.fLED,
            fAllColorsStorage,
            fPCB.fLEDStatus,
            fPCB.getLEDMap(),
            renderGlyph),
        fDefaults(&defaults)
    {
        defaultSettings();
        fEffectSelector = (selector == NULL) ? LogicEffectDefaultSelector : selector;
        fPCB.init();
    }

#if USE_LEDLIB == 1
    virtual void show() override
    {
        fPCB.show();
    }
#endif

    virtual void defaultSettings() override
    {
        fSettings = *fDefaults;
    }

    virtual void changeDefaultSettings(LogicEngineSettings& settings) override
    {
        fSettings = *fDefaults = settings;
    }

    static const unsigned TOTALCOLORS = (4 + (TWEENS * 3));
    static const unsigned TOTALCOLORSWBIZ = ((TOTALCOLORS * 2) - 2); // total colors with bizarro colors

protected:
    PCB fPCB;
    HSVColor fAllColorsStorage[TOTALCOLORS];
    LogicEngineSettings* fDefaults;
};

///////////////////////////////////////////////////////////////////////////////////////////
//
// Animation Effects
//
///////////////////////////////////////////////////////////////////////////////////////////

static bool LogicEffectNormal(LogicEngineRenderer& r)
{
    if (r.hasEffectChanged())
    {
        r.calculateAllColors();
    }
    r.updateDisplay();
    return true;
}

static bool LogicAlarmEffect(LogicEngineRenderer& r)
{
    if (r.hasEffectChanged())
    {
        // Brighter if id matches FLD
        r.setPaletteHue(2, (r.getID() == 1) ? 170 : 72);
        r.setEffectDelay(250 * (r.getEffectSpeed() + 1));
    }
    if (r.getEffectFlip())
    {
        if (r.getEffectColor() == 0)
        {
            r.restoreSettings();
        }
        else
        {
            r.setPaletteHue(2, r.getEffectHue());
        }
    }
    else
    {
        r.setPaletteHue(2, 0);
    }
    r.updateDisplayPeak();
    return true;
}

static bool LogicLeiaEffect(LogicEngineRenderer& r)
{
    // for the Leia message we'll change both logics to hologram
    // colors (palette 2 with a hue shift of 60) then we'll do
    // something with the microphone
    if (r.hasEffectChanged())
    {
        r.setPaletteHue(2, 60);
    }
    byte peakVal = min(int(r.getPeakValue()), 100);
    // if (peakVal > 127)
    {
        //shift the hue very slightly
        r.setHue(60 + map(peakVal,0,100,0,20));
        r.setBrightness(map(peakVal,0,100,50,LogicEngineDefaults::MAX_BRIGHTNESS));
        r.calculateAllColors();
        r.updateDisplay(255);
    }
    // else
    // {
    //     r.setHue(60);
    //     r.updateDisplay(127);
    // }
    // Effect ends after 34 seconds
    return r.getEffectDuration() < 34000;
}

static bool LogicMarchEffect(LogicEngineRenderer& r)
{
    uint32_t effectMillis = r.getEffectDuration();
    if (r.hasEffectChanged())
    {
        r.setPaletteHue(2, r.getEffectHue());
        r.setEffectDelay(150 * (r.getEffectSpeed() + 1));
    }
    //Imperial March (non-beep version) can be divided into a few distinct segments...
    //note: version used (from Padawan sound archive) includes 500ms of silence at start
    static const unsigned int sMarchSegment[] =
    {
        0,      //     0-09800 (9800) = intro
        9800,   // 09800-14500 (4700)
        14500,  // 14500-19300 (4800) //notes every 700ms
        19300,  // 19300-28800 (9500)
        28800,  // 28800-38300 (9500)
        38300,  // 38300-45300 (7000)
        45300,  // 45300-48300 (3000) = wind down  
        48300   // 48300 end
    };
    //change color hue for each marchSegment...
    for (byte i = 1; i < SizeOfArray(sMarchSegment); i++)
    {
        if (effectMillis > sMarchSegment[i-1] && effectMillis < sMarchSegment[i])
        {
            if (r.getEffectColor() == 0)
            {
                // switch colors on segments if no specific color set
                r.setHue((i - 1) * 32);
            }
            if (i == 7)
                r.setEffectDelay(175); //make the flipflop really fast for the last segment
        }
    }
    //alternate brightness of sections(re-uses the flipFlop value from our statusLED)...
    bool flipflop = r.getEffectFlip();
    r.updateDisplaySplitRowThirds((!flipflop) ? 50 : 255, (!flipflop) ? 255 : 50);
    // Effect ends after 48.3 seconds
    return (effectMillis < 48300);
}

static bool LogicFailureEffect(LogicEngineRenderer& r)
{
    uint32_t effectMillis = r.getEffectDuration();
    if (r.hasEffectChanged())
    {
        //start off with Fade and Delay settings low (logic patterns fast), slow
        // them down towards the end and then fade out
        r.setFade(0);
        r.setDelay(0);
    }
    // FAILUREduration is 10000 milliseconds by default, and the effect sequence
    // should be roughly timed to the "128 screa-3.mp3" file
    //   0-1800 = scream
    //1800-5500 = glitch
    //5500-6500 = fade out
    if (effectMillis > 1800 && effectMillis < 6000)
    {
        //during this 'glitch' period, we'll cycle the color hues
        r.setHue(r.getHue() + 1);
    }
    else if (effectMillis > 6000 && effectMillis < 8000)
    {
        //briVal starts at around 255, and drops down to 0 as we approach 6500 millis.
        //this portion lasts 1000 millis, so we'll scale brightness of both logics
        // to a value related to this period
        r.setBrightness(map(effectMillis - 6000, 2000, 0 , 0, 255));
    }
    r.updateDisplay();
    // Effect ends after 18 seconds
    return (effectMillis < 18000);
}

static bool LogicSolidColorEffect(LogicEngineRenderer& r)
{
    if (r.hasEffectChanged())
    {
        r.setPaletteHue(2, r.getEffectHue());
    }
    r.updateDisplay();
    return true;
}

static bool LogicFlipFlopEffect(LogicEngineRenderer& r)
{
    if (r.hasEffectChanged())
    {
        r.setPaletteHue(2, r.getEffectHue());
        r.setEffectDelay(200 * (r.getEffectSpeed() + 1));
    }
    bool flipflop = r.getEffectFlip();
    r.updateDisplaySplitHalf((!flipflop) ? 50 : 255, (!flipflop) ? 255 : 50);
    return true;
}

static bool LogicFlipFlopAltEffect(LogicEngineRenderer& r)
{
    if (r.hasEffectChanged())
    {
        r.setPaletteHue(2, r.getEffectHue());
        r.setEffectDelay(200 * (r.getEffectSpeed() + 1));
    }
    bool flipflop = r.getEffectFlip();
    r.updateDisplaySplitRowThirds((!flipflop) ? 50 : 255, (!flipflop) ? 255 : 50);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////

static bool LogicFlashEffect(LogicEngineRenderer& r)
{
    if (r.hasEffectChanged())
    {
        //change both palettes to selected color
        r.setPaletteHue(2, r.getEffectHue());
        r.setEffectDelay(250 * (r.getEffectSpeed() + 1));
    }
    r.updateDisplay(r.getEffectFlip() ? 255 : 50);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////

static bool LogicColorSwapEffect(LogicEngineRenderer& r)
{
    if (r.hasEffectChanged())
    {
        r.setPaletteHue(2, 0);
        r.setEffectDelay(350 * (r.getEffectSpeed() + 1));   //we'll flipflop hues
    }
    byte effectHue = r.getEffectHue();
    if (!r.getEffectFlip())
    {
        if (r.getEffectHue() >= 128)
        {
            r.setHue(effectHue - 128);
        }
        else
        {
            r.setHue(effectHue + 128);
        }
    }
    else
    {
        r.setHue(effectHue);
    }
    r.updateDisplay();
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////

static bool LogicPSIColorWipeEffect(LogicEngineRenderer& r)
{
    int altColor = r.kDefault;
    int effectColor = r.getEffectColor();
    if (effectColor == r.kRed) altColor = r.kBlue;
    else if (effectColor == r.kBlue) altColor = r.kRed;
    else if (effectColor == r.kYellow) altColor = r.kGreen;
    else if (effectColor == r.kGreen) altColor = r.kYellow;
    else if (effectColor == r.kCyan) altColor = r.kOrange;
    else if (effectColor == r.kOrange) altColor = r.kCyan;
    else if (effectColor == r.kPurple) altColor = r.kMagenta;
    else if (effectColor == r.kPink) altColor = r.kBlue;
    if (r.hasEffectChanged())
    {
        r.setPaletteHue(2, r.getEffectHue());
        r.setEffectData(0);
        r.setEffectDelay(50 * (r.getEffectSpeed()+1));
        r.setEffectData2(r.hasEffectChangedType());
    }
    if (r.getEffectFlip())
    {
        int y;
        int x = r.getEffectData() & 0xFF;
        int px = (r.getEffectData() >> 8) & 0xFF;
        int dir = (r.getEffectData() >> 16) & 0x1;
        // px contains previous row in case we want to erase it
        // if (px != x)
        //     for (y = 0; y < r.height(); y++)
        //         r.setPixel(px, y, r.getEffectHue(), 0);
        for (y = 0; y < r.height(); y++)
            r.setPixel(x, y, (dir) ? r.mapSelectColorToHue(altColor) : r.getEffectHue(), 200);
        px = x;
        x += (!dir) ? 1 : -1;
        int pdir = dir;
        if (x >= r.width())
        {
            dir = 1;
            x--;
        }
        else if (x < 0)
        {
            dir = 0;
            x = 0;
        }
        if (pdir != dir)
        {
            r.setEffectDelay(1000+2000*random(3));
        }
        //decide if we're going to get 'stuck'
        else if (random(100) <= 5)
        {
            r.setEffectDelay(1000+2000*random(3));
        }
        else
        {
            r.setEffectDelay(50 * (r.getEffectSpeed()+1));
        }
        r.setEffectData((uint32_t(dir)<<16L) | ((uint32_t(px)<<8L)) | x);
        r.setEffectFlip(false);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////

static bool LogicRainbowEffect(LogicEngineRenderer& r)
{
    if (r.hasEffectChanged())
    {
        r.setPaletteHue(2, r.getEffectHue());
        r.setEffectDelay(200 * (r.getEffectSpeed() + 1)); //speed of hue swap
        r.setEffectData(r.getEffectFlip());
    }
    if (r.getEffectFlip() != r.getEffectData())
    {
        r.setHue(r.getHue() + 20);
        r.setEffectData(r.getEffectFlip());
    }
    r.updateDisplay();
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////

static bool LogicRedAlertEffect(LogicEngineRenderer& r)
{
    if (r.hasEffectChanged())
    {
        r.calculateAllColors();
    }
    if (r.getPeakValue() < r.getEffectSpeed() * 256 / 10)
    {
        r.restoreSettings();
        r.calculateAllColors();
    }
    else
    {
        // show color while loud
        r.setPaletteHue(2, r.getEffectHue());
    }
    r.updateDisplay();
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////

static bool LogicMicBrightEffect(LogicEngineRenderer& r)
{
    if (r.hasEffectChanged())
    {
        r.calculateAllColors();
    }
    byte peakVal = r.getPeakValue();
    int speedVal = r.getEffectSpeed() * 256 / 10;
    if (peakVal < speedVal)
    {
        r.setBrightness(speedVal);
    }
    else
    {
        r.setBrightness(peakVal);
    }
    if (r.getEffectColor() == 0)
    {
        // use default colors
        r.restoreSettings(); // reset to defaults for a clean slate
        r.calculateAllColors();
    }
    else
    {
        // show color
        r.setPaletteHue(2, r.getEffectHue());
    }
    r.updateDisplay();
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////

static bool LogicMicRainbowEffect(LogicEngineRenderer& r)
{
    if (r.hasEffectChanged())
    {
        r.setPaletteHue(2, r.getEffectHue());
    }
    r.setHue(r.getPeakValue() + r.getEffectHue() - 256);
    r.updateDisplay();
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////

static bool LogicTextEffect(LogicEngineRenderer& r)
{
    if (r.hasEffectChanged())
    {
        r.setBrightness(0);
        r.setPaletteHue(2, r.getEffectHue());
        r.setupTextMessage(r.getEffectTextMsg());
        r.setEffectData2(r.hasEffectChangedType());
        r.setEffectData(false);
    }
    if (r.getEffectData2() && r.getEffectDuration() < 2000)
    {
        r.updateDisplay(0);
    }
    else if (!r.getEffectData())
    {
        int x = r.width() / 2 - r.getEffectMsgWidth() / 2;
        r.renderText(x, 0, r.getEffectHue());
        r.setEffectData(true);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////

static bool LogicTextScrollLeftEffect(LogicEngineRenderer& r)
{
    if (r.hasEffectChanged())
    {
        r.setBrightness(0);
        r.setPaletteHue(2, r.getEffectHue());
        r.setupTextMessage(r.getEffectTextMsg());
        r.setEffectData(r.width());
        r.setEffectDelay(50 * (r.getEffectSpeed() + 1));
        r.setEffectData2(r.hasEffectChangedType());
    }
    if (r.getEffectData2() && r.getEffectDuration() < 2000)
    {
        r.updateDisplay(0);
    }
    else if (r.getEffectFlip())
    {
        // scrolling text
        int x = int(r.getEffectData());
        r.renderText(x, 0, r.getEffectHue());
        x -= 1;
        if (x + r.getEffectMsgWidth() <= 0)
        {
            x = r.width();
            return false;
        }
        r.setEffectData(x);
        r.setEffectFlip(false);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////

static bool LogicTextScrollRightEffect(LogicEngineRenderer& r)
{
    if (r.hasEffectChanged())
    {
        r.setBrightness(0);
        r.setPaletteHue(2, r.getEffectHue());
        r.setupTextMessage(r.getEffectTextMsg());
        r.setEffectData(-r.getEffectMsgWidth());
        r.setEffectDelay(50 * (r.getEffectSpeed() + 1));
        r.setEffectData2(r.hasEffectChangedType());
    }
    if (r.getEffectData2() && r.getEffectDuration() < 2000)
    {
        r.updateDisplay(0);
    }
    else if (r.getEffectFlip())
    {
        // scrolling text
        int x = int(r.getEffectData());
        r.renderText(x, 0, r.getEffectHue());
        x += 1;
        if (x > r.width())
        {
            x = -r.getEffectMsgWidth();
            return false;
        }
        r.setEffectData(x);
        r.setEffectFlip(false);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////

static bool LogicTextScrollUpEffect(LogicEngineRenderer& r)
{
    if (r.hasEffectChanged())
    {
        r.setBrightness(0);
        r.setPaletteHue(2, r.getEffectHue());
        r.setupTextMessage(r.getEffectTextMsg());
        r.setEffectData(r.height());
        r.setEffectDelay(50 * (r.getEffectSpeed() + 1));
        r.setEffectData2(r.hasEffectChangedType());
    }
    if (r.getEffectData2() && r.getEffectDuration() < 2000)
    {
        r.updateDisplay(0);
    }
    else if (r.getEffectFlip())
    {
        // scrolling text
        int x = r.width() / 2 - r.getEffectMsgWidth() / 2;
        int y = int(r.getEffectData());
        r.renderText(x, y, r.getEffectHue());
        y -= 1;
        if (y + r.getEffectMsgHeight() <= 0)
        {
            y = r.height();
            return false;
        }
        r.setEffectData(y);
        r.setEffectFlip(false);
    }
    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////

static bool LogicRoamingPixelEffect(LogicEngineRenderer& r)
{
    if (r.hasEffectChanged())
    {
        r.setBrightness(0);
        r.setPaletteHue(2, r.getEffectHue());
        r.setupTextMessage(r.getEffectTextMsg());
        r.setEffectData(0);
        r.setEffectDelay(50 * (r.getEffectSpeed()));
        r.setEffectData2(r.hasEffectChangedType());
    }
    if (r.getEffectData2() && r.getEffectDuration() < 2000)
    {
        r.updateDisplay(0);
    }
    else if (r.getEffectFlip())
    {
        uint32_t xy = r.getEffectData();
        int x = int(xy >> 16);
        int y = int(xy & 0xFFFFL);
        r.setPixel(x, y, r.getEffectHue(), 200);
        x += 1;
        if (x > r.width())
        {
            for (x = 0; x < r.width(); x++)
                r.setPixel(x, y, r.getEffectHue(), 0);
            x = 0;
            y += 1;
        }
        if (y >= r.height())
        {
            x = 0; y = 0;
        }
        r.setEffectData((uint32_t(x)<<16L) | ((uint32_t(y)&0XFFFF)));
        r.setEffectFlip(false);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////

static bool LogicHorizontalScanLineEffect(LogicEngineRenderer& r)
{
    if (r.hasEffectChanged())
    {
        r.setBrightness(0);
        r.setPaletteHue(2, r.getEffectHue());
        r.setupTextMessage(r.getEffectTextMsg());
        r.setEffectData(0);
        r.setEffectDelay(50 * (r.getEffectSpeed()+1));
        r.setEffectData2(r.hasEffectChangedType());
    }
    if (r.getEffectData2() && r.getEffectDuration() < 2000)
    {
        r.updateDisplay(0);
    }
    else if (r.getEffectFlip())
    {
        int x;
        int y = r.getEffectData() & 0xFF;
        int py = (r.getEffectData() >> 8) & 0xFF;
        int dir = (r.getEffectData() >> 16) & 0x1;
        if (py != y)
            for (x = 0; x < r.width(); x++)
                r.setPixel(x, py, r.getEffectHue(), 0);
        for (x = 0; x < r.width(); x++)
            r.setPixel(x, y, r.getEffectHue(), 200);
        py = y;
        y += (!dir) ? 1 : -1;
        if (y >= r.height())
        {
            dir = 1;
            y--;
        }
        else if (y < 0)
        {
            dir = 0;
            y = 0;
        }
        r.setEffectData((uint32_t(dir)<<16L) | ((uint32_t(py)<<8L)) | y);
        r.setEffectFlip(false);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////

static bool LogicVerticalScanLineEffect(LogicEngineRenderer& r)
{
    if (r.hasEffectChanged())
    {
        r.setBrightness(0);
        r.setPaletteHue(2, r.getEffectHue());
        r.setupTextMessage(r.getEffectTextMsg());
        r.setEffectData(0);
        r.setEffectDelay(50 * (r.getEffectSpeed()+1));
        r.setEffectData2(r.hasEffectChangedType());
    }
    if (r.getEffectData2() && r.getEffectDuration() < 2000)
    {
        r.updateDisplay(0);
    }
    else if (r.getEffectFlip())
    {
        int y;
        int x = r.getEffectData() & 0xFF;
        int px = (r.getEffectData() >> 8) & 0xFF;
        int dir = (r.getEffectData() >> 16) & 0x1;
        if (px != x)
            for (y = 0; y < r.height(); y++)
                r.setPixel(px, y, r.getEffectHue(), 0);
        for (y = 0; y < r.height(); y++)
            r.setPixel(x, y, r.getEffectHue(), 200);
        px = x;
        x += (!dir) ? 1 : -1;
        if (x >= r.width())
        {
            dir = 1;
            x--;
        }
        else if (x < 0)
        {
            dir = 0;
            x = 0;
        }
        r.setEffectData((uint32_t(dir)<<16L) | ((uint32_t(px)<<8L)) | x);
        r.setEffectFlip(false);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////

static bool LogicFireEffect(LogicEngineRenderer& r)
{
    static const unsigned char sValueMask[10][24] PROGMEM = {
        {32 , 0  , 0  , 0  , 0  , 0  , 0  , 32 , 32 , 0  , 0  , 0  , 0  , 0  , 0  , 32,   32 ,  0  , 0  ,  0  , 0  , 0  , 0  , 32 },
        {64 , 0  , 0  , 0  , 0  , 0  , 0  , 64 , 64 , 0  , 0  , 0  , 0  , 0  , 0  , 64,   64 ,  0  , 0  ,  0  , 0  , 0  , 0  , 64 },
        {96 , 32 , 0  , 0  , 0  , 0  , 32 , 96 , 96 , 32 , 0  , 0  , 0  , 0  , 32 , 96,   32 ,  0  , 0  ,  0  , 0  , 0  , 32 , 96 },
        {128, 64 , 32 , 0  , 0  , 32 , 64 , 128, 128, 64 , 32 , 0  , 0  , 32 , 64 , 128,  64 , 32  , 0  ,  0  , 0  , 32 , 64 , 128},
        {160, 96 , 64 , 32 , 32 , 64 , 96 , 160, 160, 96 , 64 , 32 , 32 , 64 , 96 , 160,  96 , 64  , 32 , 32  , 64 , 96 , 160, 160},
        {160, 96 , 64 , 32 , 32 , 64 , 96 , 160, 160, 96 , 64 , 32 , 32 , 64 , 96 , 160,  96 , 64  , 32 , 32  , 64 , 96 , 160, 160},
        {192, 128, 96 , 64 , 64 , 96 , 128, 192, 192, 128, 96 , 64 , 64 , 96 , 128, 192, 192, 128  , 96 , 64  , 64 , 96 , 128, 192},
        {255, 160, 128, 96 , 96 , 128, 160, 255, 255, 160, 128, 96 , 96 , 128, 160, 255, 255, 160  , 128, 96  , 96 , 128, 160, 255},
        {255, 192, 160, 128, 128, 160, 192, 255, 255, 192, 160, 128, 128, 160, 192, 255, 255, 192  , 160, 128 ,128 , 160, 192, 255},
        {255, 192, 160, 128, 128, 160, 192, 255, 255, 192, 160, 128, 128, 160, 192, 255, 255, 192  , 160, 128 ,128 , 160, 192, 255},
    };

    static const unsigned char sHueMask[10][24] PROGMEM = {
        {1 , 11, 19, 25, 25, 22, 11, 1 , 1 , 11, 19, 25, 25, 22, 11, 1, 1 , 1 , 11, 19, 25, 25, 22, 11 },
        {1 , 8 , 13, 19, 25, 19, 8 , 1 , 1 , 8 , 13, 19, 25, 19, 8 , 1, 1 , 1 , 8 , 13, 19, 25, 19, 8  },
        {1 , 8 , 13, 16, 19, 16, 8 , 1 , 1 , 8 , 13, 16, 19, 16, 8 , 1, 1 , 1 , 8 , 13, 16, 19, 16, 8  },
        {1 , 5 , 11, 13, 13, 13, 5 , 1 , 1 , 5 , 11, 13, 13, 13, 5 , 1, 1 , 1 , 5 , 11, 13, 13, 13, 5  },
        {1 , 5 , 11, 11, 11, 11, 5 , 1 , 1 , 5 , 11, 11, 11, 11, 5 , 1, 1 , 1 , 5 , 11, 11, 11, 11, 5  },
        {1 , 5 , 11, 11, 11, 11, 5 , 1 , 1 , 5 , 11, 11, 11, 11, 5 , 1, 1 , 1 , 5 , 11, 11, 11, 11, 5  },
        {0 , 1 , 5 , 8 , 8 , 5 , 1 , 0 , 0 , 1 , 5 , 8 , 8 , 5 , 1 , 0, 0 , 1 , 5 , 8 , 8 , 5 , 1 , 0  }, 
        {0 , 0 , 1 , 5 , 5 , 1 , 0 , 0 , 0 , 0 , 1 , 5 , 5 , 1 , 0 , 0, 0 , 0 , 1 , 5 , 5 , 1 , 0 , 0  },
        {0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 1 , 0 , 0 , 0, 0 , 0 , 0 , 1 , 1 , 0 , 0 , 0  },
        {0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 1 , 0 , 0 , 0, 0 , 0 , 0 , 1 , 1 , 0 , 0 , 0  },
    };

    int count = r.count();
    int width = r.width();
    int height = r.height();
    CRGB* leds = r.getUnmappedLEDs();
    LEDStatus* ledStatus = r.getUnmappedLEDStatus();
    if (r.hasEffectChanged())
    {
        r.setBrightness(0);
        r.setPaletteHue(2, r.getEffectHue());
        r.setupTextMessage(r.getEffectTextMsg());
        r.setEffectData(0);
        r.setEffectDelay(100 * (r.getEffectSpeed() + 1));
        r.setEffectData2(r.hasEffectChangedType());
        for (int i = 0; i < count; i++)
        {
            ledStatus[i].fColorNum = 0;
            ledStatus[i].fColorPause = 0;
        }
        /* Generate line */
        for (uint8_t x = 0; x < width; x++)
        {
           ledStatus[x].fColorPause = random(64, 255);
        }
    }
    if (r.getEffectFlip())
    {
        int pcnt = r.getEffectData();
        if (pcnt >= 100)
        {
            /* shift up */
            for (uint8_t y = height - 1; y > 0; y--)
            {
                for (uint8_t x = 0; x < width; x++)
                {
                    ledStatus[r.mapLED(y*width+x)].fColorNum = ledStatus[r.mapLED((y-1)*width+x)].fColorNum;
                }
            }
            for (uint8_t x = 0; x < width; x++)
            {
                ledStatus[r.mapLED(x)].fColorNum = ledStatus[x].fColorPause;
            }
            /* Generate line */
            for (uint8_t x = 0; x < width; x++)
            {
               ledStatus[x].fColorPause = random(64, 255);
            }
        }
        int nextv;

        //each row interpolates with the one before it
        int heightm1 = height - 1;
        for (byte y = heightm1; y > 0; y--)
        {
            for (byte x = 0; x < width; x++)
            {
                nextv = (((100.0-pcnt) * ledStatus[r.mapLED(y*width+x)].fColorNum +
                    pcnt * ledStatus[r.mapLED((y-1)*width+x)].fColorNum) / 100.0) - pgm_read_byte(&sValueMask[y][x]);
                leds[r.mapLED((heightm1-y)*width+x)].setHSV(pgm_read_byte(&sHueMask[y][x]), 255, (uint8_t)max(0, nextv));
            }
        }

        //first row interpolates with the "next" line
        for (byte x = 0; x < width; x++)
        {
            leds[r.mapLED(heightm1*width+x)].setHSV(pgm_read_byte(&sHueMask[0][x]), 255,
                (uint8_t)(((100.0-pcnt) * ledStatus[r.mapLED(x)].fColorNum + pcnt * ledStatus[x].fColorPause)/100.0));
        }
        pcnt += 20;
        r.setEffectData(pcnt);
        r.setEffectFlip(false);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////

static bool LogicLightsOutEffect(LogicEngineRenderer& r)
{
    if (r.hasEffectChanged())
    {
        r.setBrightness(0);
        r.setPaletteHue(2, r.getEffectHue());
    }
    r.updateDisplay(0);
    return true;
}

static bool LogicPulseEffect(LogicEngineRenderer& r)
{
    if (r.hasEffectChanged())
    {
        r.setBrightness(0);
        r.setPaletteHue(2, r.getEffectHue());
        r.setupTextMessage(r.getEffectTextMsg());
        r.setEffectData(0);
        r.setEffectDelay(70 * (r.getEffectSpeed()+1));
        r.setEffectData2(r.hasEffectChangedType());
    }
    if (r.getEffectData2() && r.getEffectDuration() < 2000)
    {
        r.updateDisplay(0);
    }
    else if (r.getEffectFlip())
    {
        float xmid = r.width()/2;
        float ymid = r.height()/2;
        int dy;
        int dx;
        int x = r.getEffectData() & 0xFF;
        int px = (r.getEffectData() >> 8) & 0xFF;
        int dir = (r.getEffectData() >> 16) & 0x1;

        for (dy = 0; dy < r.height(); dy++) {
            for (dx = 0; dx < r.width(); dx++) {
                if ((dy+px >= ymid && dy-px <= ymid) && (dx+px >= xmid && dx-px <= xmid)) 
                {
                    r.setPixel(dx, dy, r.getEffectHue(), 150);
                } else {
                    r.setPixel(dx, dy, r.getEffectHue(), 0);
                }
            }
        }
        px += (!dir) ? 1 : -1;
        if (px > r.width()/2 || px < 1) {
            dir =  !dir;
        }
        r.setEffectData((uint32_t(dir)<<16L) | ((uint32_t(px)<<8L)) | x);
        r.setEffectFlip(false);
    }
    return true; 
}

///////////////////////////////////////////////////////////////////////////////////////////

LogicEffect LogicEffectDefaultSelector(unsigned selectSequence)
{
    static const LogicEffect sLogicEffects[] = {
        LogicEffectNormal,
        LogicAlarmEffect,
        LogicFailureEffect,
        LogicLeiaEffect,
        LogicMarchEffect,
        LogicSolidColorEffect,
        LogicFlashEffect,
        LogicFlipFlopEffect,
        LogicFlipFlopAltEffect,
        LogicColorSwapEffect,
        LogicRainbowEffect,
        LogicRedAlertEffect,
        LogicMicBrightEffect,
        LogicMicRainbowEffect,
        LogicLightsOutEffect,
        LogicTextEffect,
        LogicTextScrollLeftEffect,
        LogicTextScrollRightEffect,
        LogicTextScrollUpEffect,
        LogicRoamingPixelEffect,
        LogicHorizontalScanLineEffect,
        LogicVerticalScanLineEffect,
        LogicFireEffect,
        LogicPSIColorWipeEffect,
        LogicPulseEffect,
    };
    if (selectSequence == LogicEngineDefaults::RANDOM)
        selectSequence = random(SizeOfArray(sLogicEffects));
    if (selectSequence > SizeOfArray(sLogicEffects))
        selectSequence = 0;
    return LogicEffect(sLogicEffects[selectSequence]);
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// Text Rendering
//
///////////////////////////////////////////////////////////////////////////////////////////

byte getlsbposm1(byte x)
{
    for (int i = 0; i < 8; i++)
        if (x & (1<<i))
            return 8-i;
    return 0;
}

enum LogicStaggerType {
    kNone,
    kEven,
    kOdd
};

template <LogicStaggerType staggerType>
byte LogicRenderGlyph4Pt(char ch, byte fontNum, const CRGB fontColors[], int x, int y, CRGB* leds, const byte* ledMap, int w, int h, byte* outGlyphHeight)
{
    UNUSED_ARG(fontNum)
    byte dstWidth = w;
    byte dstHeight = h;
    byte dstRowBytes = w;
    byte advance;
    byte glyphRowBytes;
    byte glyphHeight = 4;
    byte glyphBitmap[2*8];  // max size 2 bytes wide 8 bytes high
    FontVar4Pt* font = LatinFontVar4pt::instance();
    bool found = font->getLetter(ch, glyphBitmap, glyphRowBytes, advance);
    if (!found)
    {
        // try upper-case if lower-case is missing
        if (ch >= 'a' && ch < 'z')
            found = font->getLetter('A'+(ch-'a'), glyphBitmap, glyphRowBytes, advance);
        // try lower-case if upper-case is missing
        else if (ch >= 'A' && ch < 'Z')
            found = font->getLetter('a'+(ch-'A'), glyphBitmap, glyphRowBytes, advance);
    }
    if (outGlyphHeight != NULL)
        *outGlyphHeight = glyphHeight;
    // render from baseline
    int starty = y;
    y += glyphHeight - 1;
    while (glyphHeight > 0 && y >= 0)
    {
        int adv = advance;
        byte* s = &glyphBitmap[(y-starty) * glyphRowBytes];
        if (ledMap != NULL && y < dstHeight)
        {
            for (byte rb = 0; rb < glyphRowBytes; rb++)
            {
                byte i = 6;
                byte b = *s++;
                byte m = 0b11000000;
                int xx = x;
                while (adv > 0 && m != 0 && xx < dstWidth)
                {
                    if ((b & m) != 0 && xx >= 0)
                    {
                        leds[pgm_read_byte(&ledMap[y * dstRowBytes + xx])] = fontColors[((b >> i) & 3)-1];
                    }
                    adv--;
                    m >>= 2;
                    i -= 2;
                    xx++;
                }
            }
        }
        x += (staggerType == LogicStaggerType::kOdd) ? (y&1) : (staggerType == LogicStaggerType::kEven) ? ((y&1) ^ 1) : 0;
        glyphHeight--;
        y--;
    }
    return advance + 1;
}

static byte LogicRenderGlyph5Pt(char ch, byte fontNum, const CRGB fontColors[], int x, int y, CRGB* leds, const byte* ledMap, int w, int h, byte* outGlyphHeight)
{
    byte dstRowBytes = w;
    byte glyphRowBytes = 1;
    byte glyphHeight = 5;
    byte glyphBitmap[2*8];  // max size 2 bytes wide 8 bytes high
    Font8x5* font = (fontNum == 1) ? AurabeshFont8x5::instance() : LatinFont8x5::instance();
    bool found = font->getLetter(ch, glyphBitmap);
    if (!found)
    {
        // try upper-case if lower-case is missing
        if (ch >= 'a' && ch < 'z')
            found = font->getLetter('A'+(ch-'a'), glyphBitmap);
        // try lower-case if upper-case is missing
        else if (ch >= 'A' && ch < 'Z')
            found = font->getLetter('a'+(ch-'A'), glyphBitmap);
    }
    // calculate glyph advance
    byte advance = 0;
    for (int i = 0; i < glyphHeight; i++)
    {
        byte b = glyphBitmap[i * glyphRowBytes];
        for (byte i = 0; i < 8; i++)
        {
            if (b & (1<<i))
            {
                advance = max((int)advance, 8-i);
                break;
            }
        }
    }
    if (advance == 0)
        advance = 4;

    if (outGlyphHeight != NULL)
        *outGlyphHeight = glyphHeight;
    // render from baseline
    int starty = y;
    y += glyphHeight - 1;
    while (glyphHeight > 0 && y >= 0)
    {
        int adv = advance;
        byte* s = &glyphBitmap[(y-starty) * glyphRowBytes];
        if (ledMap != NULL && y < h)
        {
            for (byte rb = 0; rb < glyphRowBytes; rb++)
            {
                byte i = 7;
                byte b = *s++;
                byte m = 0b10000000;
                int xx = x;
                while (adv > 0 && m != 0 && xx < w)
                {
                    if ((b & m) != 0 && xx >= 0)
                    {
                        leds[pgm_read_byte(&ledMap[y * dstRowBytes + xx])] = fontColors[2];
                    }
                    adv--;
                    m >>= 1;
                    i -= 1;
                    xx++;
                }
            }
        }
        glyphHeight--;
        y--;
    }
    return advance + 1;
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// Convenience types
//
///////////////////////////////////////////////////////////////////////////////////////////

/** \ingroup Dome
 *
 * \class AstroPixelFLD
 *
 * \brief 2022 AstroPixel Front Logic Display
 *
 * Example Usage:
 * \code
 * AstroPixelFLD<> FLD(FRONT_PIN_NUMBER, LogicEngineFLDDefault);
 * \endcode
 */
template <uint8_t DATA_PIN = FRONT_LOGIC_PIN>
using AstroPixelFLD = LogicEngineDisplay<AstroPixelFLDPCB0<DATA_PIN>, LogicRenderGlyph5Pt>;
/** \ingroup Dome
 *
 * \class AstroPixelRLD
 *
 * \brief 2022 AstroPixel Rear Logic Display
 *
 * Example Usage:
 * \code
 * AstroPixelRLD<> RLD(REAR_PIN_NUMBER, LogicEngineRLDDefault);
 * \endcode
 */
template <uint8_t DATA_PIN = REAR_LOGIC_PIN>
using AstroPixelRLD = LogicEngineDisplay<AstroPixelRLDPCB0<DATA_PIN>, LogicRenderGlyph4Pt<LogicStaggerType::kNone>>;
/** \ingroup Dome
 *
 * \class LogicEngineNabooFLD
 *
 * \brief Original Front Logic PCB with Naboo logo on back
 *
 * Example Usage:
 * \code
 * LogicEngineNabooFLD<> FLD(FRONT_PIN_NUMBER, LogicEngineFLDDefault);
 * \endcode
 */
template <uint8_t DATA_PIN = FRONT_LOGIC_PIN>
using LogicEngineNabooFLD = LogicEngineDisplay<LogicEngineFLDPCB0<DATA_PIN>, LogicRenderGlyph5Pt>;
/** \ingroup Dome
 *
 * \class LogicEngineNabooRLD
 *
 * \brief Original Rear Logic PCB with Naboo logo on back
 *
 * Example Usage:
 * \code
 * LogicEngineNabooRLD<> FLD(REAR_PIN_NUMBER, LogicEngineRLDDefault);
 * \endcode
 */
template <uint8_t DATA_PIN = REAR_LOGIC_PIN>
using LogicEngineNabooRLD = LogicEngineDisplay<LogicEngineRLDPCB0<DATA_PIN>, LogicRenderGlyph4Pt<LogicStaggerType::kEven>>;

/** \ingroup Dome
 *
 * \class LogicEngineKennyFLD
 *
 * \brief 2014 Version Front Logic PCB with C3PO on back
 *
 * Example Usage:
 * \code
 * LogicEngineKennyFLD<> FLD(FRONT_PIN_NUMBER, LogicEngineFLDDefault);
 * \endcode
 */
template <uint8_t DATA_PIN = FRONT_LOGIC_PIN>
using LogicEngineKennyFLD = LogicEngineDisplay<LogicEngineFLDPCB1<DATA_PIN>, LogicRenderGlyph5Pt>;
/** \ingroup Dome
 *
 * \class LogicEngineKennyRLD
 *
 * \brief 2014 Version Rear Logic PCB with Kenny & McQuarry art on back
 *
 * Example Usage:
 * \code
 * LogicEngineKennyRLD<> RLD(REAR_PIN_NUMBER, LogicEngineRLDDefault);
 * \endcode
 */
template <uint8_t DATA_PIN = REAR_LOGIC_PIN>
using LogicEngineKennyRLD = LogicEngineDisplay<LogicEngineRLDPCB1<DATA_PIN>, LogicRenderGlyph4Pt<LogicStaggerType::kEven>>;

/** \ingroup Dome
 *
 * \class LogicEngineSuperRLD
 *
 * \brief Super sized rear logic panel 32x8
 *
 * Example Usage:
 * \code
 * LogicEngineSuperRLD<> RLD(REAR_PIN_NUMBER, LogicEngineRLDDefault);
 * \endcode
 */template <uint8_t DATA_PIN = REAR_LOGIC_PIN>
using LogicEngineSuperRLD = LogicEngineDisplay<LogicEngineRLDPCBSUPER<DATA_PIN>, LogicRenderGlyph5Pt>;

/** \ingroup Dome
 *
 * \class LogicEngineDeathStarFLD
 *
 * \brief 2016 Version Front Logic PCB with Deathstar Plans on back
 *
 * Example Usage:
 * \code
 * LogicEngineDeathStarFLD<> FLD(FRONT_PIN_NUMBER, LogicEngineFLDDefault);
 * \endcode
 */
template <uint8_t DATA_PIN = FRONT_LOGIC_PIN>
using LogicEngineDeathStarFLD = LogicEngineDisplay<LogicEngineFLDPCB2<DATA_PIN>, LogicRenderGlyph5Pt>;

/** \ingroup Dome
 *
 * \class LogicEngineDeathStarFLD
 *
 * \brief 2016 Version Front Logic PCB with Deathstar Plans on back
 *
 * Example Usage:
 * \code
 * LogicEngineDeathStarFLD<> FLD(FRONT_PIN_NUMBER, LogicEngineFLDDefault);
 * \endcode
 */
template <uint8_t DATA_PIN = FRONT_LOGIC_PIN>
using LogicEngineDeathStarFLDInverted = LogicEngineDisplay<LogicEngineFLDPCB2Inverted<DATA_PIN>, LogicRenderGlyph5Pt>;

/** \ingroup Dome
 *
 * \class LogicEngineDeathStarRLD
 *
 * \brief 2016 Version Rear Logic PCB with Deathstar Plans on back
 *
 * Example Usage:
 * \code
 * LogicEngineDeathStarRLD<> FLD(REAR_PIN_NUMBER, LogicEngineRLDDefault);
 * \endcode
 */
template <uint8_t DATA_PIN = REAR_LOGIC_PIN>
using LogicEngineDeathStarRLD = LogicEngineDisplay<LogicEngineRLDPCB2<DATA_PIN>, LogicRenderGlyph4Pt<LogicStaggerType::kEven>>;
/** \ingroup Dome
 *
 * \class LogicEngineDeathStarRLDStaggerOdd
 *
 * \brief 2016 Version Rear Logic PCB with Deathstar Plans on back. LEDs are staggered on odd rows
 *
 * Example Usage:
 * \code
 * LogicEngineDeathStarRLDStaggerOdd<> FLD(REAR_PIN_NUMBER, LogicEngineRLDDefault);
 * \endcode
 */
template <uint8_t DATA_PIN = REAR_LOGIC_PIN>
using LogicEngineDeathStarRLDStaggerOdd = LogicEngineDisplay<LogicEngineRLDPCB2<DATA_PIN>, LogicRenderGlyph4Pt<LogicStaggerType::kOdd>>;
/** \ingroup Dome
 *
 * \class LogicEngineDeathStarRLDInverted
 *
 * \brief 2016 Version Rear Logic PCB with Deathstar Plans on back. Mounted upside down.
 *
 * Example Usage:
 * \code
 * LogicEngineDeathStarRLDInverted<> FLD(REAR_PIN_NUMBER, LogicEngineRLDDefault);
 * \endcode
 */
template <uint8_t DATA_PIN = REAR_LOGIC_PIN>
using LogicEngineDeathStarRLDInverted = LogicEngineDisplay<LogicEngineRLDPCB2Inverted<DATA_PIN>, LogicRenderGlyph4Pt<LogicStaggerType::kEven>>;
/** \ingroup Dome
 *
 * \class LogicEngineDeathStarRLDInvertedStaggerOdd
 *
 * \brief 2016 Version Rear Logic PCB with Deathstar Plans on back. Mounted upside down.
 *
 * Example Usage:
 * \code
 * LogicEngineDeathStarRLDInvertedStaggerOdd<> FLD(REAR_PIN_NUMBER, LogicEngineRLDDefault);
 * \endcode
 */
template <uint8_t DATA_PIN = REAR_LOGIC_PIN>
using LogicEngineDeathStarRLDInvertedStaggerOdd = LogicEngineDisplay<LogicEngineRLDPCB2Inverted<DATA_PIN>, LogicRenderGlyph4Pt<LogicStaggerType::kOdd>>;

/** \ingroup Dome
 *
 * \class LogicEngineCurvedFLD
 *
 * \brief 2020 Version Front Logic PCB for curved logics
 *
 * Example Usage:
 * \code
 * LogicEngineCurvedFLD<> FLD(FRONT_PIN_NUMBER, LogicEngineFLDDefault);
 * \endcode
 */
template <uint8_t DATA_PIN = FRONT_LOGIC_PIN>
using LogicEngineCurvedFLD = LogicEngineDisplay<LogicEngineFLDPCB2<DATA_PIN>, LogicRenderGlyph5Pt>;

/** \ingroup Dome
 *
 * \class LogicEngineCurvedFLD
 *
 * \brief 2020 Version Front Logic PCB for curved logics
 *
 * Example Usage:
 * \code
 * LogicEngineCurvedFLD<> FLD(FRONT_PIN_NUMBER, LogicEngineFLDDefault);
 * \endcode
 */
template <uint8_t DATA_PIN = FRONT_LOGIC_PIN>
using LogicEngineCurvedFLDInverted = LogicEngineDisplay<LogicEngineFLDPCB2Inverted<DATA_PIN>, LogicRenderGlyph5Pt>;

#if USE_LEDLIB == 0
/** \ingroup Dome
 *
 * \class LogicEngineCurvedRLD
 *
 * \brief 2020 Version Rear Logic PCB for curved logics
 *
 * Example Usage:
 * \code
 * LogicEngineCurvedRLD<> FLD(REAR_PIN_NUMBER, LogicEngineRLDDefault);
 * \endcode
 */
template <uint8_t DATA_PIN = REAR_LOGIC_PIN, uint8_t CLOCK_PIN = REAR_LOGIC_CLOCK_PIN>
using LogicEngineCurvedRLD = LogicEngineDisplay<LogicEngineRLDPCB3<DATA_PIN, CLOCK_PIN>, LogicRenderGlyph4Pt<LogicStaggerType::kNone>>;
/** \ingroup Dome
 *
 * \class LogicEngineCurvedRLDInverted
 *
 * \brief 2020 Version Rear Logic PCB for curved logics. Mounted upside down.
 *
 * Example Usage:
 * \code
 * LogicEngineCurvedRLDInverted<> FLD(REAR_PIN_NUMBER, LogicEngineRLDDefault);
 * \endcode
 */
template <uint8_t DATA_PIN = REAR_LOGIC_PIN, uint8_t CLOCK_PIN = REAR_LOGIC_CLOCK_PIN>
using LogicEngineCurvedRLDInverted = LogicEngineDisplay<LogicEngineRLDPCB3Inverted<DATA_PIN, CLOCK_PIN>, LogicRenderGlyph4Pt<LogicStaggerType::kNone>>;
#endif

static LogicEngineSettings LogicEngineFLDDefault(
    LogicEngineDefaults::FRONT_FADE,
    LogicEngineDefaults::FRONT_HUE,
    LogicEngineDefaults::FRONT_DELAY,
    LogicEngineDefaults::FRONT_PAL,
    LogicEngineDefaults::FRONT_BRI,
    LogicEngineDefaults::sequence(LogicEngineDefaults::NORMAL));

static LogicEngineSettings LogicEngineRLDDefault(
    LogicEngineDefaults::REAR_FADE,
    LogicEngineDefaults::REAR_HUE,
    LogicEngineDefaults::REAR_DELAY,
    LogicEngineDefaults::REAR_PAL,
    LogicEngineDefaults::REAR_BRI,
    LogicEngineDefaults::sequence(LogicEngineDefaults::NORMAL));

///////////////////////////////////////////////////////////////////////////////////////////

#endif
