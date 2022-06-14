#ifndef ServoSequencer_h
#define ServoSequencer_h

#include "ReelTwo.h"
#include "core/AnimatedEvent.h"
#include "ServoDispatch.h"

// Offset value specified in per-mille (per thousand)
#define SEQUENCE_RANGE_LIMIT(offsetFromMin, offsetFromMax) \
{ uint16_t(~0), (((offsetFromMin)>>8)&0xFF), (((offsetFromMin)>>0)&0xFF), (((offsetFromMax)>>8)&0xFF), (((offsetFromMax)>>0)&0xFF) },

/**
  * \struct ServoStep
  *
  * \brief Single frame of servo animation. Time is specified in centi seconds (1/100th of a second)
  */
struct ServoStep
{
    uint16_t cs; /* centiseconds: 1/100th of a second*/
    uint8_t servo1_8;
    uint8_t servo9_16;
    uint8_t servo17_24;
    uint8_t servo25_32;
};

typedef struct ServoStep ServoSequence[];

static const ServoSequence SeqPanelAllOpen PROGMEM =
{
    { 20,   B11111111, B11111111, B11111111, B11111111 },
};

static const ServoSequence SeqPanelAllClose PROGMEM =
{
    { 20,   B00000000, B00000000, B00000000, B00000000 },
};

static const ServoSequence SeqPanelAllOpenClose PROGMEM =
{
    { 300,  B11111111, B11111111, B11111111, B11111111 },
    { 150,  B00000000, B00000000, B00000000, B00000000 },
};

static const ServoSequence SeqPanelAllOpenCloseLong PROGMEM =
{
    { 1000, B11111111, B11111111, B11111111, B11111111 },
    { 150,  B00000000, B00000000, B00000000, B00000000 },
};

static const ServoSequence SeqPanelAllFlutter PROGMEM =
{
    // Twenty permille (per thousand) offset from start/end positions
    SEQUENCE_RANGE_LIMIT(200, 200)
    { 10,   B11111111, B11111111, B11111111, B11111111 },
    { 10,   B00000000, B00000000, B00000000, B00000000 },
    { 10,   B11111111, B11111111, B11111111, B11111111 },
    { 10,   B00000000, B00000000, B00000000, B00000000 },
    { 10,   B11111111, B11111111, B11111111, B11111111 },
    { 10,   B00000000, B00000000, B00000000, B00000000 },
    { 10,   B11111111, B11111111, B11111111, B11111111 },
    { 10,   B00000000, B00000000, B00000000, B00000000 },
    { 10,   B11111111, B11111111, B11111111, B11111111 },
    SEQUENCE_RANGE_LIMIT(0, 0)
    { 10,   B00000000, B00000000, B00000000, B00000000 },
};

static const ServoSequence SeqPanelAllFOpenCloseRepeat PROGMEM =
{
    // Twenty permille (per thousand) offset from start/end positions
    { 10,   B11111111, B11111111, B11111111, B11111111 },
    { 10,   B00000000, B00000000, B00000000, B00000000 },
    { 10,   B11111111, B11111111, B11111111, B11111111 },
    { 10,   B00000000, B00000000, B00000000, B00000000 },
    { 10,   B11111111, B11111111, B11111111, B11111111 },
    { 10,   B00000000, B00000000, B00000000, B00000000 },
    { 10,   B11111111, B11111111, B11111111, B11111111 },
    { 10,   B00000000, B00000000, B00000000, B00000000 },
    { 10,   B11111111, B11111111, B11111111, B11111111 },
    { 10,   B00000000, B00000000, B00000000, B00000000 },
};

