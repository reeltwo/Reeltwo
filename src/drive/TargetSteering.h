#ifndef TargetSteering_h
#define TargetSteering_h

#include "PID.h"

class TargetSteering
{
public:
    TargetSteering(int desiredDistance, int desiredAngle = 0) :
        fDesiredDistance(desiredDistance),
        fDesiredAngle(desiredAngle),
        fDSet(desiredDistance),
        fDistance(fDIn, fDOut, fDSet, 1.0, 0.1, 0.1),
        fASet(desiredAngle),
        fAngle(fAIn, fAOut, fASet, 0.5, 0.05, 0.02),
        fDIn(desiredDistance),
        fDOut(0),
        fAIn(desiredAngle),
        fAOut(0),
        fThrottle(0),
        fTurning(0)
    {
        fDistance.setAutomatic(true);
        fDistance.setSampleTime(10);
        setDistanceOutputLimits(190);

        fAngle.setAutomatic(true);
        fAngle.setSampleTime(10);
        setAngleOutputLimits(800);
    }

    double getThrottle()
    {
        return -fThrottle / fDistance.getOutputMax();
    }

    double getTurning()
    {
        return -fTurning / fAngle.getOutputMax()*2;
    }

    void lost()
    {
        fThrottle *= 0.95;
        fTurning *= 0.8;
    }

    void stop()
    {
        setCurrentDistance(fDesiredDistance);
        setCurrentAngle(fDesiredAngle);
    }

    void setCurrentDistance(int distance)
    {
        fDSet = distance;
        fDistance.process();
        fThrottle = fDOut;
    }

    void setSampleTime(unsigned sampleTime)
    {
        fDistance.setSampleTime(sampleTime);
        fAngle.setSampleTime(sampleTime);
    }

    void setCurrentAngle(int angle)
    {
        fASet = angle;
        fAngle.process();
        fTurning = fAOut;// * (1.0 + fabs(fThrottle) * 0.05);
    }

    void setAngleOutputLimits(double limit)
    {
        setAngleOutputLimits(-limit, limit);
    }

    void setAngleOutputLimits(double outputMin, double outputMax)
    {
        fAngle.setOutputLimits(outputMin, outputMax);
    }

    void setDistanceTunings(double Kp, double Ki, double Kd)
    {
        fDistance.setTunings(Kp, Ki, Kd);
    }

    void setDistanceOutputLimits(double limit)
    {
        setDistanceOutputLimits(-limit, limit);
    }

    void setDistanceOutputLimits(double outputMin, double outputMax)
    {
        fDistance.setOutputLimits(outputMin, outputMax);
    }

private:
    int fDesiredDistance;
    int fDesiredAngle;
    double fDSet;
    PID<double> fDistance;
    double fASet;
    PID<double> fAngle;
    double fDIn;
    double fDOut;
    double fAIn;
    double fAOut;
    double fThrottle;
    double fTurning;
};

#endif
