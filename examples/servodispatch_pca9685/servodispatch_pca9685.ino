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
    { 1,  SMALL_PANEL,   1000, 1650 },  /* 0: door 4 */
    { 2,  SMALL_PANEL,   1500, 2300 },  /* 1: door 3 */
    { 4,  SMALL_PANEL,    900, 1650 },  /* 2: door 2 */
    { 6,  SMALL_PANEL,   1200, 1900 },  /* 3: door 1 */
    { 17, MEDIUM_PANEL,  1200, 2000 },  /* 4: door 5 */
    { 9,  BIG_PANEL,     1200, 2000 },  /* 5: door 9 */
    { 8,  MINI_PANEL,    1275, 1975 },  /* 6: mini door 2 */
    { 7,  MINI_PANEL,    1550, 1900 },  /* 7: mini front psi door */
    { 3,  PIE_PANEL,     1250, 1900 },  /* 8: pie panel 1 */
    { 10, PIE_PANEL,     1075, 1700 },  /* 9: pie panel 2 */
    { 11, PIE_PANEL,     1200, 2000 },  /* 10: pie panel 3 */
    { 12, PIE_PANEL,      750, 1450 },  /* 11: pie panel 4 */
    { 5,  TOP_PIE_PANEL, 1250, 1850 },  /* 12: dome top panel */
    { 13, HOLO_HSERVO,    800, 1600 },  /* 13: horizontal front holo */
    { 14, HOLO_VSERVO,    800, 1800 },  /* 14: vertical front holo */
    { 15, HOLO_HSERVO,    800, 1600 },  /* 15: horizontal top holo */
    { 16, HOLO_VSERVO,    800, 1325 },  /* 16: vertical top holo */
    { 25, HOLO_VSERVO,    900, 1000 },  /* 17: vertical rear holo */
    { 26, HOLO_HSERVO,   1300, 1600 },  /* 18: horizontal rear holo */
};

ServoDispatchPCA9685<SizeOfArray(servoSettings)> servoDispatch(servoSettings);

void setup()
{
    REELTWO_READY();

    SetupEvent::ready();

    // Open all servos in 4 seconds
    DelayCall::schedule([] {
	servoDispatch.moveServosTo(ALL_DOME_PANELS_MASK, 150, 100, 700);
    }, 4000);

    // Close all servos in 8 seconds
    DelayCall::schedule([] {
       servoDispatch.moveServosTo(ALL_DOME_PANELS_MASK, 150, 100, 2400);
    }, 8000);
}

void loop()
{
    AnimatedEvent::process();
}
