#ifndef AVRMega2560Programmer_h
#define AVRMega2560Programmer_h

#include "core/SetupEvent.h"

#ifdef USE_AVR_DEBUG
#define AVR_DEBUG_PRINT(s) DEBUG_PRINT(s)
#define AVR_DEBUG_PRINTLN(s) DEBUG_PRINTLN(s)
#define AVR_DEBUG_PRINT_HEX(s) DEBUG_PRINT_HEX(s)
#define AVR_DEBUG_PRINTLN_HEX(s) DEBUG_PRINTLN_HEX(s)
#else
#define AVR_DEBUG_PRINT(s)
#define AVR_DEBUG_PRINTLN(s)
#define AVR_DEBUG_PRINT_HEX(s)
#define AVR_DEBUG_PRINTLN_HEX(s)
#endif

/**
  * \ingroup Flash
  *
  * \class AVRMega2560Programmer
  *
  * \brief AVR Programmer
  *
  * Communicate with the Mega2560 Arduino-stk500v2-bootloader
  *
  * \code
  * // Serial port should be operating at 115200 baud
  * AVRFlasher  megaFlasher(MEGA_RESET_PIN, Serial2);
  * \endcode
  */
class AVRMega2560Programmer : public SetupEvent
{
public:
    enum MemType
    {
        kFlash,
        kEEPROM,
        kLFuse,
        kFuse,
        kHFuse,
        kEFuse,
        kLock,
        kCalibration,
        kSignature
    };

    enum BootLoaderType
    {
        kUnknown,
        kAVRISP
    };

    AVRMega2560Programmer(uint8_t resetPin, HardwareSerial& port) :
        fResetPin(resetPin),
        fFlashPageCache(NULL),
        fEEPROMPageCache(NULL)
    {
    }

    virtual void setup() override
    {
        DEBUG_PRINT("SETTING RESET HIGH pin="); DEBUG_PRINTLN(fResetPin); delay(200);
        pinMode(fResetPin, OUTPUT);
        digitalWrite(fResetPin, HIGH);
        DEBUG_PRINTLN("SET RESET HIGH"); delay(200);
    }

    typedef void (*ProgressProc)(double percent);
    void setProgress(ProgressProc progressFunc)
    {
        fProgress = progressFunc;
    }

    void reset()
    {
        /* Perform Wiring programming mode RESET.           */
        /* This effectively *releases* both DTR and RTS.    */
        /* i.e. both DTR and RTS rise to a HIGH logic level */
        /* since they are active LOW signals.               */
        digitalWrite(fResetPin, LOW);
        delay(50);

        /* After releasing for 50 milliseconds, DTR and RTS */
        /* are asserted (i.e. logic LOW) again.             */
        digitalWrite(fResetPin, HIGH);
        delay(50);

        while (MEGA_SERIAL.available())
            MEGA_SERIAL.read();
    }

    bool startProgramming()
    {
        if (fProgMode)
            return true;
        AVR_DEBUG_PRINTLN("Attempting to enter ICSP programming mode ..."); delay(200);

        while (MEGA_SERIAL.available())
            MEGA_SERIAL.read();

        unsigned timeout = 0;
        fCommandSequence = 1;
        fFlashPageAddr = ~0u;
        fFlashPageSize = fRegions.flash.page_size;
        fFlashPageCache = (uint8_t*)ps_malloc(fFlashPageSize);

        fEEPROMPageAddr = ~0u;
        fEEPROMPageSize = fRegions.eeprom.page_size;
        fEEPROMPageCache = (uint8_t*)ps_malloc(fEEPROMPageSize);

        fProgMode = false;
        for (Memory* mem = *Memory::head(); mem != NULL; mem = mem->next)
        {
            mem->init();
        }
        for (;;)
        {
            // regrouping pause
            delay(100);

            reset();

            // Need more time for the boot loader to start
            delay(25);

            if (getsync() < 0)
            {
                if (timeout++ >= ENTER_PROGRAMMING_ATTEMPTS)
                {
                    AVR_DEBUG_PRINTLN();
                    AVR_DEBUG_PRINTLN("Failed to enter programming mode. Double-check wiring!");
                    return false;
                }
                AVR_DEBUG_PRINTLN("RETRYING TO SYNC");
                continue;
            }

        #ifdef USE_AVR_DEBUG
            if (display() < 0)
            {
                AVR_DEBUG_PRINTLN("FAILED TO RETRIEVE DEVICE INFO");
                continue;
            }
        #endif
            if (programEnable() > 0)
            {
                break;
            }
            AVR_DEBUG_PRINTLN("FAILED TO ENTER PROGRAMMING MODE");
        }
        AVR_DEBUG_PRINTLN();
        AVR_DEBUG_PRINTLN("Entered programming mode OK.");
        fProgMode = true;
        return true;    
    }

    void stopProgramming()
    {
        unsigned char buf[8];
        buf[0] = CMD_LEAVE_PROGMODE_ISP;
        buf[1] = 1; // preDelay;
        buf[2] = 1; // postDelay;

        command(buf, 3, sizeof(buf));

        if (fFlashPageCache != NULL)
            free(fFlashPageCache);
        if (fEEPROMPageCache != NULL)
            free(fEEPROMPageCache);
        for (Memory* mem = *Memory::head(); mem != NULL; mem = mem->next)
        {
            mem->dispose();
        }
        fProgMode = false;
    }

    bool dumpMemory(MemType type)
    {
        Memory* mem = findRegion(type);
        return (mem != NULL) ? dumpMemory(*mem) : false;
    }

    uint8_t* readMemory(MemType type, size_t* size)
    {
        if (size != NULL)
            *size = 0;
        Memory* mem = findRegion(type);
        if (mem != NULL && readMemory(*mem))
        {
            if (size != NULL)
                *size = mem->size;
            return mem->buf;
        }
        return NULL;
    }

    bool writeHexString(MemType type, const char* hexString, unsigned* lineno = NULL)
    {
        if (lineno != NULL)
            *lineno = 0;
        Memory* mem = findRegion(type);
        return (mem != NULL) ? writeHexString(*mem, hexString, lineno) : false;
    }

    bool writeMemory(MemType type, bool verify = true)
    {
        Memory* mem = findRegion(type);
        return (mem != NULL) ? writeMemory(*mem, verify) : false;
    }

    bool hasMemoryChanged(MemType type)
    {
        Memory* mem = findRegion(type);
        return (mem != NULL) ? hasMemoryChanged(*mem) : false;
    }

private:
    enum
    {
        CMD_SIGN_ON = 0x01,
        CMD_SET_PARAMETER = 0x02,
        CMD_GET_PARAMETER = 0x03,
        CMD_SET_DEVICE_PARAMETERS = 0x04,
        CMD_OSCCAL = 0x05,
        CMD_LOAD_ADDRESS = 0x06,
        CMD_FIRMWARE_UPGRADE = 0x07,

        // *****************[ STK ISP command constants ]******************************

        CMD_ENTER_PROGMODE_ISP = 0x10,
        CMD_LEAVE_PROGMODE_ISP = 0x11,
        CMD_CHIP_ERASE_ISP = 0x12,
        CMD_PROGRAM_FLASH_ISP = 0x13,
        CMD_READ_FLASH_ISP = 0x14,
        CMD_PROGRAM_EEPROM_ISP = 0x15,
        CMD_READ_EEPROM_ISP= 0x16,
        CMD_PROGRAM_FUSE_ISP = 0x17,
        CMD_READ_FUSE_ISP= 0x18,
        CMD_PROGRAM_LOCK_ISP = 0x19,
        CMD_READ_LOCK_ISP= 0x1A,
        CMD_READ_SIGNATURE_ISP = 0x1B,
        CMD_READ_OSCCAL_ISP = 0x1C,
        CMD_SPI_MULTI = 0x1D,

