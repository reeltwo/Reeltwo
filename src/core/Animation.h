#ifndef Animation_h
#define Animation_h

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"

typedef bool (*AnimationStep)(class AnimationPlayer& animation, unsigned step, unsigned long num, unsigned long elapsedMillis);

#define ANIMATION(name) static bool Animation_##name(class AnimationPlayer& animation,\
    unsigned step, unsigned long num, unsigned long elapsedMillis) { enum { _firstStep = __COUNTER__ };
#define DO_START() switch (animation.fStep+1) {
#define DO_CASE() case __COUNTER__-_firstStep:
#define DO_LABEL(label) enum { label = __COUNTER__-_firstStep }; case label: return true;
#define DO_ONCE_LABEL(label, p) enum { label = __COUNTER__-_firstStep }; case label: { p; return true; }
#define DO_ONCE(p) DO_CASE() { p; return true; }
#define DO_ONCE_AND_WAIT(p, ms) DO_CASE() { { if (!animation.fRepeatStep) { p; } return (ms < elapsedMillis); } }
#define DO_FOREVER(p) DO_CASE() { p; return false; }
#define DO_DURATION(ms, p) DO_CASE() { if (elapsedMillis < ms) { p; return false; } return true; }
#define DO_WAIT_MILLIS(ms) DO_CASE() { return (ms < elapsedMillis); }
#define DO_WAIT_SEC(sec) DO_CASE() { return (sec*1000L < elapsedMillis); }
#define DO_GOTO(label) DO_CASE() animation.gotoStep(label); return true;
#define DO_WHILE(cond,label) DO_CASE() if (cond) { animation.gotoStep(label); return true; } return true;
#define DO_SEQUENCE(seq,mask) DO_CASE() { SEQUENCE_PLAY_ONCE(animation.fServoSequencer, SeqPanelAllOpenLong, DOME_PANELS_MASK); return true; }
#define DO_WHILE_SEQUENCE(seq,label) DO_CASE() if (seq) { animation.gotoStep(label); return true; } return true;
#define DO_END() default: animation.end(); return false; } }
#define ANIMATION_PLAY_ONCE(player, name) player.animateOnce(Animation_##name)

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
        if (fAnimation == NULL || fFlags == kEnded)
            return;
        unsigned long currentMillis = millis();
        if (fFlags == kWaiting)
        {
            /* Time to run this step */
            fFlags = kRunning;
            fStartMillis = currentMillis;
            fNum = 0;
        }
        if (fFlags == kRunning)
        {
            if (fAnimation(*this, fStep, fNum++, currentMillis - fStartMillis))
            {
                fRepeatStep = false;
                fNum = 0;
                fStep++;
                fStartMillis = currentMillis;
            }
            else
            {
                fRepeatStep = true;
            }
        }
    }

    /**
      * Play the specified animation script once.
      */
    void animateOnce(AnimationStep animation)
    {
        reset();
        fAnimation = animation;
        fStartMillis = millis();
    }

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
        fStep = step;
        fFlags = kWaiting;
        fStartMillis = millis();
    }

    /**
      * Reset the animation player and stop any active animation script.
      */
    void reset()
    {
        fStep = 0;
        fRepeatStep = false;
        fAnimation = NULL;
        fFlags = kWaiting;
        fStartMillis = millis();
    }

    unsigned fNumSteps;
    unsigned fStep;
    bool fRepeatStep;

protected:
    enum
    {
        kWaiting = 0,
        kRunning = 1,
        kEnded = 0xFF
    };
    uint8_t fFlags;
    unsigned long fNum;
    unsigned long fStartMillis;
    AnimationStep fAnimation;
};

#endif
