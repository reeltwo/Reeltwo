#ifndef Marcduino_h
#define Marcduino_h

#include "ReelTwo.h"
#include "core/Animation.h"
#include "core/JawaCommander.h"

#define MARCDUINO_ANIMATION(name, marc) \
    ANIMATION_FUNC_DECL(name); \
    const char _marc_msg_##name[] PROGMEM = #marc; \
    Marcduino Marc_##name(Animation_##name, _marc_msg_##name); \
    ANIMATION(name)

#define MARCDUINO_ACTION(name, marc, p) \
    MARCDUINO_ANIMATION(name, marc) \
    { \
        DO_START() \
        DO_ONCE(p) \
        DO_END() \
    }

class Marcduino
{
public:
    Marcduino(AnimationStep animation, const char* marc /* PROGMEM */) :
        fMarc(marc),
        fAnimation(animation),
        fNext(NULL)
    {
        if (*head() == NULL)
            *head() = this;
        if (*tail() != NULL)
            (*tail())->fNext = this;
        *tail() = this;
    }

    static void processCommand(AnimationPlayer& player, const char* cmd)
    {
        bool found = false;
        for (Marcduino* marc = *head(); marc != NULL; marc = marc->fNext)
        {
            int len = strlen_P(marc->fMarc);
            if (strncmp_P(cmd, marc->fMarc, len) == 0)
            {
                AnimationStep animation = marc->fAnimation;
                if (animation != NULL)
                {
                    *command() = cmd + len;
                    player.animateOnce(animation);
                    found = true;
                }
            }
        }
        // Check for unprocess Jawa lite command
        if (!found && *cmd == '@')
        {
            JawaCommanderBase* base = JawaCommanderBase::get();
            if (base != NULL)
            {
                base->process(cmd+1);
            }
        }
    } 

    static void send(PROGMEMString cmd)
    {
    #ifndef USE_SMQ
        UNUSED_ARG(cmd)
    #else
        SMQ::send_start(F("MARC"));
        SMQ::send_string(F("cmd"), cmd);
        SMQ::send_end();
    #endif
    }

    static void send(const char* cmd)
    {
    #ifndef USE_SMQ
        UNUSED_ARG(cmd)
    #else
        SMQ::send_start(F("MARC"));
        SMQ::send_string(F("cmd"), cmd);
        SMQ::send_end();
    #endif
    }

    static const char* getCommand()
    {
        return *command();
    }

private:
    const char* fMarc;
    AnimationStep fAnimation;
    Marcduino* fNext;

    static const char** command()
    {
        static const char* sCmd;
        return &sCmd;
    }

    static Marcduino** head()
    {
        static Marcduino* sHead;
        return &sHead;
    }

    static Marcduino** tail()
    {
        static Marcduino* sTail;
        return &sTail;
    }
};

// class MarcduinoSerial : public AnimatedEvent
// {
// public:
//     MarcduinoSerial(Stream &stream, AnimationPlayer &player) :
//         fStream(&stream),
//         fPlayer(&player),
//         fPos(0)
//     {
//     }

//     virtual void animate()
//     {
//         if (fStream->available())
//         {
//             int ch = fStream->read();
//             if (ch == 0x0D)
//             {
//                 fBuffer[fPos] = '\0';
//                 fPos = 0;
//                 if (*fBuffer != '\0')
//                 {
//                     Marcduino::processCommand(*fPlayer, fBuffer);
//                 }
//             }
//             else if (fPos < SizeOfArray(fBuffer))
//             {
//                 fBuffer[fPos++] = ch;
//             }
//         }
//     }

// private:
//     Stream* fStream;
//     AnimationPlayer* fPlayer;
//     char fBuffer[64];
//     unsigned fPos;
// };

#endif
