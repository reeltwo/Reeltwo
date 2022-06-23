#ifndef PinInterruptHandler_h
#define PinInterruptHandler_h

#include "ReelTwo.h"

class PinInterruptHandler
{
public:
    void detachInterrupt(unsigned pin)
    {
        unsigned inum = digitalPinToInterrupt(pin);
        ::detachInterrupt(inum);
        handlerToISR(inum) = nullptr;
    }

    bool attachInterrupt(unsigned pin)
    {
        uint8_t inum =  digitalPinToInterrupt(pin);
        // Serial.println("attachInterrupt pin="+String(pin)+" interrupt="+String(inum));
        handlerToISR(inum) = this;
        switch (inum) {
            #define ATTACH_INTTERUPT(num) \
                case num: \
                    ::attachInterrupt(inum, ISR_##num, CHANGE); \
                    break;
        #ifdef __AVR__
            ATTACH_INTTERUPT(0)
            ATTACH_INTTERUPT(1)
            ATTACH_INTTERUPT(2)
            ATTACH_INTTERUPT(3)
            ATTACH_INTTERUPT(4)
            ATTACH_INTTERUPT(5)
            ATTACH_INTTERUPT(6)
            ATTACH_INTTERUPT(7)
            ATTACH_INTTERUPT(8)
            ATTACH_INTTERUPT(9)
            ATTACH_INTTERUPT(10)
            ATTACH_INTTERUPT(11)
            ATTACH_INTTERUPT(12)
            ATTACH_INTTERUPT(13)
            ATTACH_INTTERUPT(14)
            ATTACH_INTTERUPT(15)
            ATTACH_INTTERUPT(16)
            ATTACH_INTTERUPT(17)
            ATTACH_INTTERUPT(18)
            ATTACH_INTTERUPT(19)
        #elif defined(ESP32)
            ATTACH_INTTERUPT(0)
            ATTACH_INTTERUPT(2)
            ATTACH_INTTERUPT(4)
            ATTACH_INTTERUPT(5)
            ATTACH_INTTERUPT(12)
            ATTACH_INTTERUPT(13)
            ATTACH_INTTERUPT(14)
            ATTACH_INTTERUPT(15)
            ATTACH_INTTERUPT(16)
            ATTACH_INTTERUPT(17)
            ATTACH_INTTERUPT(18)
            ATTACH_INTTERUPT(19)
            ATTACH_INTTERUPT(21)
            ATTACH_INTTERUPT(22)
            ATTACH_INTTERUPT(23)
            ATTACH_INTTERUPT(25)
            ATTACH_INTTERUPT(26)
            ATTACH_INTTERUPT(27)
            ATTACH_INTTERUPT(32)
            ATTACH_INTTERUPT(33)
            ATTACH_INTTERUPT(34)
            ATTACH_INTTERUPT(35)
            ATTACH_INTTERUPT(36)
            ATTACH_INTTERUPT(39)
        #else
            #error Not supported yet
        #endif

            #undef ATTACH_INTTERUPT
            default:
                return false;
        }
        return true;
    }

protected:
	virtual void interrupt() = 0;

private:
#ifdef ESP32
    #define MAX_ISR_COUNT 40
#else
    #define MAX_ISR_COUNT 20
#endif
    static PinInterruptHandler*& handlerToISR(int isr)
    {
        static PinInterruptHandler* sHandlers[MAX_ISR_COUNT+1];
        return sHandlers[(isr >= 0 && isr < MAX_ISR_COUNT) ? isr+1 : 0];
    }

    #define DEFINE_ISR_HANDLER(n) \
        static void ISR_##n() \
        { \
            PinInterruptHandler* handler = handlerToISR(n); \
            if (handler != nullptr) \
                handler->interrupt(); \
        }

#ifdef __AVR__
    DEFINE_ISR_HANDLER(0)
    DEFINE_ISR_HANDLER(1)
    DEFINE_ISR_HANDLER(2)
    DEFINE_ISR_HANDLER(3)
    DEFINE_ISR_HANDLER(4)
    DEFINE_ISR_HANDLER(5)
    DEFINE_ISR_HANDLER(6)
    DEFINE_ISR_HANDLER(7)
    DEFINE_ISR_HANDLER(8)
    DEFINE_ISR_HANDLER(9)
    DEFINE_ISR_HANDLER(10)
    DEFINE_ISR_HANDLER(11)
    DEFINE_ISR_HANDLER(12)
    DEFINE_ISR_HANDLER(13)
    DEFINE_ISR_HANDLER(14)
    DEFINE_ISR_HANDLER(15)
    DEFINE_ISR_HANDLER(16)
    DEFINE_ISR_HANDLER(17)
    DEFINE_ISR_HANDLER(18)
    DEFINE_ISR_HANDLER(19)
#elif defined(ESP32)
    DEFINE_ISR_HANDLER(0)
    DEFINE_ISR_HANDLER(2)
    DEFINE_ISR_HANDLER(4)
    DEFINE_ISR_HANDLER(5)
    DEFINE_ISR_HANDLER(12)
    DEFINE_ISR_HANDLER(13)
    DEFINE_ISR_HANDLER(14)
    DEFINE_ISR_HANDLER(15)
    DEFINE_ISR_HANDLER(16)
    DEFINE_ISR_HANDLER(17)
    DEFINE_ISR_HANDLER(18)
    DEFINE_ISR_HANDLER(19)
    DEFINE_ISR_HANDLER(21)
    DEFINE_ISR_HANDLER(22)
    DEFINE_ISR_HANDLER(23)
    DEFINE_ISR_HANDLER(25)
    DEFINE_ISR_HANDLER(26)
    DEFINE_ISR_HANDLER(27)
    DEFINE_ISR_HANDLER(32)
    DEFINE_ISR_HANDLER(33)
    DEFINE_ISR_HANDLER(34)
    DEFINE_ISR_HANDLER(35)
    DEFINE_ISR_HANDLER(36)
    DEFINE_ISR_HANDLER(39)
#else
    #error Not supported yet
#endif
#undef MAX_ISR_COUNT
};
#endif
