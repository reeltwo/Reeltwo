#include "ReelTwo.h"
#include "dome/TeecesPSI.h"

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
