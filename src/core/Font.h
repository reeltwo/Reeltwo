#ifndef Font_h
#define Font_h

/**
  * \ingroup Font
  *
  * \class Font4x4
  *
  * \brief Base class for 4x4 fonts
*/
class Font4x4
{
public:
    virtual bool getLetter(const char inChar, byte* outBuffer) = 0;

protected:
    bool getLetter4x4(const char inChar, byte* outBuffer, const byte* fontData, size_t fontDataSize)
    {
        const byte* ptr = fontData;
        const byte* end = fontData + fontDataSize;
        for (; ptr < end; ptr += 3)
        {
            if (pgm_read_byte(ptr++) == inChar)
            {
                for (byte i = 0; i < 3; i++)
                {
                    outBuffer[i] = pgm_read_byte(ptr++);
                }
                return true;
            }
        }
        ptr = &fontData[1];
        for (byte i = 0; i < 3; i++)
        {
            outBuffer[i] = pgm_read_byte(ptr++);
        }
        return false;
    }
};

/**
  * \ingroup Font
  *
  * \class Font8x5
  *
  * \brief Base class for 8x5 fonts
*/
class Font8x5
{
public:
    virtual bool getLetter(const char inChar, byte* outBuffer) = 0;

protected:
    bool getLetter8x5(const char inChar, byte* outBuffer, const byte* fontData, size_t fontDataSize)
    {
        const byte* ptr = fontData;
        const byte* end = fontData + fontDataSize;
        for (; ptr < end; ptr += 5)
        {
            if (pgm_read_byte(ptr++) == inChar)
            {
                for (byte i = 0; i < 5; i++)
                {
                    outBuffer[i] = pgm_read_byte(ptr++);
                }
                return true;
            }
        }
        ptr = &fontData[1];
        for (byte i = 0; i < 5; i++)
        {
            outBuffer[i] = pgm_read_byte(ptr++);
        }
        return false;
    }
};

/**
  * \ingroup Font
  *
  * \class Font8x8
  *
  * \brief Base class for 8x8 fonts
*/
class Font8x8
{
public:
    virtual bool getLetter(const char inChar, byte* outBuffer) = 0;
};

/**
  * \ingroup Font
  *
  * \class FontVar4Pt
  *
  * \brief Base class for variable width 4pt fonts
*/
class FontVar4Pt
{
public:
    virtual bool getLetter(const char inChar, byte* outBuffer, byte &rowBytes, byte& advance);

protected:
    bool getLetterVar4Pt(const char inChar, byte* outBuffer, byte &rowBytes, byte& advance, const byte* fontData, size_t fontDataSize)
    {
        const byte* ptr = fontData;
        const byte* end = fontData + fontDataSize;
        for (; ptr < end;)
        {
            char ch = pgm_read_byte(ptr++);
            advance = pgm_read_byte(ptr++);
            rowBytes = (advance <= 4) ? 1 : 2;
            if (ch == inChar)
            {
                byte* dst = outBuffer;
                for (byte y = 0; y < 4; y++)
                {
                    for (byte i = 0; i < rowBytes; i++)
                        *dst++ = pgm_read_byte(ptr++);
                }
                return true;
            }
            ptr += rowBytes * 4;
        }
        // interpret missing glyph as white-space
        advance = 3;
        rowBytes = 0;
        return false;
    }
};

/**
  * \ingroup Font
  *
  * \class LatinFontVar4pt
  *
  * \brief Variable width 4pt Latin font
*/
class LatinFontVar4pt : public FontVar4Pt
{
public:
    static FontVar4Pt* instance()
    {
        static LatinFontVar4pt font;
        return &font;
    }

