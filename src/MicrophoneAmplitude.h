
#ifndef Microphone_h
#define Microphone_h

#include "ReelTwo.h"
#include "core/AnimatedEvent.h"
#include "core/PeakValueProvider.h"

/**
  * \ingroup Core
  *
  * \class MicrophoneAmplitude
  *
  * \brief Reads a microphone amplitude and makes it available as PeakValueProvider input.
*/
class MicrophoneAmplitude :
    public AnimatedEvent, public PeakValueProvider
{
public:
    /**
      * \brief Default Constructor
      */
    MicrophoneAmplitude(const byte analogPin = 0, const int periodMillis = 50) :
        fPin(analogPin),
        fPeriod(periodMillis),
        fSampleCount(0),
        fStartPeriod(0),
        fSample(0),
        fSignalMax(0),
        fSignalMin(1024),
        fPeakToPeak(0),
        fGain(10)
    {
        pinMode(fPin, INPUT);
    }

    /**
      * Reads the microphone connected to the analog port specified by the constructor and
      * calculates the amplitude.
      */
    virtual void animate()
    {
        unsigned long currentMillis = millis();
        fSample = analogRead(fPin);
        if (currentMillis - fStartPeriod < fPeriod)
        {
            if (fSample < 1024)
            {
                if (fSample > fSignalMax)
                    fSignalMax = fSample;
                if (fSample < fSignalMin)
                    fSignalMin = fSample;
            }
            fSampleCount++;
        }
        else
        {    
            fPeakValue = map(fSignalMax, 525, 1024, 0, 255);
            fPeakToPeak = map((fSignalMax - fSignalMin) * (int)fGain, 0, 1023, 0, 255);
            fSignalMax = 0; fSignalMin = 1024;
            fStartPeriod = currentMillis;
        }  
    }

    /**
      * Set the gain
      */
    inline void setGain(byte gainFactor)
    {
        fGain = gainFactor;
    }

    /**
      * \returns last recorded peak value
      */
    inline byte getPeakValue()
    {
        return fPeakToPeak;//peakValue;
    }

    /**
      * \returns last recorded raw sample
      */
    inline unsigned int getRawSample()
    {
        return fSample;
    }

protected:
    byte fPin;
    unsigned fPeriod;
    unsigned fSampleCount;
    unsigned long fStartPeriod;
    unsigned int fSample;
    unsigned int fSignalMax;
    unsigned int fSignalMin;
    byte fPeakToPeak;
    byte fGain;
};

#endif
