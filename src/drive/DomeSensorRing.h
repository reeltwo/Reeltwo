#ifndef DomeSensorRing_h
#define DomeSensorRing_h

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/MedianSampleBuffer.h"

#ifdef USE_DOME_SENSOR_DEBUG
#define DOME_SENSOR_PRINT(s) DEBUG_PRINT(s)
#define DOME_SENSOR_PRINTLN(s) DEBUG_PRINTLN(s)
#define DOME_SENSOR_PRINT_HEX(s) DEBUG_PRINT_HEX(s)
#define DOME_SENSOR_PRINTLN_HEX(s) DEBUG_PRINTLN_HEX(s)
#else
#define DOME_SENSOR_PRINT(s)
#define DOME_SENSOR_PRINTLN(s)
#define DOME_SENSOR_PRINT_HEX(s)
#define DOME_SENSOR_PRINTLN_HEX(s)
#endif

#if defined(ARDUINO_ARCH_RP2040)
static constexpr uint8_t SENSORS[] {27, 28, 29, 3, 4, 6, 25, 24, 26};
// static constexpr uint8_t SENSORS[] {27, 28, 29, 3, 4, 6, 25, 24, 26};
#elif defined(ESP32) && defined(PIN_NEOPIXEL)
static constexpr uint8_t SENSORS[] {27, 25, 26, 13, 12, 14, 33, 4, 15};
#elif defined(ESP32)
static constexpr uint8_t SENSORS[] {18, 19, 21, 22, 32, 33, 25, 26, 27};
#endif

class DomeSensorRing : public SetupEvent
{
public:
    virtual void setup() override
    {
    #if defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
        for (auto i = 0; i < SizeOfArray(SENSORS); i++)
            pinMode(SENSORS[i], INPUT_PULLUP);
    #elif defined(DOME_CONTROLLER_BOARD)
        /* Dome sensor is connected to A0-A8 */
        for (uint8_t pin = A0; pin <= A8; pin++)
            pinMode(pin, INPUT_PULLUP);
    #else
        /* Dome sensor is connected to 2-10 */
        for (uint8_t pin = 2; pin <= 10; pin++)
            pinMode(pin, INPUT_PULLUP);
    #endif
    }

    unsigned readSensors()
    {
    #if defined(ARDUINO_ARCH_RP2040)
        unsigned mask = 0;
        noInterrupts();
        for (auto i = 0; i < SizeOfArray(SENSORS); i++)
            mask |= (digitalRead(SENSORS[i]) << i);
        interrupts();
    #elif defined(ESP32)
        unsigned mask = 0;
        portDISABLE_INTERRUPTS();
        for (auto i = 0; i < SizeOfArray(SENSORS); i++)
            mask |= (digitalRead(SENSORS[i]) << i);
        portENABLE_INTERRUPTS();
    #elif defined(DOME_CONTROLLER_BOARD)
        /* Dome controller board reading sensors directly */
        unsigned pinF;
        unsigned pinK;
        cli();
        pinF = PINF;
        pinK = PINK;
        sei();
        unsigned mask =
         ((pinF & (1<<0)) |           /* A0 - 00000000X */
            (pinF & (1<<1)) |         /* A1 - 0000000X0 */
            (pinF & (1<<2)) |         /* A2 - 000000X00 */
            (pinF & (1<<3)) |         /* A3 - 00000X000 */
            (pinF & (1<<4)) |         /* A4 - 0000X0000 */
            (pinF & (1<<5)) |         /* A5 - 000X00000 */
            (pinF & (1<<6)) |         /* A6 - 00X000000 */
            (pinF & (1<<7)) |         /* A7 - 0X0000000 */
            ((pinK & (1<<0)) << 8)    /* A8 - X00000000 */
        );
    #elif defined(DOME_SENSOR_RING_FULL_SIZE)
        // Sensors are oriented right to left
        cli();
        unsigned pinD { PIND };
        unsigned pinB { PINB };
        sei();
        unsigned mask {
            (((pinB >> 2) & 1) << 0) |  /* D10 */
            (((pinB >> 1) & 1) << 1) |  /* D9 */
            (((pinB >> 0) & 1) << 2) |  /* D8 */
            (((pinD >> 7) & 1) << 3) |  /* D7 */
            (((pinD >> 6) & 1) << 4) |  /* D6 */
            (((pinD >> 5) & 1) << 5) |  /* D5 */
            (((pinD >> 4) & 1) << 6) |  /* D4 */
            (((pinD >> 3) & 1) << 7) |  /* D3 */
            (((pinD >> 2) & 1) << 8)    /* D2 */
        };
    #else
        // Sensors are oriented left to right
        cli();
        unsigned pinD { PIND };
        unsigned pinB { PINB };
        sei();
        unsigned mask {uint16_t(pinD >> 2) | uint16_t((pinB & 0b111) << 6)};
    #endif
        return mask;
    }

