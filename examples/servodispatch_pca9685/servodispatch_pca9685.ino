#include "ReelTwo.h"
#include "core/DelayCall.h"
#include "ServoDispatchPCA9685.h"

#define SMALL_PANEL        0x0001
#define MEDIUM_PANEL       0x0002
#define BIG_PANEL          0x0004
#define PIE_PANEL          0x0008
#define TOP_PIE_PANEL      0x0010
#define MINI_PANEL         0x0020

#define HOLO_HSERVO        0x1000
#define HOLO_VSERVO        0x2000

#define DOME_PANELS_MASK        (SMALL_PANEL|MEDIUM_PANEL|BIG_PANEL)
#define PIE_PANELS_MASK         (PIE_PANEL)
#define ALL_DOME_PANELS_MASK    (MINI_PANEL|DOME_PANELS_MASK|PIE_PANELS_MASK|TOP_PIE_PANEL)
#define HOLO_SERVOS_MASK        (HOLO_HSERVO|HOLO_VSERVO)

// These ServoSettings are for a two PCA9685 boards.
// Pins 1-16 apply to the first board. Any pin above that
// is automatically sent to a different board. You do have
// to fill up all the pins on any preceeding board. The number
// of boards detected is a function of the number of slots (not pin numbering). 
// So once you have 16 slots filled it adds support for the next PCA9685 board.
// Only the last PCA9685 board can have unused pins.
//
// Group ID is used by the ServoSequencer and some ServoDispatch functions to
// identify a group of servos.
//
//   Pin  Group ID,      Min,  Max
const ServoSettings servoSettings[] PROGMEM = {
    { 1,  1000, 1650, SMALL_PANEL },  /* 0: door 4 */
    { 2,  1500, 2300, SMALL_PANEL },  /* 1: door 3 */
    { 4,   900, 1650, SMALL_PANEL },  /* 2: door 2 */
    { 6,  1200, 1900, SMALL_PANEL },  /* 3: door 1 */
    { 17, 1200, 2000, MEDIUM_PANEL },  /* 4: door 5 */
    { 9,  1200, 2000, BIG_PANEL },  /* 5: door 9 */
    { 8,  1275, 1975, MINI_PANEL },  /* 6: mini door 2 */
    { 7,  1550, 1900, MINI_PANEL },  /* 7: mini front psi door */
    { 3,  1250, 1900, PIE_PANEL },  /* 8: pie panel 1 */
    { 10, 1075, 1700, PIE_PANEL },  /* 9: pie panel 2 */
    { 11, 1200, 2000, PIE_PANEL },  /* 10: pie panel 3 */
    { 12,  750, 1450, PIE_PANEL },  /* 11: pie panel 4 */
    { 5,  1250, 1850, TOP_PIE_PANEL },  /* 12: dome top panel */
    { 13,  800, 1600, HOLO_HSERVO },  /* 13: horizontal front holo */
    { 14,  800, 1800, HOLO_VSERVO },  /* 14: vertical front holo */
    { 15,  800, 1600, HOLO_HSERVO },  /* 15: horizontal top holo */
    { 16,  800, 1325, HOLO_VSERVO },  /* 16: vertical top holo */
    { 25,  900, 1000, HOLO_VSERVO },  /* 17: vertical rear holo */
    { 26, 1300, 1600, HOLO_HSERVO },  /* 18: horizontal rear holo */
};

ServoDispatchPCA9685<SizeOfArray(servoSettings)> servoDispatch(servoSettings);

void setup()
{
    REELTWO_READY();

    SetupEvent::ready();

    // Open all servos in 4 seconds
    DelayCall::schedule([] {
        // Start in 150ms
        // Duration 1000ms
        // 1.0: Move to max pulse
       servoDispatch.moveServosTo(ALL_DOME_PANELS_MASK, 150, 1000, 1.0);
    }, 4000);

    // Close all servos in 8 seconds
    DelayCall::schedule([] {
        // Start in 150ms
        // Duration 2000ms
        // 0.0: Move to min pulse
       servoDispatch.moveServosTo(ALL_DOME_PANELS_MASK, 150, 2000, 0.0);
    }, 8000);
}

void loop()
{
    AnimatedEvent::process();
}
