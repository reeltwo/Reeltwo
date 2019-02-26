#ifndef ServoDispatchPCA9685_h
#define ServoDispatchPCA9685_h

#include "ServoDispatch.h"
#include <Wire.h>

#ifdef USE_SERVO_DEBUG
#define SERVO_DEBUG_PRINT(s) DEBUG_PRINT(s)
#define SERVO_DEBUG_PRINTLN(s) DEBUG_PRINTLN(s)
#define SERVO_DEBUG_PRINT_HEX(s) DEBUG_PRINT_HEX(s)
#define SERVO_DEBUG_PRINTLN_HEX(s) DEBUG_PRINTLN_HEX(s)
#else
#define SERVO_DEBUG_PRINT(s)
#define SERVO_DEBUG_PRINTLN(s)
#define SERVO_DEBUG_PRINT_HEX(s)
#define SERVO_DEBUG_PRINTLN_HEX(s)
#endif

#ifdef USE_VERBOSE_SERVO_DEBUG
#define VERBOSE_SERVO_DEBUG_PRINT(s) DEBUG_PRINT(s)
#define VERBOSE_SERVO_DEBUG_PRINTLN(s) DEBUG_PRINTLN(s)
#define VERBOSE_SERVO_DEBUG_PRINT_HEX(s) DEBUG_PRINT_HEX(s)
#define VERBOSE_SERVO_DEBUG_PRINTLN_HEX(s) DEBUG_PRINTLN_HEX(s)
#else
#define VERBOSE_SERVO_DEBUG_PRINT(s)
#define VERBOSE_SERVO_DEBUG_PRINTLN(s)
#define VERBOSE_SERVO_DEBUG_PRINT_HEX(s)
#define VERBOSE_SERVO_DEBUG_PRINTLN_HEX(s)
#endif

//#define SERVO_DEBUG
//#define VERBOSE_SERVO_DEBUG
//chip registers and default values
#define PCA9685_MODE1 0x00
#define PCA9685_MODE2 0x01
#define PCA9685_SWRST_ADDR 0X00
#define PCA9685_SWRST_ACK 0x06
#define PCA9685_SWRST_NOACK 0x00
#define PCA9685_AI 0x20
#define PCA9685_ALLCALL 0x01
#define PCA9685_PRESCALE 0xFE
#define PCA9685_SLEEP 0x10
#define PCA9685_ALLCALLADR 0x70
#define PCA9685_ALL_LED_ON_L 0xFA
#define PCA9685_ALL_LED_ON_H 0xFB
#define PCA9685_ALL_LED_OFF_L 0xFC
#define PCA9685_ALL_LED_OFF_H 0xFD
#define PCA9685_DEFAULT_PRESCALE_VALUE 0x1A
#define LED0_ON_L 0x06
#define LED0_ON_H 0x07
#define LED0_OFF_L 0x08
#define LED0_OFF_H 0x09
#define LED_FULL_OFF_L 0x00
#define LED_FULL_OFF_H 0x10

#define POPULATED_CHANNEL_NUMBER 32
#define MIN_PWM_LENGTH 500
#define MAX_PWM_LENGTH 2500
#define DEFAULT_CHANNEL_STAGGERING false
#define CHANNEL_OFFSET_STEP 50 //in us. This value should not extend the last channel off timing to exceed the 4095 limit.
#define NOMINATED_ROOM_TEMPERATURE 25
#define DEFAULT_TEMPERATURE_CORRECTION 40 //per 30 degrees change in temperature
#define DEFAULT_SERVO_PWM_LENGTH 1500 
#define NOMINAL_CLOCK_FREQUENCY 25000000
#define DEFAULT_UPDATE_FREQUENCY 50
#define TEMPERATURE_CORRECTION_COEFFICIENT 0.00020 
#define TEMPERATURE_CORRECTION_STEP 128
#define TEMPERATURE_CORRECTION_POINTS ((MAX_PWM_LENGTH-MIN_PWM_LENGTH)/TEMPERATURE_CORRECTION_STEP)

