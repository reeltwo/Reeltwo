#ifndef DomePositionProvider_h
#define DomePositionProvider_h

class DomePositionProvider
{
public:
    virtual bool ready() = 0;
    virtual int getAngle() = 0;
};

#endif

