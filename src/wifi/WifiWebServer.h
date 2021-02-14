#ifndef WifiWebServer_h
#define WifiWebServer_h

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

class WValue
{
public:
	virtual String get() = 0;
	virtual void set(String val) = 0;
};

class WBoolean : public WValue
{
public:
	WBoolean(bool (*getValue)(), void (*setValue)(bool)) :
		fGetValue(getValue),
		fSetValue(setValue)
	{
	}

	virtual String get() override
	{
		return (fGetValue != NULL) ? (fGetValue() ? "true" : "false") : "";
	}

	virtual void set(String val) override
	{
		if (fGetValue != nullptr)
			fSetValue(val.equalsIgnoreCase("true"));
	}

protected:
  	bool (*fGetValue)();
	void (*fSetValue)(bool);
};

class WInteger : public WValue
{
public:
	WInteger(int (*getValue)(), void (*setValue)(int)) :
		fGetValue(getValue),
		fSetValue(setValue)
	{
	}

	virtual String get()
	{
		if (fGetValue != nullptr)
			return String(fGetValue());
		return "";
	}

	virtual void set(String val) override
	{
		if (fSetValue != nullptr)
			fSetValue(val.toInt());
	}

protected:
  	int (*fGetValue)();
	void (*fSetValue)(int);
};

class WElement
{
public:
    WElement(WValue* value = nullptr) :
        fValue(value)
    {
    }

    inline String getID() const { return fID; }
    inline void appendCSS(String css)    { fCSS = fCSS + css; }
    inline void appendBody(String css)   { fBody = fBody + css; }
    inline void appendScript(String css) { fScript = fScript + css; }

    inline void emitCSS(WiFiClient& client) const
    {
        client.println(fCSS);
    }

    inline void emitBody(WiFiClient& client) const
    {
        client.println(fBody);
    }

    void emitValue(WiFiClient& client) const
    {
    	if (fValue != nullptr)
    		client.println("var "+String(fID)+"_val_ = "+fValue->get()+";\n");
    }

    inline void emitScript(WiFiClient& client) const
    {
        client.println(fScript);
    }

    String getValue() const
    {
    	return (fValue != nullptr) ? fValue->get() : "";
    }

    void setValue(String val) const
    {
    	if (fValue != nullptr)
    		fValue->set(val);
    }

protected:
	String fID = "";
    String fCSS = "";
    String fBody = "";
    String fScript = "";
  	WValue* fValue = nullptr;
};

class WSlider : public WElement
{
public:
    WSlider(String title, String id, int min, int max, int (*getValue)(), void (*setValue)(int)) :
    	WElement(new WInteger(getValue, setValue))
    {
    	fID = id;
        appendCSS("."+String(id)+"_css { width: 300px; }");
        appendBody("<p>"+String(title)+": <span id=\""+String(id)+"_val\"></span></p>\n");
        appendBody("<input type=\"range\" min=\""+String(min)+"\" max=\""+String(max)+"\" class=\""+String(id)+"_css\" id=\"" +String(id)+"_slider\" onchange=\"updateValue_"+String(id)+"(this.value)\"/>\n");
        appendScript("var "+String(id)+" = document.getElementById(\""+String(id)+"_slider\");\n");
    	appendScript(String(id)+".value = "+String(id)+"_val_;\n");
        appendScript("var "+String(id)+"_priv = document.getElementById(\""+String(id)+"_val\"); "+String(id)+"_priv.innerHTML = "+String(id)+".value;\n");
        appendScript(String(id)+".oninput = function() { "+String(id)+".value = this.value; "+String(id)+"_priv.innerHTML = this.value; }\n");
        appendScript("function updateValue_"+String(id)+"(pos) {fetch(\"/?"+String(id)+"=\" + pos + \"&\"); {Connection: close};}\n");
    }
};

class WCheckbox : public WElement
{
public:
    WCheckbox(String title, String id, bool (*getValue)(), void (*setValue)(bool)) :
    	WElement(new WBoolean(getValue, setValue))
    {
    	fID = id;
        appendCSS("."+String(id)+"_css { width: 300px; }");
        appendBody("<p><span id=\""+String(id)+"_val\"></span></p>\n");
        appendBody("<input type=\"checkbox\" id=\""+String(id)+"_cbox\" onchange=\"updateValue_"+String(id)+"(this.value)\"/>\n");
        appendBody("<label class=\""+String(id)+"_css\" for=\""+String(id)+"\">"+String(title)+"</label>\n");
        appendScript("var "+String(id)+" = document.getElementById(\""+String(id)+"_cbox\");\n");
    	appendScript(String(id)+".checked = "+String(id)+"_val_;\n");
        appendScript("function updateValue_"+String(id)+"(pos) {fetch(\"/?"+String(id)+"=\" + pos + \"&\"); {Connection: close};}\n");
    }
};

class W1 : public WElement
{
public:
    W1(String title)
    {
        appendBody("<h1>"+String(title)+"</h1>");
    }
};

class WImage : public WElement
{
public:
    WImage(String alt, String data)
    {
        appendBody("<p><img src=\"data:image/png;base64, ");
        appendBody(data);
		appendBody("\" alt=\""+String(alt)+"\"></p>");
    }
};

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
template<unsigned maxClients = 1, unsigned numElements = 0>
class WifiWebServer : public WiFiServer, public SetupEvent, public AnimatedEvent
{
public:
    WiFiClient fClients[maxClients];

