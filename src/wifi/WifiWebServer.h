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
    virtual bool getQuoteValue() { return false; }
	virtual String get() = 0;
	virtual void set(String val) = 0;
};

class WAction
{
public:
    WAction(void (*action)()) :
        fAction(action)
    {
    }

    void perform()
    {
        if (fAction != nullptr)
            fAction();
    }

protected:
    void (*fAction)();
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

	virtual String get() override
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

class WString : public WValue
{
public:
    WString(String (*getValue)(), void (*setValue)(String)) :
        fGetValue(getValue),
        fSetValue(setValue)
    {
    }

    virtual bool getQuoteValue() override
    {
        return true;
    }

    virtual String get() override
    {
        if (fGetValue != nullptr)
            return fGetValue();
        return "";
    }

    virtual void set(String val) override
    {
        if (fSetValue != nullptr)
            fSetValue(val);
    }

protected:
    String (*fGetValue)();
    void (*fSetValue)(String);
};

class WElement
{
public:
    WElement(WValue* value = nullptr) :
        fValue(value)
    {
    }

    WElement(WAction* action) :
        fAction(action)
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

    virtual void emitValue(WiFiClient& client) const
    {
    	if (fValue != nullptr)
        {
            if (fValue->getQuoteValue())
                client.println("var "+String(fID)+"_val_ = \""+fValue->get()+"\";\n");
            else
                client.println("var "+String(fID)+"_val_ = "+fValue->get()+";\n");
        }
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
        else if (fAction != nullptr)
            fAction->perform();
    }

protected:
	String fID = "";
    String fCSS = "";
    String fBody = "";
    String fScript = "";
  	WValue* fValue = nullptr;
    WAction* fAction = nullptr;
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
        appendBody("<label class=\""+String(id)+"_css\" for=\""+String(id)+"_cbox\">"+String(title)+"</label>\n");
        appendScript("var "+String(id)+" = document.getElementById(\""+String(id)+"_cbox\");\n");
    	appendScript(String(id)+".checked = "+String(id)+"_val_;\n");
        appendScript("function updateValue_"+String(id)+"(pos) {fetch(\"/?"+String(id)+"=\" + pos + \"&\"); {Connection: close};}\n");
    }
};

class WButton : public WElement
{
public:
    WButton(String title, String id, void (*pressed)()) :
        WElement(new WAction(pressed))
    {
        fID = id;
        appendCSS("."+String(id)+"_css { width: 300px; }");
        appendBody("<p><span id=\""+String(id)+"_val\"></span></p>\n");
        appendBody("<input type=\"button\" id=\""+String(id)+"_btn\" value=\""+title+"\" onclick=\"pressed_"+String(id)+"()\"/>\n");
        appendScript("function pressed_"+String(id)+"() {fetch(\"/?"+String(id)+"=true&\"); {Connection: close};}\n");
    }
};

class WTextField : public WElement
{
public:
    WTextField(String title, String id, String (*getValue)(), void (*setValue)(String)) :
        WElement(new WString(getValue, setValue))
    {
        fID = id;
        appendCSS("."+String(id)+"_css { width: 300px; }");
        appendBody("<p><span id=\""+String(id)+"_val\"></span></p>\n");
        appendBody("<label class=\""+String(id)+"_css\" for=\""+String(id)+"_fld\">"+String(title)+"</label>\n");
        appendBody("<input type=\"text\" id=\""+String(id)+"_fld\" onchange=\"updateValue_"+String(id)+"(this.value)\"/>\n");
        appendScript("var "+String(id)+" = document.getElementById(\""+String(id)+"_fld\");\n");
        appendScript(String(id)+".value = "+String(id)+"_val_;\n");
        appendScript("function updateValue_"+String(id)+"(pos) {fetch(\"/?"+String(id)+"=\" + pos + \"&\"); {Connection: close};}\n");
    }
};