        // *** XPROG command constants ***
        CMD_XPROG = 0x50,
        CMD_XPROG_SETMODE = 0x51,

        // *****************[ STK status constants ]***************************

        // Success
        STATUS_CMD_OK = 0x00,

        // Warnings
        STATUS_CMD_TOUT = 0x80,
        STATUS_RDY_BSY_TOUT = 0x81,
        STATUS_SET_PARAM_MISSING = 0x82,

        // Errors
        STATUS_CMD_FAILED = 0xC0,
        STATUS_CKSUM_ERROR = 0xC1,
        STATUS_CMD_UNKNOWN = 0xC9,
        STATUS_CMD_ILLEGAL_PARAMETER = 0xCA,

        // Status
        STATUS_ISP_READY = 0x00,
        STATUS_CONN_FAIL_MOSI = 0x01,
        STATUS_CONN_FAIL_RST = 0x02,
        STATUS_CONN_FAIL_SCK = 0x04,
        STATUS_TGT_NOT_DETECTED = 0x10,
        STATUS_TGT_REVERSE_INSERTED = 0x20,

        MESSAGE_START = 0x1B,
        TOKEN = 0x0E,

        ANSWER_CKSUM_ERROR = 0xB0,

        // Error codes
        XPRG_ERR_OK = 0,
        XPRG_ERR_FAILED = 1,
        XPRG_ERR_COLLISION = 2,
        XPRG_ERR_TIMEOUT = 3,

        PARAM_HW_VER = 0x90,
        PARAM_SW_MAJOR = 0x91,
        PARAM_SW_MINOR = 0x92,
    };

    // #define BITOP (BitOp[])
    enum BitOp
    {
        O,  /* the bit is always clear on input as well as output */
        I,  /* the bit is always set on input as well as output */
        o,  /* the bit is an output data bit */
        a,  /* the bit is an address bit, the bit-number matches this bit */
            /* specifier's position within the current instruction byte */
        i,  /* the bit is an input data bit */
        x,  /* the bit is ignored on input and output */
        a0, /* the bit is the Nth address bit, bit-number = N, i.e., a16 */
        a1, /* is address bit 12 on input, a0 is address bit 0. */
        a2,
        a3,
        a4,
        a5,
        a6,
        a7,
        a8,
        a9,
        a10,
        a11,
        a12,
        a13,
        a14,
        a15,
        a16
    };

    struct ByteOp
    {
        union
        {
            struct
            {
                BitOp op1;
                BitOp op2;
                BitOp op3;
                BitOp op4;
                BitOp op5;
                BitOp op6;
                BitOp op7;
                BitOp op8;
            };
            BitOp arr[8];
        };
    };

    class MemOp
    {
    public:
        MemOp() : fValid(false)
        {
        }

        MemOp(ByteOp byte1, ByteOp byte2)
        {
            const ByteOp descs[] = { byte1, byte2 };
            parseDescription(sizeof(descs)/sizeof(descs[0]), descs);
        }

        MemOp(ByteOp byte1, ByteOp byte2,
              ByteOp byte3, ByteOp byte4)
        {
            const ByteOp descs[] = { byte1, byte2, byte3, byte4 };
            parseDescription(sizeof(descs)/sizeof(descs[0]), descs);
        }

        inline bool isValid() const
        {
            return fValid;
        }

        void setBits(uint8_t* cmd)
        {
            for (int i = 0; i < 32; i++)
            {
                if (fBit[i].fOp == kValue)
                {
                    int j = 3 - i / 8;
                    int bit = i % 8;
                    uint8_t mask = 1 << bit;
                    if (fBit[i].fValue)
                        cmd[j] |= mask;
                    else
                        cmd[j] &= ~mask;
                }
            }
        }

        void setAddress(uint8_t* cmd, uint32_t addr)
        {
            for (int i = 0; i < 32; i++)
            {
                if (fBit[i].fOp == kAddress)
                {
                    int j = 3 - i / 8;
                    int bit = i % 8;
                    uint8_t mask = 1 << bit;
                    uint32_t value = addr >> fBit[i].fNum & 0x01;
                    if (value)
                        cmd[j] |= mask;
                    else
                        cmd[j] &= ~mask;
                }
            }
        }

        void setInput(uint8_t* cmd, uint32_t data)
        {
            for (int i = 0; i < 32; i++)
            {
                if (fBit[i].fOp == kInput)
                {
                    int j = 3 - i / 8;
                    int bit = i % 8;
                    uint8_t mask = 1 << bit;
                    uint8_t value = data >> fBit[i].fNum & 0x01;
                    if (value)
                        cmd[j] |= mask;
                    else
                        cmd[j] &= ~mask;
                }
            }
        }

        void getOutput(uint8_t* res, uint8_t* data)
        {
            for (int i = 0; i < 32; i++)
            {
                if (fBit[i].fOp == kOutput)
                {
                    int j = 3 - i / 8;
                    int bit = i % 8;
                    uint8_t mask = 1 << bit;
                    uint8_t value = ((res[j] & mask) >> bit) & 0x01;
                    value = value << fBit[i].fNum;
                    if (value)
                        *data |= value;
                    else
                        *data &= ~value;
                }
            }
        }

        int getOutputIndex()
        {
            for (int i = 0; i < 32; i++)
            {
                if (fBit[i].fOp == kOutput)
                    return 3 - i / 8;
            }
            return -1;
        }

    #ifdef USE_AVR_DEBUG
        void printDescriptor(const char* name)
        {
            static const char* opname[] = {
                "IGNORE",
                "VALUE",
                "ADDRESS",
                "INPUT",
                "OUTPUT"
            };
            for (int i = 31; i >= 0; i--)
            {
                //-----------  --------  --------  -----  -----
                //12345678901  12345678  12345678  12345  12345
                printf("%-11s  %8d  %8s  %5d  %5d\n",
                    name, i, opname[fBit[i].fOp], fBit[i].fNum, fBit[i].fValue);
                name = "";
            }
        }
    #endif

    private:
        enum Op
        {
            kIgnore,
            kValue,
            kAddress,
            kInput,
            kOutput
        };
        struct Bit
        {
            Op fOp;
            uint8_t fNum;
            uint8_t fValue;
        };
        Bit fBit[32];
        bool fValid = false;

        bool parseDescription(size_t count, const ByteOp* byteops)
        {
            int bit = 32;
            fValid = false;
            for (unsigned wi = 0; wi < count; wi++)
            {
                const BitOp* op = byteops[wi].arr;
                for (unsigned bi = 0; bi < 8; bi++)
                {
                    bit--;
                    if (bit < 0)
                        return false;

                    BitOp bitop = op[bi];
                    switch (bitop)
                    {
                        case I:
                            fBit[bit].fOp  = kValue;
                            fBit[bit].fValue = 1;
                            fBit[bit].fNum = bit % 8;
                            break;
                        case O:
                            fBit[bit].fOp  = kValue;
                            fBit[bit].fValue = 0;
                            fBit[bit].fNum = bit % 8;
                            break;
                        case x:
                            fBit[bit].fOp  = kIgnore;
                            fBit[bit].fValue = 0;
                            fBit[bit].fNum = bit % 8;
                            break;
                        case a:
                            fBit[bit].fOp  = kAddress;
                            fBit[bit].fValue = 0;
                            fBit[bit].fNum = 8 * (bit / 8) + bit % 8;
                            break;
                        case i:
                            fBit[bit].fOp  = kInput;
                            fBit[bit].fValue = 0;
                            fBit[bit].fNum = bit % 8;
                            break;
                        case o:
                            fBit[bit].fOp  = kOutput;
                            fBit[bit].fValue = 0;
                            fBit[bit].fNum = bit % 8;
                            break;
                        default:
                            fBit[bit].fNum = (uint8_t)bitop-(uint8_t)BitOp::a0;
                            fBit[bit].fOp = kAddress;
                            fBit[bit].fValue = 0;
                            break;
                    }
                }
            }
            fValid = true;
            return true;
        }
    };

