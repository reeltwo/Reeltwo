#ifndef VMusic_h
#define VMusic_h

#include "ReelTwo.h"
#include "core/SetupEvent.h"

/**
  * \ingroup Core
  *
  * \class SetupEvent
  *
  * \brief
  *
  * Utility class to control a VMusic2 module. Provides for sound playback as well as disk access.
  */
class VMusic
{
public:
    enum
    {
        kRegMode = 0x0,
        kRegStatus = 0x1,
        kRegBass = 0x2,
        kRegClockFreq = 0x3,
        kRegDecodeTime = 0x4,
        kRegAuData = 0x5,
        kRegWRAM = 0x6,
        kRegWRAMAddr = 0x7,
        kRegHDAT0 = 0x8,
        kRegHDAT1 = 0x9,
        kRegAIAddr = 0xA,
        kRegVolume = 0xB,
        kRegAICtrl0 = 0xC,
        kRegAICtrl1 = 0xD,
        kRegAICtrl2 = 0xE,
        kRegAICtrl3 = 0xF
    };

    /*
     * Base class for parseTextFile(). The process() function will be called
     * for each character. Carriage return / line feed will be turned into
     * just carriage return.
     */
    class Parser
    {
    public:
        virtual void process(char ch) = 0;
    };

    /*
     * Read-only data returned from readDirectory()
     */
    struct DirEntry
    {
        const char* name;
        bool dir;
    };

    /** \brief Default Constructor
      *
      * Registers the subclass to be called from the sketch setup() routine
      */
    VMusic(Stream &stream) :
        fStream(stream)
    {
    }

    /*
     * Called once during initialization. Will return true if successful.
     */
    bool init()
    {
        uint32_t startMillis = millis();
        sendCommand();
        while (!fAvailable && millis() < startMillis + 3000)
        {
            while (fStream.available())
            {
                char ch = fStream.read();
                if (ch == 0x0D)
                {
                    fPos = 0;
                    if (*fBuffer != '\0')
                    {
                        // DEBUG_PRINTLN(fBuffer);
                        if (strcmp(fBuffer, "No Disk") == 0)
                        {
                            fAvailable = true;
                            fDriveInserted = false;
                            break;
                        }
                        if (strcmp(fBuffer, "Stopped") == 0 ||
                            strcmp(fBuffer, "D:\\>") == 0 ||
                            strcmp(fBuffer, "D:\\>Command Failed") == 0 ||
                            strcmp(fBuffer, "No Upgrade") == 0 ||
                            strcmp(fBuffer, "Bad Command") == 0 ||
                            strcmp(fBuffer, "Command Failed") == 0)
                        {
                            fAvailable = true;
                            fDriveInserted = true;
                            break;
                        }
                    }
                }
                else if (fPos < SizeOfArray(fBuffer)-1)
                {
                    fBuffer[fPos++] = ch;
                    fBuffer[fPos] = '\0';
                }
            }
        }
        if (available())
        {
            stop();
            return true;
        }
        return false;
    }

    /*
     * Call this from loop() to process any status VMusic sends inbetween commands
     */
    void process()
    {
        while (fStream.available())
        {
            size_t size;
            char* str = (char*)getResponse(size);
            if (*str == '\0')
            {
                /* ignore */
            }
            else if (strcmp(str, "Device Removed P2") == 0)
            {
                /* Drive removed event */
            }
            else if (strcmp(str, "No Disk") == 0)
            {
                fDriveInserted = false;
                DEBUG_PRINTLN("DISK REMOVED");
            }
            else if (strcmp(str, "Device Detected P2") == 0)
            {
                /* Drive detected event */
            }
            else if (strcmp(str, "No Upgrade") == 0)
            {
                fDriveInserted = true;
                DEBUG_PRINTLN("DISK INSERTED");
            }
            else if (strcmp(str, "Stopped") == 0)
            {
                fPlaying = false;
            }
            else
            {
                DEBUG_PRINT("[VMUSIC] "); DEBUG_PRINTLN(str);
            }
        }
    }

