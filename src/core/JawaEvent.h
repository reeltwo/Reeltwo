#ifndef JawaEvent_h
#define JawaEvent_h

#include "ReelTwo.h"

enum JawaID
{
    kJawaAll = 0,
    kJawaTFLD = 1,
    kJawaBFLD = 2,
    kJawaRFLD = 3,
    kJawaFrontPSI = 4,
    kJawaRearPSI = 5,
    kJawaFrontHolo = 6,
    kJawaRearHolo = 7,
    kJawaTopHolo = 8,
    kJawaOther = 9,
    kJawaRadarEye = 10,
    kJawaMagicPanel = 80,
    kJawaCBI = 83,
    kJawaDataPort = 84
};

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
	JawaEvent() :
		fNext(NULL)
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
        UNUSED_ARG(cmd)
        UNUSED_ARG(arg)
        UNUSED_ARG(value)
	}

	/**
	  * Subclasses should override this method to handle commands
	  */
	virtual void jawaCommand(char cmd, const char* arg)
	{
        UNUSED_ARG(cmd)
        UNUSED_ARG(arg)
	}

	/**
	  * Specify the JAWA address of this device
	  */
	inline void setJawaAddress(int addr)
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