/**
  * \ingroup Core
  *
  * \class ServoDispatchPCA9685
  *
  * \brief Implements ServoDispatch over i2c to PCA9685
  *
  * Implements ServoDispatch and spreads the PWM output to up to 62 PCA9685 modules (via i2c address 0x40-0x75).
  * Theoretically allowing you to control up to 992 PWM outputs.
  */
template <uint16_t numServos>
class ServoDispatchPCA9685 : public ServoDispatch, SetupEvent, AnimatedEvent
{
private:
	struct ServoState;
public:
	/**
	  * \brief Constructor
	  */
	ServoDispatchPCA9685(TwoWire* i2c, const ServoSettings* settings) :
		fI2C(i2c),
		fOutputEnablePin(-1),
		fOutputEnabled(false),
		fOutputExpireMillis(0),
		fLastTime(0)
	{
		for (byte chip = 0; chip < numberOfPCA9685Chips(); chip++)
		{
			fI2CAddress[chip] = 0x40 + chip;
			fClocks[chip] = NOMINAL_CLOCK_FREQUENCY;
			fTargetUpdateFrequency[chip] = DEFAULT_UPDATE_FREQUENCY;
		}
		memset(fServos, '\0', sizeof(fServos));
		for (uint8_t servoChannel = 0; servoChannel < SizeOfArray(fLastLength); servoChannel++)
		{
			fLastLength[servoChannel] = DEFAULT_SERVO_PWM_LENGTH;
		}
		for (uint16_t i = 0; i < numServos; i++)
		{
			ServoState* state = &fServos[i];
			state->channel = pgm_read_word(&settings[i].pinNum);
			state->group = pgm_read_dword(&settings[i].group);
			state->minPulse = pgm_read_word(&settings[i].minPulse);
			state->maxPulse = pgm_read_word(&settings[i].maxPulse);
			fLastLength[state->channel - 1] = state->minPulse;
			state->posNow = state->minPulse;
			state->init();
		}
	}

	/**
	  * \brief Constructor
	  */
	ServoDispatchPCA9685(const ServoSettings* settings) :
	#ifdef ARDUINO_SAM_DUE
		ServoDispatchPCA9685(&Wire1, settings)
	#else
		ServoDispatchPCA9685(&Wire, settings)
	#endif
	{
	}

	inline byte numberOfPCA9685Chips()
	{
		return numServos / 16 + 1;
	}

	virtual uint16_t getNumServos() override
	{
		return numServos;
	}

	void ensureEnabled()
	{
		if (!fOutputEnabled)
		{
		#ifdef SERVO_DEBUG
			DEBUG_PRINTLN("OUTPUT ENABLED");
		#endif
			setOutputAll(false);
			digitalWrite(fOutputEnablePin, HIGH);
			fOutputEnabled = true;
		}
	}

	void setOutputEnablePin(const byte outputEnablePin)
	{
		fOutputEnablePin = outputEnablePin;
	}

	void setClockCalibration(uint32_t clock[(numServos/16)+1])
	{
		for (byte i = 0; i < numberOfPCA9685Chips(); i++)
		{
			fClocks[i] = clock[i];
		}
	}

	virtual uint16_t getMinimum(uint16_t num) override
	{
		return (num < numServos) ? fServos[num].minPulse : 0;
	}

	virtual uint16_t getMaximum(uint16_t num) override
	{
		return (num < numServos) ? fServos[num].maxPulse : 0;
	}

	virtual uint16_t currentPos(uint16_t num) override
	{
		return (num < numServos) ? fServos[num].currentPos() : 0;
	}

  	virtual void setup()
  	{
		reset();

		//Enable Auto-Increment and all call.
		for (byte i = 0; i < numberOfPCA9685Chips(); i++)
		{
			writeRegister(i, PCA9685_MODE1, PCA9685_AI | PCA9685_ALLCALL);
		}

		setChannelStaggering(DEFAULT_CHANNEL_STAGGERING);
		//Initialize temperature correction array
		for (uint8_t i = 0; i < SizeOfArray(fTemperatureCorrectionArray); i++)
		{
			fTemperatureCorrectionArray[i] = 0;
		}

		setUpdateFrequency(fTargetUpdateFrequency);

		setOutputAll(false);

	#ifdef SERVO_DEBUG
		SERVO_DEBUG_PRINTLN("PCA9685 initialization completed.");
	#endif

  		if (fOutputEnablePin != -1)
  		{
			pinMode(fOutputEnablePin, OUTPUT);
			digitalWrite(fOutputEnablePin, LOW);
		}
  	}

