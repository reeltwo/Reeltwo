#ifndef ButtonController_h
#define ButtonController_h

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"

#ifdef LedControlMAX7221_h
#define DBC_TEECES_D_PIN    6
#define DBC_TEECES_C_PIN    7
#define DBC_TEECES_L_PIN    8
#endif

#define DBC_PIN_DISABLED            255
#define DBC_PIN_MAIN_POWER_RELAY    16
#define DBC_POWER_DOWN_DELAY        2000    // milliseconds time between Power Down Sound Trigger and Turning OFF Main Power Relay
#define DBC_SPEED                   1

#define DBC_LEFT_BUTTON_MAX         4     //4; A, B, C, D
#define DBC_RIGHT_BUTTON_MAX        4     //4; 1, 2, 3  // this will be increased to 4 to allow the full 16 outputs (A1 to D4)

// GENERAL CONSTANT VARIABLES
#define ON              1
#define OFF             0

// #define HIGH            1
// #define LOW             0

// REAR PSI LIGHTING MODES
#define PSI_YELLOW      1
#define PSI_GREEN       2
#define PSI_BOTH        3
#define PSI_ALTERNATE   4
#define PSI_RANDOM      -1

static const byte DBC_PIN_MAP[] PROGMEM = {
    DBC_PIN_DISABLED,   /* Dummy pin 0 */
    12,                 /* Pin 1 */
    13,                 /* Pin 2 */
#ifdef USE_SMQ
    DBC_PIN_DISABLED,   /* Pin 3 - disabled used for RX Serial */
    DBC_PIN_DISABLED,   /* Pin 4 - disabled used for TX Serial */
#else
    1,                  /* Pin 3 */
    0,                  /* Pin 4 */
#endif
    A3,                 /* Pin 5 */
    A2,                 /* Pin 6 */
    A1,                 /* Pin 7 */
    A0,                 /* Pin 8 */
    11,                 /* Pin 9 */
    10,                 /* Pin 10 */
    9,                  /* Pin 11 */
#ifdef DBC_TEECES_L_PIN
    DBC_PIN_DISABLED,   /* Pin 12 - disabled */
#else
    8,                  /* Pin 12 */
#endif
#ifdef DBC_TEECES_D_PIN
    DBC_PIN_DISABLED,   /* Pin 13 - disabled */
#else
    7,                  /* Pin 13 */
#endif
#ifdef DBC_TEECES_D_PIN
    DBC_PIN_DISABLED,   /* Pin 14 - disabled */
#else
    6,                  /* Pin 14 */
#endif
    5,                  /* Pin 15 */
    4                   /* Pin 16 - power relay */
};
#define DBC_PIN_MAP_SIZE sizeof(DBC_PIN_MAP)/sizeof(DBC_PIN_MAP[0])

/**
  * \ingroup Dome
  *
  * \class ButtonController
  *
  * \brief Controller class for ia-parts.com Dome Button Controller
  *
  * Controller class for ia-parts.com Dome Button Controller. Allows you to control a relay or send
  * i2c/serial/jawa/smq requests in response to button sequences.
  *
  * Example sketch:
  *
  * \include frontpsi.ino
  */