static const ServoSequence SeqPanelWave PROGMEM =
{
    { 30,   B00000000, B00000000, B00000000, B00000000 },
    { 30,   B10000000, B00000000, B00000000, B00000000 },
    { 30,   B01000000, B00000000, B00000000, B00000000 },
    { 30,   B00100000, B00000000, B00000000, B00000000 },
    { 30,   B00010000, B00000000, B00000000, B00000000 },
    { 30,   B00001000, B00000000, B00000000, B00000000 },
    { 30,   B00000100, B00000000, B00000000, B00000000 },
    { 30,   B00000010, B00000000, B00000000, B00000000 },
    { 30,   B00000001, B00000000, B00000000, B00000000 },
    { 30,   B00000000, B10000000, B00000000, B00000000 },
    { 30,   B00000000, B01000000, B00000000, B00000000 },
    { 30,   B00000000, B00100000, B00000000, B00000000 },
    { 30,   B00000000, B00010000, B00000000, B00000000 },
    { 30,   B00000000, B00000000, B00000000, B00000000 },
};

static const ServoSequence SeqPanelWaveFast PROGMEM =
{
    { 15,   B00000000, B00000000, B00000000, B00000000 },
    { 15,   B10000000, B00000000, B00000000, B00000000 },
    { 15,   B01000000, B00000000, B00000000, B00000000 },
    { 15,   B00100000, B00000000, B00000000, B00000000 },
    { 15,   B00010000, B00000000, B00000000, B00000000 },
    { 15,   B00001000, B00000000, B00000000, B00000000 },
    { 15,   B00000100, B00000000, B00000000, B00000000 },
    { 15,   B00000010, B00000000, B00000000, B00000000 },
    { 15,   B00000001, B00000000, B00000000, B00000000 },
    { 15,   B00000000, B10000000, B00000000, B00000000 },
    { 15,   B00000000, B01000000, B00000000, B00000000 },
    { 30,   B00000000, B00100000, B00000000, B00000000 },
    { 30,   B00000000, B00010000, B00000000, B00000000 },
    { 15,   B00000000, B00000000, B00000000, B00000000 },
    { 30,   B00000000, B00010000, B00000000, B00000000 },
    { 30,   B00000000, B00100000, B00000000, B00000000 },
    { 15,   B00000000, B01000000, B00000000, B00000000 },
    { 15,   B00000000, B10000000, B00000000, B00000000 },
    { 15,   B00000001, B00000000, B00000000, B00000000 },
    { 15,   B00000010, B00000000, B00000000, B00000000 },
    { 15,   B00000100, B00000000, B00000000, B00000000 },
    { 15,   B00001000, B00000000, B00000000, B00000000 },
    { 15,   B00010000, B00000000, B00000000, B00000000 },
    { 15,   B00100000, B00000000, B00000000, B00000000 },
    { 15,   B01000000, B00000000, B00000000, B00000000 },
    { 15,   B10000000, B00000000, B00000000, B00000000 },
    { 15,   B00000000, B00000000, B00000000, B00000000 },
};

static const ServoSequence SeqPanelOpenCloseWave PROGMEM =
{
    { 20,   B00000000, B00000000, B00000000, B00000000 },
    { 20,   B10000000, B00000000, B00000000, B00000000 },
    { 20,   B11000000, B00000000, B00000000, B00000000 },
    { 20,   B11100000, B00000000, B00000000, B00000000 },
    { 20,   B11110000, B00000000, B00000000, B00000000 },
    { 20,   B11111000, B00000000, B00000000, B00000000 },
    { 20,   B11111100, B00000000, B00000000, B00000000 },
    { 20,   B11111110, B00000000, B00000000, B00000000 },
    { 20,   B11111111, B00000000, B00000000, B00000000 },
    { 20,   B11111111, B10000000, B00000000, B00000000 },
    { 20,   B11111111, B11000000, B00000000, B00000000 },
    { 20,   B11111111, B11100000, B00000000, B00000000 },
    { 80,   B11111111, B11110000, B00000000, B00000000 },
    { 20,   B01111111, B11110000, B00000000, B00000000 },
    { 20,   B00111111, B11110000, B00000000, B00000000 },
    { 20,   B00011111, B11110000, B00000000, B00000000 },
    { 20,   B00001111, B11110000, B00000000, B00000000 },
    { 20,   B00000111, B11110000, B00000000, B00000000 },
    { 20,   B00000011, B11110000, B00000000, B00000000 },
    { 20,   B00000001, B11110000, B00000000, B00000000 },
    { 20,   B00000000, B11110000, B00000000, B00000000 },
    { 20,   B00000000, B01110000, B00000000, B00000000 },
    { 20,   B00000000, B00110000, B00000000, B00000000 },
    { 20,   B00000000, B00010000, B00000000, B00000000 },
    { 40,   B00000000, B00000000, B00000000, B00000000 },
};

