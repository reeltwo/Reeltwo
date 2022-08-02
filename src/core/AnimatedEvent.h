#ifndef AnimatedEvent_h
#define AnimatedEvent_h

#include "ReelTwo.h"

typedef void (*AnimatedLoopDone)();

/**
  * \ingroup Core
  *
  * \class AnimatedEvent
  *
  * \brief
  *
  * Base class for all animated devices. AnimatedEvent::animate() is called for each device once through the main loop().
  * Subclasses should not call delay() or otherwise block.
  */
class AnimatedEvent
{
public:
    /** \brief Default Constructor
      *
      * Registers the subclass to be called from the main loop()
      */
    AnimatedEvent() :
        fNext(NULL)
    {
        if (*head() == NULL)
            *head() = this;
        if (*tail() != NULL)
            (*tail())->fNext = this;
        *tail() = this;
    }

    /**
      * Calls animate() for each created AnimatedEvent subclass.
      */
    static void process()
    {
        static AnimatedEvent* sGuard;
        AnimatedLoopDone* loopProc = loopDoneProc();
        *loopProc = NULL;
        for (AnimatedEvent* evt = *head(); evt != NULL; evt = evt->fNext)
        {
            // Reentrancy guard
            if (sGuard == evt)
                continue;
            sGuard = evt;
            evt->animate();
            sGuard = NULL;
        }
    #if defined(USE_SMQ) && !defined(USE_SMQ32)
        static bool sSMQReentrancy;
        if (!sSMQReentrancy)
        {
            sSMQReentrancy = true;
            SMQ::process();
            sSMQReentrancy = false;
        }
    #endif
        if ((*loopProc) != NULL)
        {
            (*loopProc)();
            *loopProc = NULL;
        }
    }

    // \private
    void setLoopDoneCallback(AnimatedLoopDone loopProc)
    {
        *loopDoneProc() = loopProc;
    }

    /**
      * Subclasses must implement this function to run through a single frame of animation/activity. 
      * Subclasses should not call delay() or otherwise block.
      */
    virtual void animate() = 0;

private:
    AnimatedEvent* fNext;

    static AnimatedEvent** head()
    {
        static AnimatedEvent* sHead;
        return &sHead;
    }

    static AnimatedEvent** tail()
    {
        static AnimatedEvent* sTail;
        return &sTail;
    }

    static AnimatedLoopDone* loopDoneProc()
    {
        static AnimatedLoopDone sProc;
        return &sProc;
    }
};

#endif