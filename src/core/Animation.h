#ifndef Animation_h
#define Animation_h

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"

#ifdef ANONYMOUS_LAMBDAS_NOT_BROKEN
typedef void (*AnimationStep)(class AnimationPlayer& animation, unsigned step, unsigned num, unsigned long elapsedMillis);

/**
  * \ingroup Core
  *
  * \struct Animation
  *
  * \brief Base class describing a single animation step.
  *
  * Use the ANIMATION macros to define an animation script.
  */
struct Animation
{
    uint16_t fDuration;
    AnimationStep fAnimationStep;
};

#define ANIMATION(name) static const Animation name[] PROGMEM =
#define ANIMATION_STEP [](AnimationPlayer& animation, unsigned step, unsigned num, unsigned long elapsedMillis)
#define ANIMATION_ONCE(p) { 0, ANIMATION_STEP p }
#define ANIMATION_FOREVER(p) { ~0, ANIMATION_STEP p }
#define ANIMATION_DURATION(ms, p) { ms, ANIMATION_STEP p }
#define ANIMATION_WAIT(ms) { ms, NULL }

#define ANIMATION_PLAY_ONCE(player, animation) player.animateOnce(animation, SizeOfArray(animation))
#else
/* gcc-avr corrupts progmem when using anonymous lambdas in data structures */
typedef bool (*AnimationStep)(class AnimationPlayer& animation, unsigned step, unsigned num, unsigned long elapsedMillis);