    MemOp pgmEnableOp = MemOp( ByteOp{ I, O, I, O, I, I, O, O }, ByteOp{ O, I, O, I, O, O, I, I },
                               ByteOp{ x, x, x, x, x, x, x, x }, ByteOp{ x, x, x, x, x, x, x, x });

    MemOp chipEraseOp = MemOp( ByteOp{ I, O, I, O, I, I, O, I }, ByteOp{ I, O, O, O, O, O, O, O },
                               ByteOp{ x, x, x, x, x, x, x, x }, ByteOp{ x, x, x, x, x, x, x, x });

    struct Memory
    {
        MemType type;
        bool paged = false;
        bool loaded = false;
        unsigned size = 0;
        unsigned page_size = 0;
        unsigned num_pages = 0;
        unsigned min_write_delay = 0;
        unsigned max_write_delay = 0;
        unsigned readback_p1 = 0;
        unsigned readback_p2 = 0;

        unsigned mode = 0;
        unsigned delay = 0;
        unsigned blocksize = 0;
        unsigned readsize = 0;

        MemOp invalid;
        MemOp read;
        MemOp write;
        MemOp read_lo;
        MemOp read_hi;
        MemOp write_lo;
        MemOp write_hi;
        MemOp loadpage_lo;
        MemOp loadpage_hi;
        MemOp load_ext_addr;
        MemOp writepage;

        uint8_t* buf = NULL;
        uint32_t* tags = NULL; 
        uint32_t* page_tags = NULL; 
        Memory* next;

        Memory(MemType memtype) :
            type(memtype),
            next(NULL)
        {
            if (*head() == NULL)
                *head() = this;
            if (*tail() != NULL)
                (*tail())->next = this;
            *tail() = this;
        }

        void init()
        {
            buf = (uint8_t*)ps_malloc(size);
            memset(buf, 0xFF, size);

            unsigned tagsize = (size/32+1)*sizeof(uint32_t);
            tags = (uint32_t*)ps_malloc(tagsize);
            memset(tags, 0, tagsize);

            if (page_size > 0)
            {
                tagsize = ((size/page_size)/32+1)*sizeof(uint32_t);
                page_tags = (uint32_t*)ps_malloc(tagsize);
                memset(page_tags, 0, tagsize);
            }
        }

        void dispose()
        {
            if (buf != NULL)
            {
                free(buf);
                buf = NULL;
            }
            if (tags != NULL)
            {
                free(tags);
                tags = NULL;
            }
            if (page_tags != NULL)
            {
                free(page_tags);
                page_tags = NULL;
            }
        }

        void tagAddress(unsigned addr, bool tag)
        {
            if (addr < size)
            {
                if (tag)
                    tags[addr / 32] |= (1 << (addr % 32));
                else
                    tags[addr / 32] &= ~(1 << (addr % 32));
            }
        }

        void tagPage(unsigned addr, bool tag)
        {
            if (page_size > 0 && addr < size)
            {
                unsigned pageAddr = addr / page_size;
                if (tag)
                    page_tags[pageAddr / 32] |= (1 << (pageAddr % 32));
                else
                    page_tags[pageAddr / 32] &= ~(1 << (pageAddr % 32));
            }
        }

        bool isAddressTagged(unsigned addr)
        {
            return (addr < size) ? ((tags[addr / 32] & (1 << (addr % 32))) != 0) : false;
        }

        bool isPageTagged(unsigned addr)
        {
            if (page_size > 0 && addr < size)
            {
                unsigned pageAddr = addr / page_size;
                return ((page_tags[pageAddr / 32] & (1 << (pageAddr % 32))) != 0);
            }
            return false;
        }

        uint8_t get(unsigned addr)
        {
            if (addr < size)
                return buf[addr];
            return 0xFF;
        }

        void set(unsigned addr, uint8_t byte)
        {
            if (addr < size && buf[addr] != byte)
            {
                buf[addr] = byte;
                tagAddress(addr, true);
                tagPage(addr, true);
            }
        }

        static Memory** head()
        {
            static Memory* sHead;
            return &sHead;
        }

        static Memory** tail()
        {
            static Memory* sTail;
            return &sTail;
        }

    #ifdef USE_AVR_DEBUG
        void printDescription()
        {
            printf("                       Block Poll               Page                       Polled\n");
            printf("Memory Type Mode Delay Size  Indx Paged  Size   Size #Pages MinW  MaxW   ReadBack\n");
            printf("----------- ---- ----- ----- ---- ------ ------ ---- ------ ----- ----- ---------\n");
            printf("%-11s %4d %5d %5d %4d %6s %6d %4d %6d %4d %4d 0x%02X 0x%02X\n",
                "signature", mode, delay, blocksize, 0, ((paged) ? "yes" : "no"),
                    size, page_size, (page_size != 0) ? size / page_size : 0,
                    min_write_delay, max_write_delay, readback_p1, readback_p2);

            printf("  Memory Ops:\n");
            printf("    Operation    Inst Bit  Bit Type  Bitno  Value\n");
            printf("    -----------  --------  --------  -----  -----\n");
            if (read.isValid())
                read.printDescriptor("READ");
        }
    #endif
    };

    struct MemoyRegions
    {
        struct Flash : Memory
        {
            Flash() : Memory(kFlash)
            {
                paged           = true;
                size            = 262144;
                page_size       = 256;
                num_pages       = 1024;
                min_write_delay = 4500;
                max_write_delay = 4500;
                readback_p1     = 0x00;
                readback_p2     = 0x00;

                mode            = 0x41;
                delay           = 10;
                blocksize       = 256;
                readsize        = 256;

                read_lo = MemOp(ByteOp{   O,   O,   I,   O,   O,   O,  O,   O },
                                ByteOp{ a15, a14, a13, a12, a11, a10, a9,  a8 },
                                ByteOp{  a7,  a6,  a5,  a4,  a3,  a2, a1,  a0 },
                                ByteOp{   o,   o,   o,   o,   o,   o,  o,   o });

                read_hi = MemOp(ByteOp{   O,   O,   I,   O,   I,   O,   O,  O },
                                ByteOp{ a15, a14, a13, a12, a11, a10,  a9, a8 },
                                ByteOp{  a7,  a6,  a5,  a4,  a3,  a2,  a1, a0 },
                                ByteOp{   o,   o,   o,   o,   o,   o,   o,  o });

                loadpage_lo = MemOp(ByteOp{ O,   I,   O,   O,      O,   O,   O,   O },
                                    ByteOp{ x,   x,   x,   x,      x,   x,   x,   x },
                                    ByteOp{ x,  a6,  a5,  a4,     a3,  a2,  a1,  a0 },
                                    ByteOp{ i,   i,   i,   i,      i,   i,   i,   i });

                loadpage_hi = MemOp(ByteOp{ O,   I,   O,   O,      I,   O,   O,   O },
                                    ByteOp{ x,   x,   x,   x,      x,   x,   x,   x },
                                    ByteOp{ x,  a6,  a5,  a4,     a3,  a2,  a1,  a0 },
                                    ByteOp{ i,   i,   i,   i,      i,   i,   i,   i });

                writepage = MemOp(ByteOp{   O,   I,   O,   O,      I,   I,   O,   O },
                                  ByteOp{ a15, a14, a13, a12,    a11, a10,  a9,  a8 },
                                  ByteOp{  a7,   x,   x,   x,      x,   x,   x,   x },
                                  ByteOp{   x,   x,   x,   x,      x,   x,   x,   x });

                load_ext_addr = MemOp(ByteOp{ O,   I,   O,   O,      I,   I,   O,   I },
                                      ByteOp{ O,   O,   O,   O,      O,   O,   O,   O },
                                      ByteOp{ O,   O,   O,   O,      O,   O,   O, a16 },
                                      ByteOp{ O,   O,   O,   O,      O,   O,   O,   O });
            }
        } flash;

