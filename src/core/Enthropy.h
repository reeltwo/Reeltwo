#ifndef RandomSeed_h
#define RandomSeed_h

#include "ReelTwo.h"
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/atomic.h>

///////////////////////////////////

volatile uint32_t Enthropy_seed;
volatile int8_t Enthropy_iter;

/**
  * Generate enthropy used for random seed without using analog pins. Uses jitter in
  * the watchdog interrupts instead.
  */
class Enthropy
{
public:
    static uint32_t generate(int8_t iter = 32)
    {
        Enthropy_seed = 0;
        Enthropy_iter = iter;

        // Enable watch dog
        setWatchDog(true);

        while (Enthropy_iter > 0);

        // Disable watch dog
        setWatchDog(false);

        return Enthropy_seed;
    }

private:
    static void setWatchDog(bool state)
    {
        cli();                                             
        MCUSR = 0;                                         
        _WD_CONTROL_REG |= (1 <<_WD_CHANGE_BIT) | (state << WDE);
        _WD_CONTROL_REG = (state << WDIE);                      
        sei();
    }
};
 
ISR(WDT_vect)
{
    Enthropy_iter--;
    Enthropy_seed = Enthropy_seed << 8;
    Enthropy_seed = Enthropy_seed ^ TCNT1L;
}

#endif