    /*
     * Returns true if a USB drive is currently inserted.
     */
    inline bool driveInserted() const
    {
        return fDriveInserted;
    }

    /*
     * Returns true if the init() function was successful.
     */
    inline bool moduleAvailable() const
    {
        return fAvailable;
    }

    /*
     * Returns true if currently playing audio
     */
    inline bool isPlaying() const
    {
        return fPlaying;
    }

    /*
     * Returns true if currently playing audio (more accurate than isPlaying)
     */
    bool isBusy()
    {
        if (available())
        {
            writeCommandRegister(kRegDecodeTime, 0);
            return (readCommandRegister(kRegDecodeTime) != 0);
        }
        return false;
    }

    /*
     * Returns true if init() was successful and a USB drive is available.
     */
    inline bool available() const
    {
        return moduleAvailable() && driveInserted();
    }

    /*
     * Delete subdirectory from current directory
     */
    bool deleteDirectory(const char* dir)
    {
        if (!available())
            return false;

        sendCommand("DLD", dir);
        return expectResponse();
    }

    /*
     * Return the disk label limited to buffer size. Disk labels are 11 characters long.
     */
    bool diskLabel(char* labelBuffer, size_t bufferSize)
    {
        if (!available())
            return false;

        size_t size;
        *labelBuffer = '\0';
        sendCommand("DVL");
        char* ret = (char*)readLine(size);
        if (strcmp(ret, "No Disk") == 0)
            return false;
        snprintf(labelBuffer, bufferSize, "%s", ret);
        return expectResponse();
    }

    /*
     * Returns the serial number of the USB volume.
     */
    bool diskSerialNumber(uint32_t &serialNumber)
    {
        if (!available())
            return false;

        sendCommand("DSN");
        uint8_t buf[5];

        for (unsigned i = 0; i < sizeof(buf); i++)
            buf[i] = readByte();
        if (buf[4] == '\r')
        {
            serialNumber = (uint32_t(buf[0])<<0  |
                            uint32_t(buf[1])<<8  |
                            uint32_t(buf[2])<<16 |
                            uint32_t(buf[3])<<24);
            return true;
        }
        while (readByte() != '\r')
            ;
        return false;
    }

    /*
     * Delete a file in the current directory
     */
    bool deleteFile(const char* fname)
    {
        if (!available())
            return false;
        sendCommand("DLF", fname);
        return expectResponse();
    }

    /*
     * Rename a file or directory
     */
    bool renameFile(const char* oldname, const char* newname)
    {
        if (!available())
            return false;
        sendCommand("REN", oldname, newname);
        return expectResponse();
    }

    /*
     * Create a subdirectory in the current directory
     */
    bool makeDirectory(const char* dir)
    {
        sendCommand("MKD", dir);
        return expectResponse();
    }

    bool openFileRead(const char* fileName)
    {
        if (!available())
            return false;
        sendCommand("OPR", fileName);
        return expectResponse();
    }

    bool openFileAppend(const char* fileName)
    {
        if (!available())
            return false;
        sendCommand("OPW", fileName);
        return expectResponse();
    }

    bool closeFile(const char* fileName)
    {
        if (!available())
            return false;
        sendCommand("CLF", fileName);
        return expectResponse();
    }

    bool readFile(const char* fileName)
    {
        sendCommand("RD", fileName);
        return expectResponse();
    }

    bool getFileSize(const char* fileName, uint32_t &size)
    {
        uint8_t* str;
        size_t bytesRead;
        size = 0;
        sendCommand("DIR", fileName);
        // Read until first empty line
        while ((str = readLine(bytesRead)) != nullptr && str[0] != '\0')
            ;
        str = getResponse(bytesRead);
        size_t len = strlen(fileName);
        if (bytesRead == len+5 && str[len] == ' ')
        {
            str += len+1;
            size = (uint32_t(str[0])<<0  |
                   uint32_t(str[1])<<8  |
                   uint32_t(str[2])<<16 |
                   uint32_t(str[3])<<24);
            return true;
        }
        return false;
    }

