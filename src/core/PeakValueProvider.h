#ifndef PeakValueProvider_h
#define PeakValueProvider_h

#include "ReelTwo.h"

/**
  * \ingroup Core
  *
  * \class PeakValueProvider
  *
  * \brief Base class peak value providers, currently only microphone amplitude.
*/
class PeakValueProvider
{
public:
    /**
      * \brief Default Constructor
      */
    PeakValueProvider() :
        fPeakValue(0)
    {}

    /**
      * \returns last recorded peak value
      */
    virtual byte getPeakValue()
    {
        return fPeakValue;
    }

    /**
      * \returns a zero PeakValueProvider. The zero provider will only return zero.
      */
    static PeakValueProvider* getZero()
    {
        static PeakValueProvider zeroProvider;
        return &zeroProvider;
    }

protected:
    byte fPeakValue;
};

#endif