class WPassword : public WElement
{
public:
    WPassword(String title, String id, String (*getValue)(), void (*setValue)(String)) :
        WElement(new WString(getValue, setValue))
    {
        fID = id;
        appendCSS("."+String(id)+"_css { width: 300px; }");
        appendBody("<p><span id=\""+String(id)+"_val\"></span></p>\n");
        appendBody("<label class=\""+String(id)+"_css\" for=\""+String(id)+"_fld\">"+String(title)+"</label>\n");
        appendBody("<input type=\"password\" id=\""+String(id)+"_fld\" onchange=\"updateValue_"+String(id)+"(this.value)\"/>\n");
        appendScript("var "+String(id)+" = document.getElementById(\""+String(id)+"_fld\");\n");
        appendScript(String(id)+".value = "+String(id)+"_val_;\n");
        appendScript("function updateValue_"+String(id)+"(pos) {fetch(\"/?"+String(id)+"=\" + pos + \"&\"); {Connection: close};}\n");
    }
};

class WFileInput : public WElement
{
public:
    WFileInput(String title, String id, String (*getValue)(), void (*setValue)(String)) :
        WElement(new WString(getValue, setValue))
    {
        fID = id;
        appendCSS("."+String(id)+"_css { width: 300px; }");
        appendBody("<p><span id=\""+String(id)+"_val\"></span></p>\n");
        appendBody("<label class=\""+String(id)+"_css\" for=\""+String(id)+"_file\">"+String(title)+"</label>\n");
        appendBody("<input type=\"file\" id=\""+String(id)+"_file\" onchange=\"updateValue_"+String(id)+"(this)\"/>\n");
        appendScript("var "+String(id)+" = document.getElementById(\""+String(id)+"_file\");\n");
        appendScript("function updateValue_"+String(id)+"(pos) {);}\n");
    }
};

class WFirmwareFile : public WElement
{
public:
    WFirmwareFile(String title, String id)
    {
        fID = id;
        appendCSS("."+String(id)+"_css { width: 300px; }");
        appendBody("<p><span id=\""+String(id)+"_val\"></span></p>\n");
        appendBody("<label class=\""+String(id)+"_css\" for=\""+String(id)+"_file\">"+String(title)+"</label>\n");
        appendBody("<input type=\"file\" id=\""+String(id)+"_file\" accept=\".bin\" onchange=\"updateValue_"+String(id)+"(this)\"/>\n");
        appendScript("var "+String(id)+" = document.getElementById(\""+String(id)+"_file\");\n");
        appendScript("function updateValue_"+String(id)+"(pos) { document.getElementById(\""+String(id)+"_upload\").disabled = false; }\n");
    }
};

