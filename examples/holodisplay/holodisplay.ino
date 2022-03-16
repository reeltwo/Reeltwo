#include "ReelTwo.h"
#include "dome/HoloDisplay.h"
#include "ServoDispatchDirect.h"

#define HOLO_HSERVO        0x1000
#define HOLO_VSERVO        0x2000

const ServoSettings servoSettings[] PROGMEM = {
    { 5,   800, 1600, HOLO_HSERVO },  /* 0: horizontal front holo */
    { 6,   800, 1800, HOLO_VSERVO },  /* 1: vertical front holo */
    { 7,   800, 1600, HOLO_HSERVO },  /* 2: horizontal top holo */
    { 8,   800, 1325, HOLO_VSERVO },  /* 3: vertical top holo */
    { 9,   900, 1000, HOLO_VSERVO },  /* 4: vertical rear holo */
    { 10, 1300, 1600, HOLO_HSERVO },  /* 5: horizontal rear holo */
};
ServoDispatchDirect<SizeOfArray(servoSettings)> servoDispatch(servoSettings);

HoloDisplay frontHolo;
HoloLights rearHolo(2);			// PIN 2
HoloLights topHolo(3);			// PIN 3

void setup()
{
    REELTWO_READY();

    frontHolo.assignServos(&servoDispatch, 0, 1);
    topHolo.assignServos(&servoDispatch, 2, 3);
    rearHolo.assignServos(&servoDispatch, 4, 5);

    SetupEvent::ready();

   	// Play the Leia movie using the front holoprojector
    frontHolo.play("Leia.bd2");
}

void loop()
{
	AnimatedEvent::process();
}