    /** \brief Constructor
      *
      * Only a single instance of WifiSerialBridge should be created per sketch.
      *
      * \param port the port number of this service
      */
    WifiWebServer(const WElement pageContents[], const char* wifiAP = "Astromech", const char* wifiPassword = "R2D2", bool accessPoint = true, uint16_t port = 80) :
        WiFiServer(port),
        fWifiAP(wifiAP),
        fWifiPassword(wifiPassword),
        fWifiAccessPoint(accessPoint),
        fHeader(""),
        fRequest(""),
        fContents(pageContents)
    {
    }

    void setConnect(void (*callback)())
    {
    	fConnectedCallback = callback;
    }

    virtual void setup() override
    {
        if (fWifiAccessPoint)
        {
            Serial.println("Wifi Access Point: "+String(fWifiAP));
            Serial.println("Password: "+String(fWifiPassword));
            WiFi.softAP(fWifiAP, fWifiPassword);
            delay(500);
            IPAddress myIP = WiFi.softAPIP();
            DEBUG_PRINTLN("AP IP address: ");
            DEBUG_PRINTLN(myIP);
        }
        else
        {
            Serial.println("Joining: "+String(fWifiAP));
            WiFi.begin(fWifiAP, fWifiPassword);
            while (WiFi.status() != WL_CONNECTED) {
        		delay(500);
        		Serial.print(".");
      		}
      		// Print local IP address and start web server
      		Serial.println("");
      		Serial.println("WiFi connected.");
      		Serial.println("IP address: ");
      		Serial.println(WiFi.localIP());
        }
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
                    fRequest.reserve(4096);
                    fHeader.reserve(4096);
                    if (fConnectedCallback)
                    	fConnectedCallback();
                    break;
                }
            }
            if (i >= maxClients)
            {
                //no free/disconnected spot so reject
                Serial.println("NO CLIENTS AVAILABLE");
                available().stop();
            }
        }
        //check clients for data
        for (i = 0; i < maxClients; i++)
        {
            if (fClients[i] && fClients[i].connected())
            {
                //get data from the telnet client and push it to the UART
                while (fClients[i].available())
                {
					char c = fClients[i].read();
					//Serial.write(c);
					fHeader.concat(c);
					if (c == '\n')
					{
						// if the byte is a newline character
						// if the current line is blank, you got two newline characters in a row.
						// that's the end of the client HTTP request, so send a response:
						if (fRequest.length() == 0)
						{
							// HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
							// and a content-type so the client knows what's coming, then a blank line:
							fClients[i].println("HTTP/1.0 200 OK");
							fClients[i].println("Content-type:text/html");
							fClients[i].println("Connection: close");
							fClients[i].println();

							handleRequest(fClients[i]);

							// The HTTP response ends with another blank line
							fClients[i].println();
							// Break out of the while loop
							fHeader = "";
							fClients[i].stop();
							break;
						}
						else
						{
							fRequest = "";
						}
					}
					else if (c != '\r')
					{
						fRequest.concat(c);
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
    const char* fWifiAP;
    const char* fWifiPassword;
    bool fWifiAccessPoint;
    String fHeader;
    String fRequest;
    const WElement* fContents;
    void (*fConnectedCallback)() = nullptr;

    void handleRequest(WiFiClient& client)
    {
        client.println(
            R"RAW(
                <!DOCTYPE html><html>
                <head><meta name="viewport" content="width=device-width, initial-scale=1">
                <link rel="icon" href="data:,">
                <style>body { text-align: center; font-family: "Trebuchet MS", Arial; margin-left:auto; margin-right:auto;}
            )RAW");
        for (unsigned i = 0; i < numElements; i++)
        	fContents[i].emitCSS(client);
        client.println(
            R"RAW(
                </style>
                </head><body>
            )RAW");
        for (unsigned i = 0; i < numElements; i++)
        	fContents[i].emitBody(client);
        client.println("<script>");
        for (unsigned i = 0; i < numElements; i++)
        	fContents[i].emitValue(client);
        for (unsigned i = 0; i < numElements; i++)
        	fContents[i].emitScript(client);
        client.println(
            R"RAW(
                {Connection: close};
                </script>
                </body></html>
            )RAW");
        if (fHeader.startsWith("GET /?"))
        {
            int pos1 = fHeader.indexOf('?');
            int pos2 = fHeader.indexOf('=');
            int pos3 = fHeader.indexOf('&');
            if (pos1 != 0 && pos2 != 0 && pos3 != 0)
            {
            	String var = fHeader.substring(pos1+1, pos2);
            	String val = fHeader.substring(pos2+1, pos3);
            	DEBUG_PRINT("SET "); DEBUG_PRINT(var); DEBUG_PRINT(" = "); DEBUG_PRINTLN(val);
            	for (unsigned i = 0; i < numElements; i++)
            	{
            		if (var == fContents[i].getID())
            		{
            			fContents[i].setValue(val);
            			break;
            		}
            	}
            }
        }
    }
};


#endif