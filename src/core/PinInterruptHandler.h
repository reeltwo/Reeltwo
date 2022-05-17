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
        handlerToISR(inum) = this;
        switch (inum) {
            #define ATTACH_INTTERUPT(num) \
                case num: \
                    ::attachInterrupt(inum, ISR_##num, CHANGE); \
                    break;
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

            #undef ATTACH_INTTERUPT
            default:
                return false;
        }
        return true;
    }

protected:
	virtual void interrupt() = 0;

private:
    static PinInterruptHandler*& handlerToISR(unsigned isr)
    {
        static PinInterruptHandler* sHandlers[20];
        return sHandlers[isr];
    }

    #define DEFINE_ISR_HANDLER(n) \
        static void ISR_##n() \
        { \
            PinInterruptHandler* handler = handlerToISR(n); \
            if (handler != nullptr) \
                handler->interrupt(); \
        }

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
};
#endif
