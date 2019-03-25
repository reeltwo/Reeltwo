#include "ReelTwo.h"
#include "dome/TeecesPSI.h"

LedControlMAX7221<1> ledChain(6, 7, 8);
TeecesPSI frontSI(ledChain);

void setup()
{
    REELTWO_READY();
    SetupEvent::ready();
}

void loop()
{
	AnimatedEvent::process();
}