    bool parseTextFile(Parser &parser, const char* fileName, const char* dir = nullptr)
    {
        if (!available())
            return false;
        if (!changeDirectory(dir))
            return false;

        uint32_t fileSize = 0;
        if (!available() || !getFileSize(fileName, fileSize) || fileSize == 0)
        {
            DEBUG_PRINT("Can't read ");
            DEBUG_PRINT(fileName);
            DEBUG_PRINTLN('.');
            return false;
        }

        process();
        if (openFileRead(fileName))
        {
            bool newline = false;
            char readCmd[9] = { 'R', 'D', 'F', ' ' };
            uint8_t chunk[128];
            uint32_t chunkSize = sizeof(chunk);
            while (fileSize != 0)
            {
                if (fileSize < chunkSize)
                    chunkSize = fileSize;
                readCmd[4] = (chunkSize>>24)&0xFF;
                readCmd[5] = (chunkSize>>16)&0xFF;
                readCmd[6] = (chunkSize>>8)&0xFF;
                readCmd[7] = (chunkSize>>0)&0xFF;
                readCmd[8] = '\r';

                fStream.write(readCmd, sizeof(readCmd));
                if (!expectResponse())
                    break;

                uint8_t* ptr = getChunkResponse(chunk, chunkSize);
                for (size_t i = 0; i < chunkSize; i++)
                {
                    char ch = (char)ptr[i];
                    if (ch == '\n' || ch == '\r')
                    {
                        // ignore following new line characters
                        if (newline)
                            continue;
                        // turn all new line characters to carriage return
                        newline = (ch == '\n' || ch == '\r');
                        if (newline)
                            ch = '\r';
                    }
                    else
                    {
                        newline = false;
                    }
                    parser.process(ch);
                }
                // DEBUG_PRINTLN();
                // DEBUG_PRINT("======= GOT CHUNK: ");
                // DEBUG_PRINTLN(chunkSize);
                fileSize -= chunkSize;
            }
            if (!newline)
            {
                parser.process('\r');
            }
            expectResponse();
            return closeFile(fileName);
        }
        return false;
    }

    bool seekFile(uint32_t pos)
    {
        if (!available())
            return false;

        uint8_t seekCmd[9] = { 'S', 'E', 'K', ' ' };
        seekCmd[4] = (pos>>24)&0xFF;
        seekCmd[5] = (pos>>16)&0xFF;
        seekCmd[6] = (pos>>8)&0xFF;
        seekCmd[7] = (pos>>0)&0xFF;
        seekCmd[8] = '\r';
        fStream.write(seekCmd, sizeof(seekCmd));
        return expectResponse();
    }

    bool readOpenFile(uint8_t* ptr, uint32_t len)
    {
        uint8_t readCmd[9] = { 'R', 'D', 'F', ' ' };
        readCmd[4] = (len>>24)&0xFF;
        readCmd[5] = (len>>16)&0xFF;
        readCmd[6] = (len>>8)&0xFF;
        readCmd[7] = (len>>0)&0xFF;
        readCmd[8] = '\r';
        fStream.write(readCmd, sizeof(readCmd));
        if (expectResponse())
        {
            getChunkResponse(ptr, len);
            return true;
        }
        return false;
    }

    bool writeToFile(uint8_t* ptr, uint32_t len)
    {
        if (!available())
            return false;

        uint8_t writeCmd[9] = { 'W', 'R', 'F', ' ' };
        writeCmd[4] = (len>>24)&0xFF;
        writeCmd[5] = (len>>16)&0xFF;
        writeCmd[6] = (len>>8)&0xFF;
        writeCmd[7] = (len>>0)&0xFF;
        writeCmd[8] = '\r';
        fStream.write(writeCmd, sizeof(writeCmd));
        fStream.write(ptr, len);
        return expectResponse();
    }

    bool writeStringToFile(const char* ptr)
    {
        return writeToFile((uint8_t*)ptr, strlen(ptr));
    }

