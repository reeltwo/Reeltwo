#ifndef DelayCall_h
#define DelayCall_h

#include "ReelTwo.h"
#include "core/AnimatedEvent.h"

#define MAX_DELAY_CALL 10

typedef void (*DelayCallPtr)();

/**
  * \ingroup Core
  *
  * \class DelayCall
  *
  * \brief Schedules a function to be called at a later time
  *
  * Example Usage:
  * \code
  *
  *  // Open all servos in 4 seconds
  *  DelayCall::schedule([] { servoDispatch.moveServosTo(ALL_DOME_PANELS_MASK, 150, 100, 700); }, 4000);
  *  / Close all servos in 8 seconds
  *  DelayCall::schedule([] { servoDispatch.moveServosTo(ALL_DOME_PANELS_MASK, 150, 100, 2400); }, 8000);
  *
  * \endcode
  */
class DelayCall : public AnimatedEvent
{
public:
	/**
	  * Schedules a function to be called after "delayMillis" milliseconds.
	  */
	static void schedule(DelayCallPtr callptr, uint32_t delayMillis)
	{
		Call* call = storage();
		for (int i = 0; i < MAX_DELAY_CALL; i++, call++)
		{
			if (call->fCallptr == NULL)
			{
				call->fScheduleTime = millis() + delayMillis;
				call->fCallptr = callptr;
				break;
			}
		}
	}

	/**
	  * Call any pending delay call if its delay timer has expired
	  */
	virtual void animate()
	{
		Call* call = storage();
		for (int i = 0; i < MAX_DELAY_CALL; i++, call++)
		{
			if (call->fCallptr != NULL && millis() >= call->fScheduleTime)
			{
				DelayCallPtr callptr = call->fCallptr;
				call->fCallptr = NULL;
				callptr();
			}
		}
	}

private:
	DelayCall() {}
	struct Call
	{
		DelayCallPtr fCallptr;
		uint32_t fScheduleTime;
	};
	Call fStorage[MAX_DELAY_CALL];

	static Call* storage()
	{
		static DelayCall myself;
		return myself.fStorage;
	}
};
#endif