        struct EEPROM : Memory {
            EEPROM() : Memory(kEEPROM)
            {
                paged           = false;
                page_size       = 8;
                size            = 4096;
                min_write_delay = 9000;
                max_write_delay = 9000;
                readback_p1     = 0x00;
                readback_p2     = 0x00;

                mode            = 0x41;
                delay           = 10;
                blocksize       = 8;
                readsize        = 256;

                read = MemOp(ByteOp{  I,   O,   I,   O,      O,   O,   O,   O },
                             ByteOp{  x,   x,   x,   x,    a11, a10,  a9,  a8 },
                             ByteOp{ a7,  a6,  a5,  a4,     a3,  a2,  a1,  a0 },
                             ByteOp{  o,   o,   o,   o,      o,   o,   o,   o });

                write = MemOp(ByteOp{  I,   I,   O,   O,      O,   O,   O,   O },
                              ByteOp{  x,   x,   x,   x,    a11, a10,  a9,  a8 },
                              ByteOp{ a7,  a6,  a5,  a4,     a3,  a2,  a1,  a0 }, 
                              ByteOp{  i,   i,   i,   i,      i,   i,   i,   i });

                loadpage_lo = MemOp(ByteOp{  I,  I,  I,  I,     O,  O,  O,  I},
                                    ByteOp{  O,  O,  O,  O,     O,  O,  O,  I},
                                    ByteOp{  O,  O,  O,  O,     O, a2, a1, a0},
                                    ByteOp{  i,  i,  i,  i,     i,  i,  i,  i});

                writepage = MemOp(ByteOp{  I,   I,   O,   O,     O,   O,   I,   O},
                                  ByteOp{  O,   O,   x,   x,   a11, a10,  a9,  a8},
                                  ByteOp{ a7,  a6,  a5,  a4,    a3,   O,   O,   O},
                                  ByteOp{  x,   x,   x,   x,     x,   x,   x,   x});
            }
        } eeprom;

        struct Fuse : Memory {
            Fuse(MemType type) : Memory(type)
            {
                size = 1;
                min_write_delay = 9000;
                max_write_delay = 9000;
            }
        };
        struct LFuse : Fuse {
            LFuse() : Fuse(kLFuse)
            {
                write = MemOp(ByteOp{ I, O, I, O,  I, I, O, O },
                              ByteOp{ I, O, I, O,  O, O, O, I },
                              ByteOp{ x, x, x, x,  x, x, x, x },
                              ByteOp{ i, i, i, i,  i, i, i, i });

                read  = MemOp(ByteOp{ O, I, O, I,  O, O, O, O },
                              ByteOp{ O, O, O, O,  O, O, O, O },
                              ByteOp{ x, x, x, x,  x, x, x, x },
                              ByteOp{ o, o, o, o,  o, o, o, o });
            }
        } lfuse;

        struct HFuse : Fuse {
            HFuse() : Fuse(kHFuse)
            {
                write = MemOp(ByteOp{ I, O, I, O,  I, I, O, O },
                              ByteOp{ I, O, I, O,  I, O, O, O },
                              ByteOp{ x, x, x, x,  x, x, x, x },
                              ByteOp{ i, i, i, i,  i, i, i, i });

                read  = MemOp(ByteOp{ O, I, O, I,  I, O, O, O },
                              ByteOp{ O, O, O, O,  I, O, O, O },
                              ByteOp{ x, x, x, x,  x, x, x, x },
                              ByteOp{ o, o, o, o,  o, o, o, o });
            }
        } hfuse;

        struct EFuse : Fuse {
            EFuse() : Fuse(kEFuse)
            {
                write = MemOp(ByteOp{ I, O, I, O,  I, I, O, O },
                              ByteOp{ I, O, I, O,  O, I, O, O },
                              ByteOp{ x, x, x, x,  x, x, x, x },
                              ByteOp{ x, x, x, x,  x, i, i, i });

                read  = MemOp(ByteOp{ O, I, O, I,  O, O, O, O },
                              ByteOp{ O, O, O, O,  I, O, O, O },
                              ByteOp{ x, x, x, x,  x, x, x, x },
                              ByteOp{ o, o, o, o,  o, o, o, o });
            }
        } efuse;

        struct Lock : Fuse {
            Lock() : Fuse(kLock)
            {
                read  = MemOp(ByteOp{ O, I, O, I,  I, O, O, O },
                              ByteOp{ O, O, O, O,  O, O, O, O },
                              ByteOp{ x, x, x, x,  x, x, x, x },
                              ByteOp{ x, x, o, o,  o, o, o, o });

                write = MemOp(ByteOp{ I, O, I, O,  I, I, O, O },
                              ByteOp{ I, I, I, x,  x, x, x, x },
                              ByteOp{ x, x, x, x,  x, x, x, x },
                              ByteOp{ I, I, i, i,  i, i, i, i });
            }
        } lock;

        struct Calibration : Memory {
            Calibration() : Memory(kLock)
            {
                size = 1;
                read = MemOp(ByteOp{ O, O, I, I,  I, O, O, O },
                             ByteOp{ x, x, x, x,  x, x, x, x },
                             ByteOp{ O, O, O, O,  O, O, O, O },
                             ByteOp{ o, o, o, o,  o, o, o, o });
            }
        } calibration;

        struct Signature : Memory {
            Signature() : Memory(kSignature)
            {
                size = 3;
                read = MemOp(ByteOp{ O,  O,  I,  I,   O,  O,  O,  O },
                             ByteOp{ x,  x,  x,  x,   x,  x,  x,  x },
                             ByteOp{ x,  x,  x,  x,   x,  x, a1, a0 },
                             ByteOp{ o,  o,  o,  o,   o,  o,  o,  o });
            }
        } signature;
    } fRegions;

    static constexpr uint8_t fTimeOut = 200;
    static constexpr uint8_t fStabDelay = 100;
    static constexpr uint8_t fCmdExeDelay = 25;
    static constexpr uint8_t fSynchLoops = 32;
    static constexpr uint8_t fByteDelay = 0;
    static constexpr uint8_t fPollValue = 0x53;
    static constexpr uint8_t fPollIndex = 3;
    static constexpr unsigned fReadSize = 256;
    static constexpr unsigned fBlockSize = 256;
    static constexpr unsigned fChipEraseDelay = 9000;
    static constexpr unsigned fDelay = 10;
    static constexpr unsigned ENTER_PROGRAMMING_ATTEMPTS = 50;
    uint8_t fResetPin;
    uint8_t fCommandSequence;
    uint8_t* fFlashPageCache;
    uint32_t fFlashPageAddr;
    unsigned fFlashPageSize;

