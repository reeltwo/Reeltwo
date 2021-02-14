/*
 * AnalogMonitor.h
 * Arduino library for eliminating noise in analogRead inputs without decreasing responsiveness
 *
 * Copyright (c) 2016 Damien Clarke
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef AnalogMonitor_h
#define AnalogMonitor_h

#include "core/AnimatedEvent.h"

/**
  * \ingroup Core
  *
  * \class AnalogMonitor
  *
  * \brief 
  *
  * Used for eliminating noise in analogRead inputs without decreasing responsiveness. It sets out to achieve the following:
  *
  * \li Be able to reduce large amounts of noise when reading a signal. So if a voltage is unchanging aside from noise,
  * the values returned should never change due to noise alone.
  *
  * \li Be extremely responsive (i.e. not sluggish) when the voltage changes quickly.
  *
  * \li Have the option to be responsive when a voltage stops changing - when enabled the values returned must stop changing
  * almost immediately after. When this option is enabled, a very small sacrifice in accuracy is permitted.
  *
  * \li The returned values must avoid 'jumping' up several numbers at once, especially when the input signal changes very
  * slowly. It's better to transition smoothly as long as that smooth transition is short.
  */
class AnalogMonitor : public AnimatedEvent
{
public:
    /** \brief Default Constructor
      *
      * Default constructor must be followed by a call to begin()
      */
    AnalogMonitor()
    {
    }

    /** Constructor with explicit call to begin()
      *
      * \param pin the pin to read
      * \param sleepEnable enabling sleep will cause values to take less time to stop changing and potentially stop changing more abruptly,
      *  where as disabling sleep will cause values to ease into their correct position smoothly
      * \param snapMultiplier a value from 0 to 1 that controls the amount of easing
      *   increase this to lessen the amount of easing (such as 0.1) and make the responsive values more responsive
      *   but doing so may cause more noise to seep through if sleep is not enabled
      *
      *
      * \sa begin(byte, bool, flooat) 
      */
    AnalogMonitor(byte pin, bool sleepEnable = true, float snapMultiplier = 0.01)
    {
        begin(pin, sleepEnable, snapMultiplier);
    }

    /** Start reading data from analog port. 
      *
      * \param pin the pin to read
      * \param sleepEnable enabling sleep will cause values to take less time to stop changing and potentially stop changing more abruptly,
      *  where as disabling sleep will cause values to ease into their correct position smoothly
      * \param snapMultiplier a value from 0 to 1 that controls the amount of easing
      *   increase this to lessen the amount of easing (such as 0.1) and make the responsive values more responsive
      *   but doing so may cause more noise to seep through if sleep is not enabled
      */
    void begin(byte pin, bool sleepEnable = true, float snapMultiplier = 0.01)
    {
        pinMode(pin, INPUT);    // ensure button pin is an input
        digitalWrite(pin, LOW); // ensure pullup is off on button pin

        fPin = pin;
        fSleepEnable = sleepEnable;
        setSnapMultiplier(snapMultiplier);
    }

    /**
      *\returns the responsive value from last update
      */
    inline int getValue()
    {
        return fResponsiveValue;
    }

    /**
      * \returns the raw analogRead() value from last update
      */
    inline int getRawValue()
    {
        return fRawValue;
    }

    /**
      * \returns true if the responsive value has changed during the last update
      */
    inline bool hasChanged()
    {
        return fResponsiveValueHasChanged;
    }

    /**
      * \returns true if the algorithm is currently in sleeping mode
      */
    inline bool isSleeping()
    {
        return fSleeping;
    }

    virtual void animate()
    {
        if (fPin == 0xFF)
            return;
        fRawValue = analogRead(fPin);
        fPrevResponsiveValue = fResponsiveValue;
        fResponsiveValue = getResponsiveValue(fRawValue);
        fResponsiveValueHasChanged = (fResponsiveValue != fPrevResponsiveValue);
    }

    void setSnapMultiplier(float newMultiplier)
    {
        fSnapMultiplier =
            (newMultiplier > 1.0) ? 1.0 :
            (newMultiplier < 0.0) ? 0.0 : newMultiplier;
    }

    inline void enableSleep()
    {
        fSleepEnable = true;
    }

    inline void disableSleep()
    {
        fSleepEnable = false;
    }

    inline void enableEdgeSnap()
    {
        fEdgeSnapEnable = true;
    }

