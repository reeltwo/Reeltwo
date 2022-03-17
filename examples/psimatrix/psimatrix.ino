#include "ReelTwo.h"
#include "dome/PSIMatrix.h"

PSIMatrix rearPSI(22,23,2);    // SDA Pin, SCL Pin, PSI.
PSIMatrix frontPSI(24,25,1);   // PSI 1 = Front, PSI 2 = Rear

void setup()
{
    REELTWO_READY();
    SetupEvent::ready();
}

void loop()
{
    AnimatedEvent::process();
}