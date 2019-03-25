#include "ReelTwo.h"
#include "dome/TeecesPSI.h"

#define TEECES_DATAIN_PIN 6
#define TEECES_CLOCK_PIN  7
#define TEECES_LOAD_PIN   8

LedControlMAX7221<1> ledChain(TEECES_DATAIN_PIN, TEECES_CLOCK_PIN, TEECES_LOAD_PIN);
TeecesPSI frontPSI(ledChain);

void setup()
{
    REELTWO_READY();

    SetupEvent::ready();
}

void loop()
{
    AnimatedEvent::process();
}