static const ServoSequence SeqPanelMarchingAnts PROGMEM =
{
    // Alternating pattern of on/off
    { 20,   B00000000, B00000000, B00000000, B00000000 },
    { 50,   B10101010, B10101010, B10101010, B10101010 },
    { 50,   B01010101, B01010101, B01010101, B01010101 },
    { 50,   B10101010, B10101010, B10101010, B10101010 },
    { 50,   B01010101, B01010101, B01010101, B01010101 },
    { 50,   B10101010, B10101010, B10101010, B10101010 },
    { 50,   B01010101, B01010101, B01010101, B01010101 },
    { 50,   B10101010, B10101010, B10101010, B10101010 },
    { 50,   B01010101, B01010101, B01010101, B01010101 },
    { 50,   B10101010, B10101010, B10101010, B10101010 },
    { 50,   B01010101, B01010101, B01010101, B01010101 },
    { 50,   B10101010, B10101010, B10101010, B10101010 },
    { 50,   B01010101, B01010101, B01010101, B01010101 },
    { 50,   B10101010, B10101010, B10101010, B10101010 },
    { 50,   B01010101, B01010101, B01010101, B01010101 },
    { 50,   B10101010, B10101010, B10101010, B10101010 },
    { 50,   B01010101, B01010101, B01010101, B01010101 },
    { 50,   B10101010, B10101010, B10101010, B10101010 },
    { 50,   B01010101, B01010101, B01010101, B01010101 },
    { 50,   B10101010, B10101010, B10101010, B10101010 },
    { 50,   B01010101, B01010101, B01010101, B01010101 },
    { 50,   B10101010, B10101010, B10101010, B10101010 },
    { 50,   B01010101, B01010101, B01010101, B01010101 },
    { 50,   B10101010, B10101010, B10101010, B10101010 },
    { 50,   B01010101, B01010101, B01010101, B01010101 },
    { 50,   B10101010, B10101010, B10101010, B10101010 },
    { 50,   B01010101, B01010101, B01010101, B01010101 },
    { 50,   B10101010, B10101010, B10101010, B10101010 },
    { 50,   B01010101, B01010101, B01010101, B01010101 },
    { 50,   B10101010, B10101010, B10101010, B10101010 },
    { 50,   B01010101, B01010101, B01010101, B01010101 },
    { 100,  B00000000, B00000000, B00000000, B00000000 },
};

static const ServoSequence SeqPanelAlternate PROGMEM =
{
    // Alternating pattern of on/off
    { 20,   B00000000, B00000000, B00000000, B00000000 },
    { 50,   B10101010, B10101010, B10101010, B10101010 },
    { 50,   B01010101, B01010101, B01010101, B01010101 },
    { 100,  B00000000, B00000000, B00000000, B00000000 },
};