    virtual bool getLetter(const char inChar, byte* outBuffer, byte &rowBytes, byte& advance)
    {
        static const byte sData[] PROGMEM =
        {
            '.', 1,
            0b00000000,
            0b00000000,
            0b00000000,
            0b11000000,

            'A', 3,
            0b10111000,
            0b11001100,
            0b11111100,
            0b11001100,

            'B', 3,
            0b11000000,
            0b11111000,
            0b11001100,
            0b11111000,

            'C', 3,
            0b10111100,
            0b11000000,
            0b11000000,
            0b10111100,

            'D', 3,
            0b11111000,
            0b11001100,
            0b11001100,
            0b11111000,

            'E', 3,
            0b11111100,
            0b11000000,
            0b11110000,
            0b11111100,

            'F', 3,
            0b11111100,
            0b11000000,
            0b11110000,
            0b11000000,

            'G', 3,
            0b10111100,
            0b11000000,
            0b11001100,
            0b10111100,

            'H', 3,
            0b11001100,
            0b11001100,
            0b11111100,
            0b11001100,

            'I', 1,
            0b11000000,
            0b11000000,
            0b11000000,
            0b11000000,

            'J', 2,
            0b00110000,
            0b00110000,
            0b00110000,
            0b11100000,

            'K', 3,
            0b11001100,
            0b11110000,
            0b11001100,
            0b11001100,

            'L', 3,
            0b11000000,
            0b11000000,
            0b11000000,
            0b11111100,

            'M', 3,
            0b11101100,
            0b11111100,
            0b11001100,
            0b11001100,

            'N', 3,
            0b11001100,
            0b11111100,
            0b11111100,
            0b11001100,

            'O', 3,
            0b10111000,
            0b11001100,
            0b11001100,
            0b10111000,

            'P', 3,
            0b11111000,
            0b11001100,
            0b11111000,
            0b11000000,

            'Q', 4,
            0b10111000,
            0b11001100,
            0b11001100,
            0b10111111,

            'R', 3,
            0b11110000,
            0b11001100,
            0b11110000,
            0b11001100,

            'S', 3,
            0b11111100,
            0b11000000,
            0b00001100,
            0b11111100,

            'T', 3,
            0b11111100,
            0b00110000,
            0b00110000,
            0b00110000,

            'U', 3,
            0b11001100,
            0b11001100,
            0b11001100,
            0b10111000,

            'V', 3,
            0b11001100,
            0b11001100,
            0b11001100,
            0b00110000,

            'W', 5,
            0b11000000, 0b11000000,
            0b11001100, 0b11000000,
            0b11101110, 0b11000000,
            0b00110011, 0b00000000,

            'X', 3,
            0b11001100,
            0b00110000,
            0b00110000,
            0b11001100,

            'Y', 3,
            0b11001100,
            0b11001100,
            0b00110000,
            0b00110000,

            'Z', 3,
            0b11111100,
            0b00101100,
            0b11100000,
            0b11111100,

            '\'', 1,
            0b11000000,
            0b00000000,
            0b00000000,
            0b00000000,

            '!', 1,
            0b11000000,
            0b11000000,
            0b00000000,
            0b11000000,

            '-', 3,
            0b00000000,
            0b00000000,
            0b00111100,
            0b00000000,

            '0', 3,
            0b10111000,
            0b11001100,
            0b11001100,
            0b10111000,

            '1', 2,
            0b00110000,
            0b10110000,
            0b00110000,
            0b00110000,

            '2', 3,
            0b11110000,
            0b10001100,
            0b00110000,
            0b11111100,

            '3', 3,
            0b11111100,
            0b00111100,
            0b00001100,
            0b11111100,

            '4', 3,
            0b11001100,
            0b11001100,
            0b11111100,
            0b00001100,

            '5', 3,
            0b11111100,
            0b11000000,
            0b00001100,
            0b11111100,

            '6', 3,
            0b11000000,
            0b11111000,
            0b11001100,
            0b11111100,

            '7', 3,
            0b11111100,
            0b00001100,
            0b00001100,
            0b00001100,

            '8', 3,
            0b11111100,
            0b11001100,
            0b11101100,
            0b11111100,

            '9', 3,
            0b11111100,
            0b11001100,
            0b10111100,
            0b00001100,
        };
        return getLetterVar4Pt(inChar, outBuffer, rowBytes, advance, sData, sizeof(sData));
    }
};

/**
  * \ingroup Font
  *
  * \class LatinFont4x4
  *
  * \brief Variable width 4pt Latin font
*/
class LatinFont4x4 : public Font4x4
{
public:
    static Font4x4* instance()
    {
        static LatinFont4x4 font;
        return &font;
    }

