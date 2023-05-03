#ifndef REELTWO_H
#define REELTWO_H

/////////////////////////////////

#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
#endif
#if defined(__AVR__)
#include <avr/pgmspace.h>
#endif
#include <Wire.h>

#ifndef USE_LEDLIB
#define USE_LEDLIB 1 //0 for FastLED, 1 for Adafruit_NeoPixel, 2 for NeoPixelBus
#endif

#define I2C_MAGIC_PANEL   0x14

#define JEDI_BAUD_RATE    2400

#ifndef UNUSED_ARG
 #define UNUSED_ARG(arg) (void)arg;
#endif
#if defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__) || defined(TEENSYDUINO)
 // Teensy
 #define REELTWO_TEENSY
 #if defined(__MK20DX256__)
  #define REELTWO_TEENSY_3_2
  #ifdef USE_TEENSY_PROPSHIELD
   #include <SPI.h>
   // Teensy Prop Shield Pin Definitions
   #define PIN_PROP_AMP_ENABLE        5       // Pin to enable amp
   #define PIN_PROP_FLASH_CHIP_SELECT 6       // Pin to enable flash chip SPI
   #define PIN_PROP_LED_ENABLE        7       // Pin to enable 11/13 as outputs
   #define PIN_PROP_LED_DATA          11      // Pin #1 for 5v LEDs (data for APA102/dotstar leds)

   #define TEENSY_PROP_NEOPIXEL_SETUP() \
   { \
      if (getPin() == PIN_PROP_LED_DATA) \
      { \
          SPI.begin(); \
          pinMode(PIN_PROP_LED_ENABLE, OUTPUT); \
          digitalWrite(PIN_PROP_LED_ENABLE, LOW); \
      } \
      else \
      { \
          begin(); \
      } \
   }

   #define TEENSY_PROP_NEOPIXEL_BEGIN() \
   { \
      if (getPin() == PIN_PROP_LED_DATA) \
      { \
          static SPISettings neopixel_spi(20000000, MSBFIRST, SPI_MODE0); \
          begin();  \
          SPI.beginTransaction(neopixel_spi); \
          digitalWrite(PIN_PROP_LED_ENABLE, HIGH); \
      } \
   }

   #define TEENSY_PROP_NEOPIXEL_END() \
   { \
      if (getPin() == PIN_PROP_LED_DATA) \
      { \
          SPI.endTransaction(); \
          volatile uint32_t *reg; \
          reg = portConfigRegister(11); \
          *reg =PORT_PCR_MUX(2); \
          digitalWrite(PIN_PROP_LED_ENABLE, LOW); \
      } \
   }
  #endif
 #endif
 #ifdef USE_SMQ
  #define SMQ_SERIAL Serial1
 #endif
 #ifndef DEFAULT_BAUD_RATE
  #define DEFAULT_BAUD_RATE 115200
 #endif
 #define DEBUG_SERIAL Serial
#elif defined(__SAMD21G18A__)
 // Zero
 #define REELTWO_ZERO
 #ifdef USE_SMQ
  #define SMQ_SERIAL Serial1
 #endif
 #define DEBUG_SERIAL SerialUSB
 #ifndef DEFAULT_BAUD_RATE
  #define DEFAULT_BAUD_RATE 115200
 #endif
#elif defined(__AVR__)
 // AVR
 #define REELTWO_AVR
 #ifdef __AVR_ATmega2560__
  #define REELTWO_AVR_MEGA
 #endif
 #ifdef USE_SMQ
  #define SMQ_SERIAL Serial
  #ifdef HAVE_HWSERIAL1
   #define DEBUG_SERIAL Serial1
  #endif
 #else
   #define DEBUG_SERIAL Serial
 #endif
 #define FALL_THROUGH [[fallthrough]];