// 1-4 panels
// 5-6 large panels
// 7-10 pie panels
// 11-12 mini doors
// 13 pie door
static const ServoSequence SeqPanelDance PROGMEM =
{
    { 10,   B00000000, B00000000, B00000000, B00000000 }, // 4 pie, 1 by one
    { 45,   B00000010, B00000000, B00000000, B00000000 },
    { 45,   B00000011, B00000000, B00000000, B00000000 },
    { 45,   B00000011, B10000000, B00000000, B00000000 },
    { 45,   B00000011, B11000000, B00000000, B00000000 },
    { 45,   B00000011, B10000000, B00000000, B00000000 },
    { 45,   B00000011, B00000000, B00000000, B00000000 },
    { 45,   B00000010, B00000000, B00000000, B00000000 },

    { 45,   B00000000, B00000000, B00000000, B00000000 }, // 4 side, 1 by one
    { 45,   B10000000, B00000000, B00000000, B00000000 },
    { 45,   B11000000, B00000000, B00000000, B00000000 },
    { 45,   B11100000, B00000000, B00000000, B00000000 },
    { 45,   B11110000, B00000000, B00000000, B00000000 },
    { 45,   B01110000, B00000000, B00000000, B00000000 },
    { 45,   B00110000, B00000000, B00000000, B00000000 },
    { 45,   B00010000, B00000000, B00000000, B00000000 },

    { 45,   B00000000, B00000000, B00000000, B00000000 }, // 4 pies. 2 by 2
    { 45,   B00000010, B10000000, B00000000, B00000000 },
    { 45,   B00000000, B00000000, B00000000, B00000000 },
    { 45,   B00000001, B01000000, B00000000, B00000000 },
    { 45,   B00000000, B00000000, B00000000, B00000000 },
    { 45,   B00000011, B11000000, B00000000, B00000000 },
    { 45,   B00000001, B01000000, B00000000, B00000000 },
    { 45,   B00000010, B10000000, B00000000, B00000000 },

    { 45,   B00000000, B00000000, B00000000, B00000000 }, // 2 large sides
    { 45,   B00001000, B00000000, B00000000, B00000000 },
    { 45,   B00000000, B00000000, B00000000, B00000000 },
    { 45,   B00000100, B00000000, B00000000, B00000000 },
    { 45,   B00000000, B00000000, B00000000, B00000000 },
    { 45,   B11111100, B00000000, B00000000, B00000000 },
    { 45,   B01111100, B00000000, B00000000, B00000000 },
    { 45,   B00011100, B00000000, B00000000, B00000000 },

    { 45,   B00000000, B00000000, B00000000, B00000000 }, // interleaved
    { 45,   B01010000, B00000000, B00000000, B00000000 },
    { 45,   B00000000, B00000000, B00000000, B00000000 },
    { 45,   B10100000, B00000000, B00000000, B00000000 },
    { 45,   B00000000, B00000000, B00000000, B00000000 },
    { 45,   B00000001, B01000000, B00000000, B00000000 },
    { 45,   B00000010, B10000000, B00000000, B00000000 },
    { 45,   B00000001, B01000000, B00000000, B00000000 },

    { 45,   B00000000, B00000000, B00000000, B00000000 }, // 2nd interleaved
    { 45,   B00000011, B11000000, B00000000, B00000000 },
    { 45,   B00000001, B01000000, B00000000, B00000000 },
    { 45,   B00000010, B10000000, B00000000, B00000000 },
    { 45,   B00000001, B01000000, B00000000, B00000000 },
    { 45,   B00000010, B10000000, B00000000, B00000000 },
    { 45,   B00000001, B01000000, B00000000, B00000000 },
    { 45,   B10101010, B10000000, B00000000, B00000000 },

    { 45,   B00000000, B00000000, B00000000, B00000000 }, //
    { 45,   B00000010, B10000000, B00000000, B00000000 },
    { 45,   B00000001, B01000000, B00000000, B00000000 },
    { 45,   B00000010, B10000000, B00000000, B00000000 },
    { 45,   B00000001, B01000000, B00000000, B00000000 },
    { 45,   B10101010, B10000000, B00000000, B00000000 },
    { 45,   B01010101, B01000000, B00000000, B00000000 },
    { 45,   B10101010, B10000000, B00000000, B00000000 },

    { 45,   B00000000, B00000000, B00000000, B00000000 }, //
    { 45,   B11000000, B00000000, B00000000, B00000000 },
    { 45,   B00110000, B00000000, B00000000, B00000000 },
    { 45,   B00001100, B00000000, B00000000, B00000000 },
    { 45,   B00000000, B00000000, B00000000, B00000000 },
    { 45,   B11001100, B11000000, B00000000, B00000000 },
    { 45,   B00110011, B00000000, B00000000, B00000000 },
    { 45,   B11001100, B11000000, B00000000, B00000000 },

    { 45,   B00000000, B00000000, B00000000, B00000000 }, // transition
    { 45,   B10000000, B00000000, B00000000, B00000000 },
    { 45,   B00000000, B00000000, B00000000, B00000000 },
    { 45,   B01000000, B00000000, B00000000, B00000000 },
    { 45,   B00000000, B00000000, B00000000, B00000000 },
    { 45,   B11100000, B00000000, B00000000, B00000000 },
    { 45,   B01100000, B00000000, B00000000, B00000000 },
    { 45,   B00100000, B00000000, B00000000, B00000000 },
    { 45,   B00000000, B00000000, B00000000, B00000000 },

    { 45,   B00000010, B10000000, B00000000, B00000000 }, // good
    { 45,   B00000000, B00000000, B00000000, B00000000 },
    { 45,   B00000001, B01000000, B00000000, B00000000 },
    { 45,   B00000000, B00000000, B00000000, B00000000 },
    { 45,   B00000010, B10000000, B00000000, B00000000 },
    { 45,   B00000001, B01000000, B00000000, B00000000 },
    { 45,   B00000010, B10000000, B00000000, B00000000 },
    { 45,   B00000000, B00000000, B00000000, B00000000 },
    { 45,   B10101010, B10000000, B00000000, B00000000 },
    { 45,   B01010101, B01000000, B00000000, B00000000 },
    { 45,   B00000000, B00000000, B00000000, B00000000 },
    { 45,   B11111111, B11000000, B00000000, B00000000 },
    { 45,   B00000011, B11000000, B00000000, B00000000 },
    { 45,   B11111100, B00000000, B00000000, B00000000 },
    { 45,   B00000000, B00000000, B00000000, B00000000 },
    { 45,   B00000000, B00001000, B00000000, B00000000 }, // tip of the hat
    { 500,  B00000000, B00000000, B00000000, B00000000 },
};

