#ifndef EEPROMSettings_h
#define EEPROMSettings_h

#include "ReelTwo.h"
#include <EEPROM.h>

#ifdef EEPROM_FLASH_PARTITION_NAME
 #ifndef EEPROM_SIZE
  #define EEPROM_SIZE 4096
 #endif
#else
 #define EEPROM_SIZE EEPROM.length()
#endif

template <class T, uint32_t VERSION = 0xba5eba11>
class EEPROMSettings : public T
{
public:
    static constexpr int kMaximumCommandLength = 0xFF;

    T* data()
    {
        return (T*)this;
    }

    bool read()
    {
        uint32_t offs = validate();
        if (offs)
        {
            uint16_t siz = 0;
            EEPROM.get(offs, siz); offs += sizeof(siz);
            if (siz == sizeof(*this))
            {
                EEPROM.get(offs, *this);
                return true;
            }
        }
        return false;
    }

    void write()
    {
        size_t offs = 0;
        uint32_t magic = VERSION;
        EEPROM.put(offs, magic); offs += sizeof(magic);

        uint16_t siz_offs = offs; offs += sizeof(siz_offs);
        EEPROM.put(offs, *this);
        offs += sizeof(*this);

        // Update offset
        uint16_t siz = offs-siz_offs-sizeof(siz_offs);

        EEPROM.put(siz_offs, siz);

        // Check for the command section magic code
        // if it's missing we need to write out a terminating byte
        EEPROM.get(offs, magic);
        if (magic != kCommandListMagic)
        {
            magic = kCommandListMagic;
            EEPROM.put(offs, magic); offs += sizeof(magic);
        
            uint8_t tag = kEndTag;
            EEPROM.put(offs, tag); offs += sizeof(tag);
        }
        updateCRC();
    #ifdef EEPROM_FLASH_PARTITION_NAME
        EEPROM.commit();
    #endif
    }

    bool clearCommands()
    {
        uint32_t offs = validateCommandList();
        if (offs)
        {
            uint8_t tag = kEndTag;
            EEPROM.put(offs, tag);

            updateCRC();
        #ifdef EEPROM_FLASH_PARTITION_NAME
            EEPROM.commit();
        #endif
            return true;
        }
        return false;
    }

    size_t getCommandCount()
    {
        unsigned count = 0;
        uint32_t offs = validateCommandList();
        if (offs)
        {
            while (offs <= EEPROM_SIZE)
            {
                uint8_t len;
                uint8_t snum;
                EEPROM.get(offs, snum); offs += sizeof(snum);
                if (snum == kEndTag)
                    break;
                count++;
                EEPROM.get(offs, len); offs += sizeof(len) + len;
            }
        }
        return count;
    }

    size_t getCommands(uint8_t* buffer, size_t maxBufferSize)
    {
        size_t size = 0;
        uint32_t offs = validateCommandList();
        if (offs)
        {
            while (offs <= EEPROM_SIZE)
            {
                uint8_t len;
                uint8_t snum;
                EEPROM.get(offs, snum); offs += sizeof(snum);
                if (snum == kEndTag)
                    break;
                if (size < maxBufferSize)
                    buffer[size++] = snum;
                EEPROM.get(offs, len); offs += sizeof(len) + len;
            }
        }
        return size;
    }

    bool listCommands(Print& stream)
    {
        uint32_t offs = validateCommandList();
        if (offs)
        {
            while (offs <= EEPROM_SIZE)
            {
                uint8_t snum;
                EEPROM.get(offs, snum); offs += sizeof(snum);
                if (snum == kEndTag)
                    break;
                stream.print('[');
                stream.print(snum);
                stream.print(']');
                stream.print(' ');
                uint8_t len;
                EEPROM.get(offs, len); offs += sizeof(len);
                while (len > 0)
                {
                    char ch;
                    EEPROM.get(offs, ch); offs += sizeof(ch);
                    stream.print((char)ch);
                    len--;
                }
                stream.println();
            }
            return true;
        }
        return false;
    }

    bool listSortedCommands(Print& stream)
    {
        typedef uint32_t BMAPWORD;
    #define BMAPWORD_GRANULARITY    (sizeof(BMAPWORD))
    #define BMAPWORD_BIT_SIZE       (sizeof(BMAPWORD) * 8)
    #define BIT_MAPWORD(bitpos)     (0x1L << (BMAPWORD_BIT_SIZE - 1 - (bitpos)))
    #define SET_MAPWORD_BIT(value, bit) {\
        uint8_t bitpos = (value) % sizeof(usedBitmap[0]); \
        uint8_t wordpos = (value) / sizeof(usedBitmap[0]); \
        usedBitmap[wordpos] = (usedBitmap[wordpos] & ~(BIT_MAPWORD(bitpos))) | (uint32_t(bit) << (BMAPWORD_BIT_SIZE - 1 - bitpos)); }
    #define MAPWORD_BIT(value) \
        ((usedBitmap[(value) / sizeof(usedBitmap[0])] & BIT_MAPWORD((value) % sizeof(usedBitmap[0]))) != 0)

