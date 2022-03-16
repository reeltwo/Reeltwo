#include "ReelTwo.h"
#include "core/Animation.h"
#include "dome/BadMotivator.h"
#include "dome/FireStrip.h"
#include "i2c/StealthBoardI2C.h"
#include "ServoDispatchDirect.h"
#include "ServoSequencer.h"

#define PIE_PANEL          0x0008
#define TOP_PIE_PANEL      0x0010
#define PIE_PANELS_MASK    (PIE_PANEL)

#define SMOKE_RELAY_PIN 7
#define FIRESTRIP_PIN 8

const ServoSettings servoSettings[] PROGMEM = {
    { 2,  1250, 1900, PIE_PANEL },  /* 0: pie panel 1 */
    { 3,  1075, 1700, PIE_PANEL },  /* 1: pie panel 2 */
    { 4,  1200, 2000, PIE_PANEL },  /* 2: pie panel 3 */
    { 5,  750,  1450, PIE_PANEL },  /* 3: pie panel 4 */
    { 6,  1250, 1850, TOP_PIE_PANEL },  /* 4: dome top panel */
};

ServoDispatchDirect<SizeOfArray(servoSettings)> servoDispatch(servoSettings);
ServoSequencer servoSequencer(servoDispatch);

StealthBoardI2C stealthBoard();
BadMotivator badMotivator(SMOKE_RELAY_PIN);
FireStrip fireStrip(FIRESTRIP_PIN);
AnimationPlayer player(servoSequencer);

ANIMATION(badMotivator)
{
    DO_START()
    // Temp max volume
    DO_COMMAND_AND_WAIT("STtmpvol=100,15", 100)
    // Temp stop random sounds on main controller
    DO_COMMAND_AND_WAIT("STtmprnd=60", 100)
    // Short Circuit MP3 - play sound bank 8
    DO_COMMAND_AND_WAIT("ST$08", 500)
    // Smoke on and allow smoke to build up in dome
    DO_COMMAND_AND_WAIT("BMON", 3000)
    // Open pie panels
    DO_ONCE_AND_WAIT({
        servoDispatch.moveServosTo(PIE_PANELS_MASK, 150, 100, 700);
    }, 500)
    // Spark fire strip
    DO_ONCE({ fireStrip.spark(500); })
    // Electrical Crackle MP3 -  sound bank 14
    DO_ONCE_AND_WAIT({ StealthCommand("ST$14"); }, 500)
    // Smoke off
    DO_COMMAND("BMOFF")
    // Fake being dead for 8 seconds
    DO_WAIT_SEC(8)
    // Ok We are back!
    DO_COMMAND("ST$0109")
    // Close pies
    DO_ONCE({
        // Close pie panels
        servoDispatch.moveServosTo(PIE_PANELS_MASK, 150, 100, 2400);
    })
    DO_END()
}

void setup()
{
    REELTWO_READY();
    SetupEvent::ready();

    ANIMATION_PLAY_ONCE(player, badMotivator);
}
 
void loop()
{
    AnimatedEvent::process();
}