    short getAngle()
    {
        auto mask = readSensors();
        if (mask != fLastMask || fSampleCount < 6)
        {
        #ifdef USE_DOME_SENSOR_DEBUG
            if (fSampleCount > 0 && fLastMask != ~0u && countChangedBits(mask, fLastMask) > 1)
            {
                DOME_SENSOR_PRINTLN();
                DOME_SENSOR_PRINTLN();
                DOME_SENSOR_PRINT("fLastMask=");
                DOME_SENSOR_PRINTLN_HEX(fLastMask);
                // More than one bit changed bad state
                DOME_SENSOR_PRINTLN("BAD DOME POSITION STATE");
                DOME_SENSOR_PRINTLN();
            }
        #endif
            auto currentAngle = getDomeAngle(mask);
            if (currentAngle != -1)
            {
                fSamples.append(currentAngle);
                if (fSampleCount < 6)
                {
                    // Return the raw angle
                    fLastPosition = currentAngle;
                    fSampleCount++;
                }
                else
                {
                    // Return the filtered angle
                    fLastPosition = fSamples.median();
                }
            }
        #ifdef USE_DOME_SENSOR_DEBUG
            if (fSampleCount > 0)
            {
                DOME_SENSOR_PRINT("                                           ");
                DOME_SENSOR_PRINT('\r');
                DOME_SENSOR_PRINT_HEX(mask);
                DOME_SENSOR_PRINT(" ");
                printBinary(mask, 9);
                DOME_SENSOR_PRINT(" - ");
                DOME_SENSOR_PRINT(mask);
                DOME_SENSOR_PRINT(" - ");
                DOME_SENSOR_PRINT(countChangedBits(mask, fLastMask));
                DOME_SENSOR_PRINT(" - ");
                DOME_SENSOR_PRINT(fLastPosition);
                if (currentAngle == -1)
                {
                    DOME_SENSOR_PRINT(" *BAD*");
                }
                DOME_SENSOR_PRINT('\r');
            }
        #endif
            fLastMask = mask;
        }
        return fLastPosition;
    }

    inline bool ready()
    {
        return (fSampleCount > 1);
    }

private:
    int fSampleCount = 0;
    unsigned fLastMask = ~0;
    short fLastPosition = -1;
    MedianSampleBuffer<short, 5> fSamples;

#ifdef USE_DOME_SENSOR_DEBUG
    static unsigned countChangedBits(unsigned a, unsigned b)
    {
        unsigned n = 0;
        for (unsigned i = 0; i < 9; i++) 
        {
            if ((a & (1 << i)) != (b & (1 << i)))
                n++;
        }
        return n;
    }

