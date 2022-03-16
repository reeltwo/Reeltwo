#ifndef WifiAccess_h
#define WifiAccess_h

#include "ReelTwo.h"
#include "core/AnimatedEvent.h"
#include <WiFi.h>
#include <WiFiAP.h>
#include <vector>

/**
  * \ingroup wifi
  *
  * \class WifiWebServer
  *
  * \brief Simple WiFi web server
  *
  * \code
  * #include "wifi/WifiAccess.h"
  *
  * WifiAccess wifiAccess("MyAccessPoint", "MyPassword");
  * 
  * \endcode
  *
  */
class WifiAccess : public AnimatedEvent
{
public:
    /** \brief Constructor
      *
      * \param interval the number of milliseconds before attempting to reconnect
      */
    WifiAccess(uint32_t interval = 30000) :
        fWifiAP(""),
        fWifiPassword(""),
        fWifiInterval(interval),
        fWifiSoftAP(false),
        fWifiEnabled(false),
        fWifiAttemptReconnect(true),
        fWifiActive(false)
    {
    }

    /** \brief Constructor
      *
      * \param interval the number of milliseconds before attempting to reconnect
      */
    WifiAccess(const String& wifiAP, const String& wifiPassword, bool accessPoint = true, bool enabled = true, uint32_t interval = 30000) :
        fWifiAP(wifiAP),
        fWifiPassword(wifiPassword),
        fWifiInterval(interval),
        fWifiSoftAP(accessPoint),
        fWifiEnabled(enabled),
        fWifiAttemptReconnect(true),
        fWifiActive(false)
    {
    }

    void setNetworkCredentials(const String& wifiAP, const String& wifiPassword, bool accessPoint, bool enabled)
    {
        fWifiAP = wifiAP;
        fWifiPassword = wifiPassword;
        fWifiSoftAP = accessPoint;
        fWifiEnabled = enabled;
    }

    bool isConnected()
    {
    	bool connected = false;
        if (fWifiActive)
        {
        	connected = (fWifiSoftAP) ? true : WiFi.isConnected();
        }
        return connected;
    }

    IPAddress getIPAddress()
    {
        if (isConnected())
        {
        	return (fWifiSoftAP) ? WiFi.softAPIP() : WiFi.localIP();
        }
        return IPAddress();
    }

    String getMacAddress()
    {
    	return WiFi.macAddress();
    }

    String getWifiAP()
    {
    	return fWifiAP;
    }

    String getWifiPassword()
    {
    	return fWifiPassword;
    }

    bool isSoftAP()
    {
    	return fWifiSoftAP;
    }

    void setReconnect(bool reconnect)
    {
    	fWifiAttemptReconnect = reconnect;
    }

    virtual void animate()
    {
        if (!fWifiEnabled)
            return;
        if (fWifiSoftAP)
        {
        	if (!fWifiActive)
        	{
	            DEBUG_PRINTLN("Wifi Access Point: "+fWifiAP);
	            DEBUG_PRINTLN("Password: "+fWifiPassword);
	            WiFi.softAP(fWifiAP.c_str(), fWifiPassword.c_str());
	            IPAddress myIP = WiFi.softAPIP();
	            fWifiActive = true;
	            notifyConnected();
	        }
        }
        else if (!fWifiStarted)
        {
            DEBUG_PRINTLN("Joining: "+fWifiAP);
            WiFi.begin(fWifiAP.c_str(), fWifiPassword.c_str());
            fWifiStarted = true;
        }
        else if (!WiFi.isConnected())
        {
        	uint32_t now = millis();
        	if (fWifiActive)
        	{
        		notifyDisconnected();
        		fWifiActive = false;
        	}
        	if (fWifiWasConnected && now - fWifiPrevMillis >= fWifiInterval)
        	{
        		DEBUG_PRINTLN("Reconnecting: "+fWifiAP);
				WiFi.disconnect();
				WiFi.reconnect();
				fWifiPrevMillis = now;
        	}
        }
        else if (!fWifiWasConnected || !fWifiActive)
        {
        	fWifiWasConnected = true;
            fWifiActive = true;
			notifyConnected();
        }
    }

    inline bool enabled()
    {
    	return fWifiEnabled;
    }

    void setEnabled(bool enabled = true)
    {
    	fWifiEnabled = enabled;
    }

	class Notify
	{
	public:
		virtual void wifiConnected(WifiAccess &access) = 0;
		virtual void wifiDisconnected(WifiAccess &access) = 0;
	};

	void addNotify(WifiAccess::Notify* client)
	{
		fWifiNotify.push_back(client);
	}

	void notifyWifiConnected(void (*notify)(WifiAccess& wifi))
	{
		fNotifyConnectedCallback = notify;
	}

	void notifyWifiDisconnected(void (*notify)(WifiAccess& wifi))
	{
		fNotifyDisconnectedCallback = notify;
	}

protected:
    String fWifiAP;
    String fWifiPassword;
    uint32_t fWifiInterval;
    uint32_t fWifiPrevMillis = 0;
    bool fWifiSoftAP;
    bool fWifiStarted = false;
    bool fWifiEnabled = true;
	bool fWifiAttemptReconnect = true;
    bool fWifiActive = false;
    bool fWifiWasConnected = false;
    std::vector<Notify*> fWifiNotify;
    void (*fNotifyConnectedCallback)(WifiAccess& wifi) = nullptr;
    void (*fNotifyDisconnectedCallback)(WifiAccess& wifi) = nullptr;

    void notifyConnected()
    {
    	for (auto client: fWifiNotify)
    	{
    		client->wifiConnected(*this);
    	}
    	if (fNotifyConnectedCallback != nullptr)
    		fNotifyConnectedCallback(*this);
    }

    void notifyDisconnected()
    {
    	if (fNotifyDisconnectedCallback != nullptr)
    		fNotifyDisconnectedCallback(*this);
    	for (auto client: fWifiNotify)
    	{
    		client->wifiDisconnected(*this);
    	}
    }
};

#endif