    uint8_t* fEEPROMPageCache;
    uint32_t fEEPROMPageAddr;
    unsigned fEEPROMPageSize;

    bool fProgMode = false;
    BootLoaderType fBootlLoader = kUnknown;
    ProgressProc fProgress = NULL;

    Memory* findRegion(MemType type)
    {
        for (Memory* mem = *Memory::head(); mem != NULL; mem = mem->next)
        {
            if (mem->type == type)
            {
                return mem;
            }
        }
        return NULL;
    }

    // *****************[ STK ISP command constants ]******************************
    void send(uint8_t b)
    {
        //Serial.print("SEND b="); Serial.println(b, HEX);
        MEGA_SERIAL.write(&b, 1); MEGA_SERIAL.flush();
    }

    bool send(unsigned char* data, size_t len)
    {
        unsigned char buf[fBlockSize + 19 + 6];     // max MESSAGE_BODY of 275 bytes, 6 bytes overhead
        int i;

        buf[0] = MESSAGE_START;
        buf[1] = fCommandSequence;
        buf[2] = len / fBlockSize;
        buf[3] = len % fBlockSize;
        buf[4] = TOKEN;
        memcpy(buf+5, data, len);

        // calculate the XOR checksum
        buf[5+len] = 0;
        for (i=0;i<5+len;i++)
            buf[5+len] ^= buf[i];

    #ifdef USE_AVR_DEBUG
        AVR_DEBUG_PRINT("AVR.send(");
        for (i=0;i<len+6;i++) {
            AVR_DEBUG_PRINT_HEX(buf[i]);
            AVR_DEBUG_PRINT(" ");
        }
        AVR_DEBUG_PRINT(", "); AVR_DEBUG_PRINT(len+6); AVR_DEBUG_PRINTLN();
    #endif
        if (MEGA_SERIAL.write(buf, len+6) != len+6)
        {
            DEBUG_PRINTLN("failed to send command to serial port");
            return false;
        }
        return true;
    }

    size_t serialRecv(uint8_t* buf, size_t len = 1)
    {
        return MEGA_SERIAL.readBytes(buf, len);
    }

    int recv(unsigned char* msg, size_t maxsize)
    {
        enum states { sINIT, sSTART, sSEQNUM, sSIZE1, sSIZE2, sTOKEN, sDATA, sCSUM, sDONE }  state = sSTART;
        unsigned int msglen = 0;
        unsigned int curlen = 0;
        int timeout = 0;
        unsigned char c, checksum = 0;

        uint32_t tstart = millis();

        while (state != sDONE && !timeout)
        {
            if (serialRecv(&c, 1) == 0)
                goto timedout;
            AVR_DEBUG_PRINT_HEX(c); AVR_DEBUG_PRINT(" ");
            checksum ^= c;

            switch (state)
            {
                case sSTART:
                    AVR_DEBUG_PRINT("hoping for start token...");
                    if (c == MESSAGE_START)
                    {
                        AVR_DEBUG_PRINTLN("got it");
                        checksum = MESSAGE_START;
                        state = sSEQNUM;
                    }
                    else
                    {
                        AVR_DEBUG_PRINTLN("sorry");
                    }
                    break;
                case sSEQNUM:
                    AVR_DEBUG_PRINT("hoping for sequence...");
                    if (c == fCommandSequence)
                    {
                        AVR_DEBUG_PRINTLN("got it, incrementing");
                        state = sSIZE1;
                        fCommandSequence++;
                    }
                    else
                    {
                        AVR_DEBUG_PRINT("sorry");
                        state = sSTART;
                    }
                    break;
                case sSIZE1:
                    AVR_DEBUG_PRINTLN("hoping for size LSB");
                    msglen = (unsigned)c * fBlockSize;
                    state = sSIZE2;
                    break;
                case sSIZE2:
                    AVR_DEBUG_PRINTLN("hoping for size MSB...");
                    msglen += (unsigned)c;
                    //DEBUG(" msg is %u bytes\n",msglen);
                    state = sTOKEN;
                    break;
                case sTOKEN:
                    if (c == TOKEN)
                        state = sDATA;
                    else
                        state = sSTART;
                    break;
                case sDATA:
                    if (curlen < maxsize)
                    {
                        msg[curlen] = c;
                    }
                    else
                    {
                        AVR_DEBUG_PRINTLN("recv(): buffer too small");
                        return -2;
                    }
                    if ((curlen == 0) && (msg[0] == ANSWER_CKSUM_ERROR))
                    {
                        AVR_DEBUG_PRINTLN("recv(): previous packet sent with wrong checksum");
                        return -3;
                    }
                    curlen++;
                    if (curlen == msglen)
                        state = sCSUM;
                    break;
                case sCSUM:
                    if (checksum == 0)
                    {
                        state = sDONE;
                    }
                    else
                    {
                        state = sSTART;
                        AVR_DEBUG_PRINTLN("recv(): checksum error");
                        return -4;
                    }
                    break;
                default:
                    AVR_DEBUG_PRINTLN("recv(): unknown state");
                    return -5;
            }

            if (millis() - tstart > MEGA_SERIAL_TIMEOUT)
            {           // wuff - signed/unsigned/overflow
            timedout:
                AVR_DEBUG_PRINTLN("recv(): timeout");
                return -1;
            }

        }
        AVR_DEBUG_PRINTLN();
        return (int)(msglen+6);
    }

#define RETRIES 5

    int getsync()
    {
        int tries = 0;
        unsigned char buf[1], resp[32];
        int status;

    retry:
        tries++;

        // send the sync command and see if we can get there
        buf[0] = CMD_SIGN_ON;
        send(buf, 1);

        // try to get the response back and see where we got
        status = recv(resp, sizeof(resp));

        // if we got bytes returned, check to see what came back
        if (status > 0)
        {
            if ((resp[0] == CMD_SIGN_ON) && (resp[1] == STATUS_CMD_OK) &&
                (status > 3))
            {
                // success!
                unsigned siglen = resp[2];
                const char* sig = "AVRISP_2";
                size_t len = strlen(sig);
                if (siglen >= len &&
                    memcmp(resp + 3, sig, len) == 0)
                {
                    fBootlLoader = kAVRISP;
                }
                else
                {
                    Serial.println("UNSUPPORTED BOOTLOADER");
                    return -1;
                }
                return 0;
            }
            else
            {
                if (tries > RETRIES)
                {
                    Serial.println("Can't communicate with device");
                    return -6;
                }
                else
                {
                    goto retry;
                }
            }

            // or if we got a timeout
        }
        else if (status == -1)
        {
            if (tries > RETRIES)
            {
                Serial.println("Timeout");
                return -1;
            }
            else
            {
                goto retry;
            }
        // or any other error
        }
        else
        {
            if (tries > RETRIES)
            {
                Serial.println("Error");
            }
            else
            {
                goto retry;
            }
        }
        return 0;
    }

