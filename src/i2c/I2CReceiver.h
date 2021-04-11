#ifndef I2CReceiver_h
#define I2CReceiver_h

#include "ReelTwo.h"
#include "core/AnimatedEvent.h"
#include "core/CommandEvent.h"
#include <Wire.h>

/**
  * \ingroup i2c
  *
  * \class I2CReceiverBase
  *
  * \brief Base template of automatic forwarder from i2c to CommandEvent
  *
  * Create an instance of this template to automatically forward i2c string commands to CommandEvent.
  * A convenience type of I2CReceiver is provided that uses the default buffer size of 32 bytes. Only
  * a single instance of I2CReceiver should be created per sketch.
  *
  * \code
  * #include "i2c/I2CReceiver.h"
  *
  * I2CReceiver i2cReceiver(0x19);
  * \endcode
  *
  * To create a receiver with a buffer size of 42 (for example) use:
  *
  * \code
  * I2CReceiverBase<42> i2cReceiver(0x19);
  * \endcode
  *
  */
template<int bufferSize = 32>
class I2CReceiverBase : public AnimatedEvent
{
public:
    /** \brief Constructor
      *
      * Only a single instance of I2CReceiverBase should be created per sketch.
      *
      * \param i2caddress i2c address of this controller
      */
    I2CReceiverBase()
    {
        *myself() = this;
        Wire.onReceive(i2cEvent);                // Register event so when we receive something we jump to i2cEvent();
    }

    /** \brief Constructor
      *
      * Only a single instance of I2CReceiverBase should be created per sketch.
      *
      * \param i2caddress i2c address of this controller
      */
    I2CReceiverBase(byte i2caddress)
    {
        *myself() = this;
        Wire.onReceive(i2cEvent);                // Register event so when we receive something we jump to i2cEvent();
        begin(i2caddress);
    }

    void begin(byte i2caddress = 0x19)
    {
        Wire.begin(i2caddress);                  // Connects to I2C Bus and establishes address.
    }

    /**
      * Dispatch any received i2c event to CommandEvent
      */
    virtual void animate() override
    {
        if (fCmdReady && *fCmdString != 0)
        {
            if (fCmdString[1] == 0 && fCmdString[0] >= 0 && fCmdString[0] <= 9)
            {
                fCmdString[0] += '0';
            }
            CommandEvent::process(fCmdString);
        }
        fCmdReady = false;
    }

private:
    char fCmdString[bufferSize];
    volatile bool fCmdReady = false;

    void handleEvent(int howMany)
    {
        UNUSED_ARG(howMany)
        /* ignore any new i2c event until previous one has been processed */
        if (fCmdReady)
            return;
        for (byte i = 0; Wire.available();)
        {
            char ch = (char)Wire.read();
            // Dont add leading whitespace
            if (i < sizeof(fCmdString) - 1 && (i != 0 || !isspace(ch)))
            {
                fCmdString[i++] = (ch != '\r') ? ch : '\n';
                fCmdString[i] = 0;
            }
        }
        // DEBUG_PRINTLN(fCmdString);
        fCmdReady = true;
    }

    static void i2cEvent(int howMany)
    {
        (*myself())->handleEvent(howMany);
    }

    static I2CReceiverBase<bufferSize>** myself()
    {
        static I2CReceiverBase<bufferSize>* self;
        return &self;
    }

    static constexpr bool sCreated = false;
};

/**
  * \ingroup i2c
  *
  * \class I2CReceiver
  *
  * \brief Default instantiation of automatic forwarder from i2c to CommandEvent
  *
  * Default instantiation of I2CReceiverBase with a buffer size of 32 bytes.
  *
  * \code
  * #include "i2c/I2CReceiver.h"
  *
  * I2CReceiver i2cReceiver(0x19);
  * \endcode
  *
  */
typedef I2CReceiverBase<> I2CReceiver;
#endif
