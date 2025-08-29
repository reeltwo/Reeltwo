#ifndef HoloLights_h
#define HoloLights_h

#include "ReelTwo.h"
#include "core/LEDPixelEngine.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include "core/CommandEvent.h"
#include "core/JawaEvent.h"
#include "ServoDispatch.h"

#ifdef USE_DEBUG
#define HOLO_DEBUG
#endif

#if USE_LEDLIB == 0
template<uint8_t DATA_PIN, uint32_t RGB_ORDER, uint16_t NUM_LEDS>
class HoloLEDPCB : public FastLED_NeoPixel<NUM_LEDS, DATA_PIN, RGB_ORDER>
{
public:
    HoloLEDPCB() {}
};
#define USE_HOLO_TEMPLATE 1
#elif USE_LEDLIB == 1
template<uint8_t DATA_PIN, uint32_t RGB_ORDER, uint16_t NUM_LEDS>
class HoloLEDPCB : public Adafruit_NeoPixel
{
public:
    HoloLEDPCB() :
        Adafruit_NeoPixel(NUM_LEDS, DATA_PIN, RGB_ORDER)
    {
    }
};
#else
 #error Unsupported
#endif

/**
  * \ingroup Dome
  *
  * \class HoloLights
  *
  * \brief Controls the movement and display functions of a single Holoprojector
  *
  * Controls the movement and display functions of a single Holoprojector, by default Holoprojectors are 7 LED Neopixel boards
  * inside a HP to produce a life-like emulation of a hologram projection. Servo control is managed by a ServoDispatcher instance.
  *
  * Example usage:
  * \code
  *  HoloLights frontHolo(1);
  *  HoloLights rearHolo(2);
  *  HoloLights topHolo(3, 12);
  * \endcode
  */
#if USE_HOLO_TEMPLATE
template<uint8_t DATA_PIN, uint32_t RGB_ORDER = GRB, uint16_t NUM_LEDS = 7>
class HoloLights :
    public HoloLEDPCB<DATA_PIN, RGB_ORDER, NUM_LEDS>, SetupEvent, AnimatedEvent, CommandEvent, JawaEvent
#else
class HoloLights :
    public Adafruit_NeoPixel, SetupEvent, AnimatedEvent, CommandEvent, JawaEvent
