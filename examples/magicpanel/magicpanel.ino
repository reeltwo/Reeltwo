#include "ReelTwo.h"
#include "dome/MagicPanel.h"
#include "i2c/I2CReceiver.h"

MagicPanel magicPanel;
// Enable i2c commands
I2CReceiver i2cReceiver(I2C_MAGIC_PANEL);

void setup()
{
    REELTWO_READY();
    SetupEvent::ready();

    // Play game of life for no more than 100 seconds then switch to normal operation (blank)
    magicPanel.setSequence(MagicPanel::kLife, 0, 100);
}

void loop()
{
    AnimatedEvent::process();
}

