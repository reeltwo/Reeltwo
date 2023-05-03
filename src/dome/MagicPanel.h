#ifndef MagicPanel_h
#define MagicPanel_h

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include "core/CommandEvent.h"
#include "core/LedControlMAX7221.h"

/**
  * \ingroup Dome
  *
  * \class MagicPanelBase
  *
  * \brief Base class for Magic Panel
  *
  * The MagicPanelBase implements the functionality of the magic panel and depends on the LedControl
  * instance to drive the actual LEDs. There are two instances of LedControlMAX7221 and LedControlNeoPixelGrid
  *
  */
class MagicPanelBase :
    public AnimatedEvent, SetupEvent, CommandEvent
{
public:
    /**
      * \brief Default Constructor
      */
    MagicPanelBase(LedControl& ledControl) :
        fLC(ledControl),
        fDisplayEffect(kNormal),
        fPreviousEffect(~fDisplayEffect),
        fFlipFlop(false),
        fFlipFlopLast(false),
        fStatusDelay(1000),
        fStatusMillis(0),
        fEffectSeqCount(0),
        fEffectSeqDir(0),
        fPrevEffectSeqCount(0),
        fEffectLengthMillis(0),
        fEffectStartMillis(0),
        fDisplayEffectVal(0)
    {
        fLC.setAllPower(true);
    }

    enum EffectValue
    {
        kNormalVal = 0
    };

    enum Sequence
    {
        kNormal = 0,
        kSolid = 1,
        kToggle = 2,
        kFlash = 3,
        kAlert = 4,
        kHorizontalScan = 5,
        kVerticalScan = 6,
        kLife = 7,
        kExpandSolid = 8,
        kCollapseSolid = 9,
        kExpandHollow = 10,
        kCollapseHollow = 11,
        kForwardQ = 12,
        kReverseQ = 13,
        kCistercian = 14
    };

    /**
      * Perform any initialzation not possible in the constructor
      */
    virtual void setup() override
    {
        selectEffect(80000 + 100);
        fStatusMillis = millis();
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
        fDisplayEffectVal = inputNum;
    }

    /**
      * MP00000 - Normal
      * MP10000 - Solid
      * MP20000 - Toggle
      * MP30000 - Flash
      * MP40000 - Alert
      * MP50000 - Horizontal Scan
      * MP60000 - Vertical Scan
      * MP70000 - Game of Life
      * MP80000 - Expand Solid
      * MP90000 - Collapse Solid
      * MP100000 - Expand Hollow
      * MP110000 - Collapse Hollow
      * MP120000 - Forward Q
      * MP130000 - Reverse Q
      * MP140000 - Cistercian
      */
    virtual void handleCommand(const char* cmd) override
    {
        if (*cmd++ == 'M' && *cmd++ == 'P')
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

    /**
      * Perform a single frame of LED animation based on the selected sequence.
      */
    virtual void animate() override
    {
        bool timerExpired = false;
        unsigned long currentMillis = millis();
        if (currentMillis - fStatusMillis >= fStatusDelay)
        {
            timerExpired = true;
            fStatusMillis = currentMillis;
            fFlipFlop = !fFlipFlop;
            fEffectSeqCount += fEffectSeqDir;
            randomSeed(analogRead(A0));
        }

        int selectSequence = (fDisplayEffectVal % 1000000) / 10000;
        // 100ms - 9s
        int selectSpeed = (fDisplayEffectVal % 10000) / 100;
        int selectLength = (fDisplayEffectVal % 100);
        int defaultSpeed = 200;

        switch (selectSequence)
        {
            case kNormal:
            case kSolid:
            case kToggle:
                defaultSpeed = 1000;
                break;
            case kFlash:
            case kHorizontalScan:
            case kVerticalScan:
            case kExpandSolid:
            case kExpandHollow:
            case kForwardQ:
            case kCollapseSolid:
            case kCollapseHollow:
            case kReverseQ:
            case kLife:
                break;
            case kCistercian:
                defaultSpeed = 1000;
                break;
            case kAlert:
                selectSequence = kForwardQ;
                defaultSpeed = 100;
                break;
            default:
                selectSequence = kNormal;
                defaultSpeed = 1000;
                break;
        }
        fDisplayEffect = selectSequence;
        if (fPreviousEffect != fDisplayEffect)
        {
            timerExpired = true;
            fEffectSeqDir = +1;
            fEffectSeqCount = 0;
            switch (selectSequence)
            {
                case kCollapseSolid:
                case kCollapseHollow:
                case kReverseQ:
                    // move frame to end of sequence
                    fEffectSeqDir = -1;
                    fEffectSeqCount = 0x7FFF;
                    break;
            }
            fPrevEffectSeqCount = 0;
            fStatusDelay = (selectSpeed) ? 100 * (selectSpeed) : defaultSpeed;
            fEffectStartMillis = currentMillis;
            fEffectLengthMillis = selectLength * 1000;
            fLC.clearAllDisplays();
        }
        unsigned int effectMillis = currentMillis - fEffectStartMillis;
        if (timerExpired)
        {
            switch (fDisplayEffect)
            {
                case kNormal:
                    /* Do nothing */
                    break;
                case kSolid:
                {
                    if (fPreviousEffect != fDisplayEffect)
                    {
                        for (int i = 0; i < 8; i++)
                            setRow(i, B11111111);
                    }
                    break;
                }
                case kToggle:
                {
                    int i;
                    for (i = 0; i < 4; i++)
                        setRow(i, (!fFlipFlop) ? B11111111 : B00000000);
                    for (; i < 8; i++)
                        setRow(i, (!fFlipFlop) ? B00000000 : B11111111);
                    break;
                }
                case kFlash:
                {
                    if (!fFlipFlop)
                    {
                        for (int i = 0; i < 8; i++)
                            setRow(i, B11111111);
                    }
                    else
                    {
                        for (int i = 0; i < 8; i++)
                            setRow(i, B00000000);
                    }
                    break;
                }
                case kHorizontalScan:
                {
                    if (fPrevEffectSeqCount != fEffectSeqCount)
                        setCol(fPrevEffectSeqCount, 0);
                    if (fEffectSeqCount > 7)
                    {
                        fEffectSeqDir = -1;
                        fEffectSeqCount += fEffectSeqDir;
                    }
                    else if (fEffectSeqCount < 0)
                    {
                        fEffectSeqDir = +1;
                        fEffectSeqCount += fEffectSeqDir;
                    }
                    setCol(fEffectSeqCount, 1);
                    break;
                }
                case kVerticalScan:
                {
                    if (fPrevEffectSeqCount != fEffectSeqCount)
                        setRow(fPrevEffectSeqCount, B00000000);
                    if (fEffectSeqCount > 7)
                    {
                        fEffectSeqDir = -1;
                        fEffectSeqCount += fEffectSeqDir;
                    }
                    else if (fEffectSeqCount < 0)
                    {
                        fEffectSeqDir = +1;
                        fEffectSeqCount += fEffectSeqDir;
                    }
                    setRow(fEffectSeqCount, B11111111);
                    break;
                }
                case kLife:
                {
                    if (fPreviousEffect != fDisplayEffect)
                    {
                        for (int i = 0; i < 8; i++)
                            setRow(i, random(255));
                    }
                    play();
                    bool empty = true;
                    for (int y = 0; y < 8; y++)
                    {
                        if (getRow(y) != 0)
                        {
                            empty = false;
                            break;
                        }
                    }
                    if (empty)
                    {
                        selectEffect(30000 + 100 + 2);
                    }
                    else
                    {
                        setPixel(random(8), random(8), 1);
                    }
                    break;
                }
                case kExpandSolid:
                case kCollapseSolid:
                {
                    const byte* ptr = getFrame_ExpandSolid(fEffectSeqCount, fEffectSeqDir);
                    for (int i = 0; i < 8; i++, ptr++)
                    {
                        setRow(i, pgm_read_byte(ptr));
                    }
                    break;
                }
                case kExpandHollow:
                case kCollapseHollow:
                {
                    const byte* ptr = getFrame_ExpandHollow(fEffectSeqCount, fEffectSeqDir);
                    for (int i = 0; i < 8; i++, ptr++)
                    {
                        setRow(i, pgm_read_byte(ptr));
                    }
                    break;
                }
                case kForwardQ:
                case kReverseQ:
                {
                    const byte* ptr = getFrame_Q(fEffectSeqCount, fEffectSeqDir);
                    for (int i = 0; i < 8; i++, ptr++)
                    {
                        setRow(i, pgm_read_byte(ptr));
                    }
                    break;
                }
                case kCistercian:
                {
                    unsigned count = fEffectSeqCount;
                    unsigned thousands = count / 1000;
                    count -= thousands*1000;
                    unsigned hundreds = count / 100;
                    count -= hundreds*100;
                    unsigned tens = count / 10;
                    count -= tens*10;
                    unsigned ones = count;
                    DEBUG_PRINT(thousands); DEBUG_PRINT(' ');
                    DEBUG_PRINT(hundreds); DEBUG_PRINT(' ');
                    DEBUG_PRINT(tens); DEBUG_PRINT(' ');
                    DEBUG_PRINTLN(ones);
                    byte frame[8];
                    memset(frame, '\0', sizeof(frame));
                    if (thousands != 0)
                    {
                        const byte* ptr = getFrame_Cistercian(thousands*1000);
                        for (int i = 0; i < 8; i++, ptr++)
                        {
                            frame[i] |= pgm_read_byte(ptr);
                        }
                    }
                    if (hundreds != 0)
                    {
                        const byte* ptr = getFrame_Cistercian(hundreds*100);
                        for (int i = 0; i < 8; i++, ptr++)
                        {
                            frame[i] |= pgm_read_byte(ptr);
                        }
                    }
                    if (tens != 0)
                    {
                        const byte* ptr = getFrame_Cistercian(tens*10);
                        for (int i = 0; i < 8; i++, ptr++)
                        {
                            frame[i] |= pgm_read_byte(ptr);
                        }
                    }
                    if (ones != 0)
                    {
                        const byte* ptr = getFrame_Cistercian(ones);
                        for (int i = 0; i < 8; i++, ptr++)
                        {
                            frame[i] |= pgm_read_byte(ptr);
                        }
                    }
                    for (int i = 0; i < 8; i++)
                    {
                        setRow(i, frame[i]);
                    }
                    break;
                }
            }
            fPrevEffectSeqCount = fEffectSeqCount;
        }
        if (fEffectLengthMillis > 0 && fEffectLengthMillis < effectMillis)
        {
            selectEffect(kNormalVal); //go back to normal operation if its time
        }
        fPreviousEffect = fDisplayEffect;
    }

private:
    LedControl& fLC;
    unsigned long fDisplayEffect;
    unsigned long fPreviousEffect;
    bool fFlipFlop; 
    bool fFlipFlopLast;
    unsigned long fStatusDelay;
    unsigned long fStatusMillis;
    int fEffectSeqCount;
    int fEffectSeqDir;
    int fPrevEffectSeqCount;
    unsigned int fEffectLengthMillis;
    unsigned long fEffectStartMillis;
    unsigned long fDisplayEffectVal;

    void setRow(int row, uint8_t bits)
    {
        row *= 2;
        byte device = (row / 8);
        row %= 8;
        fLC.setRow(device, row,   (byte)((bits >> 4) << 4));
        fLC.setRow(device, row+1, (byte)(bits & 0XF));
    }

    byte getRow(int y)
    {
        y *= 2;
        byte device = (y / 8);
        y %= 8;
        return fLC.getRow(device, y) |
                fLC.getRow(device, y+1);
    }

    byte getPixel(int x, int y)
    {
        y *= 2;
        byte device = (y / 8);
        y %= 8;
        if (x < 4)
        {
            byte bits = fLC.getRow(device, y) >> 4;
            return ((bits & (1<<(3-x))) != 0);
        }
        else
        {
            byte bits = fLC.getRow(device, y+1);
            return ((bits & (1<<(7-x))) != 0);
        }
    }

    void setPixel(int x, int y, uint8_t set)
    {
        y *= 2;
        byte device = (y / 8);
        y %= 8;
        if (x < 4)
        {
            byte bits = fLC.getRow(device, y) >> 4;
            bits = (set) ? (bits | (1<<(3-x))) : (bits & ~(1<<(3-x)));
            bits <<= 4;
            fLC.setRow(device, y, (byte)((bits >> 4) << 4));
        }
        else
        {
            byte bits = fLC.getRow(device, y+1);
            bits = (set) ? (bits | (1<<(7-x))) : (bits & ~(1<<(7-x)));
            fLC.setRow(device, y+1, (byte)(bits & 0XF));
        }
    }

    void setCol(int col, uint8_t bit)
    {
        for (int y = 0; y < 8; y++)
        {
            setPixel(col, y, bit);
        }
    }

    static const byte* getFrame_Cistercian(unsigned number)
    {
        static const byte sFrameData[] PROGMEM = {
            ///////// 1
            B00001111,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            ///////// 2
            B00001000,
            B00001000,
            B00001000,
            B00001111,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            ///////// 3
            B00001000,
            B00001100,
            B00001010,
            B00001001,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            ///////// 4
            B00001001,
            B00001010,
            B00001100,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            ///////// 5
            B00001111,
            B00001010,
            B00001100,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            ///////// 6
            B00001001,
            B00001001,
            B00001001,
            B00001001,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            ///////// 7
            B00001111,
            B00001001,
            B00001001,
            B00001001,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            ///////// 8
            B00001001,
            B00001001,
            B00001001,
            B00001111,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            ///////// 9
            B00001111,
            B00001001,
            B00001001,
            B00001111,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            ///////// 10
            B01111000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            ///////// 20
            B00001000,
            B00001000,
            B00001000,
            B01111000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            ///////// 30
            B00001000,
            B00011000,
            B00101000,
            B01001000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            ///////// 40
            B01001000,
            B00101000,
            B00011000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            ///////// 50
            B01111000,
            B00101000,
            B00011000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            ///////// 60
            B01001000,
            B01001000,
            B01001000,
            B01001000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            ///////// 70
            B01111000,
            B01001000,
            B01001000,
            B01001000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            ///////// 80
            B01001000,
            B01001000,
            B01001000,
            B01111000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            ///////// 90
            B01111000,
            B01001000,
            B01001000,
            B01111000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            ///////// 100
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001111,
            ///////// 200
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001111,
            B00001000,
            B00001000,
            B00001000,
            ///////// 300
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001001,
            B00001010,
            B00001100,
            B00001000,
            ///////// 400
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001100,
            B00001010,
            B00001001,
            ///////// 500
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001100,
            B00001010,
            B00001111,
            ///////// 600
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001001,
            B00001001,
            B00001001,
            B00001001,
            ///////// 700
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001001,
            B00001001,
            B00001001,
            B00001111,
            ///////// 800
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001111,
            B00001001,
            B00001001,
            B00001001,
            ///////// 900
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001111,
            B00001001,
            B00001001,
            B00001111,
            ///////// 1000
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B01111000,
            ///////// 2000
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B01111000,
            B00001000,
            B00001000,
            B00001000,
            ///////// 3000
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B01001000,
            B00101000,
            B00011000,
            B00001000,
            ///////// 4000
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00011000,
            B00101000,
            B01001000,
            ///////// 5000
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B00011000,
            B00101000,
            B01111000,
            ///////// 6000
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B01001000,
            B01001000,
            B01001000,
            B01001000,
            ///////// 7000
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B01001000,
            B01001000,
            B01001000,
            B01111000,
            ///////// 8000
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B01111000,
            B01001000,
            B01001000,
            B01001000,
            ///////// 9000
            B00001000,
            B00001000,
            B00001000,
            B00001000,
            B01111000,
            B01001000,
            B01001000,
            B01111000,
        };
        unsigned ln = log10(number);
        DEBUG_PRINT("number: "); DEBUG_PRINT(number);
        if (ln > 0)
            number /= (ln*10);
        DEBUG_PRINT(" frame: "); DEBUG_PRINTLN((number-1)+ln*9);
        return &sFrameData[((number-1)+ln*9)*8];
    }

    static const byte* getFrame_ExpandSolid(int &frameIndex, int direction)
    {
        static const byte sFrameData[] PROGMEM = {
            B00000000,
            B00000000,
            B00000000,
            B00011000,
            B00011000,
            B00000000,
            B00000000,
            B00000000,
            /////////
            B00000000,
            B00000000,
            B00111100,
            B00111100,
            B00111100,
            B00111100,
            B00000000,
            B00000000,
            /////////
            B00000000,
            B01111110,
            B01111110,
            B01111110,
            B01111110,
            B01111110,
            B01111110,
            B00000000,
            /////////
            B11111111,
            B11111111,
            B11111111,
            B11111111,
            B11111111,
            B11111111,
            B11111111,
            B11111111
        };
        unsigned frameCount = sizeof(sFrameData) / 8;
        if (unsigned(frameIndex) >= frameCount)
            frameIndex = (direction > 0) ? 0 : frameCount-1;
        return &sFrameData[frameIndex*8];
    }

    static const byte* getFrame_ExpandHollow(int &frameIndex, int direction)
    {
        static const byte sFrameData[] PROGMEM = {
            B00000000,
            B00000000,
            B00000000,
            B00011000,
            B00011000,
            B00000000,
            B00000000,
            B00000000,
            /////////
            B00000000,
            B00000000,
            B00111100,
            B00100100,
            B00100100,
            B00111100,
            B00000000,
            B00000000,      
            /////////
            B00000000,
            B01111110,
            B01000010,
            B01000010,
            B01000010,
            B01000010,
            B01111110,
            B00000000,
            /////////
            B11111111,
            B10000001,
            B10000001,
            B10000001,
            B10000001,
            B10000001,
            B10000001,
            B11111111,
            /////////
            B00000000,
            B00000000,
            B00000000,
            B00000000,
            B00000000,
            B00000000,
            B00000000,
            B00000000
        };
        unsigned frameCount = sizeof(sFrameData) / 8;
        if (unsigned(frameIndex) >= frameCount)
            frameIndex = (direction > 0) ? 0 : frameCount-1;
        return &sFrameData[frameIndex*8];
    }

    static const byte* getFrame_Q(int &frameIndex, int direction)
    {
        static const byte sFrameData[] PROGMEM = {
            B00001111,
            B00001111,
            B00001111,
            B00001111,
            B11110000,
            B11110000,
            B11110000,
            B11110000,
            /////////
            B11110000,
            B11110000,
            B11110000,
            B11110000,
            B00001111,
            B00001111,
            B00001111,
            B00001111
        };
        unsigned frameCount = sizeof(sFrameData) / 8;
        if (unsigned(frameIndex) >= frameCount)
            frameIndex = (direction > 0) ? 0 : frameCount-1;
        return &sFrameData[frameIndex*8];
    }

    static inline int wrap(int i, int a)
    {
        i += a;
        while (i < 0)
            i += 8;
        while (i >= 8)
            i -= 8;
        return i;
    }

    void play()
    {
        byte newbits[8];
        for (int y = 0; y < 8; y++)
        {
            newbits[y] = 0;
            for (int x = 0; x < 8; x++)
            {
                int a = 0;
                for (int k = -1; k <= 1; k++)
                {
                    for (int l = -1; l <= 1; l++)
                    {
                        if (k || l)
                        {
                            a += getPixel(wrap(x, k), wrap(y, l));
                        }
                    }
                }
                if (a == 2) newbits[y] |= (getPixel(x, y)<<(7-x));
                if (a == 3) newbits[y] |= (1<<(7-x));
            }
        }
        for (int y = 0; y < 8; y++)
        {
            setRow(y, newbits[y]);
        }
    }
};

/**
  * \ingroup Dome
  *
  * \class MagicPanel
  *
  * \brief Magic Panel by ia-parts.com
  *
  * The MagicPanel is implemented as an 8x8 LED grid using 2 device MAX7221. 
  *
  * Default pinout:
  *   pin 8 is connected to the DataIn 
  *   pin 7 is connected to the CLK 
  *   pin 6 is connected to LOAD 
  */
class MagicPanel :
    protected LedControlMAX7221<2>,
    public MagicPanelBase
{
public:
    /**
      * \brief Default Constructor
      */
    MagicPanel(byte dataPin = 8, byte clkPin = 7, byte csPin = 6) :
        LedControlMAX7221<2>(dataPin, clkPin, csPin),
        MagicPanelBase((LedControl&)*this)
    {
    }
};
#endif