        uint32_t usedBitmap[kMaxCommands / sizeof(uint32_t)+1];
        memset(&usedBitmap, '\0', sizeof(usedBitmap));

        uint32_t offs = validateCommandList();
        if (!offs)
            return false;

        uint16_t scanoffs = offs;
        while (scanoffs <= EEPROM_SIZE)
        {
            uint8_t snum = 0;
            EEPROM.get(scanoffs, snum); scanoffs += sizeof(snum);
            if (snum == kEndTag)
                break;

            SET_MAPWORD_BIT(snum, 1);
            uint8_t len = 0;
            EEPROM.get(scanoffs, len); scanoffs += sizeof(len) + len;
        }
        for (unsigned i = 0; i < 100; i++)
        {
            if (MAPWORD_BIT(i))
            {
                uint16_t scanoffs = offs;
                while (scanoffs <= EEPROM_SIZE)
                {
                    uint8_t snum = 0;
                    EEPROM.get(scanoffs, snum); scanoffs += sizeof(snum);
                    if (snum == kEndTag)
                        break;
                    if (snum == i)
                    {
                        stream.print('[');
                        stream.print(snum);
                        stream.print(']');
                        stream.print(' ');
                    }
                    uint8_t len = 0;
                    EEPROM.get(scanoffs, len); scanoffs += sizeof(len);
                    if (snum == i)
                    {
                        while (len > 0)
                        {
                            char ch = 0;
                            EEPROM.get(scanoffs, ch); scanoffs += sizeof(ch);
                            stream.print((char)ch);
                            len--;
                        }
                        stream.println();
                        break;
                    }
                    else
                    {
                        scanoffs += len;
                    }
                }
            }
        }
        return true;
    #undef MAPWORD_BIT
    #undef SET_MAPWORD_BIT
    #undef BIT_MAPWORD
    #undef BMAPWORD_BIT_SIZE
    #undef BMAPWORD_GRANULARITY
    }

    bool readCommand(uint8_t num, char* cmd, size_t cmdBufferSize, const char* prefix = nullptr)
    {
        return readCommandInternal(num, cmd, cmdBufferSize, nullptr, prefix);
    }

    bool deleteCommand(uint8_t num)
    {
        uint16_t writeoffs = 0;
        if (!readCommandInternal(num, nullptr, 0, &writeoffs))
            return false;

        uint8_t len = 0;
        uint16_t readoffs = writeoffs + 1;
        EEPROM.get(readoffs, len); readoffs += sizeof(len);
        readoffs += len;

        while (readoffs < EEPROM_SIZE)
        {
            uint8_t tag = 0;
            EEPROM.get(readoffs, tag); readoffs += sizeof(tag);
            EEPROM.put(writeoffs, tag); writeoffs += sizeof(tag);
            if (tag == kEndTag)
            {
                // End of buffer
                break;
            }
            uint8_t readlen = 0;
            EEPROM.get(readoffs, readlen); readoffs += sizeof(readlen);
            EEPROM.put(writeoffs, readlen); writeoffs += sizeof(readlen);
            while (readlen > 0)
            {
                char ch = 0;
                EEPROM.get(readoffs, ch); readoffs += sizeof(ch);
                EEPROM.put(writeoffs, ch); writeoffs += sizeof(ch);
                readlen--;
            }
        }
        updateCRC();
    #ifdef EEPROM_FLASH_PARTITION_NAME
        EEPROM.commit();
    #endif
        return true;
    }

    bool writeCommand(uint8_t num, const char* cmd)
    {
        if (strlen(cmd) > kMaximumCommandLength)
        {
            // command too long
            return false;
        }
        // delete old command if it exists
        deleteCommand(num);

        uint32_t offs = validate();
        if (offs)
        {
            uint16_t siz = 0;
            EEPROM.get(offs, siz); offs += siz + sizeof(siz);

            uint32_t magic;
            EEPROM.get(offs, magic); offs += sizeof(magic);
            if (magic == kCommandListMagic)
            {
                // append command to end of buffer
                while (offs <= EEPROM_SIZE)
                {
                    uint8_t tag = 0;
                    EEPROM.get(offs, tag);
                    if (tag == kEndTag)
                    {
                        EEPROM.put(offs, num); offs += sizeof(num);

                        uint8_t len = strlen(cmd);
                        EEPROM.put(offs, len); offs += sizeof(len);
                        while (len > 0)
                        {
                            EEPROM.put(offs, *cmd); offs += sizeof(*cmd);
                            cmd++;
                            len--;
                        }
                        // Write terminate byte
                        num = kEndTag;
                        EEPROM.put(offs, num); offs += sizeof(num);
                        updateCRC();
                    #ifdef EEPROM_FLASH_PARTITION_NAME
                        EEPROM.commit();
                    #endif
                        return true;
                    }
                    offs += sizeof(tag);
                    uint8_t len = 0;
                    EEPROM.get(offs, len); offs += sizeof(len) + len;
                }
            }
            else
            {
                // start new command buffer
                magic = kCommandListMagic;
                EEPROM.put(offs, magic); offs += sizeof(magic);

                EEPROM.put(offs, num); offs += sizeof(num);

                uint8_t len = strlen(cmd);
                EEPROM.put(offs, len); offs += sizeof(len);
                while (len > 0)
                {
                    EEPROM.put(offs, *cmd); offs += sizeof(*cmd);
                    cmd++;
                    len--;
                }
                // Write terminate byte
                num = kEndTag;
                EEPROM.put(offs, num); offs += sizeof(num);
                updateCRC();
            #ifdef EEPROM_FLASH_PARTITION_NAME
                EEPROM.commit();
            #endif
                return true;
            }
        }
        return false;
    }

