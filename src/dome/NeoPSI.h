
#ifndef NeoPSI_h
#define NeoPSI_h

#include "ReelTwo.h"
#include "dome/LogicEngine.h"

#ifndef FRONT_PSI_PIN
 #if defined(REELTWO_TEENSY)
  #define FRONT_PSI_PIN 21 /* TODO figure out correct default pin */
 #elif defined(REELTWO_ZERO)
  #define FRONT_PSI_PIN 5 /* TODO figure out correct default pin */
 #elif defined(REELTWO_AVR_MEGA)
  #define FRONT_PSI_PIN 5
 #elif defined(REELTWO_AVR)
  #define FRONT_PSI_PIN 6
 #elif defined(ESP32)
  #define FRONT_PSI_PIN 32
 #else
  #error Unsupported platform
 #endif
#endif

#ifndef REAR_PSI_PIN
 #if defined(REELTWO_TEENSY)
  #define REAR_PSI_PIN 22 /* TODO figure out correct default pin */
 #elif defined(REELTWO_ZERO)
  #define REAR_PSI_PIN 3 /* TODO figure out correct default pin */
 #elif defined(REELTWO_AVR_MEGA)
  #define REAR_PSI_PIN 6
 #elif defined(REELTWO_AVR)
  #define REAR_PSI_PIN 6
 #elif defined(ESP32)
  #define REAR_PSI_PIN 23
 #else
  #error Unsupported platform
 #endif
#endif

/// \private
template <uint8_t DATA_PIN>
class NeoPSIPCB : public FastLEDPCB<WS2812B, DATA_PIN, 35, 0, 35, 5, 7>
{
public:
    static inline const byte* getLEDMap()
    {
        // Use dummy pixel 31 for no pixel
        static const byte sLEDmap[] PROGMEM =
        {
            31, 11, 12, 25, 31,
             0, 10, 13, 24, 26,
             1,  9, 14, 23, 27,
             2,  8, 15, 22, 28,
             3,  7, 16, 21, 29,
             4,  6, 17, 20, 30,
            31,  5, 18, 19, 31,
        };
        return sLEDmap;
    }
};

template <uint8_t DATA_PIN>
class AstroPixelPSIPCB : public FastLEDPCB<WS2812B, DATA_PIN, 25, 0, 25, 5, 5>
{
public:
    static inline const byte* getLEDMap()
    {
        // Use dummy pixel 31 for no pixel
        static const byte sLEDmap[] PROGMEM =
        {
            31, 0,  1,  2,  31,
            3,  4,  5,  6,  7,
            8,  9,  10, 11, 12,
            13, 14, 15, 16, 17,
            31, 18, 19, 20, 31,
        };
        return sLEDmap;
    }
};

template <uint8_t DATA_PIN>
class AstroPixelPSI8PCB : public FastLEDPCB<WS2812B, DATA_PIN, 64, 0, 64, 8, 8>
{
public:
    static inline const byte* getLEDMap()
    {
        // Use dummy pixel 31 for no pixel
        static const byte sLEDmap[] PROGMEM =
        {
      99,99,3,2,1,0,99,99,
      99,9,8,7,6,5,4,99,
      17,16,15,14,13,12,11,10,
      25,24,23,22,21,20,19,18,
      33,32,31,30,29,28,27,26,
      41,40,39,38,37,36,35,34,
      99,47,46,45,44,43,42,99,
      99,99,51,50,49,48,99,99
        };
        return sLEDmap;
    }
};

/** \ingroup Dome
 *
 * \class NeoFrontPSI
 *
 * \brief Neopixel based Front PSI PCB
 *
 * Example Usage:
 * \code
 * NeoRearPSILD<REAR_PSI_PIN> rearPSI(id, LogicEngineRearPSIDefault);
 * \endcode
 */
template <uint8_t DATA_PIN = FRONT_PSI_PIN>
using NeoFrontPSI = LogicEngineDisplay<NeoPSIPCB<DATA_PIN>, LogicRenderGlyph5Pt>;

