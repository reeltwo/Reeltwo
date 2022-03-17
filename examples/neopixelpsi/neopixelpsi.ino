#include "ReelTwo.h"
#include "dome/NeoPixelPSI.h"

NeoPixelPSI rearPSI(2, 5);
NeoPixelPSI frontPSI(3, 5); 

void setup()
{
    REELTWO_READY();
    SetupEvent::ready();
    rearPSI.set_color(1, 0, 255, 0);  // Set the rear PSI colours
    rearPSI.set_color(2, 255, 255, 0);  // Without this it does the standard front colours
}

void loop()
{
    AnimatedEvent::process();
}