class ButtonController :
    public SetupEvent, AnimatedEvent
{
public:
    ButtonController(
            void (*flasher)(byte color, int rate) = NULL,
            const byte leftInputPin = 2,
            const byte rightInputPin = 3) :
        fLeftInputPin(leftInputPin),
        fRightInputPin(rightInputPin),
        fFlasher(flasher),
        fMainState(0),
        fLastTime(0),
        fNONECounter(0),
        fBOTHCounter(0),
        fLEFTCounter(0),
        fRIGHTCounter(0),
        fLeftButtonCount(0),
        fRightButtonCount(0),
        fInvalidInput(false),
        fInvalidTime(0),
        fLedStatusState(0),
        fYellowRate(0),
        fGreenRate(0),
        fYellowFlash(0),
        fGreenFlash(0),
        fLEDColor(0),
        fLEDRate(0),
        fPowerDown(false),
        fPowerDownInitialize(false),
        fPowerDownTime(0),
        fPowerDownDelay(DBC_POWER_DOWN_DELAY),
        fDisplaySequence(0)
    {
        for (byte i = 0; i < sizeof(fIO); i++)
            fIO[0] = 0;
    }

    virtual void setup() override
    {
        // Switch 3 - Power Up Trigger (LEFT)
        pinMode(fLeftInputPin, INPUT);
        // Switch 1 - Power Up Trigger (RIGHT)
        pinMode(fRightInputPin, INPUT);

        // turn on pullup resistors
        digitalWrite(fLeftInputPin, HIGH);
        digitalWrite(fRightInputPin, HIGH);
    }

    virtual void animate() override
    {
        unsigned long now = millis();
        if (now - fLastTime > DBC_SPEED)
        {
            fLastTime = now;
            switch (fMainState)
            {
                // BOTH INPUTS HIGH (NOT PRESSED)
                case 0:
                {  
                    if (fPowerDown)
                    {
                        fLEDColor = OFF;
                    }
                    else
                    {
                        fLEDColor = PSI_ALTERNATE;
                        fLEDRate = PSI_RANDOM;
                    }
                    // wait for Extenal Trigger #1 and #2 to go HIGH before watching for external negative trigger
                    if (digitalRead(fLeftInputPin) == HIGH && digitalRead(fRightInputPin) == HIGH)
                    {
                        // Both Buttons need to be Released for 100mS to prevent a false trigger
                        if (fNONECounter++ > 100)
                        {
                            // both external lines are not triggered - ready to watch for negative trigger (go to next state)
                            fMainState++;
                            fNONECounter = 0;
                        }
                    }
                    else
                    {
                        fNONECounter = 0;
                    }
                    break;
                }

                // KeyPad Open - reserved for sound or light action
                case 1:
                {  
                #ifdef USE_SMQ
                    SMQ::send_start(SMQID("DBCState"));
                    SMQ::send_uint8(MSGID("state"), fMainState);
                    SMQ::send_end();
                #endif
                    fMainState++;
                    break;
                }

                // Watch for LEFT and RIGHT button press
                case 2:
                {  
                    // wait for BOTH Extenal Trigger #1 and #2 to go LOW 
                    if (digitalRead(fLeftInputPin) == LOW && digitalRead(fRightInputPin) == LOW)
                    {
                        // Both Buttons need to be Held Low to trigger Lights
                        if (fBOTHCounter++ > 20)
                        {
                            fLEDColor = PSI_BOTH;
                            fLEDRate = ON;
                        }
                        // Both Buttons need to be Held Low for 100 second (100 x 10mS)
                        if (fBOTHCounter++ > 1000)
                        {
                            fBOTHCounter = 0;
                            fMainState++;
                        }
                    }
                    else
                    {
                        // BOTH buttons not pressed - continue Alternate Pattern
                        if (fPowerDown)
                        {
                            fLEDColor = OFF;
                        }
                        else
                        {
                            fLEDColor = PSI_ALTERNATE;
                            fLEDRate = PSI_RANDOM;
                        }
                        // turn on both Yellow and Green if buttons pressed
                        fBOTHCounter = 0;
                    }
                    break;
                }

                // KeyPad Activated - reserved for sound or light action
                case 3:
                {
                #ifdef USE_SMQ
                    SMQ::send_start(SMQID("DBCState"));
                    SMQ::send_uint8(MSGID("state"), fMainState);
                    SMQ::send_end();
                #endif
                    // Turn off ALL LED's when KeyPad is Unlocked and ready for user input
                    fLEDColor = OFF;
                    fLEDRate = OFF; 
                    fMainState++;
                    break;
                }

                // Wait for Both Lines to be Released before proceeding
                case 4:
                { 
                    // wait for Extenal Trigger #1 and #2 to go HIGH before watching for external negative trigger
                    if (digitalRead(fLeftInputPin) == HIGH && digitalRead(fRightInputPin) == HIGH)
                    {
                        // Both Buttons need to be Held Low for 0.200 second (20 x 10mS)
                        if (fNONECounter++ > 20)
                        {
                            fMainState++;
                            fNONECounter = 0;
                            // Turn off ALL LED's when KeyPad is Unlocked and ready for user input
                            fLEDColor = OFF;
                            fLEDRate = OFF;
                        }
                    }
                    else
                    {
                        fNONECounter = 0;
                    }
                    break;
                }

                // reserved for sound or light action
                case 5:
                {
                    fMainState++;
                    break;
                }

                // Watch for LEFT, RIGHT, or BOTH button press
                case 6:
                {
                    //
                    // Left Button(s) must be pressed first (before Right Button)
                    //
                    // Once Right Button(s) have been pressed (RightButton >0); pressing the left button again will abort the sequence
                    //
                    //
                    // after a code has been entered - reduce the double press time requirementto 150mS
                    // if no activity in 10 seconds - reset the 2 second double press time requirement
                    if (digitalRead(fLeftInputPin) == LOW && digitalRead(fRightInputPin) == HIGH)
                    {
                        // LEFT button pressed
                        fRIGHTCounter = 0;
                        fBOTHCounter  = 0;
                        fNONECounter  = 0;

                        // debounce the button press
                        if (fLEFTCounter++ > 20)
                        {
                            fLEDColor = PSI_YELLOW;
                            fLEDRate = ON;
                            fLEFTCounter = 0;
                            //check to see if Right Button has been pressed previously or if Left Button has been pressed too much
                            // if so, SET ERROR FLAG
                            if (++fLeftButtonCount > DBC_LEFT_BUTTON_MAX || fRightButtonCount)
                            {
                                fInvalidInput = 1;
                            }
                            // Look for next Button Press
                            fMainState = 4;
                        #ifdef USE_SMQ
                            SMQ::send_start(SMQID("DBCLeft"));
                            SMQ::send_uint8(MSGID("count"), fLeftButtonCount);
                            SMQ::send_end();
                        #endif
                        }
                    }
                    else if (digitalRead(fRightInputPin) == LOW && digitalRead(fLeftInputPin) == HIGH)
                    {
                        // RIGHT button pressed
                        fLEFTCounter = 0;
                        fBOTHCounter = 0;
                        fNONECounter = 0;

                        if (fRIGHTCounter++ > 20)
                        {
                            fLEDColor = PSI_GREEN;
                            fLEDRate = ON;
                            fRIGHTCounter = 0;
                            //check to see if Left Button has NOT been pressed previously or if Right Button has been pressed too much
                            // if so, SET ERROR FLAG
                            if (++fRightButtonCount > DBC_RIGHT_BUTTON_MAX || !fLeftButtonCount)
                            {
                                fInvalidInput = 1;
                            }
                            // Look for next Button Press
                            fMainState = 4;
                        #ifdef USE_SMQ
                            SMQ::send_start(SMQID("DBCRight"));
                            SMQ::send_uint8(MSGID("count"), fRightButtonCount);
                            SMQ::send_end();
                        #endif
                        }
                    }
                    else if (digitalRead(fRightInputPin) == LOW && digitalRead(fLeftInputPin) == LOW)
                    {
                        // BOTH buttons pressed
                        fLEFTCounter = 0;
                        fRIGHTCounter = 0;
                        fNONECounter = 0;

                        if (fBOTHCounter++ > 20)
                        {
                            fLEDColor = PSI_BOTH;
                            fLEDRate = ON;
                            fBOTHCounter = 0;
                            fNONECounter = 0;

                            if (!fLeftButtonCount || !fRightButtonCount)
                            {
                                fInvalidInput = 1;
                            }
                            // process user inputs
                            fMainState = 7;
                        #ifdef USE_SMQ
                            SMQ::send_start(SMQID("DBCState"));
                            SMQ::send_uint8(MSGID("state"), fMainState);
                            SMQ::send_end();
                        #endif
                        }
                    }
                    else
                    {
                        // NO buttons pressed
                        fLEFTCounter = 0;
                        fRIGHTCounter = 0;
                        fBOTHCounter = 0;

                        if (fNONECounter++ > 4000)
                        {
                            // if nothing is pressed for 5 seconds reset system
                            fNONECounter = 0;
                            fLEDColor = OFF;
                            fLEDRate = OFF;
                            fMainState = 8;
                            fInvalidInput = 1;
                        }
                    }
                    break;
                }

                case 7:
                {
                    // wait for Both Buttons to go HIGH 
                    if (digitalRead(fLeftInputPin) == HIGH && digitalRead(fRightInputPin) == HIGH)
                    {
                        if (fNONECounter++ > 20) 
                        {
                            fLEDColor = OFF;
                            fLEDRate = OFF;
                            fMainState = 9;
                        }
                    }   
                    break;
                }

                case 8:
                {
                    // KEYPAD TIMED OUT - DO SOMETHING SPECIAL
                #ifdef USE_SMQ
                    SMQ::send_start(MSGID("DBCState"));
                    SMQ::send_uint8(MSGID("state"), fMainState);
                    SMQ::send_end();
                #endif
                    fMainState++;
                    break;
                }

                case 9:
                {
                    // PROCESS USER INPUTS
                    fLEDColor = OFF;
                    fLEDRate = OFF;

                    fMainState = (fInvalidInput) ? 13 : 11;
                    break;
                }

                case 10:
                {
                    // not used
                    fMainState++;
                    break;
                }

                case 11:
                {
                #ifdef USE_SMQ
                    SMQ::send_start(MSGID("DBCPress"));
                    SMQ::send_uint8(MSGID("left"), fLeftButtonCount);
                    SMQ::send_uint8(MSGID("right"), fRightButtonCount);
                    SMQ::send_end();
                #endif
                    // Process Valid Input Command:  LEFT = 1-4  RIGHT = 1-3
                    // Now that a Valid Input has been received - allow Rear PSI Lights to turn ON
                    fFirstTime = 0;

                    // produce a value from 1 to 16 (A1-D4)
                    int address = (((DBC_LEFT_BUTTON_MAX * fLeftButtonCount) + fRightButtonCount) - DBC_LEFT_BUTTON_MAX);

                #ifdef DBC_PIN_MAIN_POWER_RELAY
                    // If The System was Previously Powered Down - Then ONLY a POWER UP CODE IS VALID - ALL OTHERS ARE INVALID
                    if (fPowerDown)
                    {
                        if (address == DBC_PIN_MAIN_POWER_RELAY)
                        {
                        #ifdef USE_SMQ
                            SMQ::send_start(SMQID("DBCPower"));
                            SMQ::send_boolean(MSGID("on"), true);
                            SMQ::send_end();
                        #endif
                            fPowerDown = 0;
                            /* TRIGGER MAIN POWER RELAY - TURN ON POWER*/
                            setOutputPin(DBC_PIN_MAIN_POWER_RELAY, HIGH);
                            fMainState++;
                            break;
                        } 
                        else
                        {
                            // Invalid Input
                            fMainState = 13;
                            break;
                        }
                    }
                #endif
                    /*
                    A1  1:1  1
                    A2  1:2  2
                    A3  1:3  3
                    A4  1:4  4
                    B1  2:1  5
                    B2  2:2  6 
                    B3  2:3  7
                    B4  2:4  8
                    C1  3:1  9
                    C2  3:2  10
                    C3  3:3  11
                    C4  3:4  12
                    D1  4:1  13
                    D2  4:2  14 
                    D3  4:3  15       
                    D4  4:4  16
                    */
                    // Toggle The IO Pin State

                    if (fIO[address] == HIGH)
                    {
                        fIO[address] = LOW;
                        setOutputPin(address, LOW);
                    #ifdef DBC_PIN_MAIN_POWER_RELAY
                        if (address == DBC_PIN_MAIN_POWER_RELAY)
                        {
                            // POWER DOWN SEQUENCE
                            fPowerDownInitialize= 1;
                            fPowerDownTime = 0;
                        }
                    #endif
                    }
                    else
                    {
                        fIO[address] = HIGH;
                    #ifdef DBC_PIN_MAIN_POWER_RELAY
                        if (address == DBC_PIN_MAIN_POWER_RELAY)
                        {
                            // POWER DOWN SEQUENCE
                        #ifdef USE_SMQ
                            SMQ::send_start(SMQID("DBCPowerDown"));
                            SMQ::send_uint16(MSGID("delay"), fPowerDownDelay);
                            SMQ::send_end();
                        #endif
                            fPowerDownInitialize = 1;
                            setOutputPin(address, LOW);
                            fPowerDownTime = 0;
                        }
                        else
                    #endif
                        {
                            setOutputPin(address, HIGH);
                        }
                    }
                    fMainState++;
                    break;
                }

                case 12:
                {
                    // Display Valid Input

                    // wait for BOTH Extenal Trigger #1 and #2 to go LOW 
                    if (digitalRead(fLeftInputPin) == LOW && digitalRead(fRightInputPin) == LOW)
                    {
                        // Both Buttons are HELD - EXIT and Allow User to Enter a New Code
                        if (fBOTHCounter++ > 50)
                        {
                            fBOTHCounter = 0;
                            fYellowRate = 0;
                            fGreenRate = 0;
                            fLeftButtonCount = 0;
                            fRightButtonCount = 0;
                            fInvalidInput = 0;
                            fLedStatusState = 0;
                            // EXIT and Allow User to Enter a New Code (state 3)
                            fMainState = 3;
                        }
                    }
                    else
                    {
                        // BOTH buttons not pressed - continue FEEDBACK ROUTINE
                        fBOTHCounter = 0;
                    }

                    /////////
                    // Flash Yellow for Left
                    // Flash Green for Right
                    switch (fLedStatusState)
                    {
                        //  TURN ALL LEDs OFF - BEFORE FLASHING STATE
                        case 0:
                        {
                            fLEDColor = OFF;
                            fLEDRate = OFF;
                            if (fYellowRate++ > 300)
                            {
                                fYellowRate = 0;  // clear flag
                                fLedStatusState++;
                            }
                            break;
                        }
                        // YELLOW ON
                        case 1:
                        {
                            fLEDColor = PSI_YELLOW;
                            fLEDRate = ON;
                            if (fYellowRate++ > 75)
                            {
                                // clear flag
                                fYellowRate = 0;
                                fLedStatusState++;
                            }
                            break;
                        }

                        // YELLOW OFF
                        case 2:
                        {
                            fLEDColor = OFF;

                            if (fYellowRate++ > 150)
                            {
                                // clear flag
                                fYellowRate = 0; 
                                // check for the need to flash Yellow again (if Left was pressed more than once)
                                if (++fYellowFlash >= fLeftButtonCount)
                                {
                                    fYellowFlash = 0;
                                    fLedStatusState++;
                                }
                                else
                                {
                                    fLedStatusState = 1;
                                }
                            }
                            break;
                        }

                        // GREEN ON
                        case 3:
                        {
                            fLEDColor = PSI_GREEN;
                            fLEDRate  = ON;

                            if (fGreenRate++ > 75)
                            {
                                // clear flag
                                fGreenRate = 0;
                                fLedStatusState++;
                            }
                            break;
                        }

                        // GREEN OFF
                        case 4:
                        {
                            fLEDColor = OFF;

                            if (fGreenRate++ > 150)
                            {
                                 // clear flag
                                fGreenRate = 0;
                                if (++fGreenFlash >= fRightButtonCount)
                                {
                                    fGreenFlash = 0;
                                    fLedStatusState++;
                                }
                                else
                                {
                                    fLedStatusState = 3;
                                }
                            }
                            break;
                        }

                        // ALL OFF
                        case 5:
                        {
                            if (fGreenRate++ > 300)
                            {
                                fGreenRate = 0;
                                fLedStatusState++;
                            }
                            break;
                        }

                        // REPEAT 
                        case 6:
                        {
                            // In Future and || (PowerDown) || (PowerDownInitialize) to prevent repeating the code sequence during a power down.
                            if (++fDisplaySequence > 1)
                            {
                                fDisplaySequence = 0;
                                fLedStatusState++;
                            }
                            else
                            {
                                fLedStatusState = 1;
                            }
                            break;
                        }

                        case 7:
                        {
                            // small delay before resuming psi routine
                            if (fGreenRate++ > 100)
                            {
                                fGreenRate = 0;
                                fLeftButtonCount = 0;
                                fRightButtonCount = 0;
                                fInvalidInput = 0;
                                fLedStatusState = 0;
                                fMainState = 0;
                            }
                            break;
                        }
                    }
                    // end of case 12
                    break;
                }

                case 13:
                {
                #ifdef USE_SMQ
                    SMQ::send_start(SMQID("DBCState"));
                    SMQ::send_uint8(MSGID("state"), fMainState);
                    SMQ::send_end();
                #endif
                    // INVALID INPUT - DO SOMETHING SPECIAL

                    // wait for BOTH Extenal Trigger #1 and #2 to go LOW 
                    if (digitalRead(fLeftInputPin) == LOW && digitalRead(fRightInputPin) == LOW)
                    {
                        // Both Buttons are HELD - EXIT and Allow User to Enter a New Code
                        if (fBOTHCounter++ > 50)
                        {
                            fBOTHCounter = 0;
                            fYellowRate = 0;
                            fGreenRate = 0;
                            fLeftButtonCount = 0;
                            fRightButtonCount = 0;
                            fInvalidInput = 0;
                            fLedStatusState = 0;
                            // EXIT and Allow User to Enter a New Code (state 3)
                            fMainState = 3;
                        }
                    }
                    else
                    {
                        // BOTH buttons not pressed - continue FEEDBACK ROUTINE
                        fBOTHCounter = 0;
                    }

                    fLEDColor = PSI_ALTERNATE;
                    fLEDRate = 100;
                    fLeftButtonCount = 0;
                    fRightButtonCount = 0;
                    fInvalidInput = 0;

                    if (fInvalidTime++ > 2000)
                    {
                        fInvalidTime = 0;
                        // RESTART ENTIRE SEQUENCE
                        fMainState = 0;
                    }
                    break;
                }
            }// end of switch(MainState)

            // update LEDs
            if (fFlasher != NULL)
                fFlasher(fLEDColor, fLEDRate);

            ///////////////////////////////////////////////////////////////////////////////////////////////
            ///////////////////////////////////////////////////////////////////////////////////////////////
            // create a power down initialization flag to begin counter - then clear a
            // IF POWER DOWN COMMAND WAS ENTERED (D3) THEN SET TIMER FOR POWER DOWN.
            if (fPowerDownInitialize && fPowerDownTime > fPowerDownDelay)
            {
                fPowerDownTime = 0;
                fPowerDown = 1;
                fPowerDownInitialize = 0;

            #ifdef USE_SMQ
                SMQ::send_start(SMQID("DBCPower"));
                SMQ::send_boolean(MSGID("on"), false);
                SMQ::send_end();
            #endif
                // RESET ALL SETTINGS AND WAIT FOR POWER UP KEYPAD SEQUENCE
                for (unsigned pin = 1; pin < DBC_PIN_MAP_SIZE; pin++)
                {
                    setOutputPin(pin, LOW);
                }
                for (unsigned n = 0; n < 17; n++)
                { 
                    // Set all outputs LOW
                    fIO[n] = 0;
                }
            }
        }

    }

    void setOutputPin(byte domePin, bool state)
    {
    #ifdef USE_SMQ
        SMQ::send_start(SMQID("DBCPin"));
        SMQ::send_uint8(MSGID("pin"), domePin);
        SMQ::send_boolean(MSGID("state"), state);
        SMQ::send_end();
    #endif
        int pin;
        if ((pin = pinMap(domePin)) != DBC_PIN_DISABLED)
            digitalWrite(pin, state);
    }

private:
    const byte fLeftInputPin;
    const byte fRightInputPin;
    void (*fFlasher)(byte color, int rate);

    // State Variables
    byte fMainState;

    unsigned long fLastTime;

    unsigned fNONECounter;
    unsigned fBOTHCounter;
    unsigned fLEFTCounter;
    unsigned fRIGHTCounter;

    // counter that is incremented on each Left Button Press
    byte fLeftButtonCount;
    // counter that is incremented on each Right Button Press
    byte fRightButtonCount;

    // flag to indicate an invalid input from user
    bool fInvalidInput;
    int fInvalidTime;

    byte fLedStatusState;
    int fYellowRate;
    int fGreenRate;
    byte fYellowFlash;
    byte fGreenFlash;

    byte fLEDColor;
    int fLEDRate;

    bool fFirstTime = true;
    bool fPowerDown;
    bool fPowerDownInitialize;
    int fPowerDownTime;
    int fPowerDownDelay;

    byte fDisplaySequence;
    byte fIO[DBC_PIN_MAP_SIZE];

    static inline int pinMap(byte pin)
    {
        return (pin < DBC_PIN_MAP_SIZE) ? pgm_read_byte(&DBC_PIN_MAP[pin]) : DBC_PIN_DISABLED;
    }
};

#endif
