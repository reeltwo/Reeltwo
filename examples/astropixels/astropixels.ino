/*****
 * 
 * Astropixels example sketch. 
 * 
 * This sketch will get the basic Astropixel setup by Darren Poulson working. Pinouts are designed
 * to be used with the Astropixel ESP32 breakout board.
 * 
 * 
 */
#include "ReelTwo.h"
#include "dome/Logics.h"
#include "dome/LogicEngineController.h"
#include "dome/NeoPixelPSI.h"

I2CReceiver i2cReceiver(0x19);

AstroPixelRLD<> RLD(LogicEngineRLDDefault, 3);
AstroPixelFLD<> FLD(LogicEngineFLDDefault, 1);

AstroPixelFrontPSI<> frontPSI(LogicEngineFrontPSIDefault, 4);
AstroPixelRearPSI<> rearPSI(LogicEngineRearPSIDefault, 5);

HoloLights frontHolo(25);
HoloLights rearHolo(26);
HoloLights topHolo(27);   

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