    /*
     * Change current directory. If directory is omitted then assume root directory.
     */
    bool changeDirectory(const char* dir = nullptr)
    {
        if (!available())
            return false;

        stop();
        sendCommand("CD", "..");
        expectResponse("Command Failed");
        if (dir != nullptr)
        {
            sendCommand("CD", dir);
            expectResponse();
        }
        return true;
    }

    /*
     * Automatically suspend disk devices when not in use
     */
    bool suspendDisk()
    {
        if (!available())
            return false;

        sendCommand("SUD");
        return expectResponse();
    }

    /*
     * Keep disks active when not in use
     */
    bool wakeDisk()
    {
        if (!available())
            return false;

        sendCommand("WKD");
        return expectResponse();
    }

    /*
     * Open directory for reading. Must call readDirectory() until it returns false.
     */
    bool openDirectory(const char* dir = nullptr)
    {
        if (!available())
            return false;

        if (changeDirectory(dir))        
        {
            size_t size;
            uint8_t* ret;
            sendCommand("DIR");
            // Read until first empty line
            while ((ret = readLine(size)) != nullptr && ret[0] != '\0')
                ;
            return (ret != nullptr && ret[0] == '\0');
        }
        return false;
    }

    /*
     * Read an open directory. Must call readDirectory() until it returns false.
     * Cannot be nested to traverse subdirectories.
     */
    bool readDirectory(DirEntry &entry)
    {
        if (!available())
            return false;

        size_t size;
        uint8_t* ret;
        while ((ret = readUntilPrompt(size)) != nullptr)
        {
            entry.dir = false;
            if (size > 4 && strcmp((char*)&ret[size-4], " DIR") == 0)
            {
                ret[size-4] = '\0';
                entry.dir = true;
            }
            /* Ignore navigation entries */
            if (strcmp((char*)ret, ".") == 0)
                continue;
            if (strcmp((char*)ret, "..") == 0)
                continue;
            /* Ignore MacOS garbage */
            if (strcmp((char*)ret, "~1.TRA") == 0)
                continue;
            if (strcmp((char*)ret, "TRASHE~1") == 0)
                continue;
            if (strcmp((char*)ret, "SPOTLI~1") == 0)
                continue;
            entry.name = (char*)ret;
            return true;
        }
        return false;
    }

    /*
     * Play the specified file in the specified directory. If directory is omitted then assume current directory.
     */
    bool play(const char* snd, const char* dir = nullptr)
    {
        if (changeDirectory(dir))
        {
            size_t size;
            sendCommand("VPF", snd);
            char* str = (char*)getResponse(size);
            if (strncmp(str, "Playing ", 8) == 0)
            {
                fPlaying = true;
                return true;
            }
        }
        return false;
    }

    /*
     * Play the specified file in the specified directory repeatedly. If directory is omitted then assume current directory.
     */
    bool playRepeatedly(const char* snd, const char* dir = nullptr)
    {
        if (changeDirectory(dir))
        {
            size_t size;
            sendCommand("VRF", snd);
            char* str = (char*)getResponse(size);
            if (strncmp(str, "Playing ", 8) == 0)
            {
                fPlaying = true;
                return true;
            }
        }
        return false;
    }

    /*
     * Play all files sequentially in the specified directory and all sub-directories.
     * If directory is omitted then assume current directory.
     */
    bool playAll(const char* dir = nullptr)
    {
        if (changeDirectory(dir))
        {
            size_t size;
            sendCommand("V3A");
            char* str = (char*)getResponse(size);
            if (strncmp(str, "Playing ", 8) == 0)
            {
                fPlaying = true;
                return true;
            }
        }
        return false;
    }

    /*
     * Play all files sequentially in the specified directory and all sub-directories. Repeat forever.
     * If directory is omitted then assume current directory.
     */
    bool playAllRepeatedly(const char* dir = nullptr)
    {
        if (changeDirectory(dir))
        {
            size_t size;
            sendCommand("VRA");
            char* str = (char*)getResponse(size);
            if (strncmp(str, "Playing ", 8) == 0)
            {
                fPlaying = true;
                return true;
            }
        }
        return false;
    }