    /**
      * Edge snap ensures that values at the edges of the spectrum (0 and 1023) can be easily reached when sleep is enabled
      */
    inline void disableEdgeSnap()
    {
        fEdgeSnapEnable = false;
    }

    inline void setActivityThreshold(float newThreshold)
    {
        fActivityThreshold = newThreshold;
    }

    /**
      * The amount of movement that must take place to register as activity and start moving the output value. Defaults to 4.0
      */
    inline void setAnalogResolution(int resolution)
    {
        // if your ADC is something other than 10bit (1024), set that here
        fAnalogResolution = resolution;
    }

private:
    byte fPin = 0xFF;
#if defined(ESP32)
    // Default 12 bit resolution
    int fAnalogResolution = 4096;
#else
    // Default 10 bit resolution
    int fAnalogResolution = 1024;
#endif
    float fSnapMultiplier;
    bool fSleepEnable;
    float fActivityThreshold = 4.0;
    bool fEdgeSnapEnable = true;

    float fSmoothValue;
    unsigned long fLastActivityMS;
    float fErrorEMA = 0.0;
    bool fSleeping = false;

    int fRawValue;
    int fResponsiveValue;
    int fPrevResponsiveValue;
    bool fResponsiveValueHasChanged;

    int getResponsiveValue(int newValue)
    {
        // if sleep and edge snap are enabled and the new value is very close to an edge, drag it a little closer to the edges
        // This'll make it easier to pull the output values right to the extremes without sleeping,
        // and it'll make movements right near the edge appear larger, making it easier to wake up
        if (fSleepEnable && fEdgeSnapEnable)
        {
            if (newValue < fActivityThreshold)
            {
                newValue = (newValue * 2) - fActivityThreshold;
            }
            else if (newValue > fAnalogResolution - fActivityThreshold)
            {
                newValue = (newValue * 2) - fAnalogResolution + fActivityThreshold;
            }
        }

        // get difference between new input value and current smooth value
        unsigned int diff = abs(newValue - fSmoothValue);

        // measure the difference between the new value and current value
        // and use another exponential moving average to work out what
        // the current margin of error is
        fErrorEMA += ((newValue - fSmoothValue) - fErrorEMA) * 0.4;

        // if sleep has been enabled, sleep when the amount of error is below the activity threshold
        if (fSleepEnable)
        {
            // recalculate sleeping status
            fSleeping = abs(fErrorEMA) < fActivityThreshold;
        }

        // if we're allowed to sleep, and we're sleeping
        // then don't update responsiveValue this loop
        // just output the existing responsiveValue
        if (fSleepEnable && fSleeping)
        {
            return (int)fSmoothValue;
        }
        // use a 'snap curve' function, where we pass in the diff (x) and get back a number from 0-1.
        // We want small values of x to result in an output close to zero, so when the smooth value is close to the input value
        // it'll smooth out noise aggressively by responding slowly to sudden changes.
        // We want a small increase in x to result in a much higher output value, so medium and large movements are snappy and responsive,
        // and aren't made sluggish by unnecessarily filtering out noise. A hyperbola (f(x) = 1/x) curve is used.
        // First x has an offset of 1 applied, so x = 0 now results in a value of 1 from the hyperbola function.
        // High values of x tend toward 0, but we want an output that begins at 0 and tends toward 1, so 1-y flips this up the right way.
        // Finally the result is multiplied by 2 and capped at a maximum of one, which means that at a certain point all larger movements are maximally snappy

        // then multiply the input by SNAP_MULTIPLER so input values fit the snap curve better.
        float snap = snapCurve(diff * fSnapMultiplier);

        // when sleep is enabled, the emphasis is stopping on a responsiveValue quickly, and it's less about easing into position.
        // If sleep is enabled, add a small amount to snap so it'll tend to snap into a more accurate position before sleeping starts.
        if (fSleepEnable)
        {
            snap *= 0.5 + 0.5;
        }

        // calculate the exponential moving average based on the snap
        fSmoothValue += (newValue - fSmoothValue) * snap;

        // ensure output is in bounds
        if (fSmoothValue < 0.0)
        {
            fSmoothValue = 0.0;
        }
        else if (fSmoothValue > fAnalogResolution - 1)
        {
            fSmoothValue = fAnalogResolution - 1;
        }

        // expected output is an integer
        return (int)fSmoothValue;
    }

    static inline float snapCurve(float x)
    {
        float y = 1.0 / (x + 1.0);
        y = (1.0 - y) * 2.0;
        return (y > 1.0) ? 1.0: y;
    }
};

#endif
