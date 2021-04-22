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
		fDistance(fDIn, fDOut, fDSet, 1.0, 0.01, 0.01),
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
		fDistance.setOutputLimits(-120, 120);

		fAngle.setAutomatic(true);
		fAngle.setSampleTime(10);
		fAngle.setOutputLimits(-800, 800);
	}

	float getThrottle()
	{
		return -fThrottle / 120.0;
	}

	float getTurning()
	{
		return -fTurning / 1600.0;
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

	void setCurrentAngle(int angle)
	{
		fASet = angle;
		fAngle.process();
		fTurning = fAOut;// * (1.0 + fabs(fThrottle) * 0.05);
	}

private:
	int fDesiredDistance;
	int fDesiredAngle;
	float fDSet;
	PID<float> fDistance;
	float fASet;
	PID<float> fAngle;
	float fDIn;
	float fDOut;
	float fAIn;
	float fAOut;
	float fThrottle;
	float fTurning;
};

#endif