    /*
     * Play all files randomly in the specified directory and all sub-directories. Repeat forever.
     * If directory is omitted then assume current directory.
     */
    bool playRandomRepeatedly(const char* dir = nullptr)
    {
        if (changeDirectory(dir))
        {
            size_t size;
            sendCommand("VRR");
            char* str = (char*)getResponse(size);
            if (strncmp(str, "Playing ", 8) == 0)
            {
                fPlaying = true;
                return true;
            }
        }
        return false;
    }

    /*
     * Skip to the next track if playing files using playAll or playAllRepeatedly
     */
    bool nextTrack()
    {
        if (!available())
            return false;
        sendCommand("VSF");
        return expectResponse();
    }

    /*
     * Skip to the previous track if playing files using playAll or playAllRepeatedly
     */
    bool previousTrack()
    {
        if (!available())
            return false;
        sendCommand("VSB");
        return expectResponse();
    }

    /*
     * Skip to next directory track if playing files using playAll or playAllRepeatedly
     */
    bool nextDirectory()
    {
        if (!available())
            return false;
        sendCommand("VSD");
        return expectResponse();
    }

    /*
     * Pauses the current file if a file is currently playing or resumes playback if the
     * playback is currently paused.
     */
    void togglePausePlay()
    {
        if (!available())
            return;
        sendCommand("VP");
        expectResponse();
    }

    /*
     * Skip forwards 5 seconds through the currently playing file.
     */
    bool fastForward()
    {
        if (!available())
            return false;
        sendCommand("VF");
        return expectResponse();
    }

    /*
     * Skip backwards 5 seconds through the currently playing file.
     */
    bool fastReverse()
    {
        if (!available())
            return false;
        sendCommand("VB");
        return expectResponse();
    }

    /*
     * Stop playback of current file
     */
    bool stop()
    {
        if (!available())
            return false;
        fPlaying = false;
        sendCommand("VST");
        return expectResponse("Command Failed");
    }

    /*
     * Set the volume for both left and right channels (0 silent - 100 maximum)
     * Volume settings is logarithmic. Every tic is 0.5db, quite reliably (ie, 20 tics is 10db, etc)
     */
    bool setVolume(uint8_t volumePercent)
    {
        if (!available())
            return false;
        sendByteCommand("VSV", map(min(volumePercent, 100), 0, 100, 100, 0));
        return expectResponse();
    }

    /*
     * Reads from the command register on the VLSI VS1003.
     */
    uint16_t readCommandRegister(uint8_t commandRegister)
    {
        uint16_t ret;
        sendByteCommand("VRD", commandRegister);
        ret = (unsigned(readByte()) << 8 | unsigned(readByte()));
        expectResponse();
        return ret;
    }

    /*
     * Reads from the command register on the VLSI VS1003.
     */
    bool writeCommandRegister(uint8_t commandRegister, uint16_t value)
    {
        sendByteWordCommand("VWR", commandRegister, value);
        return expectResponse();
    }

    /*
     * Send string to VMusic module
     */
    void outputString(const char* str)
    {
        size_t len = strlen(str);
        while (len-- > 0)
            fBuffer[fPos++] = *str++;
    }

    /*
     * Send upper case string to VMusic module
     */
    void outputUpperString(const char* str)
    {
        size_t len = strlen(str);
        while (len-- > 0)
            fBuffer[fPos++] = toupper(*str++);
    }

    uint16_t getDecodeTime()
    {
        return readCommandRegister(kRegDecodeTime);
    }

    uint32_t getSampleRate()
    {
        return readCommandRegister(kRegAuData) & ~1;
    }

    bool isStereo()
    {
        return (readCommandRegister(kRegAuData) & 1);
    }

    bool isMono()
    {
        return ((readCommandRegister(kRegAuData) & 1) == 0);
    }

