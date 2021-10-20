#include <complex>
#include <algorithm>

#define CTFFT_MAXBITS 8
#include "CTFFT.h"

#ifndef MEM_BARRIER
#define MEM_BARRIER()
#endif

////////////////////////////////////////////////////////////////////

class RealtimeAnalyser
{
public:
    static constexpr size_t kFFTSize = 256;
    static constexpr unsigned kInputBufferSize = kFFTSize * 2;
    static constexpr double kDefaultSmoothingTimeConstant = 0.8;
    static constexpr double kDefaultMinDecibels = -100;
    static constexpr double kDefaultMaxDecibels = -30;

    RealtimeAnalyser() :
        fWriteIndex(0),
        fSmoothingTimeConstant(kDefaultSmoothingTimeConstant),
        fMinDecibels(kDefaultMinDecibels),
        fMaxDecibels(kDefaultMaxDecibels)
    {
        memset(fSampleBuffer, '\0', sizeof(fSampleBuffer));
        memset(fMagnitudeBuffer, '\0', sizeof(fMagnitudeBuffer));
    }

    // sampleCount must be power of two
    void writeInput(const int16_t* samples, size_t sampleCount)
    {
        // convert int16_t data to float
        float* dst = &fSampleBuffer[fWriteIndex];
        for (size_t i = 0; i < sampleCount; i++)
        {
            dst[i] = ((float)samples[i*2]) / 32768.0f;
        }

        fWriteIndex += sampleCount;
        if (fWriteIndex >= kInputBufferSize)
            fWriteIndex = 0;
    }

    bool ready()
    {
        return (fWriteIndex > 0 && (fWriteIndex % kFFTSize) == 0);
    }

    unsigned getFrequencyBinCount() const
    {
        return kFFTSize / 2;
    }

    void getByteFrequencyData(uint8_t* destArray, size_t destArraySize)
    {
        if (!destArray)
            return;

        doFFTAnalysis();

        // Convert from linear magnitude to unsigned-byte decibels.
        size_t len = std::min(magnitudeBufferLength(), destArraySize);
        if (len > 0)
        {
            const double minDecibels = fMinDecibels;
            const double rangeScaleFactor = (fMaxDecibels != minDecibels) ? 1 / (fMaxDecibels - minDecibels) : 1;

            const float* source = fMagnitudeBuffer;
            for (unsigned i = 0; i < len; i++)
            {
                float linearValue = source[i];
                double dbMag = (linearValue != 0) ? linearToDecibels(linearValue) : minDecibels;

                // The range m_minDecibels to m_maxDecibels will be scaled to byte values from 0 to UCHAR_MAX.
                double scaledValue = UCHAR_MAX * (dbMag - minDecibels) * rangeScaleFactor;

                // Clip to valid range.
                if (scaledValue < 0)
                    scaledValue = 0;
                if (scaledValue > UCHAR_MAX)
                    scaledValue = UCHAR_MAX;

                destArray[i] = (uint8_t)scaledValue;
            }
        }
    }

private:
    float fSampleBuffer[kInputBufferSize];
    float fMagnitudeBuffer[kFFTSize];
    unsigned fWriteIndex;
    double fSmoothingTimeConstant;
    double fMinDecibels;
    double fMaxDecibels;
    CTFFT::RealDiscrete<float, kFFTSize> fAnalysisFrame;

    size_t sampleBufferLength() const
    {
        return sizeof(fSampleBuffer) / sizeof(fSampleBuffer[0]);
    }

    size_t magnitudeBufferLength() const
    {
        return sizeof(fMagnitudeBuffer) / sizeof(fMagnitudeBuffer[0]);
    }

    // linearSample must be non-zero
    static float linearToDecibels(float linearSample)
    {
        return 20 * log10f(linearSample);
    }

    void applyWindow(float* p, size_t n)
    {
        static constexpr double twoPiDouble = M_PI * 2.0;

        // Blackman window
        double alpha = 0.16;
        double a0 = 0.5 * (1 - alpha);
        double a1 = 0.5;
        double a2 = 0.5 * alpha;

        for (unsigned i = 0; i < n; ++i)
        {
            double x = (double)i / (double)n;
            double window = a0 - a1 * cos(twoPiDouble * x) + a2 * cos(twoPiDouble * 2.0 * x);
            p[i] *= double(window);
        }
    }