  	virtual void animate()
  	{
  		if (fOutputEnabled)
  		{
	  		uint32_t now = millis();
	  		if (fLastTime + 1 < now)
	  		{
		  		for (int i = 0; i < numServos; i++)
		  		{
		  			if (fServos[i].channel != 0)
		  				fServos[i].move(this, now);
		  		}
		  		fLastTime = now = millis();
		  	}
	  		if (now > fOutputExpireMillis)
	  		{
	  			SERVO_DEBUG_PRINTLN("POWER OFF");
				//digitalWrite(fOutputEnablePin, LOW);
				setOutputAll(false);
	  			fOutputEnabled = false;
	  		}
	  	}
  	}

	//Set output signal to on or off. Any PWM input command will turn the signal on. 
	void setOutput(uint16_t servoChannel, bool state)
	{
		if (servoChannel > SizeOfArray(fLastLength))
			return;
		if (state)
		{
			setPWM(servoChannel, fLastLength[servoChannel - 1]);
			SERVO_DEBUG_PRINT("Set servo channel ");
			SERVO_DEBUG_PRINT(servoChannel);
			SERVO_DEBUG_PRINTLN(" output on");
		}
		else
		{
			SERVO_DEBUG_PRINT("Set servo channel ");
			SERVO_DEBUG_PRINT(servoChannel);
			SERVO_DEBUG_PRINTLN(" output off");

			uint8_t chip = (servoChannel - 1) / 16;
			uint8_t channel = (servoChannel - 1) % 16;
			fI2C->beginTransmission(fI2CAddress[chip]);
			fI2C->write(LED0_OFF_L + 4 * channel);
			fI2C->write(LED_FULL_OFF_L);
			fI2C->write(LED_FULL_OFF_H);
			fI2C->endTransmission();
		}
	}

protected:

	// Return the prescale.
	uint8_t getPrescale(uint8_t chipNumber)
	{
		return readRegister(chipNumber, PCA9685_PRESCALE);
	}

	//set ambient temperature for temperature correction
	void setEnvironmentTemperatureCelsius(int8_t degreesInCelsius)
	{
		fDeltaTemperature = degreesInCelsius - NOMINATED_ROOM_TEMPERATURE;
		SERVO_DEBUG_PRINTLN("Temperature correction at each checkpoint:");
		//Initialize temperature correction array
		for (uint8_t i = 0; i < TEMPERATURE_CORRECTION_POINTS; i++)
		{
			uint16_t targetPWMLength = MIN_PWM_LENGTH +  i * TEMPERATURE_CORRECTION_STEP;
			fTemperatureCorrectionArray[i] = (float)fDeltaTemperature * TEMPERATURE_CORRECTION_COEFFICIENT* (float)targetPWMLength;
			VERBOSE_SERVO_DEBUG_PRINT(targetPWMLength);
			VERBOSE_SERVO_DEBUG_PRINT(" ");
			VERBOSE_SERVO_DEBUG_PRINTLN(fTemperatureCorrectionArray[i]);
		}
	}

	//if set true, output channels will stagger (phase shift) by the amout of time (us) defined in CHANNEL_OFFSET_STEP
	void setChannelStaggering(bool value)
	{
		for (uint16_t index = 0; index < SizeOfArray(fChannelOffset); index++)
		{
			fChannelOffset[index] = (value) ? CHANNEL_OFFSET_STEP * (index % 16) : 0;
		}
	}

	float getNominalUpdateFrequency(uint32_t clockFrequency, uint8_t prescale)
	{
		float nominalUpdateFrequency;
		nominalUpdateFrequency = (float)clockFrequency / (float)(((uint32_t)(prescale+1)) << 12);
		return nominalUpdateFrequency;
	}