    /*
     *
     */
    void setBassEnhancer(int trebleControl, int freqLimit, int bassEnhancer, int lowerLimit)
    {
        trebleControl = min(max(trebleControl, -8), 7);
        if (trebleControl != 0)
            trebleControl += 8;
        freqLimit = min(max(freqLimit, 0), 15);
        bassEnhancer = min(max(bassEnhancer, 0), 15);
        lowerLimit = min(max(lowerLimit, 0), 15);
        writeCommandRegister(kRegBass,
            (trebleControl<<12) |
            (freqLimit<<8) |
            (bassEnhancer<<4) |
            (lowerLimit<<0));
    }

    /*
     * Send optional command with optional arguments
     */
    void sendCommand(const char* cmd = nullptr, const char* arg1 = nullptr, const char* arg2 = nullptr)
    {
        process();

        fPos = 0;
        if (cmd != nullptr)
        {
            // output command
            outputString(cmd);
        }
        if (arg1 != nullptr)
        {
            // add space
            fBuffer[fPos++] = ' ';
            // output upper case string
            outputUpperString(arg1);
        }
        if (arg2 != nullptr)
        {
            // add space
            fBuffer[fPos++] = ' ';
            // output upper case string
            outputUpperString(arg2);
        }
        // append carriage return
        fBuffer[fPos++] = '\r';
        fStream.write(fBuffer, fPos);

        // DEBUG_PRINT("[SEND]: ");
        // for (unsigned i = 0; i < fPos; i++)
        // {
        //     DEBUG_PRINT_HEX((uint8_t)fBuffer[i]);
        //     DEBUG_PRINT(" ");
        // }
        // DEBUG_PRINTLN();
        // for (unsigned i = 0; i < fPos; i++)
        // {
        //     if ((uint8_t)fBuffer[i] >= 32 && (uint8_t)fBuffer[i] <= 127)
        //         DEBUG_PRINT(fBuffer[i]);
        //     else
        //         DEBUG_PRINT('.');
        //     DEBUG_PRINT(" ");
        // }
        // DEBUG_PRINTLN();

        fPos = 0;
    }

    /*
     * Send command with one byte argument
     */
    void sendByteCommand(const char* cmd, uint8_t arg)
    {
        process();

        // DEBUG_PRINTLN(cmd);
        fPos = 0;
        // output command
        outputString(cmd);
        // add space
        fBuffer[fPos++] = ' ';
        fBuffer[fPos++] = (char)arg;
        // append carriage return
        fBuffer[fPos++] = '\r';
        fStream.write(fBuffer, fPos);
        fPos = 0;
    }

    /*
     * Send command with one byte argument and one word argument
     */
    void sendByteWordCommand(const char* cmd, uint8_t arg, uint16_t word)
    {
        process();

        fPos = 0;
        // output command
        outputString(cmd);
        // add space
        fBuffer[fPos++] = ' ';
        fBuffer[fPos++] = (char)arg;
        fBuffer[fPos++] = (char)(word & 0xFF);
        fBuffer[fPos++] = (char)(word >> 8);
        // append carriage return
        fBuffer[fPos++] = '\r';
        fStream.write(fBuffer, fPos);
        fPos = 0;
    }

    /*
     * Block until a single byte is read from the VMusic module
     */
    uint8_t readByte()
    {
        uint8_t data;
        if (fStream.readBytes(&data, 1) == 1)
            return data;
        return 0;
    }

    /*
     * Expect response
     */
    bool expectResponse(const char* altresponse = nullptr)
    {
        size_t size;
        uint8_t* str = getResponse(size);
        if (size == 0)
            return true;
        if (altresponse != nullptr &&
            (strlen(altresponse) == size &&
             memcmp(altresponse, str, size) == 0))
        {
            return true;
        }
        DEBUG_PRINT("VMUSIC UNEXPECTED RESPONSE: \"");
        DEBUG_PRINT("\"");
        DEBUG_PRINT((char*)str);
        DEBUG_PRINTLN('"');
        return false;
    }

    uint8_t* getChunkResponse(uint8_t* buffer, size_t size)
    {
        // char text[17];
        // int cnt = 0;
        size_t pos = 0;
        while (pos < size)
        {
            if (fStream.available())
            {
                buffer[pos++] = fStream.read();
            }
        }
        return buffer;
    }

