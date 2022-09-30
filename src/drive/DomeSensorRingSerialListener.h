#ifndef DomeSensorRingSerialListener_h
#define DomeSensorRingSerialListener_h

#include "ReelTwo.h"
#include "core/AnimatedEvent.h"
#include "drive/DomePositionProvider.h"
#include "core/MedianSampleBuffer.h"

#ifndef DOMESENSOR_BAUD_RATE
#define DOMESENSOR_BAUD_RATE 57600  /* default */
#endif

#ifdef USE_DOME_SENSOR_SERIAL_DEBUG
#define DOME_SENSOR_SERIAL_PRINT(s) DEBUG_PRINT(s)
#define DOME_SENSOR_SERIAL_PRINTLN(s) DEBUG_PRINTLN(s)
#define DOME_SENSOR_SERIAL_PRINT_HEX(s) DEBUG_PRINT_HEX(s)
#define DOME_SENSOR_SERIAL_PRINTLN_HEX(s) DEBUG_PRINTLN_HEX(s)
#else
#define DOME_SENSOR_SERIAL_PRINT(s)
#define DOME_SENSOR_SERIAL_PRINTLN(s)
#define DOME_SENSOR_SERIAL_PRINT_HEX(s)
#define DOME_SENSOR_SERIAL_PRINTLN_HEX(s)
#endif

class DomeSensorRingSerialListener : public DomePositionProvider, protected AnimatedEvent
{
public:
    DomeSensorRingSerialListener(Stream& serial) :
        fStream(&serial)
    {
    }

    inline unsigned getErrorCount()
    {
        return fErrorCount;
    }

    virtual bool ready() override
    {
        return (fPosition != -1);
    }

    virtual int getAngle() override
    {
        return fPosition;
    }

    virtual void animate() override
    {
        // append commands to command buffer
        while (fStream->available())
        {
            int ch = fStream->read();
            if (ch == '\r' || ch == '\n')
            {
                if (fState == 4)
                {
                    // Update position
                    fSamples.append(fValue);
                    if (fSampleCount < 6)
                    {
                        fPosition = fValue;
                        fSampleCount++;
                    }
                    else
                    {
                        // Return the filtered angle
                        fPosition = fSamples.median();
                    }
                    DOME_SENSOR_SERIAL_PRINT(" - ");
                    DOME_SENSOR_SERIAL_PRINTLN(fPosition);
                }
                fState = 0;
                if (fStream->available() < 10)
                    return;
                continue;
            }
            else
            {
                DOME_SENSOR_SERIAL_PRINT((char)ch);
            }
            if (fState == -1)
                continue;
            switch (fState)
            {
                case 0:
                    fState = (ch == '#') ? fState+1 : -1;
                    break;
                case 1:
                    fState = (ch == 'D') ? fState+1 : -1;
                    break;
                case 2:
                    fState = (ch == 'P') ? fState+1 : -1;
                    break;
                case 3:
                    fState = (ch == '@') ? fState+1 : -1;
                    fValue = 0;
                    break;
                case 4:
                    if (ch >= '0' && ch <= '9')
                    {
                        fValue = fValue * 10 + (ch - '0');
                    }
                    else
                    {
                        fState = -1;
                    }
                    break;
            }
            if (fState == -1)
            {
                // ERROR: Ignore remaining input
                DEBUG_PRINTLN("[DOME SENSOR] ERROR READING POSITION");
                fErrorCount++;
            }
        }
    }

private:
    Stream* fStream;
    int fPosition = -1;
    int8_t fState = 0;
    int fValue = 0;
    int fSampleCount = 0;
    unsigned fErrorCount = 0;
    MedianSampleBuffer<short, 5> fSamples;
};

#endif