#elif defined(ESP32)
 // ESP32
 #define REELTWO_ESP32
 #if defined(USE_SMQ) && !defined(USE_SMQ32)
  #define SMQ_SERIAL Serial
  #ifdef HAVE_HWSERIAL1
   #define DEBUG_SERIAL Serial1
  #endif
 #else
   #define DEBUG_SERIAL Serial
 #endif
 #ifndef DEFAULT_BAUD_RATE
  #define DEFAULT_BAUD_RATE 115200
 #endif
 #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(2, 0, 8)
  // Work around breaking change in 2.0.8
  #undef F
  #undef FPSTR
  #define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
  #define F(string_literal) (FPSTR(PSTR(string_literal)))
 #endif
#elif defined(ARDUINO_ARCH_RP2040)
 // RP2040
 #define REELTWO_RP2040
 #ifdef USE_SMQ
  #define SMQ_SERIAL Serial
  #ifdef HAVE_HWSERIAL1
   #define DEBUG_SERIAL Serial1
  #endif
 #else
   #define DEBUG_SERIAL Serial
 #endif
 #ifndef DEFAULT_BAUD_RATE
  #define DEFAULT_BAUD_RATE 115200
 #endif
#elif defined(ARDUINO_ARCH_LINUX)
 #define DEBUG_SERIAL Serial
#else
 #error Platform not presently supported
#endif

#ifndef FALL_THROUGH
 #define FALL_THROUGH /* fall through */
#endif
#ifndef ESP32
 #define IRAM_ATTR /* not used */
#endif

#ifndef USE_DEBUG
 #undef DEBUG_SERIAL
#endif

#ifndef TEENSY_PROP_NEOPIXEL_SETUP
 #define TEENSY_PROP_NEOPIXEL_SETUP() { begin(); }
#endif
#ifndef TEENSY_PROP_NEOPIXEL_BEGIN
 #define TEENSY_PROP_NEOPIXEL_BEGIN()
#endif
#ifndef TEENSY_PROP_NEOPIXEL_END
 #define TEENSY_PROP_NEOPIXEL_END()
#endif

#define UNUSED(x) (void)(x)

#ifdef DEBUG_SERIAL
 #define DEBUG_SERIAL_READY() DEBUG_SERIAL.begin(DEFAULT_BAUD_RATE)
 #define DEBUG_PRINTLN(s) DEBUG_SERIAL.println(s)
 #define DEBUG_PRINT(s) DEBUG_SERIAL.print(s)
#ifdef ESP32
 #define DEBUG_PRINTF(...) ::printf(__VA_ARGS__)
#else
 #define DEBUG_PRINTF(...) DEBUG_SERIAL.printf(__VA_ARGS__)
#endif
 #define DEBUG_PRINTLN_HEX(s) DEBUG_SERIAL.println(s,HEX)
 #define DEBUG_PRINT_HEX(s) DEBUG_SERIAL.print(s,HEX)
 #define DEBUG_FLUSH() DEBUG_SERIAL.flush()
#else
 #define DEBUG_SERIAL_READY()
 #define DEBUG_PRINTLN(s) while (0)
 #define DEBUG_PRINT(s) while (0)
 #define DEBUG_PRINTF(...) while (0)
 #define DEBUG_PRINTLN_HEX(s) while (0)
 #define DEBUG_PRINT_HEX(s) while (0)
 #define DEBUG_FLUSH() while (0)
#endif

#define TYPETOSTR(T) #T

#ifdef JEDI_SERIAL
 #define JEDI_SERIAL_READY() JEDI_SERIAL.begin(JEDI_BAUD_RATE)
#else
 #define JEDI_SERIAL_READY()
#endif

#ifndef JEDI_SERIAL
 #ifdef HAVE_HWSERIAL3
  #define JEDI_SERIAL Serial3 /* Use Serial3 for JawaLite commands */
 #else
  /* TODO Software serial for JawaLite */
 #endif
#endif

#ifndef SizeOfArray
 #define SizeOfArray(arr) (sizeof(arr)/sizeof(arr[0]))