    uint8_t* getResponse(size_t& size)
    {
        size = 0;
        bool skipPrompt = false;
        while (fAvailable)
        {
            if (fStream.available())
            {
                int ch = fStream.read();
                // DEBUG_PRINT("[VMUSIC2]: ");
                // DEBUG_PRINT((int)ch);
                // DEBUG_PRINT(" - ");
                // DEBUG_PRINTLN((char)ch);
                if (ch == 0x0D)
                {
                    fBuffer[fPos] = '\0';
                    size = fPos;
                    fPos = 0;
                    if (*fBuffer != '\0')
                    {
                        // DEBUG_PRINT("[VMUSIC2]: ");
                        // for (unsigned i = 0; i < size; i++)
                        // {
                        //     DEBUG_PRINT_HEX((uint8_t)fBuffer[i]);
                        //     DEBUG_PRINT(" ");
                        // }
                        // DEBUG_PRINTLN(fBuffer);
                        break;
                    }
                    if (skipPrompt)
                        break;
                }
                else if (fPos < SizeOfArray(fBuffer)-1)
                {
                    fBuffer[fPos++] = ch;
                    fBuffer[fPos] = '\0';
                    if (!skipPrompt && strcmp(fBuffer, "D:\\>") == 0)
                    {
                        skipPrompt = true;
                        fBuffer[0] = '\0';
                        fPos = 0;
                    }
                }
            }
        }
        return (uint8_t*)fBuffer;
    }

    uint8_t* readLine(size_t& size)
    {
        size = 0;
        while (fAvailable)
        {
            if (fStream.available())
            {
                int ch = fStream.read();
                // DEBUG_PRINT("[VMUSIC3]: ");
                // DEBUG_PRINT((int)ch);
                // DEBUG_PRINT(" - ");
                // DEBUG_PRINTLN((char)ch);
                if (ch == 0x0D)
                {
                    fBuffer[fPos] = '\0';
                    size = fPos;
                    fPos = 0;
                    break;
                }
                else if (fPos < SizeOfArray(fBuffer)-1)
                {
                    fBuffer[fPos++] = ch;
                    fBuffer[fPos] = '\0';
                }
            }
        }
        return (uint8_t*)fBuffer;
    }

    uint8_t* readUntilPrompt(size_t& size)
    {
        size = 0;
        bool skipPrompt = false;
        while (fAvailable)
        {
            if (fStream.available())
            {
                int ch = fStream.read();
                // DEBUG_PRINT("[VMUSIC3]: ");
                // DEBUG_PRINT((int)ch);
                // DEBUG_PRINT(" - ");
                // DEBUG_PRINTLN((char)ch);
                if (ch == 0x0D)
                {
                    fBuffer[fPos] = '\0';
                    size = fPos;
                    fPos = 0;
                    if (*fBuffer != '\0')
                    {
                        // DEBUG_PRINT("[VMUSIC2]: ");
                        // for (unsigned i = 0; i < size; i++)
                        // {
                        //     DEBUG_PRINT_HEX((uint8_t)fBuffer[i]);
                        //     DEBUG_PRINT(" ");
                        // }
                        // DEBUG_PRINTLN(fBuffer);
                        break;
                    }
                    if (skipPrompt)
                        return nullptr;
                }
                else if (fPos < SizeOfArray(fBuffer)-1)
                {
                    fBuffer[fPos++] = ch;
                    fBuffer[fPos] = '\0';
                    if (!skipPrompt && strcmp(fBuffer, "D:\\>") == 0)
                    {
                        skipPrompt = true;
                        fBuffer[0] = '\0';
                        fPos = 0;
                    }
                }
            }
        }
        return (uint8_t*)fBuffer;
    }

private:
    Stream& fStream;
    bool fAvailable = false;
    bool fDriveInserted = false;
    bool fPlaying = false;
    char fBuffer[32];
    unsigned fPos = 0;
};

#endif
