#include "ReelTwo.h"
#include "dome/TeecesPSI.h"
#include "dome/ButtonController.h"

LedControlMAX7221<3> ledChain(DBC_TEECES_D_PIN, DBC_TEECES_C_PIN, DBC_TEECES_L_PIN);
TeecesPSI rearPSI(ledChain);

ButtonController controller([] (byte color, int rate) {
	if (!color)
	{
		// LEDs OFF
		rearPSI.setSolidState(0);
	}    
	else if (rate == ON)
	{
		// ALWAYS ON
		if (color == PSI_YELLOW)
		{
			// ALWAYS ON YELLOW
			rearPSI.setState(6);
		}
		else if (color == PSI_GREEN)
		{
			// ALWAYS ON GREEN
			rearPSI.setState(0);
		}
		else if (color == PSI_BOTH)
		{
			rearPSI.setSolidState(~0);
		}
	}
	else
	{
		rearPSI.setAnimate(ON);
	}
});

void setup()
{
    REELTWO_READY();
    SetupEvent::ready();
}

void loop()
{
	AnimatedEvent::process();
}
