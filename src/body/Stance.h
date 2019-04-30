#ifndef Stance_h
#define Stance_h

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include "USBSabertooth_NB.h"

#ifndef SABER_SERIAL
 #ifdef HAVE_HWSERIAL2
  #define SABER_SERIAL Serial2
 #endif
#endif

/**
  * Manages transition from 2 to 3 legged stance using 4 limit switches.
  */
class Stance : public AnimatedEvent, SetupEvent
{
public:
    /**
      * \brief Constructor
      *
      */
    Stance( Stream& saberSerialPort,
            byte tiltUpPin = 6,
            byte tiltDownPin = 7,
            byte legUpPin = 8,
            byte legDownPin = 9,
            byte saberAddress = 128) :
        fSaberSerial(saberSerialPort),
        fST(fSaberSerial, saberAddress),
        fTiltUpPin(tiltUpPin),
        fTiltDownPin(tiltDownPin),
        fLegUpPin(legUpPin),
        fLegDownPin(legDownPin)
    {
    }

    Stance( byte tiltUpPin = 6,
            byte tiltDownPin = 7,
            byte legUpPin = 8,
            byte legDownPin = 9,
            byte saberAddress = 128) :
        Stance(SABER_SERIAL, tiltUpPin, tiltDownPin, legUpPin, legDownPin, saberAddress)
    {
        // 9600 is the default baud rate for Sabertooth Packet Serial.
        SABER_SERIAL.begin(9600);
    }

    /**
      * Returns true if the current stance is two legged.
      */
    inline bool isTwoLegged()
    {
        return (fLegHappy == 0 && fTiltHappy == 0 && fStance == 1);
    }

    /**
      * Returns true if the current stance is three legged.
      */
    inline bool isThreeLegged()
    {
        return (fLegHappy == 0 && fTiltHappy == 0 && fStance == 2);
    }

    /**
      * Returns true if the current stance is either two legged or three legged.
      */
    inline bool isValid()
    {
        return isTwoLegged() || isThreeLegged();
    }

    /**
      * Initiate a transition to two legged mode. Does nothing if stance is invalid or already two legged.
      */
    void changeToTwoLegged()
    {
        fStanceTarget = 1;
    }

    /**
      * Initiate a transition to three legged mode. Does nothing if stance is invalid or already three legged.
      */
    void changeToThreeLegged()
    {
        fStanceTarget = 2;
    }

    /**
      * Perform any initialzation not possible in the constructor
      */
    virtual void setup() override
    {
        pinMode(fTiltUpPin, INPUT_PULLUP);
        pinMode(fTiltDownPin, INPUT_PULLUP);
        pinMode(fLegUpPin, INPUT_PULLUP);
        pinMode(fLegDownPin, INPUT_PULLUP);  
        fST.async_getBattery(1, 0);  // start cycle by requesting battery voltage with context 0
    }

    /**
      * Check status of Sabertooth and limit switches and manage transition if one has been initiated.
      */
    virtual void animate() override
    {
        int result = 0;
        int context = 0;
        uint32_t currentMillis = millis();

        if (fSaberSerial.reply_available(&result, &context))
        {
            switch (context)
            {
                case SABERTOOTH_GET_ERROR:      // Error or timeout
                case SABERTOOTH_GET_TIMED_OUT:  
                    fST.async_getBattery(1, 0);  // reset the cycle by starting from the beginning
                    break;

                case 0:  // Battery  
                    fBattery = result;
                    fST.async_getCurrent(1, 1);  // next request: get motor 1 current with context 1
                    break;

                case 1: // Current 1
                    fCurrentChannel1 = result;
                    fST.async_getCurrent(2, 2);  // next request: get motor 2 current with context 2
                    break;

                case 2: // Current 2
                    fCurrentChannel2 = result;
                    fST.async_getBattery( 1, 0 );  // next request: get battery voltage with context 0
                    break;
            }
            fLastStatusMillis = currentMillis;
        }
        if (currentMillis - fLastStatusMillis >= kMaxStatusInterval)
        {
            // Status has not been updated for kMaxStatusInterval milliseconds. Something is wrong.
            DEBUG_PRINTLN("Sabretooth not responding");
        }

        fLegDn = digitalRead(fLegDownPin);
        fLegUp = digitalRead(fLegUpPin);
        fTiltDn = digitalRead(fTiltDownPin);
        fTiltUp = digitalRead(fTiltUpPin);
        if (fTiltDn == 0)
        {
            // when the tilt down switch opens, the timer starts
            fShowTime = 0;
        }
        if (currentMillis - fLastDisplayMillis >= kDisplayInterval)
        {
            fLastDisplayMillis = currentMillis;
            Display();
        }
        if (currentMillis - fLastStanceMillis >= kStanceInterval)
        {
            fLastStanceMillis = currentMillis;
            CheckStance();
        }
        Move();
        // advance fShowTime every 100ms.
        if (currentMillis - fLastShowTimeMillis >= kShowTimeInterval)
        {
            fLastShowTimeMillis = currentMillis;
            fShowTime++;
        }
    }

private:
    USBSabertoothSerial fSaberSerial;
    USBSabertooth fST;

