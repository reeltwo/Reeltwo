#ifndef Marcduino_h
#define Marcduino_h

#include "ReelTwo.h"
#include "core/Animation.h"

#define MARCDUINO_ANIMATION(name, marc) \
    ANIMATION_FUNC_DECL(name); \
    MarcduinoAnimation Marc_##name(Animation_##name, STRID_CONST(#marc)); \
    ANIMATION(name)

class MarcduinoAnimation
{
public:
    MarcduinoAnimation(AnimationStep animation, uint32_t marc) :
        fAnimation(animation),
        fMarc(marc),
        fNext(NULL)
    {
        if (*head() == NULL)
            *head() = this;
        if (*tail() != NULL)
            (*tail())->fNext = this;
        *tail() = this;
    }

    static AnimationStep lookup(uint32_t marc)
    {
        for (MarcduinoAnimation* evt = *head(); evt != NULL; evt = evt->fNext)
        {
            if (evt->fMarc == marc)
                return evt->fAnimation;
        }
    }

private:
    uint32_t fMarc;
    AnimationStep fAnimation;
    MarcduinoAnimation* fNext;

    static MarcduinoAnimation** head()
    {
        static MarcduinoAnimation* sHead;
        return &sHead;
    }

    static MarcduinoAnimation** tail()
    {
        static MarcduinoAnimation* sTail;
        return &sTail;
    }
};

class Marcduino
{
public:
    static void processCommand(AnimationPlayer& player, const char* cmd)
    {
        AnimationStep animation =
            MarcduinoAnimation::lookup(STRID_CONST(cmd));
        if (animation != NULL)
        {
            player.animateOnce(animation);
        }
    } 
};

class MarcduinoSerial : public AnimatedEvent
{
public:
    MarcduinoSerial(Stream &stream, AnimationPlayer &player) :
        fStream(&stream),
        fPlayer(&player),
        fPos(0)
    {
    }

    virtual void animate()
    {
        if (fStream->available())
        {
            int ch = fStream->read();
            if (ch == 0x0D)
            {
                fBuffer[fPos] = '\0';
                fPos = 0;
                if (*fBuffer != '\0')
                {
                    Marcduino::processCommand(*fPlayer, fBuffer);
                }
            }
            else if (fPos < SizeOfArray(fBuffer))
            {
                fBuffer[fPos++] = ch;
            }
        }
    }

private:
    Stream* fStream;
    AnimationPlayer* fPlayer;
    char fBuffer[64];
    int fPos;
};

#endif
