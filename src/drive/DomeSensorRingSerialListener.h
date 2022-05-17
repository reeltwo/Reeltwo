#ifndef DomeSensorRingSerialListener_h
#define DomeSensorRingSerialListener_h

#include "ReelTwo.h"
#include "core/AnimatedEvent.h"

class DomeSensorRingSerialListener : public AnimatedEvent
{
public:
	DomeSensorRingSerialListener(Stream& serial) :
		fStream(&serial)
	{
	}

	inline bool ready()
	{
		return (fPosition != -1);
	}

	inline int getAngle()
	{
		return fPosition;
	}

    virtual void animate() override
    {
	    // append commands to command buffer
	    while (fStream->available())
	    {
	        int ch = fStream->read();
	        if (ch == '\r' || ch == '\n')
	        {
	        	if (fState == 4)
	        	{
	        		// Update position
	            	fPosition = fValue;
	            }
	            fState = 0;
	            return;
	        }
        	switch (fState)
        	{
        		case 0:
        			fState = (ch == '#') ? fState+1 : -1;
        			break;
        		case 1:
        			fState = (ch == 'D') ? fState+1 : -1;
        			break;
        		case 2:
        			fState = (ch == 'P') ? fState+1 : -1;
        			break;
        		case 3:
        			fState = (ch == '@') ? fState+1 : -1;
        			fValue = 0;
        			break;
        		case 4:
        			if (ch >= '0' && ch <= '9')
        			{
        				fValue = fValue * 10 + (ch - '0');
        			}
        			else
        			{
        				fState = -1;
        			}
        			break;
        		case -1:
        			// Ignore remaining input
        			break;
        	}
	    }

    }


private:
	Stream* fStream;
	int fPosition = -1;
	char fState = 0;
	int fValue = 0;

	static bool startswith(const char* &cmd, const char* str)
	{
	    size_t len = strlen(str);
	    if (strncmp(cmd, str, strlen(str)) == 0)
	    {
	        cmd += len;
	        return true;
	    }
	    return false;
	}
};

#endif