    byte fTiltUpPin;
    byte fTiltDownPin;
    byte fLegUpPin;
    byte fLegDownPin;

    int fTiltUp = 0;
    int fTiltDn = 0;
    int fLegUp = 0;
    int fLegDn = 0;
    int fStance = 0;
    int fStanceTarget = 0;
    int fLegHappy = 0;
    int fTiltHappy = 0;

    int fBattery = 0;
    int fCurrentChannel1 = 0;
    int fCurrentChannel2 = 0;

    uint32_t fLastDisplayMillis = 0; 
    uint32_t fLastStanceMillis = 0;
    uint32_t fLastShowTimeMillis = 0;
    uint32_t fLastStatusMillis = 0;
    uint32_t fShowTime = 1;

    static const int kDisplayInterval = 1000;
    static const int kStanceInterval = 100;
    static const int kShowTimeInterval = 100;
    static const int kMaxStatusInterval = 5000;

    /**
      * Actual movement commands are here,  when we send the command to move leg down, first it checks the leg down limit switch,
      * if it is closed it  stops the motor, sets a flag (happy) and then exits the loop, if it is open the down motor is triggered. 
      * All four work the same way
      */
    void MoveLegDn()
    {
        if (fLegDn == 0)
        {
            fST.motor(1, 0);     // Stop. 
            fLegHappy = 0;
        }
        else if (fLegDn == 1)
        {
            fST.motor(1, 2047);  // Go forward at full power. 
        }
    } 

    void MoveLegUp()
    {
        if (fLegUp == 0)
        {
            fST.motor(1, 0);     // Stop. 
            fLegHappy = 0;
        }
        else if (fLegUp == 1)
        {
            fST.motor(1, -2047);  // Go forward at full power. 
        }
    } 

    void MoveTiltDn()
    {
        if (fTiltDn == 0)
        {
            fST.motor(2, 0);     // Stop. 
            fTiltHappy = 0;
        }
        else if (fTiltDn == 1)
        {
            fST.motor(2, 2047);  // Go forward at full power. 
        }
    } 

    void MoveTiltUp()
    {
        if (fTiltUp == 0)
        {
            fST.motor(2, 0);     // Stop. 
            fTiltHappy = 0;
        }
        else if (fTiltUp == 1)
        {
            fST.motor(2, -2047);  // Go forward at full power. 
        }
    } 

