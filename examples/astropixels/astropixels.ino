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
#include "dome/HoloLights.h"
#include "dome/NeoPSI.h"
#include "i2c/I2CReceiver.h"

I2CReceiver i2cReceiver(0x0a);

AstroPixelRLD<> RLD(LogicEngineRLDDefault, 3);
AstroPixelFLD<> FLD(LogicEngineFLDDefault, 1);

AstroPixelFrontPSI<> frontPSI(LogicEngineFrontPSIDefault, 4);
AstroPixelRearPSI<> rearPSI(LogicEngineRearPSIDefault, 5);

HoloLights frontHolo(25, HoloLights::kRGB);
HoloLights rearHolo(26, HoloLights::kRGB);
HoloLights topHolo(27, HoloLights::kRGB);   

void setup()
{
    REELTWO_READY();
    SetupEvent::ready();
    RLD.selectScrollTextLeft("... AstroPixels ....", LogicEngineRenderer::kBlue, 0, 15);
    FLD.selectScrollTextLeft("... R2D2 ...", LogicEngineRenderer::kRed, 0, 15);
    CommandEvent::process("HPA0026|20");
}

void loop()
{
    AnimatedEvent::process();
}
