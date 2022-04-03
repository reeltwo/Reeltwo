#include "ReelTwo.h"
#include "dome/Logics.h"
#include "dome/LogicEngineController.h"

AstroPixelFrontPSI<> frontPSI(LogicEngineFrontPSIDefault, 4);
AstroPixelRearPSI<> rearPSI(LogicEngineRearPSIDefault, 5);

void setup()
{
    REELTWO_READY();
    SetupEvent::ready();
}

void loop()
{
    AnimatedEvent::process();
}