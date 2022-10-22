#ifndef Stance_h
#define Stance_h

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include "core/CommandEvent.h"
#include "Orientation.h"
#include "USBSabertooth_NB.h"

#ifndef SABER_SERIAL
 #ifdef HAVE_HWSERIAL2
  #define SABER_SERIAL Serial2
 #endif
#endif

#define MOTOR_FULL_POWER 2047
#define TILT_MOTOR_FULL_POWER 2047
#define MOTOR_HALF_3QUARTERS 1400
#define MOTOR_HALF_POWER 1024
#define MOTOR_QUARTER_POWER 300

#define YOKE_ANGLE_2LEGS_TARGET -13
#define YOKE_ANGLE_2LEGS_LIMIT -10   /* limit switch triggers at -10 */

#define YOKE_ANGLE_3LEGS_TARGET 23
#define YOKE_ANGLE_2TO3LEGS_SLOWDOWN 20

#define YOKE_ANGLE_CENTER_LEG_START -10

/**
  * Manages transition from 2 to 3 legged stance using 4 limit switches.
  */
class Stance : public AnimatedEvent, SetupEvent, CommandEvent
{
public:
    /**
      * \brief Constructor
      *
      */
    Stance( HardwareSerial& saberSerialPort,
            byte tiltUpPin = 6,
            byte tiltDownPin = 7,
            byte legUpPin = 8,
            byte legDownPin = 9,
            byte legPotPin = A0,
            byte saberAddress = 128) :
        fSaberSerialInit(saberSerialPort),
        fSaberSerial(saberSerialPort),
        fST(fSaberSerial, saberAddress),
        fTiltUpPin(tiltUpPin),
        fTiltDownPin(tiltDownPin),
        fLegUpPin(legUpPin),
        fLegDownPin(legDownPin),
        fLegPotPin(legPotPin)
    {
        for (unsigned i = 0; i < SizeOfArray(fLegPotReading); i++)
            fLegPotReading[i] = 0;
    }

    Stance( byte tiltUpPin = 6,
            byte tiltDownPin = 7,
            byte legUpPin = 8,
            byte legDownPin = 9,
            byte legPotPin = A0,
            byte saberAddress = 128) :
        Stance(SABER_SERIAL, tiltUpPin, tiltDownPin, legUpPin, legDownPin, legPotPin, saberAddress)
    {
    }

    void setOrientationSensors(Orientation& bodyIMU, Orientation& tiltIMU)
    {
        fBodyIMU = &bodyIMU;
        fTiltIMU = &tiltIMU;
    }

    virtual void handleCommand(const char* cmd) override
    {
        if (strcmp(cmd, "TWOLEGS") == 0)
        {
            changeToTwoLegged();
        }
        else if (strcmp(cmd, "THREELEGS") == 0)
        {
            changeToThreeLegged();
        }
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
        pinMode(fLegPotPin, INPUT);
        DEBUG_PRINT("fTiltUpPin   : "); DEBUG_PRINTLN(fTiltUpPin);
        DEBUG_PRINT("fTiltDownPin : "); DEBUG_PRINTLN(fTiltDownPin);
        DEBUG_PRINT("fLegUpPin    : "); DEBUG_PRINTLN(fLegUpPin);
        DEBUG_PRINT("fLegDownPin  : "); DEBUG_PRINTLN(fLegDownPin);
        DEBUG_PRINT("fLegPotPin   : "); DEBUG_PRINTLN(fLegPotPin);
        // fST.async_getBattery(1, 0);  // start cycle by requesting battery voltage with context 0
    }

