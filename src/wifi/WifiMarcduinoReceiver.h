#ifndef WifiMarcduinoReceiver_h
#define WifiMarcduinoReceiver_h

#include "ReelTwo.h"
#include "wifi/WifiAccess.h"
#include <WiFiClient.h>

#ifndef JAWALITE_IDLE_TIMEOUT
#define JAWALITE_IDLE_TIMEOUT 5000
#endif

/**
  * \ingroup wifi
  *
  * \class WifiMarcduinoReceiverBase
  *
  * \brief Base template of Marcduino receiver over WiFi
  *
  * Instances of this template will invoke the provided callback function when a complete
  * Marcduino command has been received over TCP/IP on port 2000 (default).
  *
  * \code
  * #include "wifi/WifiMarcduinoReceiver.h"
  *
  * WifiAccess wifiAccess("MyAccessPoint", "MyPassword");
  * 
  * WifiMarcduinoReceiver wifiMarcduinoReceiver(wifiAccess, 2000);
  * \endcode
  *
  * To support more than one client (for example) use:
  *
  * \code
  * WifiMarcduinoReceiverBase<2> wifiMarcduinoReceiver(wifiAccess, 2000);
  * \endcode
  *
  */
template<unsigned maxClients = 1, uint16_t BUFFER_SIZE = 64>
class WifiMarcduinoReceiverBase : public WiFiServer, public WifiAccess::Notify, public AnimatedEvent
{
public:
    WiFiClient fClients[maxClients];
    uint32_t fClientLastMsg[maxClients] = {};

    /** \brief Constructor
      *
      * Only a single instance of WifiSerialBridge should be created per sketch.
      *
      * \param port the port number of this service
      */
    WifiMarcduinoReceiverBase(WifiAccess &wifiAccess, uint16_t port = 2000) :
        WiFiServer(port)
    {
        wifiAccess.addNotify(this);
    }

    void setCommandHandler(void (*commandHandler)(const char* cmd))
    {
        fCommandHandler = commandHandler;
    }

    void setEnabled(bool enabled)
    {
        fEnabled = enabled;
    }

    bool enabled()
    {
        return fEnabled;
    }

    virtual void wifiConnected(WifiAccess& access) override
    {
        DEBUG_PRINTLN("WifiMarcduinoReceiverBase.wifiConnected");
        if (!fStarted && enabled())
        {
            begin();
            fStarted = true;
        }
    }

    virtual void wifiDisconnected(WifiAccess& access) override
    {
        DEBUG_PRINTLN("WifiMarcduinoReceiverBase.wifiDisconnected");
        for (unsigned i = 0; i < maxClients; i++)
        {
            if (fClients[i])
            {
                fClients[i].stop();
                fPos[i] = 0;
            }
        }
    }

    /**
      * Dispatch any received i2c event to CommandEvent
      */
    virtual void animate() override
    {
        unsigned i;
        if (!enabled())
            return;
        //check if there are any new clients
        if (hasClient())
        {
            ::printf("HAS CLIENT\n");
            for (i = 0; i < maxClients; i++)
            {
                //find free/disconnected spot
                if (!fClients[i] || !fClients[i].connected())
                {
                    if (fClients[i])
                        fClients[i].stop();
                    fClients[i] = available();
                    if (!fClients[i])
                        DEBUG_PRINTLN("available broken");
                    DEBUG_PRINT("New client: ");
                    DEBUG_PRINT(i); DEBUG_PRINT(' ');
                    DEBUG_PRINTLN(fClients[i].remoteIP());
                    fPos[i] = 0;
                    fClientLastMsg[i] = millis();
                    break;
                }
            }
            if (i >= maxClients)
            {
                //no free/disconnected spot so reject
                available().stop();
                DEBUG_PRINTLN("CLIENT REJECT");
            }
        }
        //check clients for data
        for (i = 0; i < maxClients; i++)
        {
            if (fClients[i] && fClients[i].connected())
            {
                if (fClients[i].available())
                {
                    //get data from the telnet client and push it to the UART
                    while (fClients[i].available())
                    {
                        char ch = fClients[i].read();
                        if (ch == 0x0D)
                        {
                            fBuffer[i][fPos[i]] = '\0';
                            fPos[i] = 0;
                            if (fCommandHandler != nullptr && fBuffer[i][0] != '\0')
                            {
                                fCommandHandler(fBuffer[i]);
                            }
                            fClientLastMsg[i] = millis();
                        }
                        else if (fPos[i] < BUFFER_SIZE-1)
                        {
                            fBuffer[i][fPos[i]++] = ch;
                        }
                        fClients[i].write(ch);
                    }
                }
                else if (fClientLastMsg[i] + JAWALITE_IDLE_TIMEOUT < millis())
                {
                    DEBUG_PRINTLN("CLIENT TIMEOUT");
                    fClients[i].stop();
                    fPos[i] = 0;
                }
            }
            else
            {
                if (fClients[i])
                {
                    DEBUG_PRINTLN("CLIENT DISCONNECTED");
                    fClients[i].stop();
                    fPos[i] = 0;
                }
            }
        }
    }

private:
    bool fStarted = false;
    bool fEnabled = true;
    void (*fCommandHandler)(const char* cmd) = nullptr;
    unsigned fPos[maxClients] = {};
    char fBuffer[maxClients][BUFFER_SIZE];
};

/**
  * \ingroup wifi
  *
  * \class WifiSerialBridge
  *
  * \brief Default instantiation of automatic forwarder from wifi to Serial
  *
  * Default instantiation of WifiSerialBridgeBase with max clients of 1.
  *
  * \code
  * #include "wifi/WifiSerialBridge.h"
  *
  * WifiSerialBridge wifiSerialBridge(2000);
  * \endcode
  *
  */
typedef WifiMarcduinoReceiverBase<> WifiMarcduinoReceiver;
#endif