#endif
{
public:
#if !USE_HOLO_TEMPLATE
    enum PixelType
    {
        kRGBW = NEO_GRBW + NEO_KHZ800,
        kRGB = NEO_GRB + NEO_KHZ800
    };
#endif
    enum HoloID
    {
        /** Front holoprojector ID */
        kFrontHolo = 1,
        /** Rear holoprojector ID */
        kRearHolo = 2,
        /** Top holoprojector ID */
        kTopHolo = 3,
        /** Other holoprojector ID */
        kRadarEye = 4,
        /** Other holoprojector ID */
        kOtherHolo = 5
    };
    enum HoloPosition
    {
        /** Holoprojector position down */
        kDown,
        /** Holoprojector position center */
        kCenter,
        /** Holoprojector position up */
        kUp,
        /** Holoprojector position left */
        kLeft,
        /** Holoprojector position upper left */
        kUpperLeft,
        /** Holoprojector position lower left */
        kLowerLeft,
        /** Holoprojector position right */
        kRight,
        /** Holoprojector position upper right */
        kUpperRight,
        /** Holoprojector position lower right */
        kLowerRight,
        /** Total count */
        kNumPositions
    };
    enum HoloColors
    {
        /** Off */
        kOff     = 0x000000,
        /** Red */
        kRed     = 0xFF0000,
        /** Orange */
        kOrange  = 0xFF8000,
        /** Yellow */
        kYellow  = 0xFFFF00,
        /** Green */
        kGreen   = 0x00FF00,
        /** Cyan */
        kCyan    = 0x00FFFF,
        /** Blue */
        kBlue    = 0x0000FF,
        /** Magenta */
        kMagenta = 0xFF00FF,
        /** Purple */
        kPurple  = 0x800080,
        /** White */
        kWhite   = 0xFFFFFF
    };

#if USE_HOLO_TEMPLATE
    /** \brief Constructor
      *
      */
    HoloLights(const int id = 0) :
        fID((id == 0) ? getNextID() : id)
#else
    /** \brief Constructor
      *
      */
    HoloLights(const byte pin, PixelType type = kRGBW, const int id = 0, const byte numPixels = 7) :
        Adafruit_NeoPixel(numPixels, pin, type),
        fID((id == 0) ? getNextID() : id)
#endif
    {
        setLEDTwitchInterval(45, 180);
        setLEDTwitchRunInterval(5, 25);
        setHPTwitchInterval(45 + random(15), 120 + random(60));

        fHPpins[0] = 0;
        fHPpins[1] = 0;

        JawaID addr = kJawaOther;
        switch (fID)
        {
            case kFrontHolo:
                addr = kJawaFrontHolo;
                break;
            case kRearHolo:
                addr = kJawaRearHolo;
                break;
            case kTopHolo:
                addr = kJawaTopHolo;
                break;
            case kRadarEye:
                addr = kJawaRadarEye;
                break;
            case kOtherHolo:
                addr = kJawaOther;
                break;
        }
        setJawaAddress(addr);
    }

#if USE_HOLO_TEMPLATE
    void begin()
    {
        HoloLEDPCB<DATA_PIN, RGB_ORDER, NUM_LEDS>::begin();
    }

    void show()
    {
        HoloLEDPCB<DATA_PIN, RGB_ORDER, NUM_LEDS>::show();
    }

    void setBrightness(uint8_t b)
    {
        HoloLEDPCB<DATA_PIN, RGB_ORDER, NUM_LEDS>::setBrightness(b);
    }

    uint8_t getBrightness()
    {
        return HoloLEDPCB<DATA_PIN, RGB_ORDER, NUM_LEDS>::getBrightness();
    }

    uint16_t numPixels()
    {
        return HoloLEDPCB<DATA_PIN, RGB_ORDER, NUM_LEDS>::numPixels();
    }

    void setPixelColor(uint16_t n, uint32_t c)
    {
        HoloLEDPCB<DATA_PIN, RGB_ORDER, NUM_LEDS>::setPixelColor(n, c);
    }
#endif

    /**
      * Command Prefix: HP
      *
      * DT##C or DT##CS or DT##R or DT##P
      *
      * D - the HP designator
      *     F - Front HP
      *     R - Rear HP
      *     T - Top HP
      *     D - Radar Eye
      *     O - Other HP
      *     A - All 3 HPs
      *     X - Front & Rear HPs
      *     Y - Front & Top HPs
      *     Z - Rear & Top HPs
      *     S - Sequences (See Below)
      * 
      * T - the Sequence Type is either 0-Led Fuctions and 1-Servo Functions
      * 
      *     ## - the Sequence Value including leading zero if necessary, ie sequence 3 is 03
      * 
      *     C - (Optional), the Color integer value from list below:
      *         Basic Color Integer Values
      *             1 = Red
      *             2 = Yellow
      *             3 = Green
      *             4 = Cyan (Aqua)
      *             5 = Blue
      *             6 = Magenta
      *             7 = Orange
      *             8 = Purple
      *             9 = White
      *             0 = Random
      * 
      *     S - (Optional), Speed setting integer for the Dim Pulse LED function below (0-9)
      * 
      *     R - (Optional), Random State for clearing LED displays
      *         Random State Integer Values
      *             1 = Use Default Sequences
      *             2 = Use Random Sequences
      * 
      *     P - (Optional), the Position integer value from list below:
      *         Preset Position Integer Values
      *             0 = Down
      *             1 = Center
      *             2 = Up
      *             3 = Left
      *             4 = Upper Left
      *             5 = Lower Left
      *             6 = Right
      *             7 = Upper Right
      *             8 = Lower Right
      * 
      *     D001    - Leia Sequence, Random shades of blue to mimic Leia Hologram
      *     D002C   - Color Projector Sequence, Like Leia above but using color command value
      *     D003CS  - Dim Pulse Sequence, Color slowly pulses on and off
      *     D004C   - Cycle Sequence, using color command value
      *     D005C   - Toggles Color, Simply sets LEDs tp solid color value
      *     D006    - Rainbow Sequence
      *     D007C   - Short Circuit, Led flashes on and off with interval slowing over time
      *     D096    - Clears LED, Disables Auto LED Sequence & "Off Color"
      *     D0971   - Clears LED, Enables Auto LED Sequence,Enables Default Sequences, Disables "Off Color"
      *     D0972   - Clears LED, Enables Auto LED Sequence,Enables Random Sequences, Disables "Off Color"
      *     D098    - Clears LED, Disables Auto LED Sequence, enables "Off Color"
      *     D0991   - Clears LED, Enables Auto LED Sequence,Enables Default Sequences, Enables "Off Color"
      *     D0992   - Clears LED, Enables Auto LED Sequence,Enables Random Sequences, Enables "Off Color"
      * 
      *     D101P   - Sends HP to a Preset Position
      *     D102    - Enables RC Control on HP (Left/Right)
      *     D103    - Enables RC Control on HP (Up/Down)
      *     D104    - Sends HP to a Random Position
      *     D105    - Wags HP Left/Right 5 times
      *     D106    - Wags HP Up/Down 5 times
      *     D198    - Disables Auto HP Twitch
      *     D199    - Enables Auto HP Twitch
      * 
      *       S1    - Leia Mode (Front HP in Down Position, Leia LED Sequence, all other HPs disabled)*
      *       S2    - Play R2 Cartoon [OLED]
      *       S3    - Play Deathstar Plans movie [OLED]
      *       S4    - Clear all LEDs, Disable Auto HP Twitch, Disable Auto LED Sequence, Disables Off Color
      *       S5    - Clear all LEDs, Enable Auto HP Twitch, Enable Auto LED Sequence (w/ default seqs.),
      *               Disables Off
      *       S9    - Clear all LEDs, Enable Auto HP Twitch, Enable Auto LED Sequence (w/ random seqs.),
      *               Disables Off Color
      *       S7    - Clear all LEDs, Disable Auto HP Twitch, Disable Auto LED Sequence, Enables Off Color
      *       S8    - Clear all LEDs, Enable Auto HP Twitch, Enable Auto LED Sequence (w/ default seqs.),
      *               Enables Off Color
      *       S9    - Clear all LEDs, Enable Auto HP Twitch, Enable Auto LED Sequence (w/ random seqs.),
      *               Enables Off Color
      * 
      * Runtime values can be added to any command string by appending a pipe (|) followed by a
      * numeric value indicating the desired time in seconds you wish the sequence to run.
      * 
      * e.g.  A007|25 would run the Rainbow Sequence on all 3 HPs for 25 seconds then clear each
      *       one, returning to the system's last known auto twitch mode.
      */
    virtual void handleCommand(const char* cmd) override
    {
        printf("COMMAND: %s\n", cmd);
        int durationSec = -1;
        byte typeState = 0;
        byte functionState = 0;
        int optionState = -1;
        int optionState2 = -1;

        if (*cmd++ != 'H' || *cmd++ != 'P')
            return;

        int commandLength = strlen(cmd);
        const char* pipeIndex = strchr(cmd, '|');
        if (pipeIndex != NULL)
        { 
            durationSec = atoi(pipeIndex+1);
            commandLength = pipeIndex - cmd;
        }
        if (cmd[0]=='F' ||      // Front HP
            cmd[0]=='R' ||      // Rear HP
            cmd[0]=='T' ||      // Top HP
            cmd[0]=='X' ||      // Front & Rear HPs
            cmd[0]=='Y' ||      // Front & Top HPs
            cmd[0]=='Z' ||      // Rear & Top HPs
            cmd[0]=='D' ||      // Radar eye
            cmd[0]=='O' ||      // Other HP
            cmd[0]=='A' )       // All three HP
        {  
            if (commandLength >= 2)
                typeState = cmd[1]-'0';
            if (commandLength >= 4)
                functionState = (cmd[2]-'0')*10+(cmd[3]-'0');

            if (commandLength >= 5)
            {
                optionState = cmd[4]-'0';
            }
            else if (typeState == 1)
            {
                optionState = 1;         // Set Center as the fallback position
            }
            if (commandLength >= 6)
            {
                optionState2 = cmd[5]-'0';
            }
            // All match A
            bool match = (cmd[0]=='A');
            // Front holo matches FXY
            match = (match || (fID == kFrontHolo && (cmd[0]=='F' || cmd[0]=='X' || cmd[0]=='Y')));
            // Rear holo matches RXZ
            match = (match || (fID == kRearHolo && (cmd[0]=='R' || cmd[0]=='X' || cmd[0]=='Z')));
            // Top holo matches TYZ
            match = (match || (fID == kTopHolo && (cmd[0]=='T' || cmd[0]=='Y' || cmd[0]=='Z')));
            // Radar eye matches D
            match = (match || (fID == kRadarEye && cmd[0]=='D'));
            // Other holo matches D
            match = (match || (fID == kOtherHolo && cmd[0]=='O'));
            if (match)
            {
                if (typeState == 0)
                {
                    flushLEDState();                    // Flushes LED Command Arrays
                    fLEDFunction = functionState;
                    if (optionState >= 0)
                    {
                        if (optionState == 0)
                            fLEDOption1 = random(0,9);
                        else
                            fLEDOption1 = optionState;
                    }
                    else
                    {
                        fLEDOption1 = (functionState == 7) ? fShortColor : fDefaultColor;
                    }
                    if (optionState2 >= 0 && functionState == 3)
                        fLEDOption2 = optionState2;
                    else if (optionState2 < 0 && functionState == 3)
                        fLEDOption2 = kDimPulseSpeed;
                    fLEDHalt = (durationSec >= 1) ? durationSec : -1;
                    fLEDHaltTime = millis();
                    varResets();
                }
                else if (typeState == 1)
                {
                    flushHPState();                     // Flushes HP Command Array
                    fHPFunction = functionState;
                    fHPOption1 = optionState;
                    if (durationSec >= 1)
                        fHPHalt = durationSec;

                    fHPHaltTime = millis();
                    fWagCount = -1;
                }
            }
        }
        else if (cmd[0] == 'S')                      // Major Sequences (Both LEDS and HP Servos)
        {
            if (commandLength >= 2)
            {
                functionState = (cmd[1]-'0');
                selectSequence(functionState, durationSec);
            }
        }
    }

    virtual void jawaCommand(char cmd, int arg, int value) override
    {
    }

    /**
      * Runs through one frame of animation for this holoprojector instance.
      */
    virtual void animate() override
    {
        if (fLEDHalt != -1)
        {
            if (fLEDHaltTime + (fLEDHalt * 1000L) < millis())
            {
                flushLEDState();
                off();                              // Clears Any Remaining Lit LEDs
                resetLEDTwitch(); 
            }
        }
        switch (fLEDFunction)
        {
            case 1:
                effectColorProjectorLED(5);     // Leia Sequence (Blue)
                break;
            case 2:
                effectColorProjectorLED(fLEDOption1);
                break; 
            case 3:
                effectDimPulse(fLEDOption1, fLEDOption2);
                break;
            case 4:
                effectCycle(fLEDOption1);
                break;
            case 5:
                setColor(fLEDOption1);
                break;
            case 6:
                effectRainbow();
                break;
            case 7:
                effectShortCircuit(fLEDOption1);
                break; 
            case 96:
                // Clear Function, Disable Random LED Twitch, enable off color override, and random sequences (if enabled)
                fEnableTwitchLED = 0;
                resetLEDTwitch();
                fOffColorOverride = true;
                flushLEDState();
                break;
            case 97:
                // Clear Function, Enable Random LED Twitch using random LED sequences, enable off color override.
                if (fLEDOption1 == 1 || fLEDOption1 == 2)
                {
                    fEnableTwitchLED = fLEDOption1;
                }
                else
                {
                    fEnableTwitchLED = fStartEnableTwitchLED;
                }
                resetLEDTwitch();
                fOffColorOverride = true;
                flushLEDState();
                break;
            case 98:
                // Clear Function, Disable Random LED Twitch, random sequences (if enabled) and disable off color override,
                fEnableTwitchLED = 0;
                resetLEDTwitch();
                fOffColorOverride = false;
                flushLEDState();
                break;
            case 99:
                // Clear Function, Enable Random LED Twitch, using random LED sequences, disable off color override,
                if (fLEDOption1 == 1 || fLEDOption1 == 2)
                {
                    fEnableTwitchLED = fLEDOption1;
                }
                else
                {
                    fEnableTwitchLED = fStartEnableTwitchLED;
                }
                resetLEDTwitch();
                fOffColorOverride = false;
                flushLEDState();
                break;
            case 100:
                setColor(fLEDOption1);
                break;
            default:
                break;
        }
        if (fHPHalt > 0)
        {
            if (millis() - fHPHaltTime >= (unsigned(fHPHalt) * 1000L))
            {
                flushHPState();
                fWagCount = -1; 
            }
        }
        switch (fHPFunction)
        {
            case 1:
                moveHP(fHPOption1, SERVO_SPEED[0]);
                flushHPState();
                break;
            case 2:
                //RCHP(1);
                break;
            case 3:
                //RCHP(2);
                break;
            case 4:
                twitchHP(0);
                flushHPState();
                break;
            case 5:
                // Wags HP Left/Right   
                wagHP(1);
                break;
            case 6:
                // Wags HP Up/Down  
                wagHP(2);
                break;
            case 98:
                // Clear Function, Disable Servo Random Twitch
                fEnableTwitchHP = false;
                flushHPState();
                break;
            case 99:
                // Clear Function, Enable Servo Random Twitch
                fEnableTwitchHP = true;
                flushHPState();
                break;
            default:
                break;
        } 
        if (millis() > fTwitchLEDTime && fEnableTwitchLED >= 1 && fLEDFunction > 99)
        {
            fTwitchLEDTime = (1000L * random(fLEDTwitchInterval[0], fLEDTwitchInterval[1])) + millis();
            fTwitchLEDRunTime = random(fLEDTwitchRunInterval[0], fLEDTwitchRunInterval[1]);
            flushLEDState();
            fLEDHaltTime = millis();
            varResets();
            if (fEnableTwitchLED == 2)
            {      
                fLEDFunction = random(2,7);
                fLEDOption1 = random(1,9);
                fLEDOption2 = random(1,9);
                fLEDHalt = fTwitchLEDRunTime;
            }
            else
            {
                fLEDFunction = fDefaultLEDTwitchCommand[0];
                fLEDOption1 = fDefaultLEDTwitchCommand[1];
                fLEDOption2 = fDefaultLEDTwitchCommand[2];
                fLEDHalt = fTwitchLEDRunTime;
            }
        }
        if (millis() > fTwitchHPTime && fEnableTwitchHP && fHPFunction > 99)
        {
            twitchHP(1);
            resetHPTwitch();
        }
        if (fDirty)
        {
            TEENSY_PROP_NEOPIXEL_BEGIN();
            show();
            TEENSY_PROP_NEOPIXEL_END();
            fDirty = false;
        } 
    }

    /**
      * Specify the sequence to animate
      */
    virtual void selectSequence(int sequence, int durationSec)
    {
        switch (sequence)
        { 
            case 1:
                varResets();                        // Resets the Variables for LEDs
                flushLEDState();                    // Flushes LED Command Arrays
                fHPFunction = 0;                        // Set HP to Do Nothing
                fLEDFunction = 0;                   // Set LED to Do Nothing
                if (fID == kFrontHolo)
                {
                    moveHP(kDown, SERVO_SPEED[0]);  // Moves Front HP to Down Position
                    fLEDFunction = 1;               // Set Leia LED Sequence to run each loop.
                }
                if (durationSec >= 1)
                {
                    fLEDHalt = durationSec;         // Set LED Halt timeout if passed via command.
                    fLEDHaltTime = millis();            // and note start time
                    fHPHalt = durationSec;          // Set HP Halt timeout if passed via command.
                    fHPHaltTime = millis();         // and note start time
                }                     
                break;
            case 7:
                fEnableTwitchHP = false;            // Disables Auto HP Twith
                flushLEDState();                    // Flushes LED Command Array
                flushHPState();                     // Flushes HP Command Array
                off();                              // Turns Off LEDs
                fEnableTwitchLED = 0;               // Disables Auto LED Twith
                break;
            case 8:
                fEnableTwitchHP = true;             // Enables Auto HP Twith
                flushLEDState();                    // Flushes LED Command Array
                flushHPState();                     // Flushes HP Command Array
                off();                              // Turns Off LEDs
                fEnableTwitchLED = 1;               // Enables Auto LED Twith
                break;
            case 9:
                fEnableTwitchHP = true;             // Enables Auto HP Twith on all HPs 
                flushLEDState();                    // Flushes LED Command Array
                flushHPState();                     // Flushes HP Command Array
                off();                              // Turns Off LEDs
                fEnableTwitchLED = 2;               // Enables Auto LED Twith on all HPs using random sequences
                break;
            default:
                break;
        }
    }

    /**
      * Assign ServoDispatcher and servos for horizontal and vertical movement
      */
    void assignServos(ServoDispatch* dispatcher, byte hServo, byte vServo)
    {
        fServoDispatch = dispatcher;

        fHPpins[0] = hServo;
        fHPpins[1] = vServo;
    }

    /**
      * \returns Holo projector ID (kFront, kRear, kTop, kOther)
      */
    inline int getID()
    {
        return fID;
    }

    /**
      * Configures the NeoPixel ring and centers the holoprojector if servos have been assigned
      */
    virtual void setup() override
    {
        TEENSY_PROP_NEOPIXEL_SETUP()

        setBrightness(BRIGHT);
        dirty();

        moveHP(kCenter);
    }

    /**
      * Increase the brightness of the holoprojector
      */
    inline void brighter()
    {
        setBrightness(getBrightness() * 2);
    }

    /**
      * Decrease the brightness of the holoprojector
      */
    inline void dimmer()
    {
        setBrightness(getBrightness() / 2);
    }

    /**
      * Turn of all LEDs
      */
    void off()
    {
        uint16_t numLEDs = numPixels();
        for (unsigned i = 0; i < numLEDs; i++)
            setPixelColor(i, kOff);
        dirty();
    }

    /**
      * Set projector to a solid color.
      *
      * 1 = Red
      * 2 = Yellow
      * 3 = Green
      * 4 = Cyan (Aqua)
      * 5 = Blue
      * 6 = Magenta
      * 7 = Orange
      * 8 = Purple
      * 9 = White
      * 0 = Random
      */
    void setColor(int c)
    {
        uint16_t numLEDs = numPixels();
        for (unsigned i = 0; i < numLEDs; i++)
        {
            setPixelColor(i, basicColor(c));
        }
        dirty();
    }

    void setDefaultLEDTwitchCommand(int s, int c, int v) {
        fDefaultLEDTwitchCommand[0] = s;
        fDefaultLEDTwitchCommand[1] = c;
        fDefaultLEDTwitchCommand[2] = v;
    }

    void resetLEDTwitch()
    {
        off();
        fTwitchLEDTime = (1000L * random(fLEDTwitchInterval[0], fLEDTwitchInterval[1])) + millis();
    }

    void resetHPTwitch()
    {
        fTwitchHPTime = (1000 * random(fHPTwitchInterval[0], fHPTwitchInterval[1])) + millis();  
    }

    void setHoloPosition(float hpos, float vpos, int speed = 0)
    {
        if (fServoDispatch != NULL)
        {
            fServoDispatch->moveTo(fHPpins[0], speed, hpos);
            fServoDispatch->moveTo(fHPpins[1], speed, vpos);
        }
    }

    /**
      * Move holoprojector to the specified position.
      */
    void moveHP(byte pos, int speed = 0)
    {
        if (fServoDispatch == NULL || pos >= kNumPositions)
            return;

        static const float sPositions[kNumPositions][2] PROGMEM =
        {
            // Down
            { 0.5, 1.0 },

            // Center
            { 0.5, 0.5 },

            // Up
            { 0.5, 0.0 },

            // Left
            { 1.0, 0.5 },

            // Upper Left
            { 1.0, 0.0 },

            // Lower Left
            { 1.0, 1.0 },

            // Right
            { 0.0, 0.5 },

            // Upper Right
            { 0.0, 0.0 },

            // Lower Right
            { 0.0, 1.0 }
        };
        setHoloPosition(pgm_read_float(&sPositions[pos][0]), pgm_read_float(&sPositions[pos][1]), speed);
    #ifdef HOLO_DEBUG
        static const char* position[] = {
            "Down",
            "Center",
            "Up",
            "Left",
            "Upper Left",
            "Lower Left",
            "Right",
            "Upper Right",
            "lower Right"
        };
        DEBUG_PRINT(F("Holo#"));
        DEBUG_PRINT(getID());
        DEBUG_PRINT(F(" moved to the "));
        DEBUG_PRINT(position[pos]);
        DEBUG_PRINTLN(F(" position."));
    #endif
    }

    void twitchHP(byte randtwitch)
    {
        UNUSED_ARG(randtwitch)
        int speed = random(SERVO_SPEED[0], SERVO_SPEED[1]);
    #ifdef HOLO_DEBUG
        if (randtwitch == 1)
        {
            DEBUG_PRINT(F("Random Holo#"));
            DEBUG_PRINT(getID());
            DEBUG_PRINTLN(F(" HP twitch triggered...."));
        }
    #endif
        moveHP(random(0, kNumPositions), speed);
    }

    void wagHP(byte type)
    {
        int speed = 250;
        if (fWagCount < 0)
        {
            fWagCount = 0;
            fWagTimer = millis();
        }
        if (millis() - fWagTimer >= 400)
        {
            fWagCount++;
            fWagTimer = millis();          
            if (type == 1)
            {
                //  On Odd Count, Wag HP Left
                //  On Even Count, Wag HP Right  
                moveHP((fWagCount % 2) ? 3 : 6 , speed);
            }
            if (type == 2)
            {
                //  On Odd Count, Wag HP Down
                //  On Even Count, Wag HP Up 
                moveHP((fWagCount % 2) ? 0 : 2, speed);
            }
        }
    }
    /// \private
    void effectColorProjectorLED(int c)
    {
        uint16_t numLEDs = numPixels();
        if ((millis() - fCounter) > fInterval)
        {
            for (unsigned i = 0; i < numLEDs; i++)
            {
                setPixelColor(i, basicColor(c, random(0,10)));
            }
            dirty();
            fCounter = millis();
            fInterval = random(50,150);
        }
    }

    /// \private
    void effectDimPulse(int c, int setting)
    {
        uint16_t numLEDs = numPixels();
        unsigned inter = map(setting, 0, 9, dimPulseSpeedRange[1], dimPulseSpeedRange[0]);
        if ((millis() - fCounter) > fInterval)
        {
            unsigned elapsed = millis() - fCounter;
            int frames = elapsed / inter;
            if (frames >= 64)
            {
                fCounter = millis();
            }
            if (frames > 32)
            {
                frames = 32 - (frames - 32);
            }
            if (elapsed >= inter)
            {
                for (unsigned i = 0; i < numLEDs; i++)
                    setPixelColor(i, dimColorVal(c, (frames * 8)));
                dirty();
            }
        }
    }

    /// \private
    void effectShortCircuit(int c)
    {
        uint16_t numLEDs = numPixels();
        const int kShortCircuitMaxLoops = 20;
        if (fSCloop <= kShortCircuitMaxLoops)
        {
            if ((millis() - fCounter) > fSCinterval)
            {
                if (fSCflag == false)
                {
                    for (unsigned i = 0; i < numLEDs; i++)
                        setPixelColor(i, kOff);
                    fSCflag = true;
                    fSCinterval = 10 + (fSCloop * random(15,25));
                } 
                else
                {
                    for (unsigned i = 0; i < numLEDs; i++)
                        setPixelColor(i, basicColor(c, random(0,10)));
                    fSCflag = false;
                    fSCloop++;
                }
                fCounter = millis();
                dirty();
            }  
        }   
    }

    /// \private
    void effectCycle(int c)
    {
        uint16_t numLEDs = numPixels();
        const unsigned int inter = 75;
        if ((millis() - fCounter) > inter)
        {
            fCounter = millis();
            if (numLEDs == 7)
            {
                /* 7 pixels with one center */
                setPixelColor(0, kOff);     // Center LED is always off
                if (fFrame >= numLEDs)
                {
                    fFrame = 0;
                }
                for (unsigned i = 1; i < numLEDs; i++)
                {
                    if (i == fFrame)
                    {
                        setPixelColor(i, basicColor(c));
                    }
                    else
                    {
                        setPixelColor(i, kOff);
                    }
                }
            }
            else
            {
                /* More than 7 pixels no center */
                if (fFrame >= numLEDs)
                {
                    fFrame = 0;
                }
                for (unsigned i = 0; i < numLEDs; i++)
                {
                    if (i == fFrame)
                    {
                        setPixelColor(i, basicColor(c));
                    }
                    else
                    {
                        setPixelColor(i, kOff);
                    }
                }
            }
            dirty(); 
            fFrame++;
        }
    }

    /// \private
    void effectRainbow()
    {
        uint16_t numLEDs = numPixels();
        const unsigned int inter = 10;
        unsigned elapsed = millis() - fCounter;
        unsigned int frames = elapsed / inter;
        if (frames > 256 * 5)
        {
            fCounter = millis();
        }
        else
        {
            for (unsigned i = 0; i < numLEDs; i++)
            {
                setPixelColor(i, getWheelColor(((i * 256 / numLEDs) + frames) & 255));
            }
            if (elapsed >= inter)
            {
                dirty();
            }
        }              
    }

    void setLEDTwitchInterval(unsigned minSeconds, unsigned maxSeconds)
    {
        if (minSeconds < maxSeconds)
        {
            fLEDTwitchInterval[0] = minSeconds;
            fLEDTwitchInterval[1] = maxSeconds;
            fTwitchLEDTime = (1000L * random(fLEDTwitchInterval[0], fLEDTwitchInterval[1])) + millis();
        }
    }

    void setLEDTwitchRunInterval(unsigned minSeconds, unsigned maxSeconds)
    {
        if (minSeconds < maxSeconds)
        {
            fLEDTwitchRunInterval[0] = minSeconds;
            fLEDTwitchRunInterval[1] = maxSeconds;
            fTwitchLEDRunTime = (1000L * random(fLEDTwitchRunInterval[0], fLEDTwitchRunInterval[1]));  // Randomly sets initial LED Twitch Run Time value
        }
    }

    void setHPTwitchInterval(unsigned minSeconds, unsigned maxSeconds)
    {
        if (minSeconds < maxSeconds)
        {
            fHPTwitchInterval[0] = minSeconds;
            fHPTwitchInterval[1] = maxSeconds;
            resetHPTwitch();
        }
    }

    inline void dirty()
    {
        fDirty = true;
    }

#if USE_LEDLIB == 2
    inline void begin() { Begin(); }
    inline void show()  { Show(); }
    inline void setBrightness(uint8_t b) { SetBrightness(b); }
    inline uint8_t getBrightness() { return GetBrightness(); }
    inline uint16_t numPixels() { return PixelCount(); }
    inline void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b)
    {
        SetPixelColor(n, RgbColor(r, g, b));
    }
    inline void setPixelColor(uint16_t n, uint32_t c)
    {
        SetPixelColor(n, RgbColor((c>>16)&0xFF, (c>>8)&0xFF, c&0xFF));
    }
