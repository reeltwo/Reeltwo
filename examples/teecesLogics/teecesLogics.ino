#include "ReelTwo.h"
#include "dome/TeecesPSI.h"

LedControlMAX7221<4> rearChain(6, 7, 8);
TeecesRLD RLD(rearChain);
TeecesPSI rearPSI(rearChain);

LedControlMAX7221<3> frontChain(6, 7, 8);
TeecesTFLD TFLD(frontChain);
TeecesBFLD BFLD(frontChain);
TeecesPSI frontPSI(frontChain);

void setup()
{
    SMQ_READY();
    SetupEvent::ready();
}

void loop()
{
	AnimatedEvent::process();
}
