#ifndef HoloDisplay_h
#define HoloDisplay_h

#include "ReelTwo.h"
#include "HoloLights.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1331.h>
#include <SD.h>
#include <SPI.h>

/**
  * \ingroup Dome
  *
  * \class HoloDisplay
  *
  * \brief Holo Projector with Adafruit SSD1131 OLED display
  *
  * Subclass of HoloLights adding support for an Adafruit SSD1131 OLED display.
  *
  * Example usage:
  * \code
  *  HoloLights frontHolo;
  * \endcode
  */
#if USE_HOLO_TEMPLATE
template<uint8_t DATA_PIN = 45, uint32_t RGB_ORDER = GRB, uint16_t NUM_LEDS = 12>
class HoloDisplay :
    public HoloLights<DATA_PIN, RGB_ORDER, NUM_LEDS>
#else
class HoloDisplay :
    public HoloLights
#endif
{
public:
#if USE_HOLO_TEMPLATE
    /**
      * \brief Constructor
      */
    HoloDisplay(const int id = 0) :
            HoloLights<DATA_PIN, RGB_ORDER, NUM_LEDS>(id),
#else
    HoloDisplay(PixelType type = kRGBW, const int id = 0, const byte pin = 45, const byte numPixels = 12) :
            HoloLights(pin, type, id, numPixels),
#endif
        fDisplay(cs, dc, rst)
    {
    }

    /**
      * Initalizes the OLED display and SD card
      */
    virtual void setup() override
    {
        HoloLights::setup();

        /* Set SD chip select to OUTPUT */
        pinMode(SD_CS, OUTPUT);
        digitalWrite(SD_CS, HIGH);

        fDisplay.begin();
    #ifdef HOLO_DEBUG
        testPattern();
        DEBUG_PRINTLN(F("Initializing SD card..."));
    #else
        clearDisplay();
    #endif

        if (!SD.begin(SD_CS))
        {
        #ifdef HOLO_DEBUG
            DEBUG_PRINTLN(F("failed!"));
        #endif
        }
    }

    /**
      * Enables support for downloading additional media onto the SD card through the specified Serial instance.
      */
    void setDownloadStream(Stream* stream)
    {
        fDownloadStream = stream;
    }

    /**
      * Specify the sequence to animate
      */
    virtual void selectSequence(int sequence, int durationSec) override
    {
        switch (sequence)
        {
            case 1:
                play("LEIA.BD2");
                break;
            case 2:
                play("R2.BD2");
                break;
            case 3:
                play("PLANS.BD2");
                break;
            default:
                HoloLights::selectSequence(sequence, durationSec);
                break;
        }
    }

    /**
      * See HoloLights::handleCommand()
      */
    virtual void handleCommand(const char* cmd) override
    {
        if (cmd[0] != 'H' || cmd[1] != 'O')
        {
            HoloLights::handleCommand(cmd);
            return;
        }
        cmd += 2;

        char filename[12];
        char* fp = filename;
        int commandLength = strlen(cmd);
        const char* ch = (commandLength >= 3) ? &cmd[2] : "";
        while (*ch != '\0')
        {
            unsigned char c = (unsigned char)*ch++;
            *fp++ = (char)((c >= 'a' && c <= 'z') ? c - ('a' - 'A') : c);
        }
        *fp = '\0';
        if (filename != fp)
        {
            *fp++ = '.'; *fp++ = 'B'; *fp++ = 'D'; *fp++ = '2'; *fp = '\0';
        #ifdef OLED_DOWNLOAD
            if (cmd[1] == 'D' && fDownloadStream != NULL)
            {
                uint32_t len = 0;
                fDownload = true;
                fDownloadStream->println("ACK");
                if (fDownloadStream->find("LEN"))
                {
                    len = fDownloadStream->readStringUntil('\n').toInt();
                }
                uint32_t bytesReceived = uploadFile(filename, len);
                fDownload = false;
                if (bytesReceived > 0)
                {
                #ifdef HOLO_DEBUG
                    DEBUG_PRINT(F("Received: "));
                    DEBUG_PRINT(filename);
                    DEBUG_PRINT(F(" "));
                    DEBUG_PRINT(bytesReceived);
                    DEBUG_PRINTLN(F(" bytes"));
                #endif
                }
            } else
        #endif
            if (cmd[1] == 'X')
            {
                SD.remove(filename);
            }
            else if (cmd[1] == 'P')
            {
                play(filename);
            }
        }
    }

#ifdef OLED_DOWNLOAD
#endif

#ifdef HOLO_DEBUG
    void testPattern(void)
    {
        unsigned y;
  
        for(y = 0; y < 64; y++)
        {
            fDisplay.setAddrWindow(0, y, 96, 1);
            uint16_t color = BLACK;
            if(y > 55) color = WHITE;
            else if(y > 47) color = BLUE;
            else if(y > 39) color = GREEN;
            else if(y > 31) color = CYAN;
            else if(y > 23) color = RED;
            else if(y > 15) color = MAGENTA;
            else if(y > 7) color = YELLOW;
            fDisplay.writeColor(color, 96);
            fDisplay.endWrite();
        }
    }
#endif

    /**
      * Clear OLED screen to black
      */
    void clearDisplay()
    {
       fDisplay.fillScreen(BLACK);
    }

    /**
      * Stop playing the current movie
      */
    void stop()
    {
        f.close();
        fFrameCount = 0;
        fFrameCountInitial = 0;
        fTimestamp = 0;
        fTimestart = 0;
        clearDisplay();
//          restoreFrontHoloBrigthness();
    }

    /**
      * Play the specified movie file on the OLD display.
      */
    void play(const char* filename)
    {
        stop();
    #ifdef HOLO_DEBUG
        DEBUG_PRINT(F("OLED play: "));
        DEBUG_PRINTLN(filename);
    #endif
        f = SD.open(filename);
        if (!f.available())
        {
        #ifdef HOLO_DEBUG
            DEBUG_PRINT(F("File not found: "));
            DEBUG_PRINTLN(filename);
        #endif
            return;
        }

    #ifdef HOLO_DEBUG
        DEBUG_PRINT(F("Playing movie: "));
        DEBUG_PRINTLN(filename);
    #endif
        // Set front holo to half-bright during movie play
        //dimFrontHoloBrigthness();
        fFrameCount = read32();
        fFrameCountInitial = fFrameCount;
        fFramesPerSec = read32();
        fTimestamp = 0;
        fTimestart = millis();
    }

    /**
      * Runs through one frame of animation for this holoprojector display instance.
      */
    virtual void animate()
    {
        HoloLights::animate();
        if (fFrameCount > 0 && millis() - fTimestamp >= 100)
        {
            fTimestamp = millis();
            drawFrame();
            fFrameCount--;
            if (!fFrameCount)
            {
            #ifdef HOLO_DEBUG
                DEBUG_PRINT(F("Time elapsed: "));
                DEBUG_PRINT(fTimestamp - fTimestart);
                DEBUG_PRINT(F("ms "));
                DEBUG_PRINT((float)fFrameCountInitial / ((fTimestamp - fTimestart) / 1000.0));
                DEBUG_PRINTLN(F(" fps"));
            #endif
                stop();
            }
        }
    }

#ifdef OLED_DOWNLOAD
    /**
      * Start uploading the specified file through the previously the stream specified by setDownloadStream()
      */
    long uploadFile(const char* filename, uint32_t fileLength)
    {
        uint32_t receivedFileSize = 0;
        const uint32_t bufferSize = 4098;
        unsigned char* buffer = (unsigned char*)malloc(bufferSize);
        if (buffer == NULL || fDownloadStream == NULL)
            return -1;
        if (SD.exists(filename))
            SD.remove(filename);
        File file = SD.open(filename, FILE_WRITE);
        while (receivedFileSize < fileLength)
        {
            uint32_t sum = 0;
            if (!fDownloadStream->find("ACK"))
            {
            #ifdef HOLO_DEBUG
                DEBUG_PRINT(F("NAK received="));
                DEBUG_PRINTLN(receivedFileSize);
            #endif
                return -1;
            }
            sum = fDownloadStream->readStringUntil('\n').toInt();
            unsigned char* bufin = buffer;
            unsigned char* bufout = buffer;
            int nprbytes = fDownloadStream->readBytesUntil('\n', buffer, bufferSize);
            uint32_t sum2 = 0;
            for (int i = 0; i < nprbytes; i++)
            {
                sum2 += buffer[i];
            }
            if (sum == sum2)
            {
                static char sPr2Six[256] PROGMEM =
                {
                    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
                    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
                    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
                    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
                    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
                    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
                    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
                };

                receivedFileSize += nprbytes;
                showDownloadProgress(receivedFileSize, fileLength);
                while (nprbytes > 4)
                {
                    *bufout++ = (unsigned char)((unsigned int) (pgm_read_byte(&sPr2Six[bufin[0]]) << 2) | (unsigned int)pgm_read_byte(&sPr2Six[bufin[1]]) >> 4);
                    *bufout++ = (unsigned char)((unsigned int) (pgm_read_byte(&sPr2Six[bufin[1]]) << 4) | (unsigned int)pgm_read_byte(&sPr2Six[bufin[2]]) >> 2);
                    *bufout++ = (unsigned char)((unsigned int) (pgm_read_byte(&sPr2Six[bufin[2]]) << 6) | (unsigned int)pgm_read_byte(&sPr2Six[bufin[3]])]);
                    bufin += 4;
                    nprbytes -= 4;
                }
                if (nprbytes > 1)
                {
                    *bufout++ = (unsigned char) (pgm_read_byte(&sPr2Six[bufin[0]]) << 2 | pgm_read_byte(&sPr2Six[bufin[1]]) >> 4);
                }
                if (nprbytes > 2)
                {
                    *bufout++ = (unsigned char) (pgm_read_byte(&sPr2Six[bufin[1]]) << 4 | pgm_read_byte(&sPr2Six[bufin[2]]) >> 2);
                }
                if (nprbytes > 3)
                {
                    *bufout++ = (unsigned char) (pgm_read_byte(&sPr2Six[bufin[2]]) << 6 | pgm_read_byte(&sPr2Six[bufin[3]]));
                }
                file.write(buffer, bufout - buffer);
            #ifdef HOLO_DEBUG
                fDownloadStream->println("ACK");
            #endif
            }
            else
            {
            #ifdef HOLO_DEBUG
                DEBUG_PRINT(F("NAK: badchecksum: "));
                DEBUG_PRINT(sum2);
                DEBUG_PRINT(F("expected: "));
                DEBUG_PRINTLN(sum);
            #endif
            }
        }
        showDownloadProgress(0, 0);
        file.flush();
        file.close();
        free((char*)buffer);
        if (receivedFileSize == 0 || receivedFileSize != fileLength)
        {
            SD.remove(filename);
            return -1;
        }
        return receivedFileSize;
    }