#endif

private:
    enum
    {
        BRIGHT = 100
    };

    void flushLEDState()
    {
        int color = -1;
        if (fOffColor >= 0 && !fOffColorOverride)
        {
            fLEDFunction = 100;
            color = fOffColor;
        }
        else
        {
            fLEDFunction = -1;
        }
        fLEDOption1 = color;
        fLEDOption2 = -1;
        fLEDHalt = -1;
    }

    void flushHPState()
    {
        fHPFunction = -1;
        fHPOption1 = -1;
        fHPHalt = -1;
        if (fEnableTwitchHP)
        {
            // Resets HP Twitch time so random twich doesn't immediately occur following external position command.  
            resetHPTwitch();
        }
    }

    void varResets()
    {
        fFrame = 0;
        fSCflag = false;
        fSCloop = 0;
        fSCinterval = 10;
        fInterval = 100;
        off();
    }

    static inline uint32_t basicColor(int color, int variant = 0)
    {
        static const uint32_t basicColors[10][10] PROGMEM =
         {{     kRed,  kYellow,   kGreen,  kCyan,    kBlue,  kMagenta,  kPurple, kWhite, kOff, kOff},    // Random
          {     kRed,     kRed,     kRed, kWhite, 0xFFA0A0,  0xFD5555, 0xFFD3D3,   kOff, kOff, kOff},    // Red
          {  kYellow,  kYellow,  kYellow, kWhite, 0xFDFD43,  0xFFFF82, 0xFFFFBA,   kOff, kOff, kOff},    // Yellow
          {   kGreen,   kGreen,   kGreen, kWhite, 0x57FC57,  0x80FC80, 0xBDFFB1,   kOff, kOff, kOff},    // Green
          {    kCyan,    kCyan,    kCyan, kWhite, 0x38FFFF,  0x71FDFD, 0xA4FDFD,   kOff, kOff, kOff},    // Cyan
          {    kBlue,    kBlue,    kBlue, kWhite, 0xACACFF,  0x7676FF, 0x5A5AFF,   kOff, kOff, kOff},    // Blue
          { kMagenta, kMagenta, kMagenta, kWhite, 0xFB3BFB,  0xFD75FD, 0xFD9EFD,   kOff, kOff, kOff},    // Magenta
          {  kOrange,  kOrange,  kOrange, kWhite, 0xFB9B3A,  0xFFBE7D, 0xFCD2A7,   kOff, kOff, kOff},    // Orange
          {  kPurple,  kPurple,  kPurple, kWhite, 0xA131A1,  0x9B449B, 0xBD5FBD,   kOff, kOff, kOff},    // Purple
          {   kWhite,   kWhite,   kWhite, kWhite, 0xB7B6B6,  0x858484, 0xA09F9F,   kOff, kOff, kOff}};   // White
        return pgm_read_dword(&basicColors[color][variant]);
    }

    static int getNextID()
    {
        static int sID;
        return ++sID;
    }

    /////////////////////////////////////////////////////////
    ///*****    RGB to Hex Color Value Converter      *****///
    /////////////////////////////////////////////////////////
    ///    Converts and returns a color's 3 seperat RGB   /// 
    ///       value as an uint32_t value.                 ///   
    /////////////////////////////////////////////////////////        

    static inline uint32_t RGB(byte r, byte g, byte b)
    {
        return (uint32_t(r)<<16) | (uint32_t(g)<<8) | (uint32_t(b));
    } 

    /////////////////////////////////////////////////////////
    ///*****             Wheel Function              *****///
    /////////////////////////////////////////////////////////
    /// Input a value 0 to 255 to get a color value. The  ///   
    /// colours are a transition r - g - b - back to r.   ///
    /////////////////////////////////////////////////////////

    static uint32_t getWheelColor(byte wheelPos)
    {
        if (wheelPos < 85)
        {
            return RGB(wheelPos * 3, 255 - wheelPos * 3, 0);
        }
        else if (wheelPos < 170)
        {
            wheelPos -= 85;
            return RGB(255 - wheelPos * 3, 0, wheelPos * 3);
        }
        wheelPos -= 170;
        return RGB(0, wheelPos * 3, 255 - wheelPos * 3);
    } 

    /////////////////////////////////////////////////////////
    ///*****          Dim Color Function 1           *****///
    /////////////////////////////////////////////////////////
    /// Input a value 0 to 255 to get a color value at a  ///   
    /// specific brightness.                              ///
    /// Takes and int color value and returns an          ///
    /// uint32_tcolor value.                              ///
    /// Used for soft fade in/outs.                       ///
    /////////////////////////////////////////////////////////  

    static uint32_t dimColorVal(int c, int brightness)
    {
        if (brightness)
        {
            switch (c)
            {
                case 1:
                    return RGB(255 / brightness, 0, 0);
                case 2:
                    return RGB(255 / brightness, 255 / brightness, 0);
                case 3:
                    return RGB(0, 255 / brightness, 0);
                case 4:
                    return RGB(0, 255 / brightness, 255 / brightness);
                case 5:
                    return RGB(0, 0, 255 / brightness);
                case 6:
                    return RGB(255 / brightness, 0, 255 / brightness);
                case 7:
                    return RGB(255 / brightness, 180 / brightness, 0);
                case 8:
                    return RGB(255 / brightness, 255 / brightness, 255 / brightness);
                case 9:
                    return kOff;
            }
        }
        return kOff;
    }  

    //////////////////////////////////////////////////////////////////////

    int fID;

    // General counters and timers shared across more than one animation
    unsigned long fCounter = 0;
    unsigned int  fInterval = 100;

    unsigned int fLEDTwitchInterval[2];
    unsigned int fLEDTwitchRunInterval[2];

    unsigned int fHPTwitchInterval[2];

    // Short Circuit Animation
    byte fSCloop = 0;
    bool fSCflag = false;
    bool fDirty = false;
    unsigned int fSCinterval = 10;

    // Cycle Animation
    byte fFrame = 0;

    // Wag Animation
    int fWagCount = -1;
    unsigned long fWagTimer = 0;

    unsigned long fTwitchLEDTime = 3800 + random(10) * 100;
    unsigned long fTwitchLEDRunTime;

    unsigned long fTwitchHPTime = 4000; //HPs Start 4 seconds after boot;

    unsigned long fLEDHaltTime = 0;
    unsigned long fHPHaltTime = 0;   

    bool fStartEnableTwitchLED = true;
    bool fEnableTwitchHP;
    byte fEnableTwitchLED = 1;

    int fOffColor = -1;
    bool fOffColorOverride = false;

    byte fLEDFunction = -1;
    byte fLEDOption1 = -1;
    byte fLEDOption2 = -1;
    int fLEDHalt = -1;

    // HP: Sequence, Color, Speed
    byte fDefaultLEDTwitchCommand[3] = { 1, 5, 0 };

    byte fHPFunction = -1;
    byte fHPOption1 = -1;
    int fHPHalt = -1;

    byte fHPpins[2];
    ServoDispatch* fServoDispatch = NULL;

    ///////////////////////////////////////////////////////////////////////////////////
    ///*****                      Default Color Settings                       *****///
    ///*****      `                                                            *****///
    ///*****     Select Default Colors for certain LED sequences using the     *****///
    ///*****                       integer values below:                       *****///
    ///*****                                                                   *****///
    ///*****             Basic Color Integer Values                            *****///
    ///*****               1 = Red                                             *****///
    ///*****               2 = Yellow                                          *****///
    ///*****               3 = Green                                           *****///
    ///*****               4 = Cyan (Aqua)                                     *****///
    ///*****               5 = Blue                                            *****///
    ///*****               6 = Magenta                                         *****///
    ///*****               7 = Orange                                          *****///
    ///*****               8 = Purple                                          *****///
    ///*****               9 = White                                           *****///
    ///*****               0 = Random                                          *****///
    ///*****                                                                   *****///
    ///////////////////////////////////////////////////////////////////////////////////        
    uint8_t fDefaultColor = 5;     // Blue, Color integer value for the hue of default sequence.
    uint8_t fShortColor = 7;       // Orange, Color integer value for the hue of ShortCircuit Message.

    const unsigned int SERVO_SPEED[2] = {150, 400};

    const int kDimPulseSpeed = 5;
    const uint8_t dimPulseSpeedRange[2] = {5, 75};      // Range used to map to value options 0-9, Lower is faster.
};

