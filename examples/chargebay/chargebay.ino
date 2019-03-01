#include "ReelTwo.h"
#include "body/DataPanel.h"
#include "body/ChargeBayIndicator.h"

#define CBI_DATAIN_PIN 8
#define CBI_CLOCK_PIN  9
#define CBI_LOAD_PIN   10

LedControlMAX7221<2> ledChain(CBI_DATAIN_PIN, CBI_CLOCK_PIN, CBI_LOAD_PIN);
ChargeBayIndicator chargeBayIndicator(ledChain);
DataPanel dataPanel(ledChain);

void setup()
{
    REELTWO_READY();

    SetupEvent::ready();
}

void loop()
{
    AnimatedEvent::process();
}