static const ServoSequence SeqPanelLongDisco PROGMEM =
{
    { 15,    B00000000, B00000000, B00000000, B00000000 },
    { 15,    B10000000, B00000000, B00000000, B00000000 },
    { 15,    B01000000, B00000000, B00000000, B00000000 },
    { 15,    B00100000, B00000000, B00000000, B00000000 },
    { 15,    B00010000, B00000000, B00000000, B00000000 },
    { 15,    B00001000, B00000000, B00000000, B00000000 },
    { 15,    B00000100, B00000000, B00000000, B00000000 },
    { 15,    B00000010, B00000000, B00000000, B00000000 },
    { 15,    B00000001, B00000000, B00000000, B00000000 },
    { 15,    B00000000, B10000000, B00000000, B00000000 },
    { 15,    B00000000, B01000000, B00000000, B00000000 },
    { 15,    B00000000, B00000000, B00000000, B00000000 },
    { 15,    B00000000, B01000000, B00000000, B00000000 },
    { 15,    B00000000, B10000000, B00000000, B00000000 },
    { 15,    B00000001, B00000000, B00000000, B00000000 },
    { 15,    B00000010, B00000000, B00000000, B00000000 },
    { 15,    B00000100, B00000000, B00000000, B00000000 },
    { 15,    B00001000, B00000000, B00000000, B00000000 },
    { 15,    B00010000, B00000000, B00000000, B00000000 },
    { 15,    B00100000, B00000000, B00000000, B00000000 },
    { 15,    B01000000, B00000000, B00000000, B00000000 },
    { 15,    B10000000, B00000000, B00000000, B00000000 },
    { 15,    B00000000, B00000000, B00000000, B00000000 }, // 3.45 seconds
    { 36000, B00000000, B00000000, B00000000, B00000000 }, // 6 minutes
    { 2200,  B00000000, B00000000, B00000000, B00000000 }, // 22 seconds
};

