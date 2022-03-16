#ifndef MallocString_h
#define MallocString_h

#pragma once

// Utility subclass of Arduino String to give it a preallocated string buffer
// that will then be freed by the destructor of String.
class MallocString : public String
{
public:
    MallocString(char* mstr) :
        String((char*)NULL)
    {
        size_t len = strlen(mstr);
        setSSO(false);
        setCapacity(len+1);
        setBuffer(mstr);
        setLen(len);
    }
};

#endif