#if USE_HOLO_TEMPLATE
template<uint8_t DATA_PIN = 45, uint32_t RGB_ORDER = GRB, uint16_t NUM_LEDS = 12>
class HoloOLED :
    public HoloLights<DATA_PIN, RGB_ORDER, NUM_LEDS>
#else
class HoloOLED :
    public HoloLights
#endif
{
public:
#if USE_HOLO_TEMPLATE
    /**
     * \brief Constructor
     */
    HoloOLED(HardwareSerial& oledSerialPort, const int id = 0, const byte resetPin = 46) :
            HoloLights<DATA_PIN, RGB_ORDER, NUM_LEDS>(id),
#else
    /**
     * \brief Constructor
     */
    HoloOLED(HardwareSerial& oledSerialPort, PixelType type = kRGBW, const int id = 0, const byte pin = 45, const byte resetPin = 46, const byte numPixels = 12) :
            HoloLights(pin, type, id, numPixels),
#endif
        fSerialInit(oledSerialPort),
        fSerialPort(oledSerialPort),
        fResetPin(resetPin),
        fMovieIndex(-1),
        fResetState(false)
    {
    }

    /**
     * Initalizes the OLED display and SD card
     */
    virtual void setup() override
    {
    #if USE_HOLO_TEMPLATE
        HoloLights<DATA_PIN, RGB_ORDER, NUM_LEDS>::setup();
    #else
        HoloLights::setup();
    #endif
        pinMode(fResetPin, OUTPUT);
    }

    /**
     * See HoloLights::handleCommand()
     */
    virtual void handleCommand(const char* cmd) override
    {
        if (cmd[0] != 'H' || cmd[1] != 'O')
        {
        #if USE_HOLO_TEMPLATE
            HoloLights<DATA_PIN, RGB_ORDER, NUM_LEDS>::handleCommand(cmd);
        #else
            HoloLights::handleCommand(cmd);
        #endif
            return;
        }
    }

    bool isPlaying()
    {
       return (fMovieIndex >= 0 || (fMovieIndex == -1 && !fResetState && fNextCmd > millis()));
    }

    virtual void animate()
    {
    #if USE_HOLO_TEMPLATE
        HoloLights<DATA_PIN, RGB_ORDER, NUM_LEDS>::animate();
    #else
        HoloLights::animate();
    #endif
        if (fResetState)
        {
            if (fNextCmd < millis())
            {
                digitalWrite(fResetPin, HIGH);
                fResetState = false;
                fNextCmd = millis() + 10000;
            }
        }
        else if (fMovieIndex == 0)
        {
            DEBUG_PRINT("STOP MOVIE: "); DEBUG_PRINTLN(fMovieIndex);
            fSerialPort.print((char)(fMovieIndex));
            fSerialPort.flush();
            fNextCmd = millis();
            fMovieIndex = -1;
        }
        else if (fMovieIndex > 0)
        {
            DEBUG_PRINT("PLAY MOVIE: "); DEBUG_PRINTLN(fMovieIndex);
            fSerialPort.print((char)(fMovieIndex));
            fSerialPort.flush();
            fNextCmd = millis() + 20000;
            fMovieIndex = -1;
        }
    }

    void reset()
    {
        digitalWrite(fResetPin, LOW);
        fNextCmd = millis() + 100;
        fResetState = true;
        fMovieIndex = -1;
    }

    void playMovie(byte movieIndex)
    {
        // if (isPlaying())
        //   reset();
        // else
        // fNextCmd = 0;
        fMovieIndex = movieIndex;
    }

    void stopMovie()
    {
        playMovie(0);
    }

private:
    class SerialInit
    {
    public:
        SerialInit(HardwareSerial& port)
        {
            // 9600 is the default baud rate for 4D Systems uOLED-96-G2 display.
            port.begin(9600);
        }
    };

    SerialInit fSerialInit;
    Stream& fSerialPort;
    byte fResetPin;
    int fMovieIndex;
    bool fResetState;
    uint32_t fNextCmd;
};

#endif