    /**
      * This is simply taking all of the possibilities of the switch positions and giving them a number. 
      * and this loop is only allowerd to run if both my happy flags have been triggered. 
      * At any time, including power up, the droid can run a check and come up with a number as to how he is standing. 
      */
    void CheckStance()
    {
        if (fLegHappy == 0 && fTiltHappy == 0)
        {
            if (fLegUp == 0 && fLegDn == 1 && fTiltUp == 0 && fTiltDn == 1)
            {
                fStance = 1;
            }
            if (fLegUp == 1 && fLegDn == 0 && fTiltUp == 1 && fTiltDn == 0)
            {
                fStance = 2;
            }
            if (fLegUp == 0 && fLegDn == 1 && fTiltUp == 1 && fTiltDn == 1)
            {
                fStance = 3;
            }
            if (fLegUp == 1 && fLegDn == 1 && fTiltUp == 0 && fTiltDn == 1)
            {
                fStance = 4;
            }
            if (fLegUp == 1 && fLegDn == 0 && fTiltUp == 0 && fTiltDn == 1)
            {
                fStance = 4;
            }
            if (fLegUp == 1 && fLegDn == 0 && fTiltUp == 1 && fTiltDn == 1)
            {
                fStance = 5;
            }
            if (fLegUp == 1 && fLegDn == 1 && fTiltUp == 1 && fTiltDn == 0)
            {
                fStance = 6;
            }
            if (fLegUp == 1 && fLegDn == 1 && fTiltUp == 1 && fTiltDn == 1)
            {
                fStance = 7;
            }
        }
    }

    void Move()
    {
        // there is no stance target 0, so turn off your motors and do nothing. 
        if (fStanceTarget == 0)
        {
            fST.motor(1, 0);
            fST.motor(2, 0);
            fLegHappy = 0;
            fTiltHappy = 0;
            return;
        }
        // if you are told to go where you are, then do nothing
        if (fStanceTarget == fStance)
        {
            fST.motor(1, 0);
            fST.motor(2, 0);
            fLegHappy = 0;
            fTiltHappy = 0;
            return;
        }
        // Stance 7 is bad, all 4 switches open, no idea where anything is.  do nothing. 
        if (fStance == 7)
        {
            fST.motor(1, 0);
            fST.motor(2, 0);
            fLegHappy = 0;
            fTiltHappy = 0;
            return;
        }
        // if you are in three legs and told to go to 2
        if (fStanceTarget == 1 && fStance == 2)
        {
            fLegHappy = 1;
            fTiltHappy = 1;
            ThreeToTwo();
        }
        // This is the first of the slight unknowns, target is two legs,  look up to stance 3, the center leg is up, but the tilt is unknown.
        //You are either standing on two legs, or already in a pile on the ground. Cant hurt to try tilting up. 
        if (fStanceTarget == 1 && fStance == 3)
        {
            fTiltHappy = 1;
            MoveTiltUp();
        }
        // Target two legs, tilt is up, center leg unknown, Can not hurt to try and lift the leg again. 
        if (fStanceTarget == 1 && fStance == 4)
        {
            fLegHappy = 1;
            MoveLegUp();
        }
        // Target is two legs, center foot is down, tilt is unknown, too risky do nothing.  
        if (fStanceTarget == 1 && fStance == 5)
        {
            fST.motor(1, 0);
            fST.motor(2, 0);
            fLegHappy = 0;
            fTiltHappy = 0;
            return;
        }
        // target is two legs, tilt is down, center leg is unknown,  too risky, do nothing. 
        if (fStanceTarget == 1 && fStance == 6)
        {
            fST.motor(1, 0);
            fST.motor(2, 0);
            fLegHappy = 0;
            fTiltHappy = 0;
            return;
        } 
        // target is three legs, stance is two legs, run two to three. 
        if (fStanceTarget == 2 && fStance == 1)
        {
            fLegHappy = 1;
            fTiltHappy = 1;
            TwoToThree();
        }
        //Target is three legs. center leg is up, tilt is unknown, safer to do nothing, Recover from stance 3 with the up command
        if (fStanceTarget == 2 && fStance == 3)
        {
            fST.motor(1, 0);
            fST.motor(2, 0);
            fLegHappy = 0;
            fTiltHappy = 0;
            return;
        }
        // target is three legs, but don't know where the center leg is.   Best to not try this, 
        // recover from stance 4 with the up command, 
        if (fStanceTarget == 2 && fStance == 4)
        {
            fST.motor(1, 0);
            fST.motor(2, 0);
            fLegHappy = 0;
            fTiltHappy = 0;
            return;
        }
        // Target is three legs, the center foot is down, tilt is unknownm. either on 3 legs now, or a smoking mess, 
        // nothing to loose in trying to tilt down again
        if (fStanceTarget == 2 && fStance == 5)
        {
            fTiltHappy = 1;
            MoveTiltDn();
        }
        // kinda like above, Target is 3 legs, tilt is down, center leg is unknown, ......got nothing to loose. 
        if (fStanceTarget == 2 && fStance == 6)
        {
            fLegHappy = 1;
            MoveLegDn();
        }
    }

