#ifndef PID_h
#define PID_h

#include "ReelTwo.h"

/**
  * \ingroup Drive
  *
  * \class PID
  *
  * \brief 
  */
template<typename T> class PID
{
public:
    enum Direction
    {
        kDirect,
        kReverse
    };
    enum {
        kManual = 0,
        kAuto = 1,
    };

    /** \brief Constructor
      *
      * Creates a PID controller linked to the specified Input, Output, and Setpoint.
      * The PID algorithm is in parallel form.
      */
    PID(    T& input,
            T& output,
            T& setpoint,
            T kp,
            T ki,
            T kd,
            Direction direction = kDirect,
            bool proportialOnError = true) :
        fAuto(false),
        fInput(input),
        fOutput(output),
        fSetpoint(setpoint),
        fSampleTime(100)
    {
        setOutputLimits(0, 255);

        setDirection(direction);
        setTunings(kp, ki, kd, proportialOnError);

        fLastTime = millis() - fSampleTime;
    }

    bool process()
    {
        uint32_t now = millis();
        uint32_t diff = (now - fLastTime);
        if (fAuto && diff >= fSampleTime)
        {
            /* Compute all the working error variables */
            T input = fInput;
            T error = fSetpoint - input;
            T dInput = (input - fLastInput);
            fOutputSum += (fKi * error);

            /* Add Proportional on Measurement if specified */
            if (getProportialOnMeasurement())
                fOutputSum-= fKp * dInput;
            if (fOutputSum > fOutMax)
                fOutputSum = fOutMax;
            else if (fOutputSum < fOutMin)
                fOutputSum = fOutMin;

            T output = (getProportialOnError()) ? fKp * error : 0;
            output += fOutputSum - fKd * dInput;

            if (output > fOutMax)
                output = fOutMax;
            else if (output < fOutMin)
                output = fOutMin;
            fOutput = output;

            fLastInput = input;
            fLastTime = now;
            return true;
        }
        return false;        
    }

    void setAutomatic(bool automatic)
    {
        if (automatic && !fAuto)
            init();
        fAuto = automatic;        
    }

    inline T getOutputMin() const
    {
        return fOutMin;
    }

    inline T getOutputMax() const
    {
        return fOutMax;
    }

    void setOutputLimits(T outputMin, T outputMax)
    {
        fOutMin = outputMin;
        fOutMax = outputMax;

        if (fAuto)
        {
            if (fOutput > fOutMax)
                fOutput = fOutMax;
            else if (fOutput < fOutMin)
                fOutput = fOutMin;

            if (fOutputSum > fOutMax)
                fOutputSum = fOutMax;
            else if (fOutputSum < fOutMin)
                fOutputSum = fOutMin;
        }
    }

    void setTunings(T Kp, T Ki, T Kd, bool pOnError)
    {
        if (Kp < 0 || Ki < 0 || Kd < 0)
            return;

        fPOnError = pOnError;

        fKpOrg = Kp;
        fKiOrg = Ki;
        fKdOrg = Kd;

        T sampleTimeInSec = ((T)fSampleTime) / 1000;
        fKp = Kp;
        fKi = Ki * sampleTimeInSec;
        fKd = Kd / sampleTimeInSec;

        if (fDirection == kReverse)
        {
            fKp = (0 - fKp);
            fKi = (0 - fKi);
            fKd = (0 - fKd);
        }
    }

    void setTunings(T Kp, T Ki, T Kd)
    {
        setTunings(Kp, Ki, Kd, fPOnError);
    }

    void setDirection(Direction direction)
    {
        if (fAuto && fDirection != direction)
        {
            fKp = (0 - fKp);
            fKi = (0 - fKi);
            fKd = (0 - fKd);
        }
        fDirection = direction;
    }

    void setSampleTime(unsigned sampleTime)
    {
        T ratio = (T)sampleTime / (T)fSampleTime;
        fKi *= ratio;
        fKd /= ratio;
        fSampleTime = (unsigned long)sampleTime;
    }

    inline bool getProportialOnMeasurement() const { return !fPOnError; }
    inline bool getProportialOnError() const       { return fPOnError;  }

    inline T getKp() const                   { return fKpOrg;     }
    inline T getKi() const                   { return fKiOrg;     }
    inline T getKd() const                   { return fKdOrg;     }
    inline int getAutomatic() const          { return fAuto;      }
    inline int getDirection() const          { return fDirection; }

private:
    T fKpOrg;
    T fKiOrg;
    T fKdOrg;

    T fKp;
    T fKi;
    T fKd;

    bool fAuto;
    bool fPOnError;
    Direction fDirection;

    T& fInput;
    T& fOutput;
    T& fSetpoint;

    uint32_t fLastTime;
    T fOutputSum;
    T fLastInput;

    uint32_t fSampleTime;
    T fOutMin;
    T fOutMax;

    void init()
    {
        fOutputSum = fOutput;
        fLastInput = fInput;
        if (fOutputSum > fOutMax)
            fOutputSum = fOutMax;
        else if (fOutputSum < fOutMin)
            fOutputSum = fOutMin;
    }
};
#endif
