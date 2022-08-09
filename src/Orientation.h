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
    Orientation(uint8_t id = 0) :
        Adafruit_BNO055(55, BNO055_ADDRESS_A+id),
        fID(id)
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
            DEBUG_PRINTLN("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR");
            //Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
        }
    }

    /**
      * Returns the last recorded yaw
      */
    float getYaw()
    {
        return int(fCurrentYaw);
    }

    /**
      * Returns true if yaw has changed since last calling this function.
      */
    bool getYawChanged(float& yaw)
    {
        float prevYaw = fPreviousYaw;
        fPreviousYaw = fCurrentYaw;
        yaw = fCurrentYaw;
        return (prevYaw != fCurrentYaw);
    }

    /**
      * Returns the last recorded roll
      */
    float getRoll()
    {
        return int(fCurrentRoll);
    }

    /**
      * Returns true if roll has changed since last calling this function.
      */
    bool getRollChanged(float& roll)
    {
        float prevRoll = fPreviousRoll;
        fPreviousRoll = fCurrentRoll;
        roll = fCurrentRoll;
        return (prevRoll != fCurrentRoll);
    }

    /**
      * Returns the last recorded pitch
      */
    float getPitch()
    {
        return int(fCurrentPitch);
    }

    /**
      * Returns true if pitch has changed since last calling this function.
      */
    bool getPitchChanged(float& pitch)
    {
        float prevPitch = fPreviousPitch;
        fPreviousPitch = fCurrentPitch;
        pitch = fCurrentPitch;
        return (prevPitch != fCurrentPitch);
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
            float yaw = float(event.orientation.x);
            float roll = float(event.orientation.y);
            float pitch = float(event.orientation.z);
            if (fCurrentYaw == -1)
            {
                fPreviousYaw = fCurrentYaw = yaw;
            }
            else
            {
                fPreviousYaw = fCurrentYaw;
                fCurrentYaw = yaw;
            }
            if (fCurrentRoll == -1)
            {
                fPreviousRoll = fCurrentRoll = roll;
            }
            else
            {
                fPreviousRoll = fCurrentRoll;
                fCurrentRoll = roll;
            }
            if (fCurrentPitch == -1)
            {
                fPreviousPitch = fCurrentPitch = roll;
            }
            else
            {
                fPreviousPitch = fCurrentPitch;
                fCurrentPitch = pitch;
            }
            fLastEvent = event;
        #ifdef USE_SMQ
            if (fCurrentYaw != fPreviousYaw ||
                fCurrentRoll != fPreviousRoll ||
                fCurrentPitch != fPreviousPitch)
            {
                if (SMQ::sendTopic("Orientation"))
                {
                    SMQ::send_uint8(F("id"), fID);
                    SMQ::send_float(F("yaw"), yaw);
                    SMQ::send_float(F("roll"), roll);
                    SMQ::send_float(F("pitch"), pitch);
                    SMQ::send_end();
                }
            }
        #endif
        }
    }

private:
    bool fReady = false;
    float fPreviousYaw = -1;
    float fCurrentYaw = -1;
    float fPreviousRoll = -1;
    float fCurrentRoll = -1;
    float fPreviousPitch = -1;
    float fCurrentPitch = -1;
    uint8_t fID;
    sensors_event_t fLastEvent;
};

#endif