    /**
      * this command to go from two to three, ended up being a combo of tilt down and leg down 
      * with a last second chech each loop on the limit switches
      * timing worked out great, by the time the tilt down needed a center foot, it was there.
      */
    void TwoToThree()
    {
        fTiltDn = digitalRead(fTiltDownPin);
        fLegDn = digitalRead(fLegDownPin);

        DEBUG_PRINTLN("  Transition to Three Legs");
        if (fLegDn == 0)
        {
            fST.motor(1, 0);
            fLegHappy = 0;
        }
        if (fLegDn == 1)
        {
            fST.motor(1, 2047);  // Go forward at full power. 
        }
        if (fTiltDn == 0)
        {
            fST.motor(2, 0);
            fTiltHappy = 0;
        }
        if (fTiltDn == 1)
        {
            fST.motor(2, 2047);  // Go forward at full power. 
        }
    }

    /**
      * Going from three legs to two needed a slight adjustment. I start a timer, called show time, and use it to delay the 
      * center foot from retracting.
      */
    void ThreeToTwo()
    {
        fTiltUp = digitalRead(fTiltUpPin);
        fLegUp = digitalRead(fLegUpPin);
        fTiltDn = digitalRead(fTiltDownPin);

        //  First if the center leg is up, do nothing. 
        DEBUG_PRINTLN("  Transition to Two Legs");
        if (fLegUp == 0)
        {
            fST.motor(1, 0);
            fLegHappy = 0; 
        }
        //  If leg up is open AND the timer is in the first 20 steps then lift the center leg at 25 percent speed
        if (fLegUp == 1 && fShowTime >= 1 && fShowTime <= 20)
        {
            fST.motor(1, -500); 
        }
        //  If leg up is open AND the timer is over 21 steps then lift the center leg at full speed
        if (fLegUp == 1 && fShowTime >= 21)
        {
            fST.motor(1, -2047); 
        }
        // at the same time, tilt up till the switch is closed
        if (fTiltUp == 0)
        {
            fST.motor(2, 0);
            fTiltHappy = 0; 
        }
        if (fTiltUp == 1)
        {
            fST.motor(2, -2047);  // Go forward at full power. 
        }
    }

    void Display()
    {
        DEBUG_PRINTF("  Battery  ");
        DEBUG_PRINT(fBattery);
        DEBUG_PRINTF("  CH1  ");
        DEBUG_PRINT(fCurrentChannel1);
        DEBUG_PRINTF("  CH2  ");
        DEBUG_PRINTLN(fCurrentChannel2);

        DEBUG_PRINTF("  Tilt Up  ");
        DEBUG_PRINT(fTiltUp);
        DEBUG_PRINTF("  Tilt Down  ");
        DEBUG_PRINT(fTiltDn); 
        DEBUG_PRINTF("  Leg Up  ");
        DEBUG_PRINT(fLegUp);
        DEBUG_PRINTF("  Leg Down  ");
        DEBUG_PRINTLN(fLegDn); 

        DEBUG_PRINTF("  Stance  ");
        DEBUG_PRINT(fStance); 
        DEBUG_PRINTF("  Stance Target  ");
        DEBUG_PRINT(fStanceTarget); 
        DEBUG_PRINTF(" Leg Happy  ");
        DEBUG_PRINT(fLegHappy); 
        DEBUG_PRINTF(" Tilt Happy  ");
        DEBUG_PRINT(fTiltHappy); 
        DEBUG_PRINTF("  Show Time  ");
        DEBUG_PRINTLN(fShowTime); 
    }
};
#endif
