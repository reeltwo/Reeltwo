#include "ReelTwo.h"
#include "core/AnimatedEvent.h"

#ifndef BatteryMonitor_h
#define BatteryMonitor_h

class BatteryMonitor: public AnimatedEvent
{
public:
	/**
	 * Creates an instance to monitor battery voltage and level.
	 * Initialization parameters depend on battery type and configuration.
	 *
	 * @param pin is the analog pin used for sensing the battery voltage
	 * @param minVoltage is the voltage, expressed in millivolts, corresponding to an empty battery
	 * @param maxVoltage is the voltage, expressed in millivolts, corresponding to a full battery
	 * @param scale is the multiplier used to obtain the real battery voltage in milli volts
	 * @param mapFunction is a pointer to the function used to map the battery voltage to the remaining capacity percentage (defaults to linear mapping)
	 */
	BatteryMonitor(
			uint8_t pin,
			uint16_t minVoltage = 3000,
			uint16_t maxVoltage = 4200,
			float scale = 2 * 3.3 * 1000,
			unsigned (*mapping)(unsigned, unsigned, unsigned) = sigmoidal) :
		fPin(pin),
		fMinVoltage(minVoltage),
		fMaxVoltage(maxVoltage),
		fScale(scale),
		fMapping(mapping)
	{
	}

	inline bool isBatteryPowered()
	{
		return isBatteryPowered(voltage());
	}

	inline bool isBatteryPowered(uint32_t voltage)
	{
		return voltage < fMaxVoltage;
	}

	inline void setRefreshCallback(uint32_t millis, void (*callback)())
	{
		fRefreshMillis = millis;
		fCallback = callback;
	}

	inline void setMapping(unsigned (*mapping)(unsigned, unsigned, unsigned))
	{
		fMapping = mapping;
	}

	/**
	 * Returns the current battery level as a number between 0 and 100, with 0 indicating an empty battery and 100 a
	 * full battery.
	 */
	inline unsigned level()
	{
		return level(voltage());
	}

	unsigned level(uint32_t voltage)
	{
		if (voltage <= fMinVoltage)
			return 0;
		if (voltage >= fMaxVoltage)
			return 100;
		return (*fMapping)(voltage, fMinVoltage, fMaxVoltage);
	}

	/**
	 * Returns the current battery voltage in millivolts.
	 */
	uint32_t voltage()
	{
	#ifdef ESP32
        uint32_t mV = (analogRead(fPin) * fScale) / 4096;
    #else
		uint32_t mV = (analogRead(fPin) * fScale) / 1024;
    #endif
		// Cache value
		fMilliVolts = mV;
		return mV;
	}

    virtual void animate() override
    {
    	if (fCallback != nullptr && fNextRefresh < millis())
    	{
    		fCallback();
    		fNextRefresh = millis() + fRefreshMillis;
    	}
    }

	/**
 	 * Symmetric sigmoidal approximation
 	 * https://www.desmos.com/calculator/7m9lu26vpy
 	 *
 	 * c - c / (1 + k*x/v)^3
 	 */
	static unsigned sigmoidal(unsigned voltage, unsigned minVoltage, unsigned maxVoltage)
	{
		unsigned result = 105 - (105 / (1 + pow(1.724 * (voltage - minVoltage)/(maxVoltage - minVoltage), 5.5)));
		return result >= 100 ? 100 : result;
	}

	/**
	* Asymmetric sigmoidal approximation
	* https://www.desmos.com/calculator/oyhpsu8jnw
	*
	* c - c / [1 + (k*x/v)^4.5]^3
	*/
	static inline unsigned asigmoidal(unsigned voltage, unsigned minVoltage, unsigned maxVoltage)
	{
		uint8_t result = 101 - (101 / pow(1 + pow(1.33 * (voltage - minVoltage)/(maxVoltage - minVoltage) ,4.5), 3));
		return result >= 100 ? 100 : result;
	}

	/**
	* Linear mapping
	* https://www.desmos.com/calculator/sowyhttjta
	*
	* x * 100 / v
	*/
	static inline unsigned linear(unsigned voltage, unsigned minVoltage, unsigned maxVoltage)
	{
		return (unsigned long)(voltage - minVoltage) * 100 / (maxVoltage - minVoltage);
	}

private:
	uint8_t fPin;
	unsigned fMinVoltage;
	unsigned fMaxVoltage;
	float fScale;
	unsigned (*fMapping)(unsigned, unsigned, unsigned);
	unsigned fMilliVolts = 0;
	uint32_t fRefreshMillis = 1000;
	uint32_t fNextRefresh = 0;
	void (*fCallback)() = nullptr;
};

#endif
