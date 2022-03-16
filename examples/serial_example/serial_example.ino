#include "ReelTwo.h"
#include "ServoDispatchDirect.h"
#include "ServoSequencer.h"
#include "core/Marcduino.h"

#define COMMAND_SERIAL Serial

#ifdef RECEIVE_MARCDUINO_COMMANDS
#define PIE_PANEL          0x0008
#define TOP_PIE_PANEL      0x0010

const ServoSettings servoSettings[] PROGMEM = {
    { 2,  1250, 1900, PIE_PANEL },  /* 0: pie panel 1 */
    { 3,  1075, 1700, PIE_PANEL },  /* 1: pie panel 2 */
    { 4,  1200, 2000, PIE_PANEL },  /* 2: pie panel 3 */
    { 5,   750, 1450, PIE_PANEL },  /* 3: pie panel 4 */
    { 6,  1250, 1850, TOP_PIE_PANEL },  /* 4: dome top panel */
};

ServoDispatchDirect<SizeOfArray(servoSettings)> servoDispatch(servoSettings);
ServoSequencer servoSequencer(servoDispatch);
AnimationPlayer player(servoSequencer);
MarcduinoSerial<> marcduinoSerial(COMMAND_SERIAL, player);

// Marcduino command starting with '*RT' followed by Reeltwo command
MARCDUINO_ACTION(DirectCommand, *RT, ({
    // Direct ReelTwo command
    CommandEvent::process(Marcduino::getCommand());
}))

#else
CommandEventSerial<> commandSerial(COMMAND_SERIAL);
#endif

void setup()
{
    REELTWO_READY();
    COMMAND_SERIAL.begin(9600);
    SetupEvent::ready();
}
 
void loop()
{
    AnimatedEvent::process();
}
