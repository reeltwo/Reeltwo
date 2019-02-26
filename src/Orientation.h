#ifndef Orientation_h
#define Orientation_h

#include <Adafruit_BNO055.h>
#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"

/**
  * \ingroup Core
  *
  * \class Orientation
  *
  * \brief Encapsulates an Adafruit BNO055 IMU
  *
  * Encapsulates a Adafruit BNO055 IMU and provides convenience functions to read the heading. It will
  * also automatically publish SMQ events for any change in heading.
  */
class Orientation :
	public SetupEvent, AnimatedEvent, Adafruit_BNO055
{
public:
    /**
      * \brief Default Constructor
      */
	Orientation() :
		Adafruit_BNO055(55),
		fReady(false),
		fPreviousHeading(-1),
		fCurrentHeading(-1)
	{
	}

    /**
      * Perform any initialzation not possible in the constructor
      */
  	virtual void setup()
	{
		if ((fReady = begin()) == true)
		{
			setExtCrystalUse(true);
		}
		else
		{
			/* There was a problem detecting the BNO055 ... check your connections */
			//Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
		}
	}

	/**
	  * Returns the last recorded heading
	  */
	float getHeading()
	{
		return int(fCurrentHeading);
	}

	/**
	  * Returns true if theading has changed since last calling this function.
	  */
	bool getHeadingChanged(float& heading)
	{
		float prevHeading = fPreviousHeading;
		fPreviousHeading = fCurrentHeading;
		heading = fCurrentHeading;
		return (prevHeading != fCurrentHeading);
	}

	/**
	  * Reads the IMU and publishes any change in heading
	  */
    virtual void animate()
    {
    	if (!fReady)
    		return;
		sensors_event_t event;
		getEvent(&event);

		if (event.orientation.x != fLastEvent.orientation.x ||
			event.orientation.y != fLastEvent.orientation.y ||
			event.orientation.z != fLastEvent.orientation.z)
		{
			float heading = float(event.orientation.x);
			if (fCurrentHeading == -1)
			{
				fPreviousHeading = fCurrentHeading = heading;
			}
			else
			{
				fPreviousHeading = fCurrentHeading;
				fCurrentHeading = heading;
			}
			fLastEvent = event;
	    #ifdef USE_SMQ
			if (fCurrentHeading != fPreviousHeading)
			{
		        SMQ::send_start(F("Orientation"));
		        SMQ::send_float(F("heading"), heading);
		        SMQ::send_end();
			}
	        SMQ::send_start(F("OrientationRaw"));
	        SMQ::send_float(F("x"), event.orientation.x);
	        SMQ::send_float(F("y"), event.orientation.y);
	        SMQ::send_float(F("z"), event.orientation.z);
	        SMQ::send_end();
	    #endif
		}
    }

private:
	bool fReady;
	float fPreviousHeading;
	float fCurrentHeading;
	sensors_event_t fLastEvent;
};

#endif