static const ServoSequence SeqPanelLongHarlemShake PROGMEM =
{
    { 45,    B00000000, B00000000, B00000000, B00000000 },
    { 45,    B10000000, B00000000, B00000000, B00000000 },
    { 45,    B01000000, B00000000, B00000000, B00000000 },
    { 45,    B00100000, B00000000, B00000000, B00000000 },
    { 45,    B00010000, B00000000, B00000000, B00000000 },
    { 45,    B00001000, B00000000, B00000000, B00000000 },
    { 45,    B00000100, B00000000, B00000000, B00000000 },
    { 45,    B00000010, B00000000, B00000000, B00000000 },
    { 45,    B00000001, B00000000, B00000000, B00000000 },
    { 45,    B00000000, B10000000, B00000000, B00000000 },
    { 45,    B00000000, B01000000, B00000000, B00000000 },
    { 45,    B00000000, B00000000, B00000000, B00000000 },
    { 45,    B00000000, B01000000, B00000000, B00000000 },
    { 45,    B00000000, B10000000, B00000000, B00000000 },
    { 45,    B00000001, B00000000, B00000000, B00000000 },
    { 45,    B00000010, B00000000, B00000000, B00000000 },
    { 45,    B00000100, B00000000, B00000000, B00000000 },
    { 45,    B00001000, B00000000, B00000000, B00000000 },
    { 45,    B00010000, B00000000, B00000000, B00000000 },
    { 45,    B00100000, B00000000, B00000000, B00000000 },
    { 45,    B01000000, B00000000, B00000000, B00000000 },
    { 45,    B10000000, B00000000, B00000000, B00000000 },
    { 45,    B00000000, B00000000, B00000000, B00000000 },
};

#define SEQUENCE_PLAY_ONCE(sequencer, sequence, groupMask) \
    (sequencer).play(sequence, SizeOfArray(sequence), groupMask)
#define SEQUENCE_PLAY_ONCE_SPEED(sequencer, sequence, groupMask, speed) \
    (sequencer).play(sequence, SizeOfArray(sequence), groupMask, speed)
#define SEQUENCE_PLAY_ONCE_VARSPEED(sequencer, sequence, groupMask, minspeed, maxspeed) \
    (sequencer).playVariableSpeed(sequence, SizeOfArray(sequence), groupMask, minspeed, maxspeed)
#define SEQUENCE_PLAY_RANDOM_STEP(sequencer, sequence, groupMask) \
    (sequencer).play(&sequence[random(SizeOfArray(sequence))], 1, groupMask)
#define SEQUENCE_PLAY_ONCE_VARSPEED_EASING(sequencer, sequence, groupMask, minspeed, maxspeed, onEasing, offEasing) \
    (sequencer).playVariableSpeed(sequence, SizeOfArray(sequence), groupMask, minspeed, maxspeed, 0.0, 1.0, onEasing, offEasing)


/**
  * \ingroup Core
  *
  * \class ServoSequencer
  *
  * \brief Plays a sequence of servo commands using a servo group mask
  */
class ServoSequencer : public AnimatedEvent
{
public:
    ServoSequencer(ServoDispatch& dispatch) :
        fDispatch(dispatch)
    {
        stop();
    }

