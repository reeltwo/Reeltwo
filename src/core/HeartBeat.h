#ifndef HeartBeat_h
#define HeartBeat_h

#include "ReelTwo.h"

#ifdef USE_SMQ
static void sendHeartBeat(const char* id, uint32_t ms)
{
    static uint32_t sLastTime;
    if (sLastTime + ms < millis())
    {
        if (SMQ::sendTopic("PULSE"))
        {
            SMQ::send_string(F("id"), id);
            SMQ::send_end();
        }
        sLastTime = millis();
    }
}
#else
#define sendHeartBeat(id,ms)
#endif

#endif