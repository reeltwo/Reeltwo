#include "ReelTwo.h"
#include "dome/TeecesPSI.h"
#include "dome/TeecesLogics.h"

LedControlMAX7221<4> rearChain(6, 7, 8);
TeecesRLD RLD(rearChain);
TeecesPSI rearPSI(rearChain);

LedControlMAX7221<3> frontChain(9, 10, 11);
TeecesTFLD TFLD(frontChain);
TeecesBFLD BFLD(frontChain);
TeecesPSI frontPSI(frontChain);

void setup()
{
    REELTWO_READY();
    SetupEvent::ready();
}

void loop()
{
	AnimatedEvent::process();
}