#endif
private:
    void drawFrame()
    {
        uint16_t sdbuffer[96];
        int frameType = (char)f.read();
        if (frameType == -1)
        {
            /* Keyframe */
            for (int y = 0; y < 64; y++)
            {
                int num = f.read((uint8_t*)sdbuffer, sizeof(sdbuffer));
                if (num != sizeof(sdbuffer))
                {
                #ifdef HOLO_DEBUG
                    DEBUG_PRINTLN(F("Error reading key frame"));
                    DEBUG_PRINT(F("Incomplete scanline: "));
                    DEBUG_PRINTLN(y);
                #endif
                    stop();
                    break;
                }
                fDisplay.setAddrWindow(0, y, 96, 1);
                fDisplay.writePixels(sdbuffer, 96);
                fDisplay.endWrite();
            }
        }
        else if (frameType == 1)
        {
            /* Incremental */
            int x = f.read();
            int y = f.read();
            int w = f.read();
            int h = f.read();
            while (h-- > 0)
            {
                int num2 = f.read((uint8_t*)sdbuffer, w*sizeof(uint16_t));
                if (num2 != int(w*sizeof(uint16_t)))
                {
                #ifdef HOLO_DEBUG
                    DEBUG_PRINTLN(F("Error reading frame"));
                    DEBUG_PRINT(F("Incomplete scanline: "));
                    DEBUG_PRINTLN(y);
                #endif
                    stop();
                    break;
                }
                fDisplay.setAddrWindow(x,y,w,1);
                fDisplay.writePixels(sdbuffer, w);
                fDisplay.endWrite();
                y++;
            }
        }
    }