    /**
      * Check status of Sabertooth and limit switches and manage transition if one has been initiated.
      */
    virtual void animate() override
    {
        // int result = 0;
        // int context = 0;
        uint32_t currentMillis = millis();

        // if (fSaberSerial.reply_available(&result, &context))
        // {
        //     switch (context)
        //     {
        //         case SABERTOOTH_GET_ERROR:      // Error or timeout
        //         case SABERTOOTH_GET_TIMED_OUT:
        //             fST.async_getBattery(1, 0);  // reset the cycle by starting from the beginning
        //             break;

        //         case 0:  // Battery
        //             fBattery = result;
        //             fST.async_getCurrent(1, 1);  // next request: get motor 1 current with context 1
        //             break;

        //         case 1: // Current 1
        //             fCurrentChannel1 = result;
        //             fST.async_getCurrent(2, 2);  // next request: get motor 2 current with context 2
        //             break;

        //         case 2: // Current 2
        //             fCurrentChannel2 = result;
        //             fST.async_getBattery( 1, 0 );  // next request: get battery voltage with context 0
        //             break;
        //     }
        //     fLastStatusMillis = currentMillis;
        // }
        // if (currentMillis - fLastStatusMillis >= kMaxStatusInterval)
        // {
        //     // Status has not been updated for kMaxStatusInterval milliseconds. Something is wrong.
        //     DEBUG_PRINTLN("Sabretooth not responding");
        // }

        fLegDn = digitalRead(fLegDownPin);
        fLegUp = digitalRead(fLegUpPin);
        fTiltDn = digitalRead(fTiltDownPin);
        fTiltUp = digitalRead(fTiltUpPin);
        // Map leg pot position value to 0..100 (top..bottom)

        // subtract the last reading:
        fLegPotTotal = fLegPotTotal - fLegPotReading[fLegPotIndex];
        // read from the sensor:
        fLegPotReading[fLegPotIndex] = analogRead(fLegPotPin);
        // add the reading to the total:
        fLegPotTotal = fLegPotTotal + fLegPotReading[fLegPotIndex++];
        if (fLegPotIndex >= SizeOfArray(fLegPotReading))
        {
            fLegPotIndex = 0;
        }
        fLegPos = map(fLegPotTotal / SizeOfArray(fLegPotReading), 0, 1023, 0, 50);
        if (fTiltDn == 0)
        {
            // when the tilt down switch opens, the timer starts
            fShowTimeThreeToTwo = 0;
        }
        if (fTiltUp == 0)
        {
            // when the tilt down switch opens, the timer starts
            fShowTimeTwoToThree = 0;
        }
        if (currentMillis - fLastDisplayMillis >= kDisplayInterval || lastMotor1 != 0 || lastMotor2 != 0 || fLegPos != fLegLastPos)
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
        // advance fShowTimeThreeToTwo every 100ms.
        if (currentMillis - fLastShowTimeMillis >= kShowTimeInterval)
        {
            fLastShowTimeMillis = currentMillis;
            fShowTimeThreeToTwo++;
            fShowTimeTwoToThree++;
        }
        fLegLastPos = fLegPos;
    }

private:
    class USBSabertoothSerialInit
    {
    public:
         USBSabertoothSerialInit(HardwareSerial& port)
         {
             // 9600 is the default baud rate for Sabertooth Packet Serial.
             port.begin(9600);
         }
    };
    USBSabertoothSerialInit fSaberSerialInit;
    USBSabertoothSerial fSaberSerial;
    USBSabertooth fST;

    byte fTiltUpPin;
    byte fTiltDownPin;
    byte fLegUpPin;
    byte fLegDownPin;
    byte fLegPotPin;

    int fTiltUp = 0;
    int fTiltDn = 0;
    int fLegUp = 0;
    int fLegDn = 0;
    int fStance = 0;
    int fStanceTarget = 0;
    int fLegHappy = 0;
    int fTiltHappy = 0;
    unsigned fLegPotIndex = 0;
    int fLegPotReading[10];
    int fLegPotTotal = 0;
    int fLegPos = 0;
    int fLegLastPos = -1;

    int fBattery = 0;
    int fCurrentChannel1 = 0;
    int fCurrentChannel2 = 0;

    Orientation* fBodyIMU = NULL;
    Orientation* fTiltIMU = NULL;

    uint32_t fLastDisplayMillis = 0; 
    uint32_t fLastStanceMillis = 0;
    uint32_t fLastShowTimeMillis = 0;
    uint32_t fLastStatusMillis = 0;
    uint32_t fShowTimeThreeToTwo = 0;
    uint32_t fShowTimeTwoToThree = 0;

    static const int kDisplayInterval = 1000;
    static const int kStanceInterval = 100;
    static const int kShowTimeInterval = 100;
    static const int kMaxStatusInterval = 5000;

    int lastMotor1 = -1;
    int lastMotor2 = -1;

    void legMotorUp()
    {
        if (lastMotor1 != -MOTOR_FULL_POWER)
        {
            DEBUG_PRINTLN("LEG MOTOR UP");
            lastMotor1 = -MOTOR_FULL_POWER;
        }
        fST.motor(1, -MOTOR_FULL_POWER);  // Go reverse at full power
    }

    void legMotorUpSlow()
    {
        if (lastMotor1 != -MOTOR_QUARTER_POWER)
        {
            DEBUG_PRINTLN("LEG MOTOR UP SLOW");
            lastMotor1 = -MOTOR_QUARTER_POWER;
        }
        fST.motor(1, -MOTOR_QUARTER_POWER);
    }

    void legMotorDown()
    {
        if (lastMotor1 != MOTOR_FULL_POWER)
        {
            DEBUG_PRINTLN("LEG MOTOR DOWN");
            lastMotor1 = MOTOR_FULL_POWER;
        }
        fST.motor(1, MOTOR_FULL_POWER);  // Go forward at full power
    }

