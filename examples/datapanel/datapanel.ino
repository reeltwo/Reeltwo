#include "ReelTwo.h"
#include "body/DataPanel.h"

#define CBI_DATAIN_PIN 8
#define CBI_CLOCK_PIN  9
#define CBI_LOAD_PIN   10

LedControlMAX7221<1> ledChain(CBI_DATAIN_PIN, CBI_CLOCK_PIN, CBI_LOAD_PIN);
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

