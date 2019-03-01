#include "ReelTwo.h"
#include "core/DelayCall.h"
#include "ServoDispatchDirect.h"

#define SMALL_PANEL        0x0001
#define MEDIUM_PANEL       0x0002
#define BIG_PANEL          0x0004
#define PIE_PANEL          0x0008
#define TOP_PIE_PANEL      0x0010

#define HOLO_HSERVO        0x1000
#define HOLO_VSERVO        0x2000

#define DOME_PANELS_MASK        (SMALL_PANEL|MEDIUM_PANEL|BIG_PANEL)
#define PIE_PANELS_MASK         (PIE_PANEL)
#define ALL_DOME_PANELS_MASK    (MINI_PANEL|DOME_PANELS_MASK|PIE_PANELS_MASK|TOP_PIE_PANEL)
#define HOLO_SERVOS_MASK        (HOLO_HSERVO|HOLO_VSERVO)

// Group ID is used by the ServoSequencer and some ServoDispatch functions to
// identify a group of servos.
//
//   Pin  Group ID,      Min,  Max
const ServoSettings servoSettings[] PROGMEM = {
    { 2,  SMALL_PANEL,   1000, 1650 },  /* 0: door 4 */
    { 3,  SMALL_PANEL,   1500, 2300 },  /* 1: door 3 */
    { 4,  SMALL_PANEL,    900, 1650 },  /* 2: door 2 */
    { 5,  SMALL_PANEL,   1200, 1900 },  /* 3: door 1 */
    { 6,  MEDIUM_PANEL,  1200, 2000 },  /* 4: door 5 */
    { 7,  BIG_PANEL,     1200, 2000 },  /* 5: door 9 */
    { 8,  PIE_PANEL,     1250, 1900 },  /* 8: pie panel 1 */
    { 9,  PIE_PANEL,     1075, 1700 },  /* 9: pie panel 2 */
    { 10, PIE_PANEL,     1200, 2000 },  /* 10: pie panel 3 */
    { 11, PIE_PANEL,      750, 1450 },  /* 11: pie panel 4 */
    { 12, TOP_PIE_PANEL, 1250, 1850 },  /* 12: dome top panel */
};

ServoDispatchDirect<SizeOfArray(servoSettings)> servoDispatch(servoSettings);

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