    void legMotorDownSlow()
    {
        if (lastMotor1 != MOTOR_HALF_POWER)
        {
            DEBUG_PRINTLN("LEG MOTOR DOWN SLOW");
            lastMotor1 = MOTOR_HALF_POWER;
        }
        fST.motor(1, MOTOR_HALF_POWER);  // Go forward at three quarters impulse
    }

    void legMotorStop()
    {
        if (lastMotor1 != 0)
        {
            DEBUG_PRINTLN("LEG STOP");
            lastMotor1 = 0;
        }
        fST.motor(1, 0);     // Stop.
        fLegHappy = 0;
    }

    void tiltMotorUp()
    {
        if (lastMotor2 != -TILT_MOTOR_FULL_POWER)
        {
            DEBUG_PRINTLN("TILT MOTOR UP");
            lastMotor2 = -TILT_MOTOR_FULL_POWER;
        }
        fST.motor(2, -TILT_MOTOR_FULL_POWER);  // Go reverse at full power
    }

    void tiltMotorDown()
    {
        if (lastMotor2 != TILT_MOTOR_FULL_POWER)
        {
            DEBUG_PRINTLN("TILT MOTOR DOWN");
            lastMotor2 = TILT_MOTOR_FULL_POWER;
        }
        fST.motor(2, TILT_MOTOR_FULL_POWER);  // Go forward at full power
    }

    void tiltMotorDownSlow()
    {
        if (lastMotor2 != MOTOR_QUARTER_POWER)
        {
            DEBUG_PRINTLN("TILT MOTOR DOWN SLOW");
            lastMotor2 = MOTOR_QUARTER_POWER;
        }
        fST.motor(2, MOTOR_QUARTER_POWER);  // Go forward at quarter impulse
    }

    void tiltMotorStop()
    {
        if (lastMotor2 != 0)
        {
            DEBUG_PRINTLN("TILT STOP");
            lastMotor2 = 0;
        }
        fST.motor(2, 0);     // Stop.
        fTiltHappy = 0;
    }

    void motorStopNow()
    {
        if (lastMotor1 != 0 || lastMotor2 != 0)
        {
            DEBUG_PRINTLN("STOP NOW");
        }
        legMotorStop();
        tiltMotorStop();
    }

    /**
      * Actual movement commands are here,  when we send the command to move leg down, first it checks the leg down limit switch,
      * if it is closed it  stops the motor, sets a flag (happy) and then exits the loop, if it is open the down motor is triggered. 
      * All four work the same way
      */
    void MoveLegDn()
    {
        if (fLegDn == 0)
        {
            legMotorStop();
        }
        else if (fLegDn == 1)
        {
            legMotorDown();
        }
    } 

    void MoveLegUp()
    {
        if (fLegUp == 0)
        {
            legMotorStop();
        }
        else if (fLegUp == 1)
        {
            legMotorUp();
        }
    } 

    void MoveTiltDn()
    {
        if (fTiltDn == 0)
        {
            tiltMotorStop();
        }
        else if (fTiltDn == 1)
        {
            tiltMotorDown();
        }
    } 

    void MoveTiltUp()
    {
        if (fTiltUp == 0)
        {
            tiltMotorStop();
        }
        else if (fTiltUp == 1)
        {
            tiltMotorUp();
        }
    } 