class WFirmwareUpload : public WElement
{
public:
    WFirmwareUpload(String title, String id)
    {
        fID = id;
        appendCSS("."+String(id)+"_css { width: 300px; }");
        appendCSS("#bar,#prgbar{background-color:#f1f1f1;border-radius:10px}#bar{background-color:#3498db;width:0%;height:10px}");
        appendBody("<p><span id=\""+String(id)+"_val\"></span></p>\n");
        appendBody("<input type=\"button\" id=\""+String(id)+"_upload\" value=\""+title+"\" onclick=\"upload_"+String(id)+"()\"/>\n");
        appendBody("<br><br>\n");
        appendBody("<div id='prg'></div>\n");
        appendBody("<br><div id='prgbar'><div id='bar'></div></div><br></form>\n");
        appendScript("var "+String(id)+"_upload = document.getElementById(\""+String(id)+"_upload\");\n");
        appendScript(String(id)+"_upload.disabled = true;\n");
        appendScript("function upload_"+String(id)+"() {\n");
        appendScript("var xhr = new XMLHttpRequest();\n");
        appendScript("xhr.addEventListener('progress', function(evt) {\n");
        appendScript("    if (evt.lengthComputable) {\n");
        appendScript("        var per = evt.loaded / evt.total;\n");
        appendScript("        $('#prg').html('progress: ' + Math.round(per*100) + '%');\n");
        appendScript("        $('#bar').css('width',Math.round(per*100) + '%');\n");
        appendScript("    }\n");
        appendScript("}, false);\n");
        appendScript("xhr.onload = function(e) {\n");
        appendScript("    if (this.readyState === 4) {\n");
        appendScript("        console.log('Server returned: ', e.target.responseText);\n");
        appendScript("    }\n");
        appendScript("}\n");
        appendScript("xhr.open('POST', 'upload/firmware', true);\n");
        appendScript("xhr.send("+String(id)+".files[0]);\n");
        appendScript("}\n");
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

class WPage
{
public:
    WPage(String url, const WElement contents[], unsigned numElements) :
        fURL(url),
        fNumElements(numElements),
        fContents(contents)
    {
    }

    inline const String& getURL() const
    {
        return fURL;
    }

    void handleRequest(WiFiClient& client, String &header) const
    {
        DEBUG_PRINTLN("handleRequest1");
        client.println(
            R"RAW(
                <!DOCTYPE html><html>
                <head><meta name="viewport" content="width=device-width, initial-scale=1">
                <link rel="icon" href="data:,">
                <style>body { text-align: center; font-family: "Trebuchet MS", Arial; margin-left:auto; margin-right:auto;}
            )RAW");
        for (unsigned i = 0; i < fNumElements; i++)
            fContents[i].emitCSS(client);
        client.println(
            R"RAW(
                </style>
                </head><body>
            )RAW");
        for (unsigned i = 0; i < fNumElements; i++)
            fContents[i].emitBody(client);
        client.println("<script>");
        for (unsigned i = 0; i < fNumElements; i++)
            fContents[i].emitValue(client);
        for (unsigned i = 0; i < fNumElements; i++)
            fContents[i].emitScript(client);
        client.println(
            R"RAW(
                {Connection: close};
                </script>
                </body></html>
            )RAW");
        DEBUG_PRINTLN("handleRequest2");
        if (header.startsWith("GET /?"))
        {
            int pos1 = header.indexOf('?');
            int pos2 = header.indexOf('=');
            int pos3 = header.indexOf('&');
            if (pos1 != 0 && pos2 != 0 && pos3 != 0)
            {
                String var = header.substring(pos1+1, pos2);
                String val = header.substring(pos2+1, pos3);
                DEBUG_PRINT("SET "); DEBUG_PRINT(var); DEBUG_PRINT(" = "); DEBUG_PRINTLN(val);
                for (unsigned i = 0; i < fNumElements; i++)
                {
                    if (var == fContents[i].getID())
                    {
        DEBUG_PRINTLN("SETVALUE1");
                        fContents[i].setValue(val);
        DEBUG_PRINTLN("SETVALUE2");
                        break;
                    }
                }
            }
        }
        DEBUG_PRINTLN("handleRequest3");
    }

protected:
    String fURL;
    unsigned fNumElements;
    const WElement* fContents;
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
template<unsigned maxClients = 10, unsigned numPages = 0>
class WifiWebServer : public WiFiServer, public SetupEvent /*, public AnimatedEvent*/
{
public:
    WiFiClient fClients[maxClients];

    /** \brief Constructor
      *
      * Only a single instance of WifiSerialBridge should be created per sketch.
      *
      * \param port the port number of this service
      */
    WifiWebServer(const WPage pages[], const char* wifiAP = "Astromech", const char* wifiPassword = "R2D2", bool accessPoint = true, uint16_t port = 80) :
        WiFiServer(port),
        fWifiAP(wifiAP),
        fWifiPassword(wifiPassword),
        fWifiAccessPoint(accessPoint),
        fEnabled(false),
        fHeader(""),
        fRequest(""),
        fPages(pages)
    {
        setNoDelay(true);
    }

    void setConnect(void (*callback)())
    {
    	fConnectedCallback = callback;
    }

    virtual void setup() override
    {
        if (fWifiAccessPoint)
        {
            DEBUG_PRINTLN("Wifi Access Point: "+String(fWifiAP));
            DEBUG_PRINTLN("Password: "+String(fWifiPassword));
            WiFi.softAP(fWifiAP, fWifiPassword);
            delay(500);
            IPAddress myIP = WiFi.softAPIP();
            DEBUG_PRINTLN("AP IP address: ");
            DEBUG_PRINTLN(myIP);
        }
        else
        {
            DEBUG_PRINTLN("Joining: "+String(fWifiAP));
            WiFi.begin(fWifiAP, fWifiPassword);
            int retryCount = 0;
            while (WiFi.status() != WL_CONNECTED)
            {
        		delay(500);
        		DEBUG_PRINT("*");
                if (retryCount++ >= 40)
                {
                    // Failed to join AP. Do not start webserver
                    DEBUG_PRINTLN();
                    DEBUG_PRINTLN("Failed to join: "+String(fWifiAP));
                    return;
                }
                if (WiFi.status() == WL_DISCONNECTED)
                {
                DEBUG_PRINT("-");
                    WiFi.begin(fWifiAP, fWifiPassword);
                DEBUG_PRINT("+");
                }
      		}
      		// Print local IP address and start web server
      		DEBUG_PRINTLN();
      		DEBUG_PRINT("WiFi connected.");
      		DEBUG_PRINTLN("IP address: ");
      		DEBUG_PRINTLN(WiFi.localIP());
        }
        DEBUG_PRINT("WTF");
        DEBUG_PRINT("WTF");
        DEBUG_PRINT("WTF");
        DEBUG_PRINT("WTF");
        DEBUG_PRINT("WTF");
        DEBUG_PRINT("WTF");
        DEBUG_PRINT("WTF");
        begin();
        fEnabled = true;
    }

    /**
      * Dispatch any received i2c event to CommandEvent
      */
    void handle()
    {
        if (!fEnabled)
            return;
        //check if there are any new clients
        if (hasClient())
        {
            DEBUG_PRINTLN("HASCLIENT");
            unsigned i;
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
            DEBUG_PRINTLN("HASCLIENT1");
            if (i >= maxClients)
            {
                //no free/disconnected spot so reject
                Serial.println("NO CLIENTS AVAILABLE");
                available().stop();
            }
            DEBUG_PRINTLN("HASCLIENT2");
        }
        //check clients for data
        for (unsigned i = 0; i < maxClients; i++)
        {
            if (fClients[i] && fClients[i].connected())
            {
                //get data from the telnet client and push it to the UART
                while (fClients[i].available())
                {
            DEBUG_PRINTLN("AVAILABLE1");
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
                            DEBUG_PRINTLN(fHeader);
							// HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
							// and a content-type so the client knows what's coming, then a blank line:
							fClients[i].println("HTTP/1.0 200 OK");
							fClients[i].println("Content-type:text/html");
							fClients[i].println("Connection: close");
							fClients[i].println();

            DEBUG_PRINTLN("HANDLEREQUEST1");
							handleRequest(fClients[i]);
            DEBUG_PRINTLN("HANDLEREQUEST2");

							// The HTTP response ends with another blank line
							fClients[i].println();
							// Break out of the while loop
							fHeader = "";
            DEBUG_PRINTLN("HANDLEREQUEST STOP1");
							fClients[i].stop();
            DEBUG_PRINTLN("HANDLEREQUEST STOP2");
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
            DEBUG_PRINTLN("STOP1");
                    fClients[i].stop();
            DEBUG_PRINTLN("STOP2");
                }
            }
        }
    }

private:
    const char* fWifiAP;
    const char* fWifiPassword;
    bool fWifiAccessPoint;
    bool fEnabled;
    String fHeader;
    String fRequest;
    const WPage* fPages;
    void (*fConnectedCallback)() = nullptr;

    const WPage& getPage() const
    {
        String url = "/";
        if (fHeader.startsWith("GET /"))
        {
            int pos1 = fHeader.indexOf('\n');
            int pos2 = fHeader.lastIndexOf(' ', pos1);
            if (pos1 != 0 && pos2 != 0)
            {
                url = fHeader.substring(4, pos2);
                DEBUG_PRINTLN("\""+url+"\"");
            }
        }
        for (unsigned i = 0; i < numPages; i++)
        {
            if (fPages[i].getURL() == url)
            {
                return fPages[i];
            }
        }
        return fPages[0];
    }

    void handleRequest(WiFiClient& client)
    {
        const WPage &page = getPage();
        page.handleRequest(client, fHeader);
    }
};


#endif