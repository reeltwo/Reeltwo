#include "ReelTwo.h"
#include "ServoDispatchDirect.h"
#include "ServoSequencer.h"
#include "core/Marcduino.h"

#define COMMAND_SERIAL Serial

#ifdef RECEIVE_MARCDUINO_COMMANDS
#define PIE_PANEL          0x0008
#define TOP_PIE_PANEL      0x0010

const ServoSettings servoSettings[] PROGMEM = {
    { 2,  PIE_PANEL,     1250, 1900 },  /* 0: pie panel 1 */
    { 3,  PIE_PANEL,     1075, 1700 },  /* 1: pie panel 2 */
    { 4,  PIE_PANEL,     1200, 2000 },  /* 2: pie panel 3 */
    { 5,  PIE_PANEL,      750, 1450 },  /* 3: pie panel 4 */
    { 6,  TOP_PIE_PANEL, 1250, 1850 },  /* 4: dome top panel */
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