    /**
      * This is simply taking all of the possibilities of the switch positions and giving them a number. 
      * and this loop is only allowerd to run if both my happy flags have been triggered. 
      * At any time, including power up, the droid can run a check and come up with a number as to how he is standing. 
      */
    void CheckStance()
    {
        float yokeAnkle = getYokeAngle();
        if (fLegHappy == 0 && fTiltHappy == 0)
        {
            if (fLegUp == 0 && fLegDn == 1 && fTiltUp == 0 && fTiltDn == 1)
            {
                // Two legged
                fStance = 1;
            }
            if (fLegUp == 1 && fLegDn == 0 && fTiltUp == 1 && fTiltDn == 0)
            {
                // Three legged
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
            motorStopNow();
            return;
        }
        // if you are told to go where you are, then do nothing
        if (fStanceTarget == fStance)
        {
            motorStopNow();
            return;
        }
        // Stance 7 is bad, all 4 switches open, no idea where anything is.  do nothing. 
        if (fStance == 7)
        {
            motorStopNow();
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
            motorStopNow();
            return;
        }
        // target is two legs, tilt is down, center leg is unknown,  too risky, do nothing. 
        if (fStanceTarget == 1 && fStance == 6)
        {
            motorStopNow();
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
            motorStopNow();
            return;
        }
        // target is three legs, but don't know where the center leg is.   Best to not try this, 
        // recover from stance 4 with the up command, 
        if (fStanceTarget == 2 && fStance == 4)
        {
            motorStopNow();
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
        float yokeAnkle = getYokeAngle();
        fTiltDn = digitalRead(fTiltDownPin);
        fLegDn = digitalRead(fLegDownPin);

        if (fLegDn == 0)
        {
            legMotorStop();
        }
        // Delay starting until yoke ankle is greater than YOKE_ANGLE_CENTER_LEG_START
        else if (fLegDn == 1 /*&& yokeAnkle >= YOKE_ANGLE_CENTER_LEG_START*/)
        {
            if (fLegPos < 40 && yokeAnkle < YOKE_ANGLE_3LEGS_TARGET)
                legMotorDown();
            else if (yokeAnkle < YOKE_ANGLE_3LEGS_TARGET)
                legMotorStop();
            else
                legMotorDown();
        }
        if (fTiltDn == 0 || yokeAnkle >= YOKE_ANGLE_3LEGS_TARGET)
        {
            Serial.print("STOP : yokeAnkle="); Serial.println(yokeAnkle);
            tiltMotorStop();
        }
        else if (fTiltDn == 1)
        {
            // if (yokeAnkle >= YOKE_ANGLE_2TO3LEGS_SLOWDOWN)
            // {
            //     Serial.print("HALFSPEED : yokeAnkle="); Serial.println(yokeAnkle);
            //     tiltMotorDownSlow();
            // }
            // else
            {
                tiltMotorDown();
            }
        }
    }

    /**
      * Going from three legs to two needed a slight adjustment. I start a timer, called show time, and use it to delay the 
      * center foot from retracting.
      */
    void ThreeToTwo()
    {
        float yokeAnkle = getYokeAngle();
        fTiltUp = digitalRead(fTiltUpPin);
        fLegUp = digitalRead(fLegUpPin);
        fTiltDn = digitalRead(fTiltDownPin);

        //  First if the center leg is up, do nothing. 
        // DEBUG_PRINTLN("  Transition to Two Legs");
        if (fLegUp == 0)
        {
            legMotorStop();
        }
        //  If leg up is open AND the timer is in the first 20 steps then lift the center leg at 25 percent speed
        if (fLegUp == 1 && fShowTimeThreeToTwo >= 22 && fShowTimeThreeToTwo <= 26)
        {
            legMotorUpSlow();
        }
        //  If leg up is open AND the timer is over 21 steps then lift the center leg at full speed
        if (fLegUp == 1 && fShowTimeThreeToTwo >= 27)
        {
            legMotorUp();
        }
        // at the same time, tilt up till the switch is closed
        if (fTiltUp == 0 || yokeAnkle <= YOKE_ANGLE_2LEGS_TARGET)
        {
            tiltMotorStop();
        }
        if (fTiltUp == 1)
        {
            tiltMotorUp();
        }
    }

    float getYokeAngle()
    {
        // Return yoke angle or NaN if no sensors are available
        return (fBodyIMU != NULL && fTiltIMU != NULL) ? fBodyIMU->getRoll() - fTiltIMU->getRoll() : 0.0 / 0.0;
    }

    void Display()
    {
        // DEBUG_PRINT(F("  Battery  "));
        // DEBUG_PRINT(fBattery);
        // DEBUG_PRINT(F("  CH1  "));
        // DEBUG_PRINT(fCurrentChannel1);
        // DEBUG_PRINT(F("  CH2  "));
        // DEBUG_PRINTLN(fCurrentChannel2);

        DEBUG_PRINT(F("  Tilt Up  "));
        DEBUG_PRINT(fTiltUp);
        DEBUG_PRINT(F("  Tilt Down  "));
        DEBUG_PRINT(fTiltDn); 
        DEBUG_PRINT(F("  Leg Up  "));
        DEBUG_PRINT(fLegUp);
        DEBUG_PRINT(F("  Leg Down  "));
        DEBUG_PRINT(fLegDn);
        DEBUG_PRINT(F("  Leg Pos  "));
        DEBUG_PRINT(fLegPos);

        DEBUG_PRINT(F("  Stance  "));
        DEBUG_PRINT(fStance); 
        DEBUG_PRINT(F("  Stance Target  "));
        DEBUG_PRINT(fStanceTarget); 
        DEBUG_PRINT(F(" Leg Happy  "));
        DEBUG_PRINT(fLegHappy); 
        DEBUG_PRINT(F(" Tilt Happy  "));
        DEBUG_PRINT(fTiltHappy); 
        DEBUG_PRINT(F("  Show Time  "));
        DEBUG_PRINT(fShowTimeThreeToTwo);
        DEBUG_PRINT(F(" "));
        DEBUG_PRINT(fShowTimeTwoToThree);
        if (fBodyIMU != NULL)
        {
            DEBUG_PRINT(F("  Tilt Angle  "));
            DEBUG_PRINT(getYokeAngle());
        }
        DEBUG_PRINTLN(F(""));
    }
};
#endif