#define ANIMATION(name) static bool Animation_##name(class AnimationPlayer& animation,\
    unsigned step, unsigned num, unsigned long elapsedMillis) { unsigned _step = 0;
#define ANIMATION_ONCE(p) { if (_step == animation.fStep && animation.fLine < __LINE__) { animation.fLine = __LINE__; p; return true; } _step++; }
#define ANIMATION_FOREVER(p) { if (_step == animation.fStep && animation.fLine <= __LINE__) { animation.fLine = __LINE__; p; return false; } _step++; }
#define ANIMATION_DURATION(ms, p) { if (_step == animation.fStep && animation.fLine <= __LINE__) { if (ms < elapsedMillis) { animation.fLine = __LINE__; p; return false; } return true; } _step++; } 
#define ANIMATION_WAIT(ms) { if (_step == animation.fStep && animation.fLine <= __LINE__) { if (ms < elapsedMillis) { animation.fLine = __LINE__; return false; } return true; } _step++; }
#define ANIMATION_END() animation.end(); return false; }
#define ANIMATION_PLAY_ONCE(player, name) player.animateOnce(Animation_##name)
#endif

/**
  * \ingroup Core
  *
  * \class AnimationPlayer
  *
  * \brief Player of animation scripts
  *
  * Variables available inside an animation script are: "animation", "step", "num", and "elapsedMillis".
  *
  * Example Script:
  * \code
  * 
  * ANIMATION(simpleMultiStepAnimation)
  * {
  *     // Step One - fires once
  *     ANIMATION_ONCE({
  *         SEQUENCE_PLAY_ONCE(servoSequencer, SeqPanelAllOpenLong, DOME_PANELS_MASK);
  *     })
  *     // Step Two - waits 300ms before advancing
  *     ANIMATION_WAIT(300)
  *     // Step Three - fires once
  *     ANIMATION_ONCE({
  *         CommandEvent::process("HPF0026|20");
  *     })
  *     // Step Four - fires repeatedly for 200ms
  *     ANIMATION_DURATION(200, {
  *         DEBUG_PRINTLN(elapsedMillis);
  *     })
  *     // Step Four
  *     ANIMATION_ONCE({
  *         frontHolo.play("Leia.bd2");
  *         AnimatedEvent::process();
  *         CommandEvent::process("HPF0026|20");
  *     })
  *     // Step Five - repeat this step until "num" reaches 100 then rewind the animation
  *     ANIMATION_FOREVER({
  *         DEBUG_PRINTLN(num);
  *         if (num == 100)
  *             animation.rewind();
  *     })
  *     ANIMATION_END()
  * }
  * \endcode
  */
class AnimationPlayer : public AnimatedEvent
{
public:
    /**
      * \brief Default Constructor
      */
    inline AnimationPlayer()
    {
        reset();
    }

    /**
      * Runs through a single step of any active animation script.
      */
    virtual void animate() override
    {
    #ifdef ANONYMOUS_LAMBDAS_NOT_BROKEN
        if (fAnimation == NULL || fStep >= fNumSteps)
            fFlags = kEnded;
    #endif
        if (fAnimation == NULL || fFlags == kEnded)
            return;
        unsigned long currentMillis = millis();
    #ifdef ANONYMOUS_LAMBDAS_NOT_BROKEN
        Animation* step = &fAnimation[fStep];
        unsigned duration = pgm_read_word(&step->fDuration);
        AnimationStep animationStep = (AnimationStep)step->fAnimationStep;
    #endif
        if (fFlags == kWaiting)
        {
            /* Time to run this step */
            fFlags = kRunning;
            fStartMillis = currentMillis;
            fNum = 0;
        }
        if (fFlags == kRunning)
        {
        #ifdef ANONYMOUS_LAMBDAS_NOT_BROKEN
            if (animationStep != NULL)
            {
                fFlags |= 0x80;
                animationStep(*this, fStep, fNum++, currentMillis - fStartMillis);
                if ((fFlags & 0x80) == 0)
                    return;
                fFlags &= ~0x80;
            }
            if (duration != ~0 && (duration == 0 || fStartMillis + duration < currentMillis))
            {
                if (fStep + 1 < fNumSteps)
                    nextStep();
                else
                    end();
            }
        #else
            if (fAnimation(*this, fStep, fNum++, currentMillis - fStartMillis))
                fStep++;
        #endif
        }
    }

#ifdef ANONYMOUS_LAMBDAS_NOT_BROKEN
    void animateOnce(Animation animation[], unsigned numSteps)
    {
        reset();
        fAnimation = animation;
        fNumSteps = numSteps;
        fStartMillis = millis();
    }
#else
    /**
      * Play the specified animation script once.
      */
    void animateOnce(AnimationStep animation)
    {
        reset();
        fAnimation = animation;
        fStartMillis = millis();
    }
#endif

    /**
      * Advance to the next step in the active animation script.
      */
    void nextStep()
    {
        gotoStep(fStep + 1);
    }

    /**
      * Rewind to the previous step in the active animation script.
      */
    void previousStep()
    {
        if (fStep > 0)
            gotoStep(fStep - 1);
    }

    /**
      * Rewind to the beginning in the active animation script.
      */
    void rewind()
    {
        gotoStep(0);
    }


    /**
      * Stop the active animation script.
      */
    void end()
    {
        fFlags = kEnded;
    }

    /**
      * Go to to the specified step in the active animation script.
      */
    void gotoStep(unsigned step)
    {
    #ifdef ANONYMOUS_LAMBDAS_NOT_BROKEN
        if (step < fNumSteps)
    #endif
        {
            fStep = step;
            fFlags = kWaiting;
            fStartMillis = millis();
        #ifndef ANONYMOUS_LAMBDAS_NOT_BROKEN
            fLine = 0;
        #endif
        }
    }

    /**
      * Reset the animation player and stop any active animation script.
      */
    void reset()
    {
        fStep = 0;
    #ifdef ANONYMOUS_LAMBDAS_NOT_BROKEN
        fNumSteps = 0;
    #else
        fLine = 0;
    #endif
        fAnimation = NULL;
        fFlags = kWaiting;
        fStartMillis = millis();
    }

#ifndef ANONYMOUS_LAMBDAS_NOT_BROKEN
    unsigned fStep;
    unsigned fLine;
#endif

protected:
    enum
    {
        kWaiting = 0,
        kRunning = 1,
        kEnded = 0xFF
    };
    uint8_t fFlags;
    unsigned fNum;
    unsigned long fStartMillis;
#ifdef ANONYMOUS_LAMBDAS_NOT_BROKEN
    unsigned fStep;
    unsigned fNumSteps;
    Animation* fAnimation;
#else
    AnimationStep fAnimation;
#endif
};

#endif
