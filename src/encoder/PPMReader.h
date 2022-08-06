/*
Copyright 2016 Aapo Nikkilä
Copyright 2021 Dmitry Grigoryev

This file is part of PPM Reader.

PPM Reader is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

PPM Reader is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with PPM Reader.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PPM_READER
#define PPM_READER

#include "ReelTwo.h"

#ifdef ESP32
class PPMReader {

    public:
    
    // The range of a channel's possible values (microseconds)
    unsigned minChannelValue = 1000;
    unsigned maxChannelValue = 2000;

    // The maximum error (in either direction) in channel value with which the channel value is still considered valid
    unsigned channelValueMaxError = 10;

    // The minimum blanking time (microseconds) after which the frame current is considered to be finished
	// Should be bigger than maxChannelValue + channelValueMaxError
    unsigned blankTime = 2100;
	
	// The timeout (microseconds) after which the channels which were not updated are considered invalid
	unsigned long failsafeTimeout = 500000L;


    private:

    // The pin from which to listen for interrupts
    byte interruptPin = 0;

    // The number of channels to be expected in the PPM signal
    byte channelAmount = 0;

    // Arrays for keeping track of channel values
    volatile unsigned *rawValues = NULL;

    // A counter variable for determining which channel is being read next
    volatile byte pulseCounter = 0;

    // A time variable to remember when the last pulse was read
    volatile unsigned long microsAtLastPulse = 0;

    // Pointer to PPMReader object used by ISR. Replace by an array if multiple PPM reader instances are needed
    static PPMReader *ppm;

    public:

    PPMReader(byte interruptPin, byte channelAmount);
    ~PPMReader(void);

    // Returns the latest raw (not necessarily valid) value for the channel (starting from 1)
    unsigned rawChannelValue(byte channel);

    // Returns the latest received value that was considered valid for the channel (starting from 1)
    // Returns defaultValue if the channel hasn't received any valid values yet, or the PPM signal was absent for more than failsafeTimeout
    unsigned latestValidChannelValue(byte channel, unsigned defaultValue);

    private:

    // An interrupt service routine for handling the interrupts activated by PPM pulses
    void handleInterrupt(void);

    // Interrupt service routine function compatible with attachInterrupt. Add more funcitons if multiple PPM reader instances are needed
    static void PPM_ISR(void);

};
#endif

#endif
