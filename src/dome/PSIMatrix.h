#ifndef PSIMatrix_h
#define PSIMatrix_h

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include <SoftwareWire.h>

#define DISPLAY_ADDRESS 0x65
#define SWIPE_SPEED 75              // Speed of swipe
#define SWIPE_FRONT_ONE 0xb1        // First colour of front PSI
#define SWIPE_FRONT_TWO 0           // Second colour of front PSI
#define SWIPE_REAR_ONE 0x5e         // First colour of rear PSI
#define SWIPE_REAR_TWO 0x24         // Second colour of rear PSI
#define SWIPE_DELAY 500             // Base delay between swipes

#ifdef USE_DEBUG
 #define PSI_DEBUG
#endif


/**
  * \ingroup Dome
  *
  * \class PSIMatrix
  *
  * \brief PSIMatrix by Darren Poulson <daz@r2djp.co.uk>
  *
  * The PSI Matrix devices are based on i2c RGB LED Matrix from Seeed Studios. This class
  * configures them to be used as a PSI using Software I2C to create a new bus.
  *
  * Example Code:
  * \code
  *  PSIMatrix rearPSI(22,23,2);
  *  PSIMatrix frontPSI(24,25,1);
  * \endcode
  *
  */
class PSIMatrix :
    public SoftwareWire, AnimatedEvent, SetupEvent, CommandEvent
{
public:
    enum EffectValue
    {
        kNormalVal = 0
    };

    enum Sequence
    {
        kNormal = 0,
        kSolid = 1,
        kHeart = 2,
        kMalf = 3,
    };

    enum HoloID
    {
        /** Front PSI ID */
        kFrontPSI = 1,
        /** Rear PSI ID */
        kRearPSI = 2,
    };

    virtual void setup() override
    {
    }

    /** \brief Constructor
    *
    */
    PSIMatrix(const byte sdaPin, const byte sclPin, const byte psi) :
	SoftwareWire(sdaPin, sclPin),
	fPSI(psi)
    {
#ifdef PSI_DEBUG
    DEBUG_PRINTF("PSI#");
    DEBUG_PRINTLN(fPSI);
#endif
	
    }


    /**
      * Select the specified effect using a 32-bit integer.
      *
      *  \li Sequence (0-99) * 10000
      *  \li Speed (0-9) * 100
      *  \li Duration (0-99)
      */
    void selectEffect(long inputNum)
    {
    }

    /** 
     * Command Prefix: PS
     *
     */
    virtual void handleCommand(const char* cmd) override
    {
        if (cmd[0] != 'P' || cmd[1] != 'S')
            return;

        long int cmdvalue = 0;
        const char* c = &cmd[2];
        while (*c >= '0' && *c <= '9')
        {
            cmdvalue = cmdvalue * 10 + (*c++ - '0');
        }
        selectEffect(cmdvalue);
    }

    /**
      * Select the specified effect sequence.
      */
    inline void setSequence(Sequence seq = kNormal, uint8_t speedScale = 0, uint8_t numSeconds = 0)
    {
        selectEffect((long int)seq * 10000L + (long int)speedScale * 100 + numSeconds);
    }

    /**
      * Perform a single frame of LED animation based on the selected sequence.
      */
    virtual void animate() override
    {
        unsigned long currentMillis = millis();
	if (currentMillis >= fSwipeSpeed)
	{
	    swipe_main(swipe_position,100, true, 1);
	    if (swipe_direction == 0) {
                if (swipe_position > 7) {
                    swipe_direction = 1;
		    fSwipeSpeed = currentMillis + random(500, 2000);
                    swipe_position--;
                } else {
		    fSwipeSpeed = fSwipeMillis + SWIPE_SPEED;
                    swipe_position++;
                }
            } else {
                if (swipe_position <= 0) {
                    swipe_direction = 0;
		    fSwipeSpeed = currentMillis + random(500, 2000);
                    swipe_position++;
                } else {
		    fSwipeSpeed = fSwipeMillis + SWIPE_SPEED;
                    swipe_position--;
                }
            }
	}

    }

private:
    int fPSI;
    int swipe_direction = 0;
    int swipe_position = 0;
    unsigned long fSwipeSpeed;
    unsigned long fSwipeMillis;

    // Heart animation
    uint64_t heart[32] = {

      0xff0000ffff0000ff,
      0x0000000000000000,
      0x0000000000000000,
      0x0000000000000000,
      0xff000000000000ff,
      0xffff00000000ffff,
      0xffffff0000ffffff,
      0xffffffffffffffff,

      0xffffffffffffffff,
      0xffff00ffff00ffff,
      0xff000000000000ff,
      0xff000000000000ff,
      0xffff00000000ffff,
      0xffffff0000ffffff,
      0xffffffffffffffff,
      0xffffffffffffffff,

      0xffffffffffffffff,
      0xffffffffffffffff,
      0xffff00ffff00ffff,
      0xffff00000000ffff,
      0xffffff0000ffffff,
      0xffffffffffffffff,
      0xffffffffffffffff,
      0xffffffffffffffff,

      0xffffffffffffffff,
      0xffff00ffff00ffff,
      0xff000000000000ff,
      0xff000000000000ff,
      0xffff00000000ffff,
      0xffffff0000ffffff,
      0xffffffffffffffff,
      0xffffffffffffffff
    };

    /// \private
    int i2cSendBytes(uint8_t *data, uint8_t len)
    {
        int ret = 0;
        beginTransmission(DISPLAY_ADDRESS);
        ret = write(data, len);
        endTransmission();
        return ret;
    }

    /// \private
    void displayFrames(uint64_t *buffer, uint16_t duration_time, bool forever_flag, uint8_t frames_number)
    {
        int ret = 0;
        uint8_t data[72] = {0, };
        // max 5 frames in storage
        if (frames_number > 5) frames_number = 5;
        else if (frames_number == 0) return;

        data[0] = 0x05;
        data[1] = 0x0;
        data[2] = 0x0;
        data[3] = 0x0;
        data[4] = frames_number;

        for (int i=frames_number-1;i>=0;i--)
        {
            data[5] = i;
            // different from uint8_t buffer
            for (int j = 0; j< 8; j++)
            {
                for (int k = 7; k >= 0; k--)
                {
                     data[8+j*8+(7-k)] = ((uint8_t *)buffer)[j*8+k+i*64];
                }
            }

            if (i == 0)
            {
                // display when everything is finished.
                data[1] = (uint8_t)(duration_time & 0xff);
                data[2] = (uint8_t)((duration_time >> 8) & 0xff);
                data[3] = forever_flag;
            }

            i2cSendBytes(data, 72);
        }
    }

    /// \private
    void displayFrames(uint8_t *buffer, uint16_t duration_time, bool forever_flag, uint8_t frames_number)
    {
        uint8_t data[72] = {0, };
        // max 5 frames in storage
        if (frames_number > 5) frames_number = 5;
        else if (frames_number == 0) return;

        data[0] = 0x05;
        data[1] = 0x0;
        data[2] = 0x0;
        data[3] = 0x0;
        data[4] = frames_number;

        for (int i=frames_number-1;i>=0;i--)
        {
            data[5] = i;
            for (int j=0;j<64;j++) data[8+j] = buffer[j+i*64];
            if (i == 0)
            {
                // display when everything is finished.
                data[1] = (uint8_t)(duration_time & 0xff);
                data[2] = (uint8_t)((duration_time >> 8) & 0xff);
                data[3] = forever_flag;
            }
            i2cSendBytes(data, 72);
        }
    }

    /// \private
    void swipe_main(uint8_t pos, uint16_t duration_time, bool forever_flag, uint8_t frames_number)
    {
        int ret = 0;
        int colour = 0;
        uint8_t data[72] = {0, };

        data[0] = 0x05;
        data[1] = 0x0;
        data[2] = 0x0;
        data[3] = 0x0;
        data[4] = frames_number;
        data[5] = 0;
        // different from uint8_t buffer
        for (int j = 0; j< 8; j++)
        {
            for (int k = 7; k >= 0; k--)
            {
                if (pos > k) {
                    if (fPSI == 1) {
                        colour = SWIPE_FRONT_ONE;
                    } else {
                        colour = SWIPE_REAR_ONE;
                    }
                } else {
                    if (fPSI == 1) {
                        colour = SWIPE_FRONT_TWO;
                    } else {
                        colour = SWIPE_REAR_TWO;
                    }
                }
                //data[8+j*8+(7-k)] = ((uint8_t *)buffer)[j*8+k+i*64];
                data[8+j*8+(7-k)] = colour;
            }
        }
        data[1] = (uint8_t)(duration_time & 0xff);
        data[2] = (uint8_t)((duration_time >> 8) & 0xff);
        data[3] = forever_flag;
#ifdef PSI_DEBUG
        DEBUG_PRINTF("SWIPE: Position - ");
        DEBUG_PRINT(swipe_position);
	DEBUG_PRINTF(" Direction - ");
	DEBUG_PRINTLN(swipe_direction);
#endif
        i2cSendBytes(data, 72);

    }

    /// \private
    // Routine to display a pulsing heart
    void do_heart(uint8_t cycles, uint8_t pulse_speed) 
    {
       for(int i=0;i<cycles;i++) 
       {
           for (int i=0;i<4;i++) 
	   {
              displayFrames(heart+i*8, 100, true, 1);
              delay(pulse_speed);
           }
       }
    }

    /// \private
    // Routine to do <cycles> of random pixels. As it loops through <cycles>
    // the frequency of pixels reduces
    void do_random(uint8_t cycles, uint8_t pulse_speed) 
    {
        uint8_t data[64] = {0, };
        int pixel;
        int colour;
        for(int i=0;i<=cycles;i++) 
	{
            for (int j = 0; j< 8; j++) 
	    {
                for (int k = 7; k >= 0; k--) 
		{
                    pixel = random(0,cycles);
                    if (pixel < cycles-i) 
		    {
                        // Pixel on
                        colour = random(0,250);
		    } else {
                        colour = 0xff;
                    }
                    data[j*8+(7-k)] = colour;
                }
            }
            displayFrames(data, 100, true, 1);
            delay(pulse_speed);
        }
        uint8_t blank[64] = {0xff, };
        for( int i = 0; i < sizeof(blank);  ++i )
            blank[i] = (char)255;
        displayFrames(blank, 100, true, 1);
        delay(2000);
    }


};

#endif
