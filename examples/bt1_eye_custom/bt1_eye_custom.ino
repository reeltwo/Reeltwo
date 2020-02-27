#define EYE_PIN 4 /* Data pin */

#include "ReelTwo.h"
#include "dome/LogicEngine.h"
#include "BT1Eye.h"

BT1Eye<> myEye(LogicEngineBT1EyeDefault, 1);

void setup()
{
    REELTWO_READY();

    SetupEvent::ready();

    myEye.selectSequence(LogicEngineDefaults::REDALERT);
}

void loop()
{
    static int sEffectIdx;
    static uint32_t sNextTime;
    AnimatedEvent::process();
    if (sNextTime <= millis())
    {
        switch (sEffectIdx++)
        {
                case 0:
                        myEye.selectSequence(LogicEngineDefaults::PSICOLORWIPE, LogicEngineRenderer::kBlue);
                        break;
                case 1:
                        break;
                case 2:
                        myEye.selectSequence(LogicEngineDefaults::PSICOLORWIPE, LogicEngineRenderer::kYellow);
                        break;
                case 3:
                        break;
                case 4:
                        myEye.selectSequence(LogicEngineDefaults::NORMAL);
                        break;
                case 5:
                        myEye.selectSequence(LogicEngineDefaults::ALARM);
                        break;
                case 6:
                        myEye.selectSequence(LogicEngineDefaults::LEIA);
                        break;
                case 7:
                        myEye.selectSequence(LogicEngineDefaults::MARCH);
                        break;
                case 8:
                        myEye.selectSequence(LogicEngineDefaults::SOLIDCOLOR, LogicEngineRenderer::kBlue);
                        break;
                case 9:
                        myEye.selectSequence(LogicEngineDefaults::RAINBOW);
                        break;
                case 10:
                        myEye.selectSequence(LogicEngineDefaults::REDALERT);
                        break;
                case 11:
                        myEye.selectSequence(LogicEngineDefaults::VERTICALSCANLINE, LogicEngineRenderer::kRed, 0);
                        break;
                case 12:
                        myEye.selectScrollTextLeft("BT-1", LogicEngineRenderer::kRed, 2);
                        break;
                case 13:
                        myEye.selectSequence(LogicEngineDefaults::FIRE);
                        break;
                case 14:
                        break;
                case 15:
                        myEye.selectSequence(LogicEngineDefaults::FAILURE);
                        break;
                case 16:
                        myEye.selectSequence(LogicEngineDefaults::SOLIDCOLOR, LogicEngineRenderer::kRed);
                        break;
                case 17:
                        myEye.selectSequence(LogicEngineDefaults::LIGHTSOUT);
                        sEffectIdx = 0;
                        break;
                default:
                        sEffectIdx = 0;
                        break;
        }
        sNextTime = millis() + 10000L;
    }
}