    void doFFTAnalysis()
    {
        float tempBuffer[kFFTSize];
        float* inputBuffer = fSampleBuffer;
        float* tempP = tempBuffer;

        // Take the previous fftSize values from the input buffer and copy into the temporary buffer.
        unsigned writeIndex = fWriteIndex;
        if (writeIndex < kFFTSize)
        {
            memcpy(tempP, inputBuffer + writeIndex - kFFTSize + kInputBufferSize, sizeof(*tempP) * (kFFTSize - writeIndex));
            memcpy(tempP + kFFTSize - writeIndex, inputBuffer, sizeof(*tempP) * writeIndex);
        }
        else
        {
            memcpy(tempP, inputBuffer + writeIndex - kFFTSize, sizeof(*tempP) * kFFTSize);
        }

        // Window the input samples.
        applyWindow(tempP, kFFTSize);

        // Do the analysis.
        fAnalysisFrame.calculate(tempP);

        // Blow away the packed nyquist component.
        tempP[1] = 0;

        // Normalize so that an input sine wave at 0dBfs registers as 0dBfs (undo FFT scaling factor).
        const double magnitudeScale = 1.0 / kFFTSize;

        // A value of 0 does no averaging with the previous result.  Larger values produce slower, but smoother changes.
        double k = fSmoothingTimeConstant;
        k = std::max(0.0, k);
        k = std::min(1.0, k);

        // Convert the analysis data from complex to magnitude and average with the previous result.
        float* destination = fMagnitudeBuffer;
        size_t n = magnitudeBufferLength();
        for (size_t i = 0; i < n; ++i)
        {
            std::complex<double> c(tempP[i*2], tempP[i*2+1]);
            double scalarMagnitude = abs(c) * magnitudeScale;
            destination[i] = float(k * destination[i] + (1 - k) * scalarMagnitude);
        }
    }
};

////////////////////////////////////////////////////////////////////

class AudioFrequencyBitmap
{
public:
    AudioFrequencyBitmap()
    {
        if (pixels != nullptr)
            memset(pixels, '\0', width * height);
    }

    void processSamples(unsigned numBits, unsigned numChannels, const int16_t* samples, int sampleCount)
    {
        // static bool printed;
        // if (!printed)
        // {
        //     Serial.println("numBits:     "+String(numBits));
        //     Serial.println("numChannels: "+String(numChannels));
        //     Serial.println("sampleCount: "+String(sampleCount));
        //     printed = true;
        // }
        // // Ignore frames that are not 640 samples
        if (sampleCount == 1024)
        {
            while (sampleCount > 0)
            {
                analyser.writeInput(samples, 256);
                samples += 256;
                sampleCount -= 256;
            }
        }
        else if (sampleCount == 256)
        {
            analyser.writeInput(samples, 256);
        }
        else if (sampleCount == 640)
        {
            // Break 640 samples into 128 samples and apply to analyser
            while (sampleCount > 0)
            {
                analyser.writeInput(samples, 128);
                samples += 128;
                sampleCount -= 128;
            }
        }
        else if (sampleCount == 1152)
        {
            // Break 640 samples into 128 samples and apply to analyser
            while (sampleCount > 0)
            {
                analyser.writeInput(samples, 8);
                samples += 8;
                sampleCount -= 8;
            }
        }
        else if (sampleCount == 0)
        {
            sampleCount = 1024;
            while (sampleCount > 0)
            {
                analyser.writeInput(samples, 256);
                samples += 256;
                sampleCount -= 256;
            }
        }
        else
        {
            Serial.println(sampleCount);
        }
        if (analyser.ready())
        {
            analyser.getByteFrequencyData(freq, width);
            memmove(&pixels[width], pixels, width * (4*60-1));
            memcpy(pixels, freq, width);
            updated = true;
            MEM_BARRIER();
        }
    }

    uint8_t* getPixels() { return pixels; }
    unsigned getWidth() { return width; }
    unsigned getHeight() { return height; }
    uint8_t get(float x, float y)
    {
        return (x >= 0.0f && x <= 1.0f && y >= 0.0f && y <= 1.0f) ? pixels[unsigned((3*y) * width + (x*width))] : 0;
    }
    uint8_t get(unsigned x, unsigned y)
    {
        return (x < width && y < height) ? pixels[y * width + x] : 0;
    }
    bool isUpdated()
    {
        if (updated)
        {
            updated = false;
            MEM_BARRIER();
            return true;
        }
        return false;
    }

private:
    RealtimeAnalyser analyser;
    unsigned width = analyser.getFrequencyBinCount();
    unsigned height = 4*60;
    bool updated = false;
    uint8_t* pixels = (uint8_t*)malloc(width * height);
    uint8_t* freq = new uint8_t[width];
};