#ifdef OLED_DOWNLOAD
    void showDownloadProgress(uint32_t receivedFileSize, uint32_t fileLength)
    {
        int hp = 0;
        uint32_t siz = 0;
        int val = (fileLength > 0) ? (int)(255 * numLEDs * (double)receivedFileSize / (double)fileLength) : 0;
        for (unsigned i = 0; i < numLEDs; i++)
        {
            if (val >= 255)
            {
                setPixelColor(i, 0, 0, 255);
                val -= 255;
            }
            else if (val > 0)
            {
                setPixelColor(i, 0, 0, val);
                val = 0;
            }
            else
            {
                setPixelColor(i, kOff);
            }
        }
        show();
    }
#endif

    uint32_t read32()
    {
        uint32_t result;
        ((uint8_t *)&result)[0] = f.read(); // LSB
        ((uint8_t *)&result)[1] = f.read();
        ((uint8_t *)&result)[2] = f.read();
        ((uint8_t *)&result)[3] = f.read(); // MSB
        return result;
    }


    Adafruit_SSD1331 fDisplay;

    File f;
    boolean fDownload = false;
    uint32_t fFrameCount;
    uint32_t fTimestamp;
    uint32_t fTimestart;
    uint32_t fFrameCountInitial;
    uint32_t fFramesPerSec;
    Stream* fDownloadStream = NULL;

    enum
    {
        BLACK   = 0x0000,
        BLUE    = 0x001F,
        RED     = 0xF800,
        GREEN   = 0x07E0,
        CYAN    = 0x07FF,
        MAGENTA = 0xF81F,
        YELLOW  = 0xFFE0,
        WHITE   = 0xFFFF
    };

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

    // GND (G) - Gnd (Black Wire)
    // VCC (+) - 5v (Red Wire)
    // SDCS (SC) - Digital #53 (Gray Wire)
    // OCS (OC) - Digital #47 (Orange Wire)
    // RST (R) - Digital #49 (Green Wire)
    // D/C (DC) - Digital #48 (Brown Wire)
    // SCK (CK) - Digital #52 (White Wire)
    // MOSI (SI) - Digital #51 (Blue Wire)
    // MISO (SO) - Digital #50 (Purple Wire)
    // CD (CD) - skip

    enum {
        miso  = 50,
        mosi  = 51,
        sclk  = 52,
        cs    = 47,
        dc    = 48,
        rst   = 49,
        SD_CS = 53
    };
#elif defined(__AVR__)
    /* It is possible to run the OLED and everything else on the Mini Pro but you need to strip all the libraries */

    // GND (G) - Gnd (Black Wire)
    // VCC (+) - 5v (Red Wire)
    // SDCS (SC) - Digital #4.(Gray Wire)
    // OCS (OC) - Digital #10 (Orange Wire)
    // RST (R) - Digital #9 (Green Wire)
    // D/C (DC) - Digital #8 (Brown Wire)
    // SCK (CK) - Digital #13 (White Wire)
    // MOSI (SI) - Digital #11 (Blue Wire)
    // MISO (SO) - Digital #12 (Purple Wire)
    // CD (CD) - skip

    enum {
        sclk  = 13,
        mosi  = 11,
        cs    = 10,
        rst   = 9,
        dc    = 8,
        SD_CS = 4
    };
#elif defined(ESP32)
    enum {
        miso  = 19,
        mosi  = 23,
        sclk  = 18,
        cs    = 5,
        dc    = 16,
        rst   = 17,
        SD_CS = 4
    };
#else
    #error Holo OLED - Target MCU not supported
#endif
};

#endif

