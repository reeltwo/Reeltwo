#include "ReelTwo.h"
#include "dome/BadMotivator.h"
#include "dome/FireStick.h"
#include "core/Animation.h"

#define SMOKE_RELAY_PIN 8
#define FIRESTICK_PIN 11

BadMotivator badMotivator(SMOKE_RELAY_PIN);
FireStick fireStick(FIRESTICK_PIN)
AnimationPlayer animationPlayer;

ANIMATION(badMotivator)
{
    ANIMATION_ONCE_AND_WAIT({
        // Temp max volume
        StealthCommand("tmpvol=100,15");
    }, 100)
    ANIMATION_ONCE_AND_WAIT({
        // Temp stop random sounds on main controller
        StealthCommand("tmprnd=60");
    }, 100)
    ANIMATION_ONCE_AND_WAIT({
        // Short Circuit MP3 - play sound bank 8
        StealthCommand("$08");
    }, 500)
    ANIMATION_ONCE_AND_WAIT({
        // Allow smoke to build up in dome
        badMotivator.smokeOn();
    }, 3000)
    ANIMATION_ONCE_AND_WAIT({
        // Open pie panels
        servoDispatch.moveServosTo(PIE_PANELS_MASK, 150, 100, 700);
    }, 500)
    ANIMATION_ONCE_AND_WAIT({
        fireStick.spark(500);
        // Electrical Crackle MP3 -  sound bank 14
        StealthCommand("$14");
        // TODO Send commands to disable all lights
    }, 500)
    ANIMATION_ONCE_AND_WAIT({
        fireStick.burn(2500);
    }, 2500)
    ANIMATION_ONCE({
        badMotivator.smokeOff();
    })
    ANIMATION_WAIT(8000)
    ANIMATION_ONCE({
        // We are back ... play MP3
        StealthCommand("$0109");
        // Close pie panels
        servoDispatch.moveServosTo(PIE_PANELS_MASK, 150, 100, 2400);
    })
    ANIMATION_END()
}

void setup()
{
    SMQ_READY();
    SetupEvent::ready();

    ANIMATION_PLAY_ONCE(animationPlayer, badMotivator);
}
 
void loop()
{
    AnimatedEvent::process();
}