/** \ingroup Dome
 *
 * \class NeoRearPSILD
 *
 * \brief Neopixel based Rear PSI PCB
 *
 * Example Usage:
 * \code
 * NeoRearPSILD<REAR_PSI_PIN> rearPSI(id, LogicEngineRearPSIDefault);
 * \endcode
 */
template <uint8_t DATA_PIN = REAR_PSI_PIN>
using NeoRearPSI = LogicEngineDisplay<NeoPSIPCB<DATA_PIN>, LogicRenderGlyph5Pt>;


/** \ingroup Dome
 *
 * \class AstroPixelFrontPSI
 *
 * \brief Neopixel based Front PSI PCB
 *
 * Example Usage:
 * \code
 * AstroPixelFrontPSI<REAR_PSI_PIN> frontPSI(id, LogicEngineRearPSIDefault);
 * \endcode
 */
template <uint8_t DATA_PIN = FRONT_PSI_PIN>
using AstroPixelFrontPSI = LogicEngineDisplay<AstroPixelPSIPCB<DATA_PIN>, LogicRenderGlyph5Pt, LogicEngineDefaults::PSICOLORWIPE>;

/** \ingroup Dome
 *
 * \class AstroPixelRearPSI
 *
 * \brief Neopixel based Rear PSI PCB
 *
 * Example Usage:
 * \code
 * AstroPixelRearPSI<REAR_PSI_PIN> rearPSI(id, LogicEngineRearPSIDefault);
 * \endcode
 */
template <uint8_t DATA_PIN = REAR_PSI_PIN>
using AstroPixelRearPSI = LogicEngineDisplay<AstroPixelPSIPCB<DATA_PIN>, LogicRenderGlyph5Pt, LogicEngineDefaults::PSICOLORWIPE>;

/** \ingroup Dome
 *
 * \class AstroPixelFrontPSI8
 *
 * \brief Neopixel based Front PSI PCB for denser displays
 *
 * Example Usage:
 * \code
 * AstroPixelFrontPSI8<REAR_PSI_PIN> frontPSI(id, LogicEngineRearPSIDefault);
 * \endcode
 */
template <uint8_t DATA_PIN = FRONT_PSI_PIN>
using AstroPixelFrontPSI8 = LogicEngineDisplay<AstroPixelPSI8PCB<DATA_PIN>, LogicRenderGlyph5Pt, LogicEngineDefaults::PSICOLORWIPE>;

/** \ingroup Dome
 *
 * \class AstroPixelRearPSI
 *
 * \brief Neopixel based Rear PSI PCB for denser displays
 *
 * Example Usage:
 * \code
 * AstroPixelRearPSI8<REAR_PSI_PIN> rearPSI(id, LogicEngineRearPSIDefault);
 * \endcode
 */
template <uint8_t DATA_PIN = REAR_PSI_PIN>
using AstroPixelRearPSI8 = LogicEngineDisplay<AstroPixelPSI8PCB<DATA_PIN>, LogicRenderGlyph5Pt, LogicEngineDefaults::PSICOLORWIPE>;

static LogicEngineSettings LogicEngineFrontPSIDefault(
    LogicEngineDefaults::FRONT_FADE,
    LogicEngineDefaults::FRONT_HUE,
    LogicEngineDefaults::FRONT_DELAY,
    LogicEngineDefaults::FRONT_PSI_PAL,
    LogicEngineDefaults::FRONT_BRI,
    LogicEngineDefaults::sequence(LogicEngineDefaults::PSICOLORWIPE, LogicEngineDefaults::kRed));

static LogicEngineSettings LogicEngineRearPSIDefault(
    LogicEngineDefaults::REAR_FADE,
    LogicEngineDefaults::REAR_HUE,
    LogicEngineDefaults::REAR_DELAY,
    LogicEngineDefaults::REAR_PSI_PAL,
    LogicEngineDefaults::REAR_BRI,
    LogicEngineDefaults::sequence(LogicEngineDefaults::PSICOLORWIPE, LogicEngineDefaults::kYellow));

#endif
