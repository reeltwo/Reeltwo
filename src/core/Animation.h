#ifndef Animation_h
#define Animation_h

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include "core/CommandEvent.h"
#include "ServoSequencer.h"

typedef signed char (*AnimationStep)(class AnimationPlayer& animation, unsigned step, unsigned long num, unsigned long elapsedMillis);

#define ANIMATION_FUNC_DECL(name) static signed char Animation_##name(class AnimationPlayer& animation,\
    unsigned step, unsigned long num, unsigned long elapsedMillis)
#define ANIMATION(name) ANIMATION_FUNC_DECL(name) { enum { _firstStep = __COUNTER__ };
#define DO_START() UNUSED_ARG(num) UNUSED_ARG(elapsedMillis) switch (step+1) {
#define DO_CASE() case __COUNTER__-_firstStep:
#define DO_LABEL(label) enum { label = __COUNTER__-_firstStep }; case label: { return true; }
#define DO_ONCE_LABEL(label, p) enum { label = __COUNTER__-_firstStep }; case label: { p; return true; }
#define DO_ONCE(p) DO_CASE() { p; return true; }
#define DO_ONCE_AND_WAIT(p, ms) DO_CASE() { if (!animation.fRepeatStep) { p; } return (ms < elapsedMillis); }
#define DO_FOREVER(p) DO_CASE() { p; return false; }
#define DO_DURATION(ms, p) DO_CASE() { if (animation.getCurrentTimeMillis() < ms) { p; return false; } return true; }
#define DO_DURATION_STEP(ms, p) DO_CASE() { if (elapsedMillis < ms) { p; return false; } return true; }
#define DO_WAIT_MILLIS(ms) DO_CASE() { return (ms < elapsedMillis); }
#define DO_WAIT_SEC(sec) DO_CASE() { return (sec*1000L < long(elapsedMillis)); }
#define DO_GOTO(label) DO_CASE() { animation.gotoStep(label); return -1; }
#define DO_WHILE(cond,label) DO_CASE() { if (cond) { animation.gotoStep(label); return -1; } return true; }
#define DO_SEQUENCE(seq,mask) DO_CASE() { SEQUENCE_PLAY_ONCE(*animation.fServoSequencer, seq, mask); return true; }
#define DO_SEQUENCE_SPEED(seq,mask,speed) DO_CASE() { SEQUENCE_PLAY_ONCE_SPEED(*animation.fServoSequencer, seq, mask, speed); return true; }
#define DO_SEQUENCE_VARSPEED(seq,mask,minspeed, maxspeed) DO_CASE() { SEQUENCE_PLAY_ONCE_VARSPEED(*animation.fServoSequencer, seq, mask, minspeed, maxspeed); return true; }
#define DO_SEQUENCE_RANDOM_STEP(seq,mask) DO_CASE() { SEQUENCE_PLAY_RANDOM_STEP(*animation.fServoSequencer, seq, mask); return true; }
#define DO_WAIT_SEQUENCE() DO_CASE() { return animation.fServoSequencer->isFinished(); }
#define DO_WHILE_SEQUENCE(label) DO_CASE() { if (!animation.fServoSequencer->isFinished()) { animation.gotoStep(label); return -1; } return true; }
#define DO_COMMAND(cmd) DO_CASE() { CommandEvent::process(cmd); return true; }
#define DO_COMMAND_AND_WAIT(cmd, ms) DO_CASE() { if (!animation.fRepeatStep) { CommandEvent::process(cmd); } return (ms < elapsedMillis); }
#define DO_MARCDUINO(cmd) DO_CASE() { Marcduino::send(cmd); return true; }
#define DO_MARCDUINO_AND_WAIT(cmd, ms) DO_CASE() { if (!animation.fRepeatStep) { Marcduino::send(cmd); } return (ms < elapsedMillis); }
#define DO_RESET(p) case 0: { p; return true; }
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
  *     DO_START()
  *     // Step One - fires once and waits 300ms before advancing
  *     DO_SEQUENCE(SeqPanelAllOpenLong, DOME_PANELS_MASK)
  *     // Step Two - fires once
  *     DO_COMMAND_AND_WAIT("HPF0026|20", 100)
  *     // Step Three - fires repeatedly for 200ms
  *     DO_DURATION(200, {
  *         DEBUG_PRINTLN(elapsedMillis);
  *     })
  *     // Step Four
  *     DO_ONCE({ frontHolo.play("Leia.bd2"); })
  *     // Step Five
  *     DO_COMMAND("HPF0026|20")
  *     // Step Six - repeat this step until "num" reaches 100 then rewind the animation
  *     DO_FOREVER({
  *         DEBUG_PRINTLN(num);
  *         if (num == 100)
  *             animation.rewind();
  *     })
  *     DO_END()
  * }
  * \endcode
  */
class AnimationPlayer : public AnimatedEvent
{
public:
    /**
      * \brief Default Constructor
      */
    inline AnimationPlayer() :
        fServoSequencer(NULL)
    {
        reset();
    }

    /**
      * \brief Default Constructor
      */
    inline AnimationPlayer(ServoSequencer &sequencer) :
        fServoSequencer(&sequencer)
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
            fStepMillis = currentMillis;
            fNum = 0;
        }
        if (fFlags == kRunning)
        {
            signed char ret = fAnimation(*this, fStep, fNum++, currentMillis - fStepMillis);
            if (ret == 1)
            {
                fRepeatStep = false;
                fNum = 0;
                fStep++;
                fStepMillis = currentMillis;
            }
            else if (ret == 0)
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
        // End any current animation
        end();
        fAnimation = animation;
        fStartMillis = fStepMillis = millis();
    }

    long getCurrentTimeMillis()
    {
        return (fFlags == kRunning) ? millis() - fStartMillis : 0;
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
        if (fFlags == kRunning)
        {
            fFlags = kEnded;
            if (fAnimation != NULL)
            {
                // Run reset step on current animation
                fAnimation(*this, ~0L, 0, millis() - fStepMillis);
            }
        }
        reset();
    }

    /**
      * Go to to the specified step in the active animation script.
      */
    void gotoStep(unsigned step)
    {
        fNum = 0;
        fStep = (step > 0) ? step-1 : 0;
        fFlags = kWaiting;
        fStepMillis = millis();
        fRepeatStep = false;
    }

    /**
      * Reset the animation player and stop any active animation script.
      */
    void reset()
    {
        fNum = 0;
        fStep = 0;
        fRepeatStep = false;
        fAnimation = NULL;
        fFlags = kWaiting;
        fStepMillis = millis();
    }

    unsigned fNumSteps;
    unsigned fStep;
    bool fRepeatStep;
    ServoSequencer* fServoSequencer;

protected:
    enum
    {
        kWaiting = 0,
        kRunning = 1,
        kEnded = 0xFF
    };
    uint8_t fFlags;
    unsigned long fNum;
    unsigned long fStepMillis;
    unsigned long fStartMillis;
    AnimationStep fAnimation;
};

#endif