    virtual bool getLetter(const char inChar, byte* outBuffer)
    {
        static const byte sData[] PROGMEM =
        {
            'A',
            0b11111001,
            0b11111001,

            'B',
            0b10001110,
            0b10101110,

            'C',
            0b11111000,
            0b10001111,

            'D',
            0b11101001,
            0b10011110,

            'E',
            0b11111110,
            0b10001111,

            'F',
            0b11111110,
            0b10001000,

            'G',
            0b11111000,
            0b10011111,

            'H',
            0b10001000,
            0b11111001,

            'I',
            0b01000100,
            0b01000100,

            'J',
            0b01000100,
            0b01001100,

            'K',
            0b10101100,
            0b10101001,

            'L',
            0b10001000,
            0b10001111,

            'M',
            0b10011111,
            0b10011001,

            'N',
            0b10011101,
            0b10111001,

            'O',
            0b11111001,
            0b10011111,

            'P',
            0b11111001,
            0b11101000,

            'Q',
            0b11111001,
            0b11110001,

            'R',
            0b11101001,
            0b11001010,

            'S',
            0b11111000,
            0b00011111,

            'T',
            0b11110100,
            0b01000100,

            'U',
            0b10011001,
            0b10011111,

            'V',
            0b10010101,
            0b00110001,

            'W',
            0b10011001,
            0b10010110,

            'X',
            0b00001010,
            0b01001010,

            'Y',
            0b10010110,
            0b00100010,

            'Z',
            0b11110100,
            0b00101111,
        };
        return getLetter4x4(inChar, outBuffer, sData, sizeof(sData));
    }
};

/**
  * \ingroup Font
  *
  * \class LatinFont8x5
  *
  * \brief Latin font 8x5
*/
class LatinFont8x5 : public Font8x5
{
public:
    static Font8x5* instance()
    {
        static LatinFont8x5 font;
        return &font;
    }

    virtual bool getLetter(const char inChar, byte* outBuffer)
    {
        ///////////////////////////////
        // Latin Alphabet
        static const byte sData[] PROGMEM =
        {
            ' ', // space
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,

            'A',
            0b01100000,
            0b10010000,
            0b11110000,
            0b10010000,
            0b10010000,

            'B',
            0b11100000,
            0b10010000,
            0b11100000,
            0b10010000,
            0b11100000,

            'C',
            0b01100000,
            0b10010000,
            0b10000000,
            0b10010000,
            0b01100000,

            'D',
            0b1110000,
            0b1001000,
            0b1001000,
            0b1001000,
            0b1110000,

            'E',
            0b1111000,
            0b1000000,
            0b1110000,
            0b1000000,
            0b1111000,
            
            'F',
            0b11110000,
            0b10000000,
            0b11100000,
            0b10000000,
            0b10000000,
            
            'G',
            0b01110000,
            0b10000000,
            0b10110000,
            0b10010000,
            0b01100000,
            
            'H',
            0b10010000,
            0b10010000,
            0b11110000,
            0b10010000,
            0b10010000,
            
            'I',
            0b11100000,
            0b01000000,
            0b01000000,
            0b01000000,
            0b11100000,
            
            'J',
            0b00010000,
            0b00010000,
            0b00010000,
            0b10010000,
            0b01100000,
            
            'K',
            0b10010000,
            0b10100000,
            0b11000000,
            0b10100000,
            0b10010000,
            
            'L',
            0b10000000,
            0b10000000,
            0b10000000,
            0b10000000,
            0b11110000,
            
            'M',
            0b10001000,
            0b11011000,
            0b10101000,
            0b10001000,
            0b10001000,
            
            'N',
            0b10010000,
            0b11010000,
            0b10110000,
            0b10010000,
            0b10010000,
            
            'O',
            0b01100000,
            0b10010000,
            0b10010000,
            0b10010000,
            0b01100000,
            
            'P',
            0b11100000,
            0b10010000,
            0b11100000,
            0b10000000,
            0b10000000,
            
            'Q',
            0b01100000,
            0b10010000,
            0b10110000,
            0b10010000,
            0b01101000,
            
            'R',
            0b11100000,
            0b10010000,
            0b11100000,
            0b10100000,
            0b10010000,
            
            'S',
            0b01110000,
            0b10000000,
            0b01100000,
            0b00010000,
            0b11100000,

            'T',
            0b11111000,
            0b00100000,
            0b00100000,
            0b00100000,
            0b00100000,

            'U',
            0b10010000,
            0b10010000,
            0b10010000,
            0b10010000,
            0b01100000,

            'V',
            0b10010000,
            0b10010000,
            0b10010000,
            0b01100000,
            0b01100000,

            'W',
            0b10001000,
            0b10001000,
            0b10101000,
            0b11011000,
            0b10001000,

            'X',
            0b10010000,
            0b10010000,
            0b01100000,
            0b10010000,
            0b10010000,

            'Y',
            0b10010000,
            0b10010000,
            0b01100000,
            0b01100000,
            0b01100000,

            'Z',
            0b11110000,
            0b00100000,
            0b01000000,
            0b10000000,
            0b11110000,

            '0',
            0b01100000,
            0b10110000,
            0b11010000,
            0b10010000,
            0b01100000,

            '1',
            0b01100000,
            0b10100000,
            0b00100000,
            0b00100000,
            0b11110000,

            '2',
            0b01100000,
            0b10010000,
            0b00100000,
            0b01000000,
            0b11110000,

            '3',
            0b11110000,
            0b00010000,
            0b01110000,
            0b00010000,
            0b11110000,

            '4',
            0b10010000,
            0b10010000,
            0b11110000,
            0b00010000,
            0b00010000,

            '5',
            0b11110000,
            0b10000000,
            0b11100000,
            0b00010000,
            0b11100000,

            '6',
            0b01100000,
            0b10000000,
            0b11100000,
            0b10010000,
            0b01100000,

            '7',
            0b11110000,
            0b00010000,
            0b00100000,
            0b00100000,
            0b00100000,

            '8',
            0b01100000,
            0b10010000,
            0b01100000,
            0b10010000,
            0b01100000,

            '9',
            0b01100000,
            0b10010000,
            0b01110000,
            0b00010000,
            0b01100000,

            '*', // Heart symbol
            0b01101100,
            0b10010010,
            0b10000010,
            0b01000100,
            0b00010000,

            '#', // Hash
            0b01010000,
            0b11111000,
            0b01010000,
            0b11111000,
            0b01010000,

            '@', // @
            0b01100000,
            0b10010000,
            0b10110000,
            0b10000000,
            0b01110000,

            '-', // dash - Symbol    
            0b00000000,
            0b00000000,
            0b11100000,
            0b00000000,
            0b00000000,

            '|', // vertical bar
            0b10000000,
            0b10000000,
            0b10000000,
            0b10000000,
            0b10000000,

            '<', // up symbol
            0b10000000,
            0b01000000,
            0b00100000,
            0b00010000,
            0b00001000,

            '>', // down symbol
            0b00001000,
            0b00010000,
            0b00100000,
            0b01000000,
            0b10000000,

            '.', // dot symbol
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b10000000,

            '?', // question mark
            0b11000000,
            0b00100000,
            0b01000000,
            0b00000000,
            0b01000000,

            '!', // exclamation point
            0b10000000,
            0b10000000,
            0b10000000,
            0b00000000,
            0b10000000,
        };
        return getLetter8x5(inChar, outBuffer, sData, sizeof(sData));
    }
};

