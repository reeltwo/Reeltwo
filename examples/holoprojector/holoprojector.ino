#include "ReelTwo.h"
#include "dome/HoloLights.h"
#include "ServoDispatchDirect.h"

#define HOLO_HSERVO        0x1000
#define HOLO_VSERVO        0x2000

const ServoSettings servoSettings[] PROGMEM = {
    { 5,  HOLO_HSERVO,    800, 1600 },  /* 0: horizontal front holo */
    { 6,  HOLO_VSERVO,    800, 1800 },  /* 1: vertical front holo */
    { 7,  HOLO_HSERVO,    800, 1600 },  /* 2: horizontal top holo */
    { 8,  HOLO_VSERVO,    800, 1325 },  /* 3: vertical top holo */
    { 9,  HOLO_VSERVO,    900, 1000 },  /* 4: vertical rear holo */
    { 10, HOLO_HSERVO,   1300, 1600 },  /* 5: horizontal rear holo */
};
ServoDispatchDirect<SizeOfArray(servoSettings)> servoDispatch(servoSettings);

HoloLights frontHolo(2);		// PIN 2
HoloLights rearHolo(3);			// PIN 3
HoloLights topHolo(4);			// PIN 4

void setup()
{
    REELTWO_READY();

    frontHolo.assignServos(&servoDispatch, 0, 1);
    topHolo.assignServos(&servoDispatch, 2, 3);
    rearHolo.assignServos(&servoDispatch, 4, 5);

    SetupEvent::ready();

   	// Send command to front holoprojector
   	CommandEvent::process("HPF0026|20");
}

void loop()
{
	AnimatedEvent::process();
}
