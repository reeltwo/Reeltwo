#ifndef CommandEvent_h
#define CommandEvent_h

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
			// trim trailing whitespace
			int len = strlen(cmd);
			while (len > 0 && isspace(cmd[len-1]))
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
	  * Subclasses should implement this function to process commands specific to their device. By convention the
	  * first two characters are the device type identifier. 'H' 'P' for holoprojector, etc. 
	  */
	virtual void handleCommand(const char* cmd) = 0;

private:
	CommandEvent* fNext;

    static CommandEvent** tail()
    {
        static CommandEvent* sTail;
        return &sTail;
    }
};

#endif