	void setClockFrequency(const uint32_t clocks[(numServos/16)+1])
	{
		for (byte chip = 0; chip < numberOfPCA9685Chips(); chip++)
		{
			fClocks[chip] = clocks[chip];
		}
	}

	void setUpdateFrequency(const float targetUpdateFrequency[(numServos/16)+1])
	{
		for (byte chip = 0; chip < numberOfPCA9685Chips(); chip++)
		{
			fTargetUpdateFrequency[chip] = targetUpdateFrequency[chip];
			SERVO_DEBUG_PRINT("Set target update frequency[");
			SERVO_DEBUG_PRINT(chip);
			SERVO_DEBUG_PRINT("] ");
			SERVO_DEBUG_PRINT(targetUpdateFrequency[chip]);
			SERVO_DEBUG_PRINTLN();

			uint8_t prescale = getCalculatedPrescale(fClocks[chip], fTargetUpdateFrequency[chip]);

			uint8_t currentMode;
			uint8_t sleepMode;

			currentMode = readRegister(chip, PCA9685_MODE1); //read current MODE1 register
			sleepMode = (currentMode & 0x7F) | PCA9685_SLEEP;
			writeRegister(chip, PCA9685_MODE1, sleepMode);
			writeRegister(chip, PCA9685_PRESCALE, prescale); //change the prescaler register
			writeRegister(chip, PCA9685_MODE1, currentMode);

			delay(1);

			SERVO_DEBUG_PRINT("Set prescale[");
			SERVO_DEBUG_PRINT(chip);
			SERVO_DEBUG_PRINT("] : ");
			SERVO_DEBUG_PRINTLN(prescale);

			prescale = getPrescale(chip);
			fCalculatedUpdateFrequency[chip] = fClocks[chip] / (float)(((uint32_t)prescale + 1) << 12);
			fCalculatedResolution[chip] = 1000000.0 / (fCalculatedUpdateFrequency[chip] * 4096);

			SERVO_DEBUG_PRINT("Calculated update frequency = ");
			SERVO_DEBUG_PRINTLN(fCalculatedUpdateFrequency[chip]);
		}
	}

	uint8_t getCalculatedPrescale(uint32_t calculatedClockFrequency, float targetUpdateFrequency)
	{
		//rounding method from here: https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library/issues/40
		uint32_t calculatedPrescale;
		calculatedPrescale = (float)calculatedClockFrequency * 100.0 / (4096.0 * targetUpdateFrequency);
		if ((calculatedPrescale % 100) >= 50)
			calculatedPrescale += 100;
		calculatedPrescale = (calculatedPrescale / 100) - 1;
		return (uint8_t)calculatedPrescale;
	}

	uint16_t getCalibratedSteps(uint8_t chip, uint16_t targetLength)
	{
		return (chip < numberOfPCA9685Chips()) ? (targetLength / fCalculatedResolution[chip]) + 0.5 : 0;
	}

	float getCalculatedResolution(uint8_t chip)
	{
		return (chip < numberOfPCA9685Chips()) ? fCalculatedResolution[chip] : 0;
	}

private:
	TwoWire *fI2C;
	uint8_t fI2CAddress[(numServos/16)+1];
	float fCalculatedUpdateFrequency[(numServos/16)+1];
	uint32_t fClocks[(numServos/16)+1];
	float fTargetUpdateFrequency[(numServos/16)+1];
	float fCalculatedResolution[(numServos/16)+1];
	int8_t fDeltaTemperature = 0;
	int8_t fTemperatureCorrection = DEFAULT_TEMPERATURE_CORRECTION;
	int8_t fTemperatureCorrectionArray[TEMPERATURE_CORRECTION_POINTS];
	uint16_t fChannelOffset[((numServos/16)+1)*16];	//array starts from index 0, servo channel starts from index 1
	uint16_t fLastLength[((numServos/16)+1)*16];

	int fOutputEnablePin;
	bool fOutputEnabled;
	uint32_t fOutputExpireMillis;
	uint32_t fLastTime;

