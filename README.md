# Modular RSeries Astromech Firmware  #

![Reeltwo](https://raw.githubusercontent.com/reeltwo/Reeltwo/master/reeltwo.gif)

This library provides modular building blocks to control various Astromech components. It also provides a serial to IP network messaging API (SMQ) based on ZeroMQ and JSON. Each Arduino should be connected via serial to a Raspberry Pi (or similar) which acts as a bridge. The serial protocol avoids the use of buffers and heavily CRC checked.

This library is the work of many people over many years.

Some of this code inspired from the following sources (in no particular order)
 John Vannoy, DanF, Joymonkey, Chris James, BigHappyDude, Curiousmarc, IOIIOOO, Mowee, FlthyMcNsty, Rotopod, Michael Erwin, and others nodoubt

# Documentation

[Reeltwo](https://reeltwo.github.io/Reeltwo)

# Playground

You can now try out the Reeltwo library using Wokwi the online Arduino simulator:

#### <ins>Simple Servo example</ins>

This example shows how to create and move a single servo when a button is pressed.
[![Simple Servo](https://thumbs.wokwi.com/projects/366510058300225537/thumbnail.jpg?tile&amp;t=1629669534812&amp;dark=1)](https://wokwi.com/projects/366510058300225537)

# <ins>Simple Servo delay call example</ins>

This example shows how to use DelayCall to call functions after specified number of milliseconds.
[![Simple Servo](https://thumbs.wokwi.com/projects/366518949882139649/thumbnail.jpg?tile&amp;t=1629669534812&amp;dark=1)](https://wokwi.com/projects/366518949882139649)

#### <ins>Simple Servo Back and Forth example</ins>

This example shows how to move a single servo using two buttons.
[![Simple Servo](https://thumbs.wokwi.com/projects/366510074163084289/thumbnail.jpg?tile&amp;t=1629669534812&amp;dark=1)](https://wokwi.com/projects/366510074163084289)

#### <ins>Simple Servo Easing example</ins>

This example shows how to change the servo easing.
[![Simple Servo](https://thumbs.wokwi.com/projects/366514412870807553/thumbnail.jpg?tile&amp;t=1629669534812&amp;dark=1)](https://wokwi.com/projects/366514412870807553)

#### <ins>Multi Servo Easing example</ins>

More servos.
[![Simple Servo](https://thumbs.wokwi.com/projects/366514809314343937/thumbnail.jpg?tile&amp;t=1629669534812&amp;dark=1)](https://wokwi.com/projects/366514809314343937)

#### <ins>Servo Groups example</ins>

Grouping servos.
[![Simple Servo](https://thumbs.wokwi.com/projects/366515197545431041/thumbnail.jpg?tile&amp;t=1629669534812&amp;dark=1)](https://wokwi.com/projects/366515197545431041)

#### <ins>Servo Sequencer example</ins>

Using ServoSequencer.
[![Simple Servo](https://thumbs.wokwi.com/projects/366520390464154625/thumbnail.jpg?tile&amp;t=1629669534812&amp;dark=1)](https://wokwi.com/projects/366520390464154625)

#### <ins>Holo Projector example</ins>

This examples shows how to drive three holo projectors.
[![Holo Projectors](https://thumbs.wokwi.com/projects/320613018220102227/thumbnail.jpg?tile&amp;t=1629669534812&amp;dark=1)](https://wokwi.com/arduino/projects/320613018220102227)

#### <ins>Logic Engine example</ins>

This example shows how to drive the logic engine lights
[![Logic Engine](https://thumbs.wokwi.com/projects/347968516066902611/thumbnail.jpg?tile&amp;t=1629669534812&amp;dark=1)](https://wokwi.com/projects/347968516066902611)


# Examples

Below are a few examples of how you can use this library to simply your sketches so you can focus on animation and the interaction between gadgets.

#### Holoprojectors example

```C++
#include "ReelTwo.h"
#include "dome/HoloLights.h"
#include "ServoDispatchDirect.h"

#define HOLO_HSERVO        0x1000
#define HOLO_VSERVO        0x2000

const ServoSettings servoSettings[] PROGMEM = {
    { 13, 800, 1600, HOLO_HSERVO, },  /* 0: horizontal front holo */
    { 14, 800, 1800, HOLO_VSERVO },  /* 1: vertical front holo */
    { 15, 800, 1600, HOLO_HSERVO },  /* 2: horizontal top holo */
    { 16, 800, 1325, HOLO_VSERVO },  /* 3: vertical top holo */
    { 25, 900, 1000, HOLO_VSERVO },  /* 4: vertical rear holo */
    { 26, 1300, 1600, HOLO_HSERVO },  /* 5: horizontal rear holo */
};

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
```

#### LogicEngine example

```C++
#include "ReelTwo.h"
#include "dome/Logics.h"

// Front Logic Device (Jawa ID#1)
LogicEngineDeathStarFLD<> FLD(1, LogicEngineFLDDefault);
// Rear Logic Device (Jawa ID#2)
LogicEngineDeathStarRLDInverted<> RLD(2, LogicEngineRLDDefault);

// LogicEngineControllerDefault reads the trimpots on the back of the board. It is optional.
LogicEngineControllerDefault controller(FLD, RLD);

void setup()
{
    REELTWO_READY();

    SetupEvent::ready();

    // Scroll text for 15 seconds before switching to normal mode
    FLD.selectScrollTextLeft("R2\n    D2", LogicEngineRenderer::kBlue, 1, 15);
    RLD.selectScrollTextLeft("... RSeries LogicEngine ....", LogicEngineRenderer::kYellow, 0, 15);
}

void loop()
{
    AnimatedEvent::process();
}
```

#### Bad Motivator example

```C++
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
```