/**
  * \ingroup Font
  *
  * \class AurabeshFont8x5
  *
  * \brief Aurabesh font 8x5
*/
class AurabeshFont8x5 : public Font8x5
{
public:
    static Font8x5* instance()
    {
        static AurabeshFont8x5 font;
        return &font;
    }

    virtual bool getLetter(const char inChar, byte* outBuffer)
    {
        ///////////////////////////////
        // Aurabesh Alphabet
        static const byte sData[] PROGMEM =
        {
            ' ', // space
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,
            0b00000000,

            '2',
            0b11110000,
            0b10010000,
            0b00100000,
            0b10010000,
            0b11110000,

            'A',
            0b10001000,
            0b01111000,
            0b00000000,
            0b01111000,
            0b10001000,

            'B',
            0b01110000,
            0b10001000,
            0b01110000,
            0b10001000,
            0b01110000,

            'C',
            0b00001000,
            0b00001000,
            0b00100000,
            0b10000000,
            0b10000000,

            'D',
            0b11111000,
            0b01000000,
            0b00111000,
            0b00010000,
            0b00001000,

            'E',
            0b11001000,
            0b11001000,
            0b11001000,
            0b10110000,
            0b10100000,

            'F',
            0b10000000,
            0b01010000,
            0b00111000,
            0b00011000,
            0b11111000,

            'G',
            0b11101000,
            0b10101000,
            0b10001000,
            0b01001000,
            0b00111000,

            'H',
            0b11111000,
            0b00000000,
            0b01110000,
            0b00000000,
            0b11111000,

            'I',
            0b10000000,
            0b11000000,
            0b10000000,
            0b10000000,
            0b10000000,

            'J',
            0b10000000,
            0b11000000,
            0b01111000,
            0b00100000,
            0b00011000,

            'K',
            0b11111000,
            0b10000000,
            0b10000000,
            0b10000000,
            0b11111000,

            'L',
            0b10000000,
            0b10001000,
            0b10010000,
            0b10100000,
            0b11000000,

            'M',
            0b11000000,
            0b00100000,
            0b00010000,
            0b00001000,
            0b11111000,

            'N',
            0b01010000,
            0b10101000,
            0b10101000,
            0b10011000,
            0b10010000,

            'O',
            0b00000000,
            0b01110000,
            0b10001000,
            0b10001000,
            0b11111000,

            'P',
            0b10110000,
            0b10101000,
            0b10001000,
            0b10001000,
            0b11110000,

            'Q',
            0b11111000,
            0b10001000,
            0b00001000,
            0b00001000,
            0b00111000,

            'R',
            0b11111000,
            0b01000000,
            0b00100000,
            0b00010000,
            0b00001000,

            'S',
            0b10000000,
            0b10010000,
            0b10101000,
            0b11010000,
            0b10100000,

            'T',
            0b01000000,
            0b01000000,
            0b01000000,
            0b11100000,
            0b01000000,

            'U',
            0b11001000,
            0b10001000,
            0b10001000,
            0b10001000,
            0b01110000,

            'V',
            0b10001000,
            0b01010000,
            0b00100000,
            0b00100000,
            0b00100000,

            'W',
            0b11111000,
            0b10001000,
            0b10001000,
            0b10001000,
            0b11111000,

            'X',
            0b00100000,
            0b01010000,
            0b10001000,
            0b10001000,
            0b11111000,

            'Y',
            0b10011000,
            0b10101000,
            0b01010000,
            0b01010000,
            0b00100000,

            'Z',
            0b10110000,
            0b10101000,
            0b10000000,
            0b10001000,
            0b11111000
        };
        return getLetter8x5(inChar, outBuffer, sData, sizeof(sData));
    }
};