	struct ServoState
	{
		uint16_t currentPos()
		{
			return posNow;
		}

		void move(ServoDispatchPCA9685<numServos>* dispatch, uint32_t timeNow)
		{
			if (finishTime != 0)
			{
				if (timeNow < startTime)
				{
					/* wait */
				}
				else if (timeNow >= finishTime)
				{
					posNow = finishPos;
					doMove(dispatch, timeNow);
					init();
				}
				else if (lastMoveTime != timeNow)
				{
					uint32_t timeSinceLastMove = timeNow - lastMoveTime;
					uint32_t denominator = finishTime - lastMoveTime;
					float fractionChange = float(timeSinceLastMove)/float(denominator);

					int distanceToGo = finishPos - posNow;
					float distanceToMove = float(distanceToGo) * fractionChange;
					int distanceToMoveInt = int(distanceToMove);

					if (abs(distanceToMoveInt) > 1)
					{
						posNow = posNow + distanceToMoveInt;
						doMove(dispatch, timeNow);
					}
				}
			}
		}

		void moveTo(ServoDispatchPCA9685<numServos>* dispatch, uint32_t startDelay, uint32_t moveTime, uint16_t startPos, uint16_t pos)
		{
			uint32_t timeNow = millis();

			startTime = startDelay + timeNow;
			finishTime = moveTime + startTime;
			finishPos = min(maxPulse, max(minPulse, pos));
			posNow = startPos;
			doMove(dispatch, timeNow);
		}

		uint8_t channel;
		uint32_t group;
		uint16_t minPulse;
		uint16_t maxPulse;
		uint32_t startTime;
		uint32_t finishTime;
		uint32_t lastMoveTime;
		uint16_t finishPos;
		uint16_t posNow;

		void doMove(ServoDispatchPCA9685<numServos>* dispatch, uint32_t timeNow)
		{
			SERVO_DEBUG_PRINT("PWM ");
			SERVO_DEBUG_PRINT(channel);
			SERVO_DEBUG_PRINT(" ");
			SERVO_DEBUG_PRINTLN(posNow);

			dispatch->setPWM(channel, posNow);
		#ifdef USE_SMQ
	        SMQ::send_start(F("PWM"));
	        SMQ::send_uint8(F("num"), channel);
	        SMQ::send_float(F("len"), posNow);
	        SMQ::send_end();
	    #endif
			lastMoveTime = timeNow;
		}

		void init()
		{
			finishTime = 0;
			lastMoveTime = 0;
			finishPos = 0;
		}
	};

	ServoState fServos[numServos];

    /////////////////////////////////////////////////////////////////////////////////

	virtual void _moveServoTo(uint16_t num, uint32_t startDelay, uint32_t moveTime, uint16_t startPos, uint16_t pos) override
	{
		if (num < numServos && fServos[num].channel != 0)
		{
			ensureEnabled();
			fServos[num].moveTo(this, startDelay, moveTime, startPos, pos);
			fOutputExpireMillis = millis() + startDelay + moveTime + 500;
			fLastTime = 0;
		}
	}

    /////////////////////////////////////////////////////////////////////////////////

	// Move all servos matching servoGroupMask starting at startDelay in moveTimeMin-moveTimeMax to position pos 
	virtual void _moveServosTo(uint32_t servoGroupMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, uint16_t pos) override
	{
		for (uint16_t i = 0; i < numServos; i++)		
		{
			uint32_t moveTime = (moveTimeMin != moveTimeMax) ? random(moveTimeMin, moveTimeMax) : moveTimeMax;
			if ((fServos[i].group & servoGroupMask) != 0)
			{
				moveTo(i, startDelay, moveTime, fServos[i].currentPos(), pos);
			}
		}
	}