    static void printBinary(unsigned num, unsigned places)
    {
        if (places)
            printBinary(num >> 1, places-1);
        DOME_SENSOR_PRINT((num & 1) ? '1' : '0');
    }
#endif

public:
    static short getDomeAngle(unsigned sensorMask)
    {
        static const short sDomeAngle[] PROGMEM = {
             -1,   0,  40,  39,  80,   1,  79,  38, 120, 129,  41,  42, 119,  86,  78,  37, 160, 171, 169, 170,  81,  84,  82,  83, 
            159, 128, 126, 127, 118,  85,  77,  76, 200,  11, 211, 212, 209, 214, 210, 213, 121, 216, 124, 217, 122, 215, 123, 218, 
            199,  12, 168,  -1, 166,  -1, 167,  -1, 158,  -1, 125,  -1, 117,  -1, 116, 115, 240,   9,  51,   8, 251,  96, 252,  -1, 
            249,  -1, 254,  -1, 250, 261, 253, 260, 161,  14, 256,  -1, 164,  -1, 257,  -1, 162,  -1, 255,  -1, 163,  -1, 258, 259, 
            239,  10,  52,  -1, 208,  -1,  -1,  -1, 206,  21,  -1,  -1, 207, 262,  -1,  -1, 198,  13,  -1,  -1, 165, 264,  -1, 265, 
            157,  20,  -1,  -1, 156, 263, 155, 154, 280, 281,  49,   6,  91,   4,  48,   5, 291, 134, 136, 135, 292,  -1,  -1,  -1, 
            289,  16,  -1, 181, 294,  -1,  -1,  -1, 290,  -1, 301, 182, 293, 184, 300, 183, 201, 284,  54,  -1, 296,  -1,  -1, 104, 
            204,  -1,  -1,  28, 297, 108,  -1, 107, 202,  17,  -1,  -1, 295, 188,  -1, 189, 203,  -1,  -1, 191, 298, 187, 299, 190, 
            279, 282,  50,   7,  92,  97,  -1,  -1, 248,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 246,  15,  61,  62,  -1, 268,  -1,  71, 
            247,  -1, 302,  -1,  -1, 269,  -1,  72, 238, 283,  53,  -1,  -1,  -1,  -1, 105, 205,  -1, 304,  29,  -1, 271, 305, 106, 
            197,  18,  60,  -1,  -1, 267,  -1, 266, 196,  19, 303, 192, 195, 270, 194, 193, 320, 359, 321, 358,  89,   2,  46, 357, 
            131, 130,  44,  43,  88,  87,  45,  36, 331, 172, 174, 173, 176, 177, 175, 178, 332,  -1,  -1,  -1,  -1,  -1,  -1,  75, 
            329, 328,  56,  -1,  -1,  -1, 221, 220, 334,  -1,  -1,  -1,  -1,  -1,  -1, 219, 330,  -1,  -1,  -1, 341,  -1, 222,  -1, 
            333,  -1, 224, 225, 340,  -1, 223, 114, 241, 326, 324, 325,  94,  95,  -1,  -1, 336, 141,  -1,  -1,  -1, 142, 144, 143, 
            244,  -1,  -1,  64,  -1, 348,  68,  67, 337,  -1, 148, 149,  -1, 151, 147, 150, 242, 327,  57,  -1,  -1,  -1,  -1,  -1, 
            335,  22, 228,  31,  -1,  -1, 229,  32, 243,  -1,  -1,  65,  -1, 349, 231,  66, 338,  -1, 227, 226, 339, 152, 230, 153, 
            319, 318, 322, 317,  90,   3,  47, 356, 132, 133, 137, 138,  -1,  -1,  -1,  35, 288,  -1,  -1, 180,  -1,  -1,  -1, 179, 
             -1,  -1,  -1,  -1,  -1, 185,  -1,  74, 286, 285,  55,  -1, 101,  -1, 102, 103,  -1,  24, 308,  27,  -1, 109, 111, 110, 
            287,  -1,  -1,  -1, 342, 351,  -1, 352,  -1,  25, 309,  26,  -1, 186, 112, 113, 278, 277, 323, 316,  93,  98,  -1, 355, 
             -1, 140,  -1, 139,  -1,  -1, 145,  34, 245,  -1,  -1,  63, 344, 347,  69,  70,  -1,  -1, 311, 312, 345, 346, 146,  73, 
            237, 276,  58, 315, 100,  99,  -1, 354,  -1,  23, 307,  30,  -1, 272, 306,  33, 236, 275,  59, 314, 343, 350, 232, 353, 
            235, 274, 310, 313, 234, 273, 233, -1
        };
        return pgm_read_word(&sDomeAngle[sensorMask & 0x1FF]);
    }
};

#endif