/**
  * \ingroup Font
  *
  * \class LatinFont8x8
  *
  * \brief Latin font 8x8
*/
class LatinFont8x8 : public Font8x8
{
public:
    static Font8x8* instance()
    {
        static LatinFont8x8 font;
        return &font;
    }

    virtual bool getLetter(const char inChar, byte* outBuffer)
    {
        ///////////////////////////////
        // Latin Alphabet
        // ---------------------------  0-127 ---------------------------
        static const byte sData[128][8] PROGMEM = {
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0000 (uni0000.dup1)
            {0x7e, 0x81, 0xa5, 0x81, 0xbd, 0x99, 0x81, 0x7e},  // 0001 (uni0001)
            {0x7e, 0xff, 0xdb, 0xff, 0xc3, 0xe7, 0xff, 0x7e},  // 0002 (uni0002)
            {0x6c, 0xfe, 0xfe, 0xfe, 0x7c, 0x38, 0x10, 0x00},  // 0003 (uni0003)
            {0x10, 0x38, 0x7c, 0xfe, 0x7c, 0x38, 0x10, 0x00},  // 0004 (uni0004)
            {0x38, 0x7c, 0x38, 0xfe, 0xfe, 0x7c, 0x38, 0x7c},  // 0005 (uni0005)
            {0x10, 0x10, 0x38, 0x7c, 0xfe, 0x7c, 0x38, 0x7c},  // 0006 (uni0006)
            {0x00, 0x00, 0x18, 0x3c, 0x3c, 0x18, 0x00, 0x00},  // 0007 (uni0007)
            {0xff, 0xff, 0xe7, 0xc3, 0xc3, 0xe7, 0xff, 0xff},  // 0008 (uni0008)
            {0x00, 0x3c, 0x66, 0x42, 0x42, 0x66, 0x3c, 0x00},  // 0009 (uni0009)
            {0xff, 0xc3, 0x99, 0xbd, 0xbd, 0x99, 0xc3, 0xff},  // 000a (uni000A)
            {0x0f, 0x07, 0x0f, 0x7d, 0xcc, 0xcc, 0xcc, 0x78},  // 000b (uni000B)
            {0x3c, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x7e, 0x18},  // 000c (uni000C)
            {0x3f, 0x33, 0x3f, 0x30, 0x30, 0x70, 0xf0, 0xe0},  // 000d (uni000D)
            {0x7f, 0x63, 0x7f, 0x63, 0x63, 0x67, 0xe6, 0xc0},  // 000e (uni000E)
            {0x99, 0x5a, 0x3c, 0xe7, 0xe7, 0x3c, 0x5a, 0x99},  // 000f (uni000F)
            {0x80, 0xe0, 0xf8, 0xfe, 0xf8, 0xe0, 0x80, 0x00},  // 0010 (uni0010)
            {0x02, 0x0e, 0x3e, 0xfe, 0x3e, 0x0e, 0x02, 0x00},  // 0011 (uni0011)
            {0x18, 0x3c, 0x7e, 0x18, 0x18, 0x7e, 0x3c, 0x18},  // 0012 (uni0012)
            {0x66, 0x66, 0x66, 0x66, 0x66, 0x00, 0x66, 0x00},  // 0013 (uni0013)
            {0x7f, 0xdb, 0xdb, 0x7b, 0x1b, 0x1b, 0x1b, 0x00},  // 0014 (uni0014)
            {0x3e, 0x63, 0x38, 0x6c, 0x6c, 0x38, 0xcc, 0x78},  // 0015 (uni0015)
            {0x00, 0x00, 0x00, 0x00, 0x7e, 0x7e, 0x7e, 0x00},  // 0016 (uni0016)
            {0x18, 0x3c, 0x7e, 0x18, 0x7e, 0x3c, 0x18, 0xff},  // 0017 (uni0017)
            {0x18, 0x3c, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x00},  // 0018 (uni0018)
            {0x18, 0x18, 0x18, 0x18, 0x7e, 0x3c, 0x18, 0x00},  // 0019 (uni0019)
            {0x00, 0x18, 0x0c, 0xfe, 0x0c, 0x18, 0x00, 0x00},  // 001a (uni001A)
            {0x00, 0x30, 0x60, 0xfe, 0x60, 0x30, 0x00, 0x00},  // 001b (uni001B)
            {0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xfe, 0x00, 0x00},  // 001c (uni001C)
            {0x00, 0x24, 0x66, 0xff, 0x66, 0x24, 0x00, 0x00},  // 001d (uni001D)
            {0x00, 0x18, 0x3c, 0x7e, 0xff, 0xff, 0x00, 0x00},  // 001e (uni001E)
            {0x00, 0xff, 0xff, 0x7e, 0x3c, 0x18, 0x00, 0x00},  // 001f (uni001F)
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0020 (space)
            {0x30, 0x78, 0x78, 0x30, 0x30, 0x00, 0x30, 0x00},  // 0021 (exclam)
            {0x6c, 0x6c, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0022 (quotedbl)
            {0x6c, 0x6c, 0xfe, 0x6c, 0xfe, 0x6c, 0x6c, 0x00},  // 0023 (numbersign)
            {0x30, 0x7c, 0xc0, 0x78, 0x0c, 0xf8, 0x30, 0x00},  // 0024 (dollar)
            {0x00, 0xc6, 0xcc, 0x18, 0x30, 0x66, 0xc6, 0x00},  // 0025 (percent)
            {0x38, 0x6c, 0x38, 0x76, 0xdc, 0xcc, 0x76, 0x00},  // 0026 (ampersand)
            {0x60, 0x60, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0027 (quotesingle)
            {0x18, 0x30, 0x60, 0x60, 0x60, 0x30, 0x18, 0x00},  // 0028 (parenleft)
            {0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60, 0x00},  // 0029 (parenright)
            {0x00, 0x66, 0x3c, 0xff, 0x3c, 0x66, 0x00, 0x00},  // 002a (asterisk)
            {0x00, 0x30, 0x30, 0xfc, 0x30, 0x30, 0x00, 0x00},  // 002b (plus)
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x60},  // 002c (comma)
            {0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00},  // 002d (hyphen)
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00},  // 002e (period)
            {0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0x80, 0x00},  // 002f (slash)
            {0x7c, 0xc6, 0xce, 0xde, 0xf6, 0xe6, 0x7c, 0x00},  // 0030 (zero)
            {0x30, 0x70, 0x30, 0x30, 0x30, 0x30, 0xfc, 0x00},  // 0031 (one)
            {0x78, 0xcc, 0x0c, 0x38, 0x60, 0xc4, 0xfc, 0x00},  // 0032 (two)
            {0x78, 0xcc, 0x0c, 0x38, 0x0c, 0xcc, 0x78, 0x00},  // 0033 (three)
            {0x1c, 0x3c, 0x6c, 0xcc, 0xfe, 0x0c, 0x1e, 0x00},  // 0034 (four)
            {0xfc, 0xc0, 0xf8, 0x0c, 0x0c, 0xcc, 0x78, 0x00},  // 0035 (five)
            {0x38, 0x60, 0xc0, 0xf8, 0xcc, 0xcc, 0x78, 0x00},  // 0036 (six)
            {0xfc, 0xcc, 0x0c, 0x18, 0x30, 0x30, 0x30, 0x00},  // 0037 (seven)
            {0x78, 0xcc, 0xcc, 0x78, 0xcc, 0xcc, 0x78, 0x00},  // 0038 (eight)
            {0x78, 0xcc, 0xcc, 0x7c, 0x0c, 0x18, 0x70, 0x00},  // 0039 (nine)
            {0x00, 0x30, 0x30, 0x00, 0x00, 0x30, 0x30, 0x00},  // 003a (colon)
            {0x00, 0x30, 0x30, 0x00, 0x30, 0x30, 0x60, 0x00},  // 003b (semicolon)
            {0x18, 0x30, 0x60, 0xc0, 0x60, 0x30, 0x18, 0x00},  // 003c (less)
            {0x00, 0x00, 0xfc, 0x00, 0x00, 0xfc, 0x00, 0x00},  // 003d (equal)
            {0x60, 0x30, 0x18, 0x0c, 0x18, 0x30, 0x60, 0x00},  // 003e (greater)
            {0x78, 0xcc, 0x0c, 0x18, 0x30, 0x00, 0x30, 0x00},  // 003f (question)
            {0x7c, 0xc6, 0xde, 0xde, 0xde, 0xc0, 0x78, 0x00},  // 0040 (at)
            {0x30, 0x78, 0xcc, 0xcc, 0xfc, 0xcc, 0xcc, 0x00},  // 0041 (A)
            {0xfc, 0x66, 0x66, 0x7c, 0x66, 0x66, 0xfc, 0x00},  // 0042 (B)
            {0x3c, 0x66, 0xc0, 0xc0, 0xc0, 0x66, 0x3c, 0x00},  // 0043 (C)
            {0xf8, 0x6c, 0x66, 0x66, 0x66, 0x6c, 0xf8, 0x00},  // 0044 (D)
            {0xfe, 0x62, 0x68, 0x78, 0x68, 0x62, 0xfe, 0x00},  // 0045 (E)
            {0xfe, 0x62, 0x68, 0x78, 0x68, 0x60, 0xf0, 0x00},  // 0046 (F)
            {0x3c, 0x66, 0xc0, 0xc0, 0xce, 0x66, 0x3e, 0x00},  // 0047 (G)
            {0xcc, 0xcc, 0xcc, 0xfc, 0xcc, 0xcc, 0xcc, 0x00},  // 0048 (H)
            {0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00},  // 0049 (I)
            {0x1e, 0x0c, 0x0c, 0x0c, 0xcc, 0xcc, 0x78, 0x00},  // 004a (J)
            {0xe6, 0x66, 0x6c, 0x78, 0x6c, 0x66, 0xe6, 0x00},  // 004b (K)
            {0xf0, 0x60, 0x60, 0x60, 0x62, 0x66, 0xfe, 0x00},  // 004c (L)
            {0xc6, 0xee, 0xfe, 0xfe, 0xd6, 0xc6, 0xc6, 0x00},  // 004d (M)
            {0xc6, 0xe6, 0xf6, 0xde, 0xce, 0xc6, 0xc6, 0x00},  // 004e (N)
            {0x38, 0x6c, 0xc6, 0xc6, 0xc6, 0x6c, 0x38, 0x00},  // 004f (O)
            {0xfc, 0x66, 0x66, 0x7c, 0x60, 0x60, 0xf0, 0x00},  // 0050 (P)
            {0x78, 0xcc, 0xcc, 0xcc, 0xdc, 0x78, 0x1c, 0x00},  // 0051 (Q)
            {0xfc, 0x66, 0x66, 0x7c, 0x6c, 0x66, 0xe6, 0x00},  // 0052 (R)
            {0x78, 0xcc, 0xe0, 0x70, 0x1c, 0xcc, 0x78, 0x00},  // 0053 (S)
            {0xfc, 0xb4, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00},  // 0054 (T)
            {0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xfc, 0x00},  // 0055 (U)
            {0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x00},  // 0056 (V)
            {0xc6, 0xc6, 0xc6, 0xd6, 0xfe, 0xee, 0xc6, 0x00},  // 0057 (W)
            {0xc6, 0xc6, 0x6c, 0x38, 0x38, 0x6c, 0xc6, 0x00},  // 0058 (X)
            {0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x30, 0x78, 0x00},  // 0059 (Y)
            {0xfe, 0xc6, 0x8c, 0x18, 0x32, 0x66, 0xfe, 0x00},  // 005a (Z)
            {0x78, 0x60, 0x60, 0x60, 0x60, 0x60, 0x78, 0x00},  // 005b (bracketleft)
            {0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x02, 0x00},  // 005c (backslash)
            {0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78, 0x00},  // 005d (bracketright)
            {0x10, 0x38, 0x6c, 0xc6, 0x00, 0x00, 0x00, 0x00},  // 005e (asciicircum)
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},  // 005f (underscore)
            {0x30, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0060 (grave)
            {0x00, 0x00, 0x78, 0x0c, 0x7c, 0xcc, 0x76, 0x00},  // 0061 (a)
            {0xe0, 0x60, 0x60, 0x7c, 0x66, 0x66, 0xdc, 0x00},  // 0062 (b)
            {0x00, 0x00, 0x78, 0xcc, 0xc0, 0xcc, 0x78, 0x00},  // 0063 (c)
            {0x1c, 0x0c, 0x0c, 0x7c, 0xcc, 0xcc, 0x76, 0x00},  // 0064 (d)
            {0x00, 0x00, 0x78, 0xcc, 0xfc, 0xc0, 0x78, 0x00},  // 0065 (e)
            {0x38, 0x6c, 0x60, 0xf0, 0x60, 0x60, 0xf0, 0x00},  // 0066 (f)
            {0x00, 0x00, 0x76, 0xcc, 0xcc, 0x7c, 0x0c, 0xf8},  // 0067 (g)
            {0xe0, 0x60, 0x6c, 0x76, 0x66, 0x66, 0xe6, 0x00},  // 0068 (h)
            {0x30, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00},  // 0069 (i)
            {0x0c, 0x00, 0x0c, 0x0c, 0x0c, 0xcc, 0xcc, 0x78},  // 006a (j)
            {0xe0, 0x60, 0x66, 0x6c, 0x78, 0x6c, 0xe6, 0x00},  // 006b (k)
            {0x70, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00},  // 006c (l)
            {0x00, 0x00, 0xcc, 0xfe, 0xfe, 0xd6, 0xc6, 0x00},  // 006d (m)
            {0x00, 0x00, 0xf8, 0xcc, 0xcc, 0xcc, 0xcc, 0x00},  // 006e (n)
            {0x00, 0x00, 0x78, 0xcc, 0xcc, 0xcc, 0x78, 0x00},  // 006f (o)
            {0x00, 0x00, 0xdc, 0x66, 0x66, 0x7c, 0x60, 0xf0},  // 0070 (p)
            {0x00, 0x00, 0x76, 0xcc, 0xcc, 0x7c, 0x0c, 0x1e},  // 0071 (q)
            {0x00, 0x00, 0xdc, 0x76, 0x66, 0x60, 0xf0, 0x00},  // 0072 (r)
            {0x00, 0x00, 0x7c, 0xc0, 0x78, 0x0c, 0xf8, 0x00},  // 0073 (s)
            {0x10, 0x30, 0x7c, 0x30, 0x30, 0x34, 0x18, 0x00},  // 0074 (t)
            {0x00, 0x00, 0xcc, 0xcc, 0xcc, 0xcc, 0x76, 0x00},  // 0075 (u)
            {0x00, 0x00, 0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x00},  // 0076 (v)
            {0x00, 0x00, 0xc6, 0xd6, 0xfe, 0xfe, 0x6c, 0x00},  // 0077 (w)
            {0x00, 0x00, 0xc6, 0x6c, 0x38, 0x6c, 0xc6, 0x00},  // 0078 (x)
            {0x00, 0x00, 0xcc, 0xcc, 0xcc, 0x7c, 0x0c, 0xf8},  // 0079 (y)
            {0x00, 0x00, 0xfc, 0x98, 0x30, 0x64, 0xfc, 0x00},  // 007a (z)
            {0x1c, 0x30, 0x30, 0xe0, 0x30, 0x30, 0x1c, 0x00},  // 007b (braceleft)
            {0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00},  // 007c (bar)
            {0xe0, 0x30, 0x30, 0x1c, 0x30, 0x30, 0xe0, 0x00},  // 007d (braceright)
            {0x76, 0xdc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 007e (asciitilde)
            {0x00, 0x10, 0x38, 0x6c, 0xc6, 0xc6, 0xfe, 0x00}   // 007f (uni007F)
        };
        if (unsigned(inChar) >= 128)
        {
            for (byte i = 0; i < 8; i++)
                outBuffer[i] = 0;
            return false;
        }
        const byte* ptr = &sData[unsigned(inChar)][0];
        for (byte i = 0; i < 8; i++)
            outBuffer[i] = pgm_read_byte(ptr++);
        return true;
    }
};

#endif