    int command(unsigned char * buf, size_t len, size_t maxlen)
    {
        int tries = 0;
        int status;

    #ifdef USE_AVR_DEBUG
        AVR_DEBUG_PRINT("STK500V2: command(");
        for (size_t i = 0; i < len; i++)
        {
            AVR_DEBUG_PRINT_HEX(buf[i]);
            AVR_DEBUG_PRINT(" ");
        }
        AVR_DEBUG_PRINT(", "); DEBUG_PRINTLN(len);
    #endif

    retry:
        tries++;

        // send the command to the programmer
        send(buf,len);
        // attempt to read the status back
        status = recv(buf,maxlen);

        // if we got a successful readback, return
        if (status > 0)
        {
            AVR_DEBUG_PRINT(" = "); AVR_DEBUG_PRINTLN(status);
            if (status < 2)
            {
                AVR_DEBUG_PRINTLN("short reply");
                return -1;
            }
            if (buf[1] >= STATUS_CMD_TOUT && buf[1] < 0xa0)
            {
                AVR_DEBUG_PRINTLN("command timed out");
            }
            else if (buf[1] == STATUS_CMD_OK)
            {
                return status;
            }
            else if (buf[1] == STATUS_CMD_FAILED)
            {
                AVR_DEBUG_PRINTLN("command failed");
            }
            else if (buf[1] == STATUS_CMD_UNKNOWN)
            {
                AVR_DEBUG_PRINTLN("unknown command");
            }
            else
            {
            }
            return -1;
        }

        // otherwise try to sync up again
        status = getsync();
        if (status != 0)
        {
            if (tries > RETRIES)
            {
                return -1;
            }
            else
            {
                goto retry;
            }
        }

        AVR_DEBUG_PRINTLN(" = 0");
        return 0;
    }

    int getparm(unsigned char parm, unsigned char * value)
    {
        unsigned char buf[32];

        buf[0] = CMD_GET_PARAMETER;
        buf[1] = parm;

        if (command(buf, 2, sizeof(buf)) < 0)
        {
            return -1;
        }
        *value = buf[2];
        return 0;
    }

    int setparm_real(unsigned char parm, unsigned char value)
    {
        unsigned char buf[32];

        buf[0] = CMD_SET_PARAMETER;
        buf[1] = parm;
        buf[2] = value;

        if (command(buf, 3, sizeof(buf)) < 0)
        {
            return -1;
        }
        return 0;
    }

    int setparm(unsigned char parm, unsigned char value)
    {
        int res;
        unsigned char current_value;

        res = getparm(parm, &current_value);
        if (res == 0 && value == current_value)
        {
            return 0;
        }
        return setparm_real(parm, value);
    }

    int getparm2(unsigned char parm, unsigned int * value)
    {
        unsigned char buf[32];

        buf[0] = CMD_GET_PARAMETER;
        buf[1] = parm;

        if (command(buf, 2, sizeof(buf)) < 0)
        {
            return -1;
        }
        *value = ((unsigned)buf[2] << 8) | buf[3];
        return 0;
    }

    int setparm2(unsigned char parm, unsigned int value)
    {
        unsigned char buf[32];

        buf[0] = CMD_SET_PARAMETER;
        buf[1] = parm;
        buf[2] = value >> 8;
        buf[3] = value;

        if (command(buf, 4, sizeof(buf)) < 0)
        {
            return -1;
        }
        return 0;
    }

    int programEnable()
    {
        unsigned char buf[16];

        buf[0] = CMD_ENTER_PROGMODE_ISP;
        buf[1] = fTimeOut;
        buf[2] = fStabDelay;
        buf[3] = fCmdExeDelay;
        buf[4] = fSynchLoops;
        buf[5] = fByteDelay;
        buf[6] = fPollValue;
        buf[7] = fPollIndex;
        pgmEnableOp.setBits(buf+8);
        buf[10] = buf[11] = 0;

        return command(buf, 12, sizeof(buf));
    }

    int display()
    {
        unsigned char maj, min, hdw;

        if (getparm(PARAM_HW_VER, &hdw) == 0 &&
            getparm(PARAM_SW_MAJOR, &maj) == 0 &&
            getparm(PARAM_SW_MINOR, &min) == 0)
        {
            DEBUG_PRINT("Programmer Model: AVRISP");
            DEBUG_PRINT("Hardware Version: "); DEBUG_PRINTLN(hdw);
            DEBUG_PRINT("Firmware Version Master : "); DEBUG_PRINT(maj); DEBUG_PRINT("."); DEBUG_PRINTLN(min);
            return 0;
        }
        return -1;
    }

    int readByte(Memory& mem, uint32_t addr, uint8_t* value)
    {
        uint8_t buf[6];

        if (mem.loaded)
            return 0;
        switch (mem.type)
        {
            case kFlash:
            case kEEPROM:
            {
                uint32_t paddr = 0UL;
                uint32_t* paddr_ptr = NULL;
                unsigned pagesize = 0;
                uint8_t *cache_ptr = NULL;
                // use paged access, and cache result
                if (mem.type == kFlash)
                {
                    pagesize = fFlashPageSize;
                    paddr = addr & ~(pagesize - 1);
                    paddr_ptr = &fFlashPageAddr;
                    cache_ptr = fFlashPageCache;
                }
                else
                {
                    pagesize = mem.page_size;
                    if (pagesize == 0)
                        pagesize = 1;
                    paddr = addr & ~(pagesize - 1);
                    paddr_ptr = &fEEPROMPageAddr;
                    cache_ptr = fEEPROMPageCache;
                }

                if (paddr == *paddr_ptr)
                {
                    *value = cache_ptr[addr & (pagesize - 1)];
                    return 0;
                }

                if (readPage(mem, pagesize, paddr, pagesize) < 0)
                    return -1;

                *paddr_ptr = paddr;
                memcpy(cache_ptr, &mem.buf[paddr], pagesize);
                *value = cache_ptr[addr & (pagesize - 1)];
                return 0;
            }

             case kLFuse:
             case kFuse:
                 buf[0] = CMD_READ_FUSE_ISP;
                 addr = 0;
                 break;

            case kHFuse:
                buf[0] = CMD_READ_FUSE_ISP;
                addr = 1;
                break;

            case kEFuse:
                buf[0] = CMD_READ_FUSE_ISP;
                addr = 2;
                break;

            case kLock:
                buf[0] = CMD_READ_LOCK_ISP;
                break;

            case kCalibration:
                buf[0] = CMD_READ_SIGNATURE_ISP;
                break;

            case kSignature:
                buf[0] = CMD_READ_SIGNATURE_ISP;
                break;

            default:
                // Not supported
                return -1;
        }

        MemOp& op = mem.read;
        memset(buf + 1, 0, 5);
        if (!op.isValid())
        {
            return -1;
        }
        op.setBits(buf + 2);
        int pollidx;
        if ((pollidx = op.getOutputIndex()) == -1)
        {
            pollidx = 3;
        }
        buf[1] = pollidx + 1;
        op.setAddress(buf + 2, addr);

        int result = command(buf, 6, sizeof(buf));
        if (result < 0)
        {
            return -1;
        }
        *value = buf[2];
        return 0;
    }