private:
    static uint32_t constexpr kCommandListMagic = 0xf005ba11;
    static uint8_t constexpr kEndTag = 0xff;
    static uint8_t constexpr kMaxCommands = 100;

    uint32_t validate()
    {
        uint16_t offs = 0;
        uint32_t magic;
        EEPROM.get(offs, magic);
        uint32_t crc = crcFrom(sizeof(magic));
        if (magic == (VERSION ^ crc))
        {
            offs += sizeof(magic);
        }
        return offs;
    }

    uint32_t validateCommandList()
    {
        uint32_t offs = validate();
        if (offs)
        {
            uint16_t siz = 0;
            EEPROM.get(offs, siz); offs += siz + sizeof(siz);

            uint32_t magic;
            EEPROM.get(offs, magic); offs += sizeof(magic);
            if (magic == kCommandListMagic)
            {
                return offs;
            }
        }
        return 0;
    }

    bool readCommandInternal(uint8_t num, char* cmd, size_t cmdBufferSize, uint16_t* cmdoffs = nullptr, const char* prefix = nullptr)
    {
        uint32_t offs = validateCommandList();
        if (offs)
        {
            while (offs <= EEPROM_SIZE)
            {
                uint8_t snum = 0;
                EEPROM.get(offs, snum);
                if (snum == kEndTag)
                    break;
                if (snum == num && cmdoffs != nullptr)
                    *cmdoffs = offs;
                uint8_t len = 0;
                offs += sizeof(snum);
                EEPROM.get(offs, len); offs += sizeof(len);
                if (snum == num)
                {
                    if (cmd != nullptr)
                    {
                        char ch;
                        char* cmd_end = cmd + cmdBufferSize - 1;
                        if (prefix != nullptr)
                        {
                            while (prefix != nullptr && (ch = *prefix++) != '\0' && cmd < cmd_end)
                            {
                                *cmd++ = ch;
                            }
                        }
                        while (len > 0)
                        {
                            EEPROM.get(offs, ch); offs += sizeof(ch);
                            if (cmd < cmd_end)
                                *cmd++ = ch;
                            len--;
                        }
                        *cmd = '\0';
                    }
                    return true;
                }
                else
                {
                    offs += len;
                }
            }
        }
        return false;
    }

    void updateCRC()
    {
        // Update header crc
        uint32_t crc = crcFrom(sizeof(uint32_t));
        EEPROM.put(0, VERSION ^ crc);
    }

    static uint32_t crcFrom(unsigned offset = 0)
    {
        static const uint32_t crc_table[16] PROGMEM =
        {
            0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
            0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
            0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
            0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
        };
        uint32_t crc = ~0L;
        while (offset < EEPROM_SIZE)
        {
            uint8_t eepromByte = EEPROM.read(offset);
            crc = pgm_read_uint32(&crc_table[(crc ^ eepromByte) & 0x0f]) ^ (crc >> 4);
            crc = pgm_read_uint32(&crc_table[(crc ^ (eepromByte >> 4)) & 0x0f]) ^ (crc >> 4);
            crc = ~crc;
            offset += 1;
        }
        return crc;
    }

    static inline uint32_t pgm_read_uint32(const uint32_t* p)
    {
    #if defined(__AVR_ATmega1280__)  || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega32U4__) || \
        defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__) || \
        defined(__AVR_ATmega128__) ||defined(__AVR_ATmega1281__)||defined(__AVR_ATmega2561__)
        return pgm_read_dword(p);
    #else
        return *p;
    #endif
    }
};

#endif