    virtual void _moveServosBy(uint32_t servoGroupMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, int16_t pos) override
    {
        for (uint16_t i = 0; i < numServos; i++)        
        {
            uint32_t moveTime = (moveTimeMin != moveTimeMax) ? random(moveTimeMin, moveTimeMax) : moveTimeMax;
            if ((fServos[i].group & servoGroupMask) != 0)
            {
                int16_t curpos = fServos[i].currentPos();
                moveTo(i, startDelay, moveTime, curpos, curpos + pos);
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////

    virtual void _moveServoSetTo(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, uint16_t onPos, uint16_t offPos) override
    {
        byte bitShift = 31;
        for (uint16_t i = 0; i < numServos; i++)
        {
            if ((fServos[i].group & servoGroupMask) == 0)
                continue;
            uint32_t moveTime = (moveTimeMin != moveTimeMax) ? random(moveTimeMin, moveTimeMax) : moveTimeMax;
            bool on = ((servoSetMask & (1L<<bitShift)) != 0);
            moveTo(i, startDelay, moveTime, fServos[i].currentPos(), (on) ? onPos : offPos);
            if (bitShift-- == 0)
                break;
        }
    }

    virtual void _moveServoSetBy(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, int16_t onPos, int16_t offPos) override
    {
        byte bitShift = 31;
        for (uint16_t i = 0; i < numServos; i++)        
        {
            if ((fServos[i].group & servoGroupMask) == 0)
                continue;
            uint32_t moveTime = (moveTimeMin != moveTimeMax) ? random(moveTimeMin, moveTimeMax) : moveTimeMax;
            bool on = ((servoSetMask & (1L<<bitShift)) != 0);
            int16_t curpos = fServos[i].currentPos();
            moveTo(i, startDelay, moveTime, curpos, curpos + ((on) ? onPos : offPos));
            if (bitShift-- == 0)
                break;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////

	uint8_t readRegister(uint8_t chipNumber, uint8_t registerAddress)
	{
		fI2C->beginTransmission(fI2CAddress[chipNumber]);
		fI2C->write(registerAddress);
		fI2C->endTransmission();
		fI2C->requestFrom(fI2CAddress[chipNumber], (uint8_t)1);
		return fI2C->read();
	}

	void writeRegister(uint8_t chipNumber, uint8_t registerAddress, uint8_t value)
	{
		fI2C->beginTransmission(fI2CAddress[chipNumber]);
		fI2C->write(registerAddress);
		fI2C->write(value);
		fI2C->endTransmission();
	}

	void reset()
	{
		//perform a software reset to both chips with reserved address PCA9685_SWRST_ADDR
		fI2C->beginTransmission(PCA9685_SWRST_ADDR);
		fI2C->write(PCA9685_SWRST_ACK);
		fI2C->endTransmission();
	}

	//Command a servo position.
	void setPWM(uint16_t servoChannel, uint16_t targetLength)
	{
		if (servoChannel > SizeOfArray(fLastLength))
			return;
		VERBOSE_SERVO_DEBUG_PRINT("Target ");
		VERBOSE_SERVO_DEBUG_PRINT(targetLength);

		//perform frequency adjustment
		uint16_t calibratedSteps;
		uint16_t calculatedLength;

		if (targetLength < MIN_PWM_LENGTH)
			targetLength = MIN_PWM_LENGTH;
		if (targetLength > MAX_PWM_LENGTH)
			targetLength = MAX_PWM_LENGTH;
		fLastLength[servoChannel - 1] = targetLength;

		//Perform temperature correction
		uint8_t chip = (servoChannel - 1) / 16;
		uint8_t channel = (servoChannel - 1) % 16;
		uint16_t temperatureCorrectedTargetLength = targetLength +
			fTemperatureCorrectionArray[(targetLength - MIN_PWM_LENGTH) >>7];
		uint16_t on = fChannelOffset[servoChannel - 1];
		uint16_t off = on + calibratedSteps;

		VERBOSE_SERVO_DEBUG_PRINT(" I2C: 0x");
		VERBOSE_SERVO_DEBUG_PRINT_HEX(fI2CAddress[chip]);
		VERBOSE_SERVO_DEBUG_PRINT(" channel: ");
		VERBOSE_SERVO_DEBUG_PRINT(channel);

		fI2C->beginTransmission(fI2CAddress[chip]);
		fI2C->write(LED0_ON_L + 4 * channel);
		fI2C->write(on);
		fI2C->write(on >> 8);
		fI2C->write(off);
		fI2C->write(off >> 8);
		fI2C->endTransmission();

		VERBOSE_SERVO_DEBUG_PRINT(" Channel ");
		VERBOSE_SERVO_DEBUG_PRINT(servoChannel);
		VERBOSE_SERVO_DEBUG_PRINT(" on ");
		VERBOSE_SERVO_DEBUG_PRINT(on);
		VERBOSE_SERVO_DEBUG_PRINT(" off ");
		VERBOSE_SERVO_DEBUG_PRINTLN(off);
	}

	// Set all channels to the command target PWM position.
	void setPWMAll(uint16_t targetLength)
	{
		if (targetLength < MIN_PWM_LENGTH)
			targetLength = MIN_PWM_LENGTH;
		if (targetLength > MAX_PWM_LENGTH)
			targetLength = MAX_PWM_LENGTH;

		uint16_t on = 0;
		uint16_t off = targetLength +
			fTemperatureCorrectionArray[(targetLength - MIN_PWM_LENGTH) >> 7];
	#if 1
		uint8_t servoChannel = 1;
		/*sequenced writing method*/
		VERBOSE_SERVO_DEBUG_PRINT("Set PWM all ");
		VERBOSE_SERVO_DEBUG_PRINT(on);
		VERBOSE_SERVO_DEBUG_PRINT(" ");
		VERBOSE_SERVO_DEBUG_PRINTLN(off);
		for (byte chip = 0; chip < numberOfPCA9685Chips(); chip++)
		{
			for (uint8_t i = 0; i < 16; i++, servoChannel++)
			{
				uint16_t channelOn = on;
				uint16_t channelOff = getCalibratedSteps(chip, off);
				fI2C->beginTransmission(fI2CAddress[chip]);
				fI2C->write(LED0_ON_L + 4 * i);
				fI2C->write(channelOn);
				fI2C->write(channelOn >> 8);
				fI2C->write(channelOff);
				fI2C->write(channelOff >> 8);
				fI2C->endTransmission();
				fLastLength[i] = off - on;

				VERBOSE_SERVO_DEBUG_PRINT("Set channel ");
				VERBOSE_SERVO_DEBUG_PRINT(servoChannel);
				VERBOSE_SERVO_DEBUG_PRINT(" ");
				VERBOSE_SERVO_DEBUG_PRINT(channelOn);
				VERBOSE_SERVO_DEBUG_PRINT(" ");
				VERBOSE_SERVO_DEBUG_PRINTLN(channelOff);
			}
		}
	#else
		/* all call method */
		fI2C->beginTransmission(PCA9685_ALLCALLADR);
		fI2C->write(PCA9685_ALL_LED_ON_L);
		fI2C->write(on);
		fI2C->write(on >> 8);
		fI2C->write(off);
		fI2C->write(off >> 8);
		fI2C->endTransmission();
	#endif
	}


	void setOutputAll(bool state)
	{
		if (state)
		{
			for (uint16_t servoChannel = 1; servoChannel <= numServos; servoChannel++)
			{
				setPWM(servoChannel, fLastLength[servoChannel - 1]);
			}
			SERVO_DEBUG_PRINTLN("Set all channels on");
			return;
		}
		fI2C->beginTransmission(PCA9685_ALLCALLADR);
		fI2C->write(PCA9685_ALL_LED_OFF_L);
		fI2C->write(LED_FULL_OFF_L);
		fI2C->write(LED_FULL_OFF_H);
		fI2C->endTransmission();
		SERVO_DEBUG_PRINTLN("Set all channels off");
	}

	int mapPulselength(double microseconds)
	{
		// The Adafruit PWM Driver library overshoots the frequency by a factor of 1/0.9 
		// therefore we must multiply the frequency value of 60 by 0.9 to correct for this
		// resulting in a more actuate map
		double pulselength = 4.521;  // (1000000/(60.00*0.9))/4096
		int pulse;
		pulse = microseconds/pulselength;
		return pulse;
	}
};

#endif