    int readPage(Memory& mem, unsigned page_size, unsigned addr, unsigned n_bytes)
    {
        if (mem.loaded)
            return n_bytes;
        unsigned block_size, hiaddr, addrshift, use_ext_addr;
        unsigned maxaddr = addr + n_bytes;
        uint8_t commandbuf[4];
        uint8_t buf[275];   // max buffer size for stk500v2 at this point
        uint8_t cmds[4];
        int result;
        // DEBUG("STK500V2: stk500v2_paged_load(..,%s,%u,%u,%u)\n", m->desc, page_size, addr, n_bytes);

        page_size = fBlockSize;
        MemOp& readOp = mem.invalid;

        hiaddr = UINT_MAX;
        addrshift = 0;
        use_ext_addr = 0;

        // determine which command is to be used
        switch (mem.type)
        {
            case kFlash:
                commandbuf[0] = CMD_READ_FLASH_ISP;
                readOp = fRegions.flash.read_lo;
                addrshift = 1;
                use_ext_addr = (1U << 31);
                break;

            case kEEPROM:
                commandbuf[0] = CMD_READ_EEPROM_ISP;
                readOp = fRegions.eeprom.read;
                break;

            default:
                return -1;
        }

        if (!readOp.isValid())
        {
            return -1;
        }
        readOp.setBits(cmds);
        commandbuf[3] = cmds[0];

        for (; addr < maxaddr; addr += page_size)
        {
            if ((maxaddr - addr) < page_size)
                block_size = maxaddr - addr;
            else
                block_size = page_size;

            memcpy(buf, commandbuf, sizeof(commandbuf));

            buf[1] = block_size >> 8;
            buf[2] = block_size & 0xff;

            if (hiaddr != (addr & ~0xFFFF))
            {
                hiaddr = addr & ~0xFFFF;
                if (loadAddress(use_ext_addr | (addr >> addrshift)) < 0)
                    return -1;
            }

            result = command(buf, 4, sizeof(buf));
            if (result < 0)
            {
                return -1;
            }
            memcpy(&mem.buf[addr], &buf[2], block_size);
        }
        return n_bytes;
    }

    int loadAddress(unsigned int addr)
    {
        unsigned char buf[16];

        // DEBUG("STK500V2: stk500v2_loadaddr(%d)\n",addr);

        buf[0] = CMD_LOAD_ADDRESS;
        buf[1] = (addr >> 24) & 0xff;
        buf[2] = (addr >> 16) & 0xff;
        buf[3] = (addr >> 8) & 0xff;
        buf[4] = addr & 0xff;

        return command(buf, 5, sizeof(buf));
    }

    int chipErase()
    {
        unsigned char buf[16];

        buf[0] = CMD_CHIP_ERASE_ISP;
        buf[1] = fChipEraseDelay / 1000;
        buf[2] = 0;
        chipEraseOp.setBits(buf+3);
        return (command(buf, 7, sizeof(buf)) >= 0) ? 0 : -1;
    }

    int writePage(Memory& mem, unsigned page_size, uint32_t addr, unsigned n_bytes)
    {
        unsigned block_size;
        unsigned last_addr;
        unsigned addrshift;
        unsigned use_ext_addr;
        uint32_t maxaddr = addr + n_bytes;
        uint8_t commandbuf[10];
        uint8_t buf[266];
        uint8_t cmds[4];
        int result;
        MemOp& rop = mem.invalid;
        MemOp& wop = mem.invalid;

        if (page_size == 0)
            page_size = fBlockSize;
        addrshift = 0;
        use_ext_addr = 0;

        // determine which command is to be used
        if (mem.type == kFlash)
        {
            addrshift = 1;
            commandbuf[0] = CMD_PROGRAM_FLASH_ISP;
            /*
             * If bit 31 is set, this indicates that the following read/write
             * operation will be performed on a memory that is larger than
             * 64KBytes. This is an indication to STK500 that a load extended
             * address must be executed.
             */
            if (mem.load_ext_addr.isValid())
            {
                use_ext_addr = (1U << 31);
            }
        }
        else if (mem.type == kEEPROM)
        {
            commandbuf[0] = CMD_PROGRAM_EEPROM_ISP;
        }
        commandbuf[4] = mem.delay;

        if (addrshift == 0)
        {
            wop = mem.write;
            rop = mem.read;
        }
        else
        {
            wop = mem.write_lo;
            rop = mem.read_lo;
        }

        // if the memory is paged, load the appropriate commands into the buffer
        if (mem.mode & 0x01)
        {
            commandbuf[3] = mem.mode | 0x80;     // yes, write the page to flash

            if (!mem.loadpage_lo.isValid())
            {
                return -1;
            }
            mem.loadpage_lo.setBits(cmds);
            commandbuf[5] = cmds[0];

            if (!mem.writepage.isValid())
            {
                return -1;
            }
            mem.writepage.setBits(cmds);
            commandbuf[6] = cmds[0];

            // otherwise, we need to load different commands in
        } 
        else
        {
            commandbuf[3] = mem.mode | 0x80;     // yes, write the words to flash

            if (!wop.isValid())
            {
                return -1;
            }
            wop.setBits(cmds);
            commandbuf[5] = cmds[0];
            commandbuf[6] = 0;
        }

        // the read command is common to both methods
        if (!rop.isValid())
        {
            return -1;
        }
        rop.setBits(cmds);
        commandbuf[7] = cmds[0];

        commandbuf[8] = mem.readback_p1;
        commandbuf[9] = mem.readback_p2;

        last_addr = ~0u;
        for (; addr < maxaddr; addr += page_size)
        {
            if ((maxaddr - addr) < page_size)
                block_size = maxaddr - addr;
            else
                block_size = page_size;

            memcpy(buf,commandbuf,sizeof(commandbuf));

            buf[1] = block_size >> 8;
            buf[2] = block_size & 0xff;

            if (last_addr == UINT_MAX || last_addr + block_size != addr)
            {
                if (loadAddress(use_ext_addr | (addr >> addrshift)) < 0)
                    return -1;
            }
            last_addr = addr;

            memcpy(buf + 10, &mem.buf[addr], block_size);

            result = command(buf, block_size + 10, sizeof(buf));
            if (result < 0)
            {
                return -1;
            }
        }
        return n_bytes;
    }

    bool readMemory(Memory& mem)
    {
        if (mem.loaded)
            return true;
        if (!startProgramming())
            return false;

        mem.loaded = false;
        uint32_t addr = 0;
        while (addr < mem.size)
        {
            unsigned minpagesize = (mem.page_size > 0) ? mem.page_size : 16;
            if (mem.page_size)
            {
                if (readPage(mem, mem.page_size, addr, mem.page_size) < 0)
                    return false;
            }
            else
            {
                for (int i = 0; i < minpagesize; i += 1)
                {
                    if (addr+i < mem.size && readByte(mem, addr+i, &mem.buf[addr+i]) < 0)
                        return false;
                }
            }
            addr += minpagesize;
        }
        mem.loaded = true;
        return true;
    }

