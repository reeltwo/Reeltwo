
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
