#include "ReelTwo.h"
#include "dome/Logics.h"
#include "dome/LogicEngineController.h"
#include "dome/NeoPixelPSI.h"

AstroPixelRLD<> RLD(LogicEngineRLDDefault, 1);
AstroPixelFLD<> FLD(LogicEngineFLDDefault, 2);

NeoPixelPSI rearPSI(2, 5);
NeoPixelPSI frontPSI(3, 5); 

void setup()
{
    REELTWO_READY();
    SetupEvent::ready();
    rearPSI.set_color(1, 0, 255, 0);  // Set the rear PSI colours
    rearPSI.set_color(2, 255, 255, 0);  // Without this it does the standard front colours
    RLD.selectScrollTextLeft("... AstroPixels ....", LogicEngineRenderer::kBlue, 0, 15);
    FLD.selectScrollTextLeft("... R2D2 ...", LogicEngineRenderer::kRed, 0, 15);
}

void loop()
{
    AnimatedEvent::process();

}