    void playVariableSpeed(const ServoStep* sequence, uint16_t length, uint32_t servoGroupMask,
            uint16_t speedMinMS, uint16_t speedMaxMS, float startPos = 0.0, float endPos = 1.0,
            float (*onEasingMethod)(float) = NULL, float (*offEasingMethod)(float) = NULL)
    {
        stop();
        fSequence = (ServoStep*)sequence;
        fServoGroupMask = servoGroupMask;
        fLength = length;
        fStartPos = startPos;
        fEndPos = endPos;
        fSpeedMinMS = speedMinMS;
        fSpeedMaxMS = speedMaxMS;
        fNextStepMS = millis();
        fOffsetFromStart = 0;
        fOffsetFromEnd = 0;
        fIndex = 0;
        fOnEasingMethod = onEasingMethod;
        fOffEasingMethod = offEasingMethod;
    }

    void play(const ServoStep* sequence, uint16_t length, uint32_t servoGroupMask,
            uint16_t speedMS = 0, float startPos = 0.0, float endPos = 1.0)
    {
        playVariableSpeed(sequence, length, servoGroupMask, speedMS, speedMS, startPos, endPos);
    }

    void stop()
    {
        fSequence = NULL;
        fLength = 0;
        fNextStepMS = 0;
    }

    inline bool isFinished() const
    {
        return (fSequence == nullptr);
    }

    virtual void animate() override
    {
        unsigned long currentTime;
        if (fSequence == NULL || (currentTime = millis()) < fNextStepMS)
            return;
        if (fIndex >= fLength)
        {
            stop();
            return;
        }
        uint32_t servoSetMask = 0;
        const ServoStep* step = &fSequence[fIndex];
        while (pgm_read_word(&step->cs) == uint16_t(~0))
        {
            // Check for an initial range limiting step
            uint16_t offsetFromStart = 0;
            uint16_t offsetFromEnd = 0;
            offsetFromStart |= (uint32_t)pgm_read_byte(&step->servo1_8) << 8;
            offsetFromStart |= (uint32_t)pgm_read_byte(&step->servo9_16);
            offsetFromEnd |= (uint32_t)pgm_read_byte(&step->servo17_24) << 8;
            offsetFromEnd |= (uint32_t)pgm_read_byte(&step->servo25_32);
            fOffsetFromStart = (offsetFromStart / 1000.0);
            fOffsetFromEnd = (offsetFromEnd / 1000.0);
            fIndex++;
            if (fIndex >= fLength)
            {
                stop();
                return;
            }
            step = &fSequence[fIndex];
        }
        servoSetMask |= (uint32_t)pgm_read_byte(&step->servo1_8) << 24;
        servoSetMask |= (uint32_t)pgm_read_byte(&step->servo9_16) << 16;
        servoSetMask |= (uint32_t)pgm_read_byte(&step->servo17_24) << 8;
        servoSetMask |= (uint32_t)pgm_read_byte(&step->servo25_32) << 0;
        uint32_t ms = pgm_read_word(&fSequence[fIndex].cs) * 10L;
        fNextStepMS = currentTime + fSpeedMinMS + ms;
        fDispatch.moveServoSetTo(fServoGroupMask, servoSetMask, 0,
            fSpeedMinMS, fSpeedMaxMS, fEndPos-fOffsetFromEnd, fStartPos+fOffsetFromStart, fOnEasingMethod, fOffEasingMethod);
        fIndex++;
    }

private:
    ServoDispatch& fDispatch;
    ServoStep* fSequence;
    uint16_t fLength = 0;
    uint16_t fIndex = 0;
    float fOffsetFromStart;
    float fOffsetFromEnd;
    float fStartPos;
    float fEndPos;
    uint16_t fSpeedMinMS;
    uint16_t fSpeedMaxMS;
    uint32_t fServoGroupMask;
    uint32_t fNextStepMS;
    float (*fOnEasingMethod)(float);
    float (*fOffEasingMethod)(float);
};

#endif
