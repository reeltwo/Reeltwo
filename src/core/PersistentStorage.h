#ifndef PERSISTENT_STORAGE_H
#define PERSISTENT_STORAGE_H

#if defined(__SAMD21G18A__)
#include <FlashAsEEPROM.h>
#else
#include <EEPROM.h>
#endif

/// \private
class PersistentStorage
{
public:
    static unsigned reserveSpace(unsigned numBytes)
    {
        static unsigned sSize;
        unsigned offset = sSize;
        if (sSize + numBytes >= EEPROM.length())
            return ~0;
        sSize += numBytes;
        return offset;
    }
};

/**
  *\ingroup Core
  *
  * \class Persistent
  *
  * \brief Template manages persistance of specific type
  *
  * Uses a 16-bit signature
  */
template <typename T, uint16_t kSignature>
class Persistent : protected PersistentStorage
{
public:
    Persistent()
    {
        fByteIndex = PersistentStorage::reserveSpace(sizeof(T) + sizeof(uint16_t));
    }

    constexpr uint16_t signature()
    {
        return kSignature + (sizeof(T) + sizeof(uint16_t));
    }

    bool isValid()
    {
        return (fByteIndex != ~0 && readSignature() == signature());
    }

    T &get( T &t )
    {
        uint8_t *dst = (uint8_t*) &t;
        if (isValid())
        {
            DEBUG_PRINT(F("Loaded: "));
            DEBUG_PRINTLN(TYPETOSTR(T));
            // Signature is valid return data from EEPROM
            uint8_t index = fByteIndex + sizeof(uint16_t);
            for (unsigned count = sizeof(T); count; count--)
                *dst++ = EEPROM.read(index++);
        }
        else
        {
            DEBUG_PRINT(F("INVALID: "));
            DEBUG_PRINTLN(TYPETOSTR(T));
            // Signature is invalid return 0 data
            memset(dst, '\0', sizeof(T));
        }
        return t;
    }

    const T &put(const T &t)
    {
        bool needsCommit = false;
        if (fByteIndex == ~0)
        {
            /* Insufficent space */
            return t;
        }
        // Update the signature if necessary
        uint8_t index = fByteIndex;
        uint8_t hisig = (signature() >> 8);
        uint8_t losig = (signature() & 0xFF);
        if (EEPROM.read(index) != hisig)
        {
            EEPROM.write(index, hisig);
            needsCommit = true;
        }
        index++;
        if (EEPROM.read(index) != losig)
        {
            EEPROM.write(index, losig);
            needsCommit = true;
        }
        index++;
        // Update the data if necessary
        const uint8_t *ptr = (const uint8_t*) &t;
        for (int count = sizeof(T); count; count--, index++, ptr++)
        {
            if (EEPROM.read(index) != *ptr)
            {
                EEPROM.write(index, *ptr);
                needsCommit = true;
            }
        }
        // Flash based devices need a call to commit()
        if (needsCommit)
        {
            DEBUG_PRINT(F("Saved: "));
            DEBUG_PRINTLN(TYPETOSTR(T));
        }
    #ifdef EEPROM_EMULATION_SIZE
        if (needsCommit)
        {
            DEBUG_PRINT(F("Saved: "));
            DEBUG_PRINTLN(TYPETOSTR(T));
            EEPROM.commit();
        }
    #endif
        return t;
    }

private:
    uint16_t readSignature()
    {
        return ((uint16_t(EEPROM.read(fByteIndex)) << 8) | (EEPROM.read(fByteIndex+1)));
    }

    unsigned fByteIndex;
};

#define PERSIST(T) Persistent<T,WSID16(#T)>

#endif
