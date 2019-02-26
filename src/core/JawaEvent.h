#ifndef JawaEvent_h
#define JawaEvent_h

#include "ReelTwo.h"

/**
  * \ingroup Core
  *
  * \class JawaEvent
  *
  * \brief Base class for all devices implementing JAWA lite support.
*/
class JawaEvent
{
public:
    /** \brief Default Constructor
      *
      * Registers the subclass to handle JAWA commands.
      */
	JawaEvent()
	{
		if (*head() == NULL)
			*head() = this;
		if (*tail() != NULL)
			(*tail())->fNext = this;
		*tail() = this;
	}

	static void process(int addr, char cmd, int arg = 0, int value = 0)
	{
		for (JawaEvent* jawa = *head(); jawa != NULL; jawa = jawa->fNext)
			if (addr == 0 || addr == jawa->fJawaAddress)
				jawa->jawaCommand(cmd, arg, value);
	}

	static void process(int addr, char cmd, const char* arg)
	{
		for (JawaEvent* jawa = *head(); jawa != NULL; jawa = jawa->fNext)
			if (addr == 0 || addr == jawa->fJawaAddress)
				jawa->jawaCommand(cmd, arg);
	}

	/**
	  * Subclasses should override this method to handle commands specifying a value
	  */
	virtual void jawaCommand(char cmd, int arg, int value)
	{
	}

	/**
	  * Subclasses should override this method to handle commands
	  */
	virtual void jawaCommand(char cmd, const char* arg)
	{
	}

	/**
	  * Specify the JAWA address of this device
	  */
	inline void setJawaAddresss(int addr)
	{
		fJawaAddress = addr;
	}

protected:
	int fJawaAddress;

private:
	JawaEvent* fNext;

    static JawaEvent** head()
    {
        static JawaEvent* sHead;
        return &sHead;
    }

    static JawaEvent** tail()
    {
        static JawaEvent* sTail;
        return &sTail;
    }
};

#endif