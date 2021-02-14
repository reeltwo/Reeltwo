#ifndef WifiSerialBridge_h
#define WifiSerialBridge_h

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

/**
  * \ingroup wifi
  *
  * \class WifiSerialBridgeBase
  *
  * \brief Base template of automatic forwarder from i2c to CommandEvent
  *
  * Create an instance of this template to automatically forward i2c string commands to CommandEvent.
  * A convenience type of I2CReceiver is provided that uses the default buffer size of 32 bytes. Only
  * a single instance of I2CReceiver should be created per sketch.
  *
  * \code
  * #include "wifi/WifiSerialBridge.h"
  *
  * WifiSerialBridge wifiSerialBridge(2000);
  * \endcode
  *
  * To support more than one client (for example) use:
  *
  * \code
  * WifiSerialBridgeBase<2> wifiSerialBridge(2000);
  * \endcode
  *
  */
template<unsigned maxClients = 1>
class WifiSerialBridgeBase : public WiFiServer, public SetupEvent, public AnimatedEvent
{
public:
    WiFiClient fClients[maxClients];

    /** \brief Constructor
      *
      * Only a single instance of WifiSerialBridge should be created per sketch.
      *
      * \param port the port number of this service
      */
    WifiSerialBridgeBase(HardwareSerial& serial, const char* wifiAP = "Astromech", const char* wifiPassword = "R2D2", int channel = 10, uint16_t port = 2000) :
        WiFiServer(port),
        fSerial(serial),
        fWifiAP(wifiAP),
        fWifiPassword(wifiPassword),
        fWifiChannel(channel)
    {
    }

    virtual void setup() override
    {
        int channel = fWifiChannel;
        channel = (channel >= 1 && channel < 14) ? channel : 1;
        WiFi.softAP(fWifiAP, fWifiPassword);
        delay(500);
        IPAddress myIP = WiFi.softAPIP();
        DEBUG_PRINTLN("AP IP address: ");
        DEBUG_PRINTLN(myIP);
        begin();
    }

    /**
      * Dispatch any received i2c event to CommandEvent
      */
    virtual void animate() override
    {
        unsigned i;
        //check if there are any new clients
        if (hasClient())
        {
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
                    break;
                }
            }
            if (i >= maxClients)
            {
                //no free/disconnected spot so reject
                available().stop();
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
                        fClients[i].write(ch);
                        fSerial.write(ch);
                    }
                }
            }
            else
            {
                if (fClients[i])
                {
                    fClients[i].stop();
                }
            }
        }
    }

private:
    HardwareSerial& fSerial;
    const char* fWifiAP;
    const char* fWifiPassword;
    int fWifiChannel;
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
typedef WifiSerialBridgeBase<> WifiSerialBridge;
#endif