#endif

#ifndef DEFAULT_BAUD_RATE
 #ifdef __AVR_ATmega2560__
  #define DEFAULT_BAUD_RATE 115200
 #else
  #define DEFAULT_BAUD_RATE 57600
 #endif
#endif

#define _REELTWO_READY_ \
   {DEBUG_SERIAL_READY(); \
    JEDI_SERIAL_READY();}
#ifndef USE_SMQ
 #define REELTWO_READY() _REELTWO_READY_
#endif

#ifndef USE_SMQ
#define SMQMESSAGE(topic, handler)
#endif

typedef const __FlashStringHelper* PROGMEMString;

// crc-16 poly 0x8005 (x^16 + x^15 + x^2 + 1)
constexpr uint16_t _crc16_table[] =
{
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241, 0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40, 0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40, 0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641, 0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240, 0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41, 0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41, 0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640, 0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240, 0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41, 0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41, 0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640, 0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241, 0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40, 0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40, 0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641, 0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

#ifndef __AVR__
/* Compile time calculate CRC32 table */
template <uint32_t c, int k = 8>
struct f : f<((c & 1) ? 0xedb88320 : 0) ^ (c >> 1), k - 1> {};
template <uint32_t c> struct f<c, 0>{enum {value = c};}; /* {Secret} */

#define _CRCA(x) _CRCB(x) _CRCB(x + 128)
#define _CRCB(x) _CRCC(x) _CRCC(x +  64)
#define _CRCC(x) _CRCD(x) _CRCD(x +  32)
#define _CRCD(x) _CRCE(x) _CRCE(x +  16)
#define _CRCE(x) _CRCF(x) _CRCF(x +   8)
#define _CRCF(x) _CRCG(x) _CRCG(x +   4)
#define _CRCG(x) _CRCH(x) _CRCH(x +   2)
#define _CRCH(x) _CRCI(x) _CRCI(x +   1)
#define _CRCI(x) f<x>::value ,

constexpr uint32_t _crc32_table[] = { _CRCA(0L) };
#else
/* avr-gcc compiler broken */
constexpr uint32_t _crc32_table[] =
{
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};
#endif

constexpr uint16_t crc16_impl(const char* p, size_t len, uint16_t crc)
{
    return len ?
            crc16_impl(p+1,len-1,(crc>>8)^_crc16_table[((crc^*p)&0xFF)])
            : crc;
}

constexpr uint32_t crc32_impl(const char* p, size_t len, uint32_t crc)
{
    return len ?
            crc32_impl(p+1L,len-1L,(crc>>8L)^_crc32_table[(crc&0xFF)^(*p&0xFF)])
            : crc;
}

constexpr uint16_t crc16(const char* data, size_t length)
{
    return ~crc16_impl(data, length, ~0);
}

constexpr uint32_t crc32(const char* data, size_t length)
{
    return ~crc32_impl(data, length, ~0L);
}

constexpr size_t strlen_c(const char* str)
{
    return *str ? 1+strlen_c(str+1) : 0;
}

constexpr uint16_t WSID16(const char* str)
{
    return crc16(str, strlen_c(str));
}

constexpr uint32_t WSID32(const char* str)
{
    return crc32(str, strlen_c(str));
}

/* Anonymous lambda's broken in avr-gcc version 5.4.0 */
#define FSTR(str) (str)/*[](){static const char _str[] PROGMEM = str; return (PROGMEMString)_str;}()*/
#define FORCE_CT_EVAL(func) [](){constexpr auto ___expr = func; return ___expr;}()
#define SMQID_CONST(str) WSID16(str) /*WSID32(str)*/
#define MSGID_CONST(str) WSID16(str)
#define STRID_CONST(str) WSID32(str)
#define SMQID(str) FORCE_CT_EVAL(SMQID_CONST(str))
#define MSGID(str) FORCE_CT_EVAL(MSGID_CONST(str))
#define STRID(str) FORCE_CT_EVAL(STRID_CONST(str))

#define casePString(str) \
    case STRID_CONST(str): return F(str)

#ifdef DEBUG_SERIAL
#define DEBUG_LINE(Serial) \
    { DEBUG_SERIAL.print("LINE:"); DEBUG_SERIAL.println(__LINE__); DEBUG_SERIAL.flush(); }
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////

/// \private
constexpr unsigned int CompileTimeCounterLimit = 250;

/// \private
template <unsigned int ValueArg> struct CompileTimeCounterTemplateInt { constexpr static unsigned int Value = ValueArg; };

/// \private
template <unsigned int GetID, typename, typename TagID>
constexpr unsigned int HighestCompileTimeEval(TagID, CompileTimeCounterTemplateInt<0>)
{
    return 0;
}

/// \private
template <unsigned int GetID, typename, typename TagID, unsigned int Index>
constexpr unsigned int HighestCompileTimeEval(TagID, CompileTimeCounterTemplateInt<Index>)
{
    return HighestCompileTimeEval<GetID, void>(TagID(), CompileTimeCounterTemplateInt<Index - 1>());
}

/**
  * \brief ReadCompileTimeCounter
  *
  * Read current value of a compile time counter
  */
#define ReadCompileTimeCounter(...) \
HighestCompileTimeEval<__COUNTER__, void>(__VA_ARGS__(), CompileTimeCounterTemplateInt<CompileTimeCounterLimit>())

/// \private
template<bool B, class T = void>
struct std_enable_if {};

/// \private
template<class T>
struct std_enable_if<true, T> { typedef T type; };

/**
  * \brief IncrementCompileTimeCounter
  *
  * Increment a compile time counter
  */
#define IncrementCompileTimeCounter(TagID) \
template <unsigned int GetID, typename = typename std_enable_if<(GetID > __COUNTER__ + 1)>::type> \
constexpr unsigned int Highest( \
    TagID, \
    CompileTimeCounterTemplateInt<ReadCompileTimeCounter(TagID) + 1> Value) \
{ \
      return decltype(Value)::Value; \
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef STEALTHI2C
#define STEALTHI2C 0
#endif

template <byte i2cAddress = STEALTHI2C>
void StealthCommand(String cmd)
{
    byte sum = 0;
    Wire.beginTransmission(i2cAddress);
    for (int i = 0; i < cmd.length(); i++)
    {
        Wire.write(cmd[i]);
        sum += byte(cmd[i]);
    }
    Wire.write(sum);
    Wire.endTransmission();  
}

template <byte i2cAddress = STEALTHI2C>
void StealthCommand(const char* cmd)
{
    byte sum = 0;
    Wire.beginTransmission(i2cAddress);
    for (size_t len = strlen(cmd); len-- > 0; cmd++)
    {
        Wire.write(*cmd);
        sum += byte(*cmd);
    }
    Wire.write(sum);
    Wire.endTransmission();  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

inline void PrintReelTwoInfo(Print& out, const char* desc)
{
    out.print(desc);
    out.print(F(" - "));
    out.print(__DATE__);
#ifdef ESP_ARDUINO_VERSION_MAJOR
    out.print(F(" - ESP: ["));
    out.print(ESP_ARDUINO_VERSION_MAJOR);
    out.print('.');
    out.print(ESP_ARDUINO_VERSION_MINOR);
    out.print('.');
    out.print(ESP_ARDUINO_VERSION_PATCH);
    out.print(']');
#endif
    out.println();
#ifdef BUILD_VERSION
    out.print(F("Source: "));
    out.print(F(BUILD_VERSION));
    out.println();
#endif
#ifdef REELTWO_BUILD_VERSION
    out.print(F("ReelTwo: "));
    out.print(F(REELTWO_BUILD_VERSION));
    out.println();
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
