#ifndef CommandEvent_h
#define CommandEvent_h

#include "ReelTwo.h"
#include "core/AnimatedEvent.h"

/**
  * \ingroup Core
  *
  * \class CommandEvent
  *
  * \brief
  *
  * Base class for all command enabled devices. CommandEvent::handleCommand() is called for each device
  * every time CommandEvent::process() is called.
  */
class CommandEvent
{
public:
    /** \brief Default Constructor
      *
      * Registers the subclass to handle command strings.
      */
	CommandEvent()
	{
		fNext = *tail();
		*tail() = this;
	}

    /**
      * Calls handleCommand() for each created CommandEvent subclass.
      */
	static void process(char* cmd)
	{
		if (*cmd != 0)
		{
			// trim trailing whitespace and line feed
			int len = strlen(cmd);
			while (len > 0 && (isspace(cmd[len-1]) || cmd[len-1] == '\n'))
				cmd[--len] = '\0';
			if (len > 0)
			{
				for (CommandEvent* evt = *tail(); evt != NULL; evt = evt->fNext)
				{
					evt->handleCommand(cmd);
				}
			}
		}
	}

    /**
      * Calls handleCommand() for each created CommandEvent subclass.
      */
    static void process(const char* cmd)
    {
        if (*cmd != 0)
        {
            for (CommandEvent* evt = *tail(); evt != NULL; evt = evt->fNext)
            {
                evt->handleCommand(cmd);
            }
        }
    }

    /**
      * Calls handleCommand() for each created CommandEvent subclass.
      */
	static void process(PROGMEMString pcmd)
	{
        char buffer[20];
        const char* cmd = reinterpret_cast<const char *>(pcmd);
        const char* cmd_end = cmd + strlen_P(cmd);
        while (cmd < cmd_end)
        {
            char ch;
            const char* pch = cmd;
            do
            {
                if (((ch = pgm_read_byte(pch++)) == 0) || ch == '\n')
                {
                    size_t len = min(size_t(pch - cmd - 1), sizeof(buffer)-1);
                    strncpy_P(buffer, cmd, len);
                    buffer[len] = 0;
                    cmd = pch;
                    // trim trailing whitespace and newline
                    while (len > 0)
                    {
                        ch = buffer[len-1];
                        if (isspace(ch) || ch == '\n')
                            buffer[--len] = '\0';
                        else
                            break;
                    }
                    if (len > 0)
                    {
                        for (CommandEvent* evt = *tail(); evt != NULL; evt = evt->fNext)
                        {
                            evt->handleCommand(buffer);
                        }
                    }
                }
            }
            while (cmd < cmd_end && ch != 0);
        }
	}

	/**
	  * Subclasses should implement this function to process commands specific to their device. By convention the
	  * first two characters are the device type identifier. 'H' 'P' for holoprojector, etc. 
	  */
	virtual void handleCommand(const char* cmd) = 0;

	/**
	  * Subclasses should implement this function to process commands specific to their device. By convention the
	  * first two characters are the device type identifier. 'H' 'P' for holoprojector, etc. 
	  */
	virtual void handleCommand(String cmd)
	{
		handleCommand(cmd.c_str());
	}

private:
	CommandEvent* fNext;

    static CommandEvent** tail()
    {
        static CommandEvent* sTail;
        return &sTail;
    }
};

template<uint16_t BUFFER_SIZE=64> class CommandEventSerial : public AnimatedEvent
{
public:
    CommandEventSerial(HardwareSerial &serial) :
        fStream(&serial),
        fPos(0)
    {
    }

    CommandEventSerial(Stream* stream) :
        fStream(stream),
        fPos(0)
    {
    }

    virtual void animate()
    {
        if (fStream->available())
        {
            int ch = fStream->read();
            if (ch == '\r' || ch == '\n' || ch == 0)
            {
                fBuffer[fPos] = '\0';
                fPos = 0;
                if (*fBuffer != '\0')
                {
                    CommandEvent::process(fBuffer);
                }
            }
            else if (fPos < SizeOfArray(fBuffer))
            {
                fBuffer[fPos++] = ch;
            }
            if (fPos == SizeOfArray(fBuffer))
            {
                fBuffer[fPos-1] = '\0';
                fPos = 0;
                if (*fBuffer != '\0')
                {
                    CommandEvent::process(fBuffer);
                }
            }
        }
    }

private:
    Stream* fStream;
    char fBuffer[BUFFER_SIZE];
    unsigned fPos;
};

#endif