    bool dumpMemory(Memory& mem)
    {
        if (!startProgramming())
            return false;

        uint32_t addr = 0;
        bool lastRowRepeats = false;
        bool lastRowRepeatPrinted = false;
        uint8_t printedRow[16];
        memset(printedRow, 0xFF, sizeof(printedRow));
        int progressIndex = 0;
        static constexpr char progress[] = { '|', '/', '-', '\\' };
        printf("=======\n");
        uint32_t startTime = millis();
        while (addr < mem.size)
        {
            unsigned minpagesize = (mem.page_size > 0) ? mem.page_size : 16;
            if (mem.page_size)
            {
                if (readPage(mem, mem.page_size, addr, mem.page_size) < 0)
                    return false;
            }
            else
            {
                for (int i = 0; i < minpagesize; i += 1)
                {
                    if (addr+i < mem.size && readByte(mem, addr+i, &mem.buf[addr+i]) < 0)
                        return false;
                }
            }
            for (int i = 0; i < minpagesize; i += 16)
            {
                unsigned rowsize = 16;
                if (addr+i+16 >= mem.size)
                    rowsize = mem.size - addr+i;
                bool rowRepeats = (addr+i > 0) ? memcmp(&mem.buf[addr+i], printedRow, rowsize) == 0 : false;
                if (!rowRepeats)
                {
                    lastRowRepeats = false;
                    lastRowRepeatPrinted = false;
                }
                if (!lastRowRepeats)
                {
                    if (!rowRepeats)
                    {
                        int x;
                        memcpy(printedRow, &mem.buf[addr+i], sizeof(printedRow));
                        printf("%08x  ", addr+i);
                        for (x = 0; x < 16 && addr+i+x < mem.size; x++)
                        {
                            printf("%02x ", printedRow[x]);
                            if (((x+1) % 8) == 0) printf(" ");
                        }
                        while (x < 16)
                        {
                            printf("   ");
                            if (((++x) % 8) == 0) printf(" ");
                        }
                        printf("|");
                        for (x = 0; x < 16 && addr+i+x < mem.size; x++)
                        {
                            uint8_t ch = printedRow[x];
                            printf("%c", ((ch >= 32 && ch <= 127) ? ch : '.'));
                        }
                        printf("|\n");
                    }
                    else
                    {
                        lastRowRepeats = true;
                    }
                }
                else if (!lastRowRepeatPrinted)
                {
                    printf("*\n");
                    lastRowRepeatPrinted = true;
                }
            }
            addr += minpagesize;
            if (lastRowRepeats && !mem.loaded)
            {
                printf("\r%c\r", progress[progressIndex++>>1]); fflush(stdout);
                if (progressIndex == sizeof(progress)*2)
                    progressIndex = 0;
            }
        }
        printf("\r%08x\n", mem.size);
        printf("=======\n");
        uint32_t stopTime = millis();
        printf("elapsed : %u\n", stopTime - startTime);
        mem.loaded = true;
        return true;
    }

    int parseHexLine(char* line, uint8_t* bytes, unsigned &addr, unsigned &num, int &status)
    {
        int sum, len, cksum;
        char *ptr;

        num = 0;
        if (line[0] != ':')
            return 0;
        if (strlen(line) < 11)
            return 0;
        ptr = line + 1;
        if (!sscanf(ptr, "%02x", &len))
            return 0;
        ptr += 2;
        if (strlen(line) < (11 + (len * 2)))
            return 0;
        if (!sscanf(ptr, "%04x", &addr))
            return 0;
        ptr += 4;
        if (!sscanf(ptr, "%02x", &status))
            return 0;
        ptr += 2;
        sum = (len & 255) + ((addr >> 8) & 255) + (addr & 255) + (status & 255);
        while (num != len)
        {
            int data;
            if (!sscanf(ptr, "%02x", &data))
                return 0;
            bytes[num] = (uint8_t)data;
            ptr += 2;
            sum += data;
            num++;
            if (num >= fBlockSize)
                return 0;
        }
        if (!sscanf(ptr, "%02x", &cksum))
            return 0;
        if (((sum & 255) + (cksum & 255)) & 255)
            return 0; /* checksum error */
        return 1;
    }

    bool writeHexString(Memory& mem, const char* str, unsigned* lineno)
    {
        if (!startProgramming())
            return false;

        char c;
        char line[fBlockSize];
        const char* ch = str;
        unsigned linei = 0;
        if (lineno != NULL)
            *lineno = 1;
        // for non-paged memory just read the whole region
        if (!mem.page_size && !mem.loaded && !readMemory(mem))
            return false;
        unsigned lastPageRead = ~0u;
        while ((c = *ch++) != '\0')
        {
            line[linei] = '\0';
            if (c == '\n' || c == '\r')
            {
                int status;
                unsigned n;
                unsigned addr;
                uint8_t bytes[fBlockSize];
                if (linei != 0 && parseHexLine(line, bytes, addr, n, status))
                {
                    if (status == 0)
                    {
                        // printf("address: %08x  ", addr);
                        for (unsigned i = 0; i <= (n-1); i++)
                        {
                            if (mem.page_size > 0)
                            {
                                unsigned pageNumber = addr / mem.page_size;
                                if (!mem.loaded && lastPageRead != pageNumber)
                                {
                                    unsigned pageAddr = pageNumber * mem.page_size;
                                    if (!readPage(mem, mem.page_size, pageAddr, mem.page_size))
                                        return false;
                                    lastPageRead = pageNumber;
                                    progressCallback((double)addr / (double)mem.size);
                                }
                            }
                            mem.set(addr, bytes[i]);
                            addr++;
                        }
                    }
                    else if (status == 1)
                    {
                        /* end of file */
                        return true;
                    }
                    else if (status == 2)
                    {
                        /* begin of file */
                    }
                    else
                    {
                        return false;
                    }
                }
                linei = 0;
                if (lineno != NULL)
                    (*lineno)++;
            }
            else if (linei < sizeof(line)-1)
            {
                line[linei++] = c;
            }
            else
            {
                /* ignore */
            }
        }
        return false;
    }

    bool hasMemoryChanged(Memory& mem)
    {
        if (mem.page_size != 0)
        {
            for (unsigned addr = 0; addr < mem.size; addr += mem.page_size)
            {
                if (mem.isPageTagged(addr))
                {
                    return true;
                }
            }
            return false;
        }
        for (unsigned addr = 0; addr < mem.size; addr++)
        {
            if (mem.isAddressTagged(addr))
                return true;
        }
        return false;
    }

    bool writeMemory(Memory& mem, bool verify = true)
    {
        if (mem.page_size == 0)
            return false;

        if (fBootlLoader == kAVRISP)
        {
            // We would like to update only tagged pages, but the default
            // Arduino-stk500v2-bootloader has a bug that maintains a separate
            // page erase address pointer from the page address being updated.
            // The erase address pointer is incremented page by page. This forces
            // us to have to write changed pages starting with zero and ending with
            // the last modified page.

            // figure out min/max changed page addresses
            unsigned maxChangedPageAddr = 0;
            for (unsigned addr = 0; addr < mem.size; addr += mem.page_size)
            {
                if (mem.isPageTagged(addr))
                    maxChangedPageAddr = addr;
            }
            // tag all pages before maxChangedPageAddr
            for (unsigned addr = 0; addr < maxChangedPageAddr; addr += mem.page_size)
            {
                mem.tagPage(addr, true);
            }
            // Make sure the erase address starts at zero.
            // Boot loader will return failed so just ignore the error
            (void) chipErase();
        }
        uint8_t* verifyBuffer = (verify) ? (uint8_t*)alloca(mem.page_size) : NULL;
        for (unsigned addr = 0; addr < mem.size; addr += mem.page_size)
        {
            if (mem.isPageTagged(addr))
            {
                if (writePage(mem, 0, addr, mem.page_size) < 0)
                {
                    return false;
                }
                if (verify)
                {
                    memcpy(verifyBuffer, &mem.buf[addr], mem.page_size);
                    if (readPage(mem, mem.page_size, addr, mem.page_size) < 0)
                    {
                        return false;
                    }
                    if (memcmp(verifyBuffer, &mem.buf[addr], mem.page_size) != 0)
                    {
                        // verify failed
                        return false;
                    }
                    progressCallback((double)addr / (double)mem.size);
                }
            }
        }
        return true;
    }

    void progressCallback(double percent)
    {
        if (fProgress != NULL)
            fProgress(percent);
    }
};

#endif
