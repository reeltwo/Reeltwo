#ifndef SetupEvent_h
#define SetupEvent_h

#include "ReelTwo.h"

/**
  * \ingroup Core
  *
  * \class SetupEvent
  *
  * \brief
  *
  * Base class for all devices that require setup that cannot happen in the constructor. SetupEvent::setup() is called for each device once from the sketch setup() routine.
  */
class SetupEvent
{
public:
    /** \brief Default Constructor
      *
      * Registers the subclass to be called from the sketch setup() routine
      */
	SetupEvent() :
		fNext(NULL)
	{
		if (*head() == NULL)
			*head() = this;
		if (*tail() != NULL)
			(*tail())->fNext = this;
		*tail() = this;
	}

	/**
	  * Calls setup() for each created AnimatedEvent subclass.
	  */
	static void ready()
	{
		for (SetupEvent* evt = *head(); evt != NULL; evt = evt->fNext)
		{
			evt->setup();
		}
	}

	/**
	  * Subclasses must implement this function to perform any necessary setup that cannot happen in the constructor. 
	  */
	virtual void setup() = 0;

private:
	SetupEvent* fNext;

    static SetupEvent** head()
    {
        static SetupEvent* sHead;
        return &sHead;
    }

    static SetupEvent** tail()
    {
        static SetupEvent* sTail;
        return &sTail;
    }
};

#endif