#ifndef BT1EYE_H
#define BT1EYE_H
#include "dome/LogicEngine.h"

/// \private
template <uint8_t DATA_PIN = FRONT_LOGIC_PIN>
class BT1EyePCB : public FastLEDPCB<SK6812, DATA_PIN, 99, 0, 99, 9, 11>
{
public:
    static inline const byte* getLEDMap()
    {
        static const byte sLEDmap[] PROGMEM =
        {
			//    XXX
			//   X X X
			//    X X
			//  XX X XX
			//    XXX
			// XXXXXXXXX
			//    XXX
			//  XX X XX
			//    X X
			//   X X X
			//    XXX
			#define __ 99
            __, __, __,  0,  1,  2, __, __, __,
            __, __,  3, __,  4, __,  5, __, __,
            __, __, __,  6, __,  7, __, __, __,
            __,  8,  9, __, 10, __, 11, 12, __,
            __, __, __, 13, 14, 15, __, __, __,
            16, 17, 18, 19, 20, 21, 22, 23, 24,
            __, __, __, 25, 26, 27, __, __, __,
            __, 28, 29, __, 30, __, 31, 32, __,
            __, __, __, 33, __, 34, __, __, __,
            __, __, 35, __, 36, __, 37, __, __,
            __, __, __, 38, 39, 40, __, __, __,
            #undef __
        };
        return sLEDmap;
    }
};

template <uint8_t DATA_PIN = EYE_PIN>
using BT1Eye = LogicEngineDisplay<BT1EyePCB<DATA_PIN>, LogicRenderGlyph5Pt>;

static LogicEngineSettings LogicEngineBT1EyeDefault(
    LogicEngineDefaults::FRONT_FADE,
    LogicEngineDefaults::FRONT_HUE,
    LogicEngineDefaults::FRONT_DELAY,
    LogicEngineDefaults::FRONT_PAL,
    LogicEngineDefaults::FRONT_BRI);

#endif
