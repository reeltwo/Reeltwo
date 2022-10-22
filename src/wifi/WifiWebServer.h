#ifndef WifiWebServer_h
#define WifiWebServer_h

#include "ReelTwo.h"
#include "wifi/WifiAccess.h"
#include <WiFiClient.h>
#include <FS.h>

#include "core/FormatString.h"
#include "core/MallocString.h"
#include "core/PSRamBufferedPrintStream.h"

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

class WDynamic
{
public:
    virtual void emitCSS(Print& out) const {}

    virtual void emitBody(Print& out) const = 0;

    virtual void emitScript(Print& out) const {}
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

    inline bool needsReload() const { return fReload; }
    inline String getID() const { return fID; }
    inline void appendCSS(String str)    { fCSS = fCSS + str; }
    inline void appendBody(String str)   { fBody = fBody + str; }
    inline void appendScript(String str) { fScript = fScript + str; }

    void appendCSSf(const char* fmt, ...)
    {
        char* str = NULL;
        va_list ap;
        va_start(ap, fmt);
        int r = FormatString(&str, fmt, ap);
        va_end(ap);
        if (r != -1)
        {
            appendCSS(MallocString(str));
        }
    }

    void appendBodyf(const char* fmt, ...)
    {
        char* str = NULL;
        va_list ap;
        va_start(ap, fmt);
        int r = FormatString(&str, fmt, ap);
        va_end(ap);
        if (r != -1)
        {
            appendBody(MallocString(str));
        }
    }

    void appendScriptf(const char* fmt, ...)
    {
        char* str = NULL;
        va_list ap;
        va_start(ap, fmt);
        int r = FormatString(&str, fmt, ap);
        va_end(ap);
        if (r != -1)
        {
            appendScript(MallocString(str));
        }
    }

    inline void emitCSS(Print& out) const
    {
        if (fEnabled != nullptr && !fEnabled())
            return;
        if (fDynamic)
            fDynamic->emitCSS(out);
        else
            out.println(fCSS);
    }

    inline void emitBody(Print& out) const
    {
        if (fEnabled != nullptr && !fEnabled())
            return;
        if (fDynamic)
            fDynamic->emitBody(out);
        else
            out.println(fBody);
    }

    inline void emitValue(Print& out) const
    {
        if (fEnabled != nullptr && !fEnabled())
            return;
        if (fValue != nullptr)
        {
            if (fValue->getQuoteValue())
                out.println("var "+String(fID)+"_val_ = '"+fValue->get()+"';\n");
            else
                out.println("var "+String(fID)+"_val_ = "+fValue->get()+";\n");
        }
    }

    inline void emitScript(Print& out) const
    {
        if (fEnabled != nullptr && !fEnabled())
            return;
        if (fDynamic)
            fDynamic->emitScript(out);
        else
            out.println(fScript);
    }

    String getValue() const
    {
        if (fEnabled != nullptr && !fEnabled())
            return "";
        return (fValue != nullptr) ? fValue->get() : "";
    }

    void setValue(String val) const
    {
        if (fEnabled != nullptr && !fEnabled())
            return;
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
    bool fReload = false;
    const WDynamic* fDynamic = nullptr;
    bool (*fEnabled)() = nullptr;

    bool& verticalAlignment()
    {
        static bool sAlign = true;
        return sAlign;
    }
};

class WDynamicElement : public WElement
{
public:
    WDynamicElement(const WDynamic& dynamicRef) :
        WElement()
    {
        fDynamic = &dynamicRef;
    }

    WDynamicElement(String id, const WDynamic& dynamicRef, WValue* value = nullptr) :
        WElement(value)
    {
        fID = id;
        fDynamic = &dynamicRef;
    }
};

class WDynamicElementInt : public WElement
{
public:
    WDynamicElementInt(String id, const WDynamic& dynamicRef, int (*getValue)(), void (*setValue)(int)) :
        WElement(new WInteger(getValue, setValue))
    {
        fID = id;
        fDynamic = &dynamicRef;
        fReload = true;
    }
};

class WVerticalAlign : public WElement
{
public:
    WVerticalAlign()
    {
        verticalAlignment() = true;
    }
};

class WHorizontalAlign : public WElement
{
public:
    WHorizontalAlign()
    {
        verticalAlignment() = false;
    }
};

class WStyle : public WElement
{
public:
    WStyle(String style)
    {
        appendCSS(style);
    }
};

class WSlider : public WElement
{
public:
    WSlider(String title, String id, int min, int max, int (*getValue)(), void (*setValue)(int)) :
        WElement(new WInteger(getValue, setValue))
    {
        fID = id;
        appendCSSf(".%s_css { width: 300px; }", id.c_str());
        if (verticalAlignment())
            appendBodyf("<p>%s: <span id='%s_val'></span></p>\n", title.c_str(), id.c_str());
        else
            appendBodyf("<span id='%s_val'></span>\n", id.c_str());
        appendBodyf("<input type='range' min='%d' max='%d' class='%s_css' id='%s_slider' onchange='updateValue_%s(this.value)'/>\n", min, max, id.c_str(), id.c_str(), id.c_str());

        appendScriptf("var %s = document.getElementById('%s_slider');\n", id.c_str(), id.c_str());
        appendScriptf("%s.value = %s_val_;\n", id.c_str(), id.c_str());
        appendScriptf("var %s_priv = document.getElementById('%s_val'); %s_priv.innerHTML = %s.value;\n", id.c_str(), id.c_str(), id.c_str(), id.c_str());
        appendScriptf("%s.oninput = function() { %s.value = this.value; %s_priv.innerHTML = this.value; }\n", id.c_str(), id.c_str(), id.c_str());
        appendScriptf("function updateValue_%s(pos) {fetchNoload('%s', pos); {Connection: close};}\n", id.c_str(), id.c_str());
    }
};

class WCheckbox : public WElement
{
public:
    WCheckbox(String title, String id, bool (*getValue)(), void (*setValue)(bool), bool (*enabled)() = nullptr) :
        WElement(new WBoolean(getValue, setValue))
    {
        fID = id;
        fEnabled = enabled;
        appendCSSf(".%s_css { width: 300px; }", id.c_str());
        if (verticalAlignment())
            appendBodyf("<p><span id='%s_val'></span></p>\n", id.c_str());
        else
            appendBodyf("<span id='%s_val'></span>\n", id.c_str());
        appendBodyf("<input type='checkbox' id='%s_cbox' onchange='updateValue_%s(this.checked)'/>\n", id.c_str(), id.c_str());
        appendBodyf("<label class='%s_css' for='%s_cbox'>%s</label>\n", id.c_str(), id.c_str(), title.c_str());
        appendScriptf("var %s = document.getElementById('%s_cbox');\n", id.c_str(), id.c_str());
        appendScriptf("%s.checked = %s_val_;\n", id.c_str(), id.c_str());
        appendScriptf("function updateValue_%s(pos) {fetchNoload('%s', pos); {Connection: close};}\n", id.c_str(), id.c_str());
    }
};

class WCheckboxReload : public WElement
{
public:
    WCheckboxReload(String title, String id, bool (*getValue)(), void (*setValue)(bool), bool (*enabled)() = nullptr) :
        WElement(new WBoolean(getValue, setValue))
    {
        fID = id;
        fEnabled = enabled;
        fReload = true;
        appendCSSf(".%s_css { width: 300px; }", id.c_str());
        if (verticalAlignment())
            appendBodyf("<p><span id='%s_val'></span></p>\n", id.c_str());
        else
            appendBodyf("<span id='%s_val'></span>\n", id.c_str());
        appendBodyf("<input type='checkbox' id='%s_cbox' onchange='updateValue_%s(this.checked)'/>\n", id.c_str(), id.c_str());
        appendBodyf("<label class='%s_css' for='%s_cbox'>%s</label>\n", id.c_str(), id.c_str(), title.c_str());
        appendScriptf("var %s = document.getElementById('%s_cbox');\n", id.c_str(), id.c_str());
        appendScriptf("%s.checked = %s_val_;\n", id.c_str(), id.c_str());
        appendScriptf("function updateValue_%s(pos) {fetchLoad('%s', pos); {Connection: close};}\n", id.c_str(), id.c_str());
    }
};

class WButton : public WElement
{
public:
    WButton(String title, String id, void (*pressed)()) :
        WElement(new WAction(pressed))
    {
        fID = id;
        appendCSSf(".%s_css { width: 300px; }", id.c_str());
        if (verticalAlignment())
            appendBodyf("<p><span id='%s_val'></span></p>\n", id.c_str());
        appendBodyf("<input type='button' id='%s_btn' value='%s' onclick='pressed_%s()'/>\n", id.c_str(), title.c_str(), id.c_str());
        appendScriptf("function pressed_%s() {fetchNoload('%s', true); {Connection: close};}\n", id.c_str(), id.c_str());
    }

    WButton(String title, String id, String href)
    {
        fID = id;
        appendCSSf(".%s_css { width: 300px; }", id.c_str());
        if (verticalAlignment())
            appendBodyf("<p><span id='%s_val'></span></p>\n", id.c_str());
        appendBodyf("<input type='button' id='%s_btn' value='%s' onclick='window.location.href=\"%s\"'/>\n", id.c_str(), title.c_str(), href.c_str());
    }

    WButton(String title, String id, String href, void (*pressed)()) :
        WElement(new WAction(pressed))
    {
        fID = id;
        fReload = true;
        appendCSSf(".%s_css { width: 300px; }", id.c_str());
        if (verticalAlignment())
            appendBodyf("<p><span id='%s_val'></span></p>\n", id.c_str());
        appendBodyf("<input type='button' id='%s_btn' value='%s' onclick='pressed_%s()'/>\n", id.c_str(), title.c_str(), id.c_str());
        appendScriptf("function pressed_%s() {window.location.href='\"%s?%s=true&\"'; {Connection: close};}\n", id.c_str(), href.c_str(), id.c_str());
    }

    WButton(String title, String id, bool reload, void (*pressed)()) :
        WElement(new WAction(pressed))
    {
        fID = id;
        fReload = true;
        appendCSSf(".%s_css { width: 300px; }", id.c_str());
        if (verticalAlignment())
            appendBodyf("<p><span id='%s_val'></span></p>\n", id.c_str());
        appendBodyf("<input type='button' id='%s_btn' value='%s' onclick='pressed_%s()'/>\n", id.c_str(), title.c_str(), id.c_str());
        appendScriptf("function pressed_%s() {fetchLoad('%s', true);  {Connection: close};}\n", id.c_str(), id.c_str());
    }
};

class WButtonReload : public WButton
{
public:
    WButtonReload(String title, String id, void (*pressed)()) :
        WButton(title, id, true, pressed)
    {
    }
};

class WLabel : public WElement
{
public:
    WLabel(String text, String id, bool (*enabled)() = nullptr)
    {
        fID = id;
        fEnabled = enabled;
        appendCSSf(".%s_css { width: 300px; }", id.c_str());
        appendBodyf("<p><span id='%s_val'></span></p>\n", id.c_str());
        appendBodyf("<label class='%s_css'>%s</label>\n", id.c_str(), text.c_str());
    }
};

class WTextField : public WElement
{
public:
    WTextField(String title, String id, String (*getValue)(), void (*setValue)(String), bool (*enabled)() = nullptr) :
        WElement(new WString(getValue, setValue))
    {
        fID = id;
        fEnabled = enabled;
        appendCSSf(".%s_css { width: 300px; }", id.c_str());
        appendBodyf("<p><span id='%s_val'></span></p>\n", id.c_str());
        appendBodyf("<label class='%s_css' for='%s_fld'>%s</label>\n", id.c_str(), id.c_str(), title.c_str());
        appendBodyf("<input type='text' id='%s_fld' onchange='updateValue_%s(this.value)'/>\n", id.c_str(), id.c_str());
        appendScriptf("var %s = document.getElementById('%s_fld');\n", id.c_str(), id.c_str());
        appendScriptf("%s.value = %s_val_;\n", id.c_str(), id.c_str());
        appendScriptf("function updateValue_%s(pos) {fetchNoload('%s', pos); {Connection: close};}\n", id.c_str(), id.c_str());
    }
};

class WTextFieldInteger : public WElement
{
public:
    WTextFieldInteger(String title, String id, String (*getValue)(), void (*setValue)(String), bool (*enabled)() = nullptr) :
        WElement(new WString(getValue, setValue))
    {
        fID = id;
        fEnabled = enabled;
        appendCSSf(".%s_css { width: 300px; }", id.c_str());
        appendBodyf("<p><span id='%s_val'></span></p>\n", id.c_str());
        appendBodyf("<label class='%s_css' for='%s_fld'>%s</label>\n", id.c_str(), id.c_str(), title.c_str());
        appendBodyf("<input type='text' id='%s_fld' onchange='updateValue_%s(this.value)'/>\n", id.c_str(), id.c_str());
        appendScriptf("var %s = document.getElementById('%s_fld');\n", id.c_str(), id.c_str());
        appendScriptf("setInputFilter(%s, function(value) {", id.c_str());
        appendScriptf("  return /^[0-9]*$/.test(value);");
        appendScriptf("});");
        appendScriptf("%s.value = %s_val_;\n", id.c_str(), id.c_str());
        appendScriptf("function updateValue_%s(pos) {fetchNoload('%s', pos); {Connection: close};}\n", id.c_str(), id.c_str());
    }
};

class WTextFieldIntegerRange : public WElement
{
public:
    WTextFieldIntegerRange(String title, String id, int minValue, int maxValue, String (*getValue)(), void (*setValue)(String), bool (*enabled)() = nullptr) :
        WElement(new WString(getValue, setValue))
    {
        fID = id;
        fEnabled = enabled;
        appendCSSf(".%s_css { width: 300px; }", id.c_str());
        appendBodyf("<p><span id='%s_val'></span></p>\n", id.c_str());
        appendBodyf("<label class='%s_css' for='%s_fld'>%s</label>\n", id.c_str(), id.c_str(), title.c_str());
        appendBodyf("<input type='text' id='%s_fld' onkeypress='limitKeypress(event,this.value,%d)' onchange='updateValue_%s(this.value)'/>\n",
            id.c_str(), int(log(maxValue) * M_LOG10E + 1), id.c_str());
        appendScriptf("var %s = document.getElementById('%s_fld');\n", id.c_str(), id.c_str());
        appendScriptf("setInputFilter(%s, function(value) {", id.c_str());
        appendScriptf("  return /^[0-9]*$/.test(value);");
        appendScriptf("});");
        appendScriptf("%s.value = %s_val_;\n", id.c_str(), id.c_str());
        appendScriptf("function updateValue_%s(pos) {if(pos<%d) {", id.c_str(), minValue);
        appendScriptf(" alert('Minimum allowed value is: '+%d);", minValue);
        appendScriptf(" %s.value = %d", id.c_str(), minValue);
        appendScriptf("} else if (pos > %d) {", maxValue);
        appendScriptf(" alert('Maximum allowed value is: '+%d);", maxValue);
        appendScriptf(" %s.value = %d", id.c_str(), maxValue);
        appendScriptf("} else {");
        appendScriptf(" fetchNoload('%s', pos);\n", id.c_str());
        appendScriptf("}");
        appendScriptf("{Connection: close};}\n");
    }
};
        // appendScript("  return /^\\d*\\.?\\d*$/.test(value);");

class WSelect : public WElement
{
public:
    WSelect(String title, String id, String options[], unsigned numOptions, int (*getValue)(), void (*setValue)(int), bool (*enabled)() = nullptr) :
        WElement(new WInteger(getValue, setValue))
    {
        fID = id;
        fEnabled = enabled;
        appendCSSf(".%s_css { width: 300px; }\n", id.c_str());
        appendBodyf("<p><span id='%s_val'></span></p>\n", id.c_str());
        appendBodyf("<label class='%s_css' for='%s_fld'>%s</label>\n", id.c_str(), id.c_str(), title.c_str());
        appendBodyf("<select id='%s_fld' onchange='updateValue_%s(this.value)'>\n", id.c_str(), id.c_str());
        for (unsigned i = 0; i < numOptions; i++)
        {
            appendBodyf("<option value='%d'>%s</option>\n", i, options[i].c_str());
        }
        appendBodyf("</select>\n");
        appendScriptf("var %s = document.getElementById('%s_fld');\n", id.c_str(), id.c_str());
        appendScriptf("%s.value = %s_val_;\n", id.c_str(), id.c_str());
        appendScriptf("function updateValue_%s(pos) {fetchNoload('%s', pos); {Connection: close};}\n", id.c_str(), id.c_str());
    }

    WSelect(String title, String id, String options[], String values[], unsigned numOptions, int (*getValue)(), void (*setValue)(int), bool (*enabled)() = nullptr) :
        WElement(new WInteger(getValue, setValue))
    {
        fID = id;
        fEnabled = enabled;
        appendCSSf(".%s_css { width: 300px; }\n", id.c_str());
        appendBodyf("<p><span id='%s_val'></span></p>\n", id.c_str());
        appendBodyf("<label class='%s_css' for='%s_fld'>%s</label>\n", id.c_str(), id.c_str(), title.c_str());
        appendBodyf("<select id='%s_fld' onchange='updateValue_%s(this.value)'>\n", id.c_str(), id.c_str());
        for (unsigned i = 0; i < numOptions; i++)
        {
            // appendBodyf("<option value='%s'>%s</option>\n", values[i].c_str(), options[i].c_str());
        }
        appendBodyf("</select>\n");
        appendScriptf("var %s = document.getElementById('%s_fld');\n", id.c_str(), id.c_str());
        appendScriptf("%s.value = %s_val_;\n", id.c_str(), id.c_str());
        appendScriptf("function updateValue_%s(pos) {fetchNoload('%s', pos); {Connection: close};}\n", id.c_str(), id.c_str());
    }
};

class WPassword : public WElement
{
public:
    WPassword(String title, String id, String (*getValue)(), void (*setValue)(String)) :
        WElement(new WString(getValue, setValue))
    {
        fID = id;
        appendCSSf(".%s_css { width: 300px; }", id.c_str());
        if (verticalAlignment())
            appendBodyf("<p><span id='%s_val'></span></p>\n", id.c_str());
        appendBodyf("<label class='%s_css' for='%s_fld'>%s</label>\n", id.c_str(), id.c_str(), title.c_str());
        appendBodyf("<input type='password' id='%s_fld' onchange='updateValue_%s(this.value)'/>\n", id.c_str(), id.c_str());
        appendScriptf("var %s = document.getElementById('%s_fld');\n", id.c_str(), id.c_str());
        appendScriptf("%s.value = %s_val_;\n", id.c_str(), id.c_str());
        appendScriptf("function updateValue_%s(pos) {fetchNoload('%s', pos); {Connection: close};}\n", id.c_str(), id.c_str());
    }
};

class WFileInput : public WElement
{
public:
    WFileInput(String title, String id, String (*getValue)(), void (*setValue)(String)) :
        WElement(new WString(getValue, setValue))
    {
        fID = id;
        appendCSSf(".%s_css { width: 300px; }", id.c_str());
        if (verticalAlignment())
            appendBodyf("<p><span id='%s_val'></span></p>\n", id.c_str());
        appendBodyf("<label class='%s_css' for='%s_file'>%s</label>\n", id.c_str(), id.c_str(), title.c_str());
        appendBodyf("<input type='file' id='%s_file' onchange='updateValue_%s(this)'/>\n", id.c_str(), id.c_str());
        appendScriptf("var %s = document.getElementById('%s_file');\n", id.c_str(), id.c_str());
        appendScriptf("function updateValue_%s(pos) {);}\n", id.c_str());
    }
};

class WFirmwareFile : public WElement
{
public:
    WFirmwareFile(String title, String id)
    {
        fID = id;
        appendCSSf(".%s_css { width: 300px; }", id.c_str());
        if (verticalAlignment())
            appendBodyf("<p><span id='%s_val'></span></p>\n", id.c_str());
        appendBodyf("<label class='%s_css' for='%s_file'>%s</label>\n", id.c_str(), id.c_str(), title.c_str());
        appendBodyf("<input type='file' id='%s_file' accept='.bin' onchange='updateValue_%s(this)'/>\n", id.c_str(), id.c_str());
        appendScriptf("var %s = document.getElementById('%s_file');\n", id.c_str(), id.c_str());
        appendScriptf("function updateValue_%s(pos) { document.getElementById('%s_upload').disabled = false; }\n", id.c_str(), id.c_str());
    }
};

class WFirmwareUpload : public WElement
{
public:
    WFirmwareUpload(String title, String id)
    {
        fID = id;
        appendCSS("."+String(id)+"_css { width: 300px; }");
        appendCSS("#"+String(id)+"_prg,#"+String(id)+"_prgbar{background-color:#f1f1f1;border-radius:10px}");
        appendCSS("#"+String(id)+"_bar{background-color:#3498db;width:0%;height:10px}");
        if (verticalAlignment())
            appendBody("<p><span id='"+String(id)+"_val'></span></p>\n");
        appendBody("<input type='button' id='"+String(id)+"_upload' value='"+title+"' onclick='upload_"+String(id)+"()'/>\n");
        appendBody("<br><br>\n");
        appendBody("<div id='"+String(id)+"_prg'></div>\n");
        appendBody("<br><div id='"+String(id)+"_prgbar'><div id='"+String(id)+"_bar'></div></div><br></form>\n");
        appendScript("var "+String(id)+"_upload = document.getElementById('"+String(id)+"_upload');\n");
        appendScript("var "+String(id)+"_prg = document.getElementById('"+String(id)+"_prg');\n");
        appendScript("var "+String(id)+"_prgbar = document.getElementById('"+String(id)+"_prgbar');\n");
        appendScript("var "+String(id)+"_bar = document.getElementById('"+String(id)+"_bar');\n");
        appendScript(String(id)+"_upload.disabled = true;\n");
        appendScript("function upload_"+String(id)+"() {\n");
        appendScript("const xhr = new XMLHttpRequest();\n");
        appendScript("xhr.upload.onprogress = (evt) => {\n");
        appendScript("    if (evt.lengthComputable) {\n");
        appendScript("        var per = evt.loaded / evt.total;\n");
        appendScript("        "+String(id)+"_prg.innerHTML = 'progress: ' + Math.round(per*100) + '%';\n");
        appendScript("        "+String(id)+"_bar.style.width = Math.round(per*100) + '%';\n");
        appendScript("    }\n");
        appendScript("};\n");
        appendScript("xhr.upload.onerror = () => {\n");
        appendScript("   "+String(id)+"_prg.innerHTML = 'Upload failed!';\n");
        appendScript("   "+String(id)+"_bar.style.width = '0%';\n");
        appendScript("};\n");
        appendScript("xhr.upload.abort = () => {\n");
        appendScript("   "+String(id)+"_prg.innerHTML = 'Upload cancelled';\n");
        appendScript("   "+String(id)+"_bar.style.width = '0%';\n");
        appendScript("};\n");
        appendScript("xhr.upload.onload = () => {\n");
        appendScript("   "+String(id)+"_prg.innerHTML = 'Upload complete. Please wait .';\n");
        appendScript("   "+String(id)+"_bar.style.width = '0%';\n");
        appendScript("   xhr.counter = 0;\n");
        appendScript("   setInterval(function() {\n");
        appendScript("     if (xhr.counter++ >= 8) window.location.href='/';\n");
        appendScript("    "+String(id)+"_prg.innerHTML = "+String(id)+"_prg.innerHTML + '.';\n");
        appendScript("   }, 2000);\n");
        appendScript("};\n");
        appendScript("xhr.open('POST', 'upload/firmware', true);\n");
        appendScript("xhr.send("+String(id)+".files[0]);\n");
        appendScript("}\n");
    }
};

struct WMenuData
{
    const char* title;
    const char* href;
};

class WVerticalMenu : public WElement
{
public:
    WVerticalMenu(String id, const WMenuData* menuData, unsigned menuCount, unsigned active = 0)
    {
        appendCSSf(".%s_vertical_menu { width: 300px; margin-left: auto; margin-right: auto; }\n", id.c_str());
        appendCSSf(".%s_vertical_menu a { background-color: #eee; color: black; display: block; padding: 12px; text-decoration: none; }\n", id.c_str());
        appendCSSf(".%s_vertical_menu a:hover { background-color: #ccc; }\n", id.c_str());
        appendCSSf(".%s_vertical_menu a:active { background-color: #4CAF50; color: white; }\n", id.c_str());
        appendBodyf("<div class='%s_vertical_menu'>\n", id.c_str());
        for (unsigned i = 0; i < menuCount; i++)
        {
            if (i == active)
                appendBodyf("<a href='%s' class='active'>%s</a>\n", menuData[i].href, menuData[i].title);
            else
                appendBodyf("<a href='%s'>%s</a>\n", menuData[i].href, menuData[i].title);
        }
        appendBodyf("</div>\n");
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

class WHR : public WElement
{
public:
    WHR()
    {
        appendBody("<hr>");
    }
};

class WHRef : public WElement
{
public:
    WHRef(String link, String text)
    {
        appendBodyf("<a href=\"%s\">%s</a>", link.c_str(), text.c_str());
    }
};

class WImage : public WElement
{
public:
    WImage(String alt, String data)
    {
        appendBody("<p><img src='data:image/png;base64, ");
        appendBody(data);
        appendBody("' alt='"+String(alt)+"'></p>");
    }
};

class WSVG : public WElement
{
public:
    WSVG(String data)
    {
        appendBody("<p>");
        appendBody(data);
        appendBody("</p>");
    }
};

class WHTML : public WElement
{
public:
    WHTML(String data)
    {
        appendBody(data);
    }
};

class WJavaScript : public WElement
{
public:
    WJavaScript(String data)
    {
        appendScript(data);
    }
};

class WTableRow : public WElement
{
public:
    WTableRow()
    {
        appendBody("<tr>");
    }
};

class WTableCol : public WElement
{
public:
    WTableCol()
    {
        appendBody("<td>");
        verticalAlignment() = false;
    }

    WTableCol(String styleClass)
    {
        appendBody("<td class=\"");
        appendBody(styleClass);
        appendBody("\">");
        verticalAlignment() = false;
    }
};

class WTableColEnd : public WElement
{
public:
    WTableColEnd()
    {
        appendBody("</td>");
        verticalAlignment() = true;
    }
};

class WTableRowEnd : public WElement
{
public:
    WTableRowEnd()
    {
        appendBody("</tr>");
    }
};

class WTableLabel : public WElement
{
public:
    WTableLabel(String text, String id, bool (*enabled)() = nullptr)
    {
        fID = id;
        fEnabled = enabled;
        appendBodyf("<label class='%s_css'>%s</label>\n", id.c_str(), text.c_str());
    }
};

class WTableTextField : public WElement
{
public:
    WTableTextField(String id, String (*getValue)(), void (*setValue)(String), bool (*enabled)() = nullptr) :
        WElement(new WString(getValue, setValue))
    {
        fID = id;
        fEnabled = enabled;
        appendBodyf("<input type='text' id='%s_fld' onchange='updateValue_%s(this.value)'/>\n", id.c_str(), id.c_str());
        appendScriptf("var %s = document.getElementById('%s_fld');\n", id.c_str(), id.c_str());
        appendScriptf("%s.value = %s_val_;\n", id.c_str(), id.c_str());
        appendScriptf("function updateValue_%s(pos) {fetchNoload('%s', pos); {Connection: close};}\n", id.c_str(), id.c_str());
    }
};

#ifndef HTTP_UPLOAD_BUFLEN
#define HTTP_UPLOAD_BUFLEN 1436
#endif

enum WUploadStatus
{
    UPLOAD_FILE_START,
    UPLOAD_FILE_WRITE,
    UPLOAD_FILE_END,
    UPLOAD_FILE_ABORTED
};

class WUploader
{
public:
    WUploadStatus status;
    String  filename;
    String  name;
    String  type;
    size_t  fileSize;     // file size
    size_t  receivedSize; // received size
    size_t  currentSize;  // size of data currently in buf
    uint8_t buf[HTTP_UPLOAD_BUFLEN];
};

class WPage
{
public:
    WPage(String url, const WElement contents[], unsigned numElements, String title = "", String lang = "en") :
        fURL(url),
        fTitleOrPath(title),
        fLanguageOrMimeType(lang),
        fNumElements(numElements),
        fContents(contents)
    {
    }

    WPage(String url, fs::FS* fs, String path, const WElement contents[], unsigned numElements) :
        fURL(url),
        fTitleOrPath(path),
        fLanguageOrMimeType("text/html"),
        fFS(fs),
        fNumElements(numElements),
        fContents(contents)
    {
    }

    WPage(String url, fs::FS* fs, String mimeType) :
        fURL(url),
        fTitleOrPath(url),
        fLanguageOrMimeType(mimeType),
        fFS(fs),
        fNumElements(0),
        fContents(nullptr)
    {
    }

    inline const String& getURL() const
    {
        return fURL;
    }

    inline bool isGet() const
    {
        return (fCompleteProc == nullptr && fUploaderProc == nullptr);
    }

    void handleGetRequest(Print& out, String &header) const
    {
        bool needsReload = true;
        String prefix = "GET "+fURL+"?";
        if (header.startsWith(prefix))
        {
            if (!isGet())
                return;

            if (fAPIProc)
            {
                int end = header.indexOf(' ', prefix.length());
                fAPIProc(out, header.substring(prefix.length(), end));
                return;
            }
            int skiplen = 2;
            int pos1 = header.indexOf("&?");
            if (pos1 == -1)
            {
                skiplen = 1;
                pos1 = header.indexOf('?');
            }
            int pos2 = header.indexOf('=',pos1);
            int pos3 = header.indexOf('&',pos2);
            if (pos1 != -1 && pos2 != -1 && pos3 != -1)
            {
                String var = header.substring(pos1+skiplen, pos2);
                String val = header.substring(pos2+1, pos3);
                DEBUG_PRINT("SET "); DEBUG_PRINT(var); DEBUG_PRINT(" = "); DEBUG_PRINTLN(val);
                for (unsigned i = 0; i < fNumElements; i++)
                {
                    if (var == fContents[i].getID())
                    {
                        fContents[i].setValue(val);
                        needsReload = fContents[i].needsReload();
                        break;
                    }
                }
            }
        }
        if (needsReload)
        {
            if (fFS != nullptr)
            {
                fs::File file = fFS->open(fTitleOrPath);
                if (file)
                {
                    DEBUG_PRINTLN("FILE: "+String(fTitleOrPath));
                    size_t fileSize = file.size();
                    out.println("HTTP/1.0 200 OK");
                    out.print("Content-type:"); out.println(fLanguageOrMimeType);
                    out.print("Content-Length:"); out.println(fileSize);
                    out.println("Connection: close");
                    out.println();
                    char* buffer = (char*)malloc(1024);
                    while (file.available())
                    {
                        size_t bytesRead = file.readBytes(buffer, 1024);
                        out.write(buffer, bytesRead);
                    }
                    free(buffer);
                }
                else
                {
                    DEBUG_PRINTLN("FILE NOT FOUND: "+String(fTitleOrPath));
                }
            }
            else
            {
                out.println("HTTP/1.0 200 OK");
                out.println("Content-type:text/html");
                out.println("Connection: close");
                out.println();
                out.print(R"RAW(<!DOCTYPE html><html lang=")RAW");
                out.print(fLanguageOrMimeType);
                out.print(
                    R"RAW("><head><meta charset="UTF-8"><meta name="viewport", content="width=device-width, initial-scale=1">
                        <link rel="icon" href="data:,"><title>)RAW");
                out.print(fTitleOrPath);
                out.print(
                    R"RAW(</title>
                        <style>body { text-align: center; font-family: "Trebuchet MS", Arial; margin-left:auto; margin-right:auto;}
                    )RAW");
                for (unsigned i = 0; i < fNumElements; i++)
                    fContents[i].emitCSS(out);
                out.print(
                    R"RAW(
                        </style>
                        </head><body>
                    )RAW");
                for (unsigned i = 0; i < fNumElements; i++)
                    fContents[i].emitBody(out);
                out.print(
                    R"RAW(
                        <script>
                        function fetchNoload(key,val) {
                            var baseurl = window.location.protocol+'//'+window.location.host+location.pathname;
                            fetch(baseurl+'?'+key+'='+val+'&');
                        }
                        function fetchLoad(key,val) {
                            var baseurl = window.location.protocol+'//'+window.location.host+location.pathname;
                            window.location.href=baseurl+'?'+key+'='+val+'&';
                        }
                        function limitKeypress(event, value, maxLength) {
                          if (value != undefined && value.toString().length >= maxLength) {
                            event.preventDefault();
                          }
                        }
                        function setInputFilter(textbox, inputFilter) {
                          ["input", "keydown", "keyup", "mousedown", "mouseup", "select", "contextmenu", "drop"].forEach(function(event) {
                            textbox.addEventListener(event, function() {
                              if (inputFilter(this.value)) {
                                this.oldValue = this.value;
                                this.oldSelectionStart = this.selectionStart;
                                this.oldSelectionEnd = this.selectionEnd;
                              } else if (this.hasOwnProperty("oldValue")) {
                                this.value = this.oldValue;
                                this.setSelectionRange(this.oldSelectionStart, this.oldSelectionEnd);
                              } else {
                                this.value = "";
                              }
                            });
                          });
                        }
                    )RAW");
                for (unsigned i = 0; i < fNumElements; i++)
                    fContents[i].emitValue(out);
                for (unsigned i = 0; i < fNumElements; i++)
                    fContents[i].emitScript(out);
                out.print(
                    R"RAW(
                        {Connection: close};
                        </script>
                        </body></html>
                    )RAW");
            }
        }
        else
        {
            out.println("HTTP/1.0 200 OK");
            out.println("Content-type:text/html");
            out.println("Connection: close");
            out.println();
        }
    }

    inline void callComplete(Client& client) const
    {
        if (fCompleteProc != nullptr)
            fCompleteProc(client);
    }

    inline void callUploader(WUploader &uploader) const
    {
        if (fUploaderProc != nullptr)
            fUploaderProc(uploader);
    }

protected:
    String fURL;
    String fTitleOrPath;
    String fLanguageOrMimeType;
    fs::FS* fFS;
    unsigned fNumElements;
    const WElement* fContents;
    void (*fCompleteProc)(Client& client) = nullptr;
    void (*fUploaderProc)(WUploader &uploader) = nullptr;
    void (*fAPIProc)(Print& out, String queryString) = nullptr;
};

class WAPI : public WPage
{
public:
    WAPI(String url, void (*apiProc)(Print& out, String queryString)) :
        WPage(url, nullptr, 0)
    {
        fAPIProc = apiProc;
    }
};

class WUpload : public WPage
{
public:
    WUpload(String url, void (*completeProc)(Client& client), void (*uploaderProc)(WUploader &uploader)) :
        WPage(url, nullptr, 0)
    {
        fCompleteProc = completeProc;
        fUploaderProc = uploaderProc;
    }
};

/**
  * \ingroup wifi
  *
  * \class WifiWebServer
  *
  * \brief Simple WiFi web server
  *
  * \code
  * #include "wifi/WifiWebServer.h"
  *
  * WElement mainContents[] = {
  *    W1("Hello World"),
  *    WButton("Hello", "hello", []() {
  *       DEBUG_PRINTLN("Hello World");
  *    })
  * };
  * WPage pages[] = {
  *    WPage("/", mainContents, SizeOfArray(mainContents)),
  * };
  * WifiWebServer<1,SizeOfArray(pages)> myWeb(pages, WIFI_AP_NAME, WIFI_AP_PASSPHRASE, WIFI_ACCESS_POINT);
  * \endcode
  *
  */
template<unsigned maxClients = 10, unsigned numPages = 0>
class WifiWebServer : public WiFiServer, public WifiAccess::Notify
{
public:
    WiFiClient fClients[maxClients];

    /** \brief Constructor
      *
      * Only a single instance of WifiSerialBridge should be created per sketch.
      *
      * \param port the port number of this service
      */
    WifiWebServer(const WPage pages[], WifiAccess &wifiAccess, uint16_t port = 80) :
        WiFiServer(port),
        fHeader(""),
        fRequest(""),
        fPages(pages)
    {
        wifiAccess.addNotify(this);
    }

    void setConnect(void (*callback)())
    {
        fConnectedCallback = callback;
    }

    bool enabled()
    {
        return fEnabled;
    }

    virtual void wifiConnected(WifiAccess& access) override
    {
        DEBUG_PRINTLN("WifiWebServer.wifiConnected");
        if (!fStarted)
        {
            begin();
            fStarted = true;
        }
    }

    virtual void wifiDisconnected(WifiAccess& access) override
    {
        DEBUG_PRINTLN("WifiWebServer.wifiDisconnected");
        for (unsigned i = 0; i < maxClients; i++)
        {
            if (fClients[i])
            {
                fClients[i].stop();
            }
        }
    }

    /**
      * Dispatch any received i2c event to CommandEvent
      */
    void handle()
    {
        if (!fEnabled || !fStarted)
            return;
        //check if there are any new clients
        if (hasClient())
        {
            unsigned i;
            for (i = 0; i < maxClients; i++)
            {
                //find free/disconnected spot
                if (!fClients[i] || !fClients[i].connected())
                {
                    if (fClients[i])
                        fClients[i].stop();
                    fClients[i] = available();
                    fClients[i].setNoDelay(true);
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
        for (unsigned i = 0; i < maxClients; i++)
        {
            if (fClients[i] && fClients[i].connected())
            {
                //get data from the telnet client and push it to the UART
                if (fUploader != nullptr)
                {
                    if (fUploader->receivedSize + fUploader->currentSize == fUploader->fileSize)
                    {
                        if (fUploader->currentSize)
                        {
                            DEBUG_PRINT("END SIZE: "+String(fUploader->currentSize));
                            fUploader->status = UPLOAD_FILE_WRITE;
                            fUploaderPage->callUploader(*fUploader);
                            fUploader->receivedSize += fUploader->currentSize;
                            fUploader->currentSize = 0;
                        }
                        fUploader->status = UPLOAD_FILE_END;
                        fUploaderPage->callUploader(*fUploader);
                        fUploaderPage->callComplete(fClients[i]);
                        delete fUploader;
                        fUploaderPage = nullptr;
                        fUploader = nullptr;

                        // The HTTP response ends with another blank line
                        // Break out of the while loop
                        fHeader = "";
                        fClients[i].stop();
                    }
                }
                while (fClients[i].available())
                {
                    char c = fClients[i].read();
                    // Serial.write(c);
                    if (fUploader != nullptr)
                    {
                        if (fUploader->currentSize == HTTP_UPLOAD_BUFLEN)
                        {
                            fUploader->status = UPLOAD_FILE_WRITE;
                            fUploaderPage->callUploader(*fUploader);
                            fUploader->receivedSize += fUploader->currentSize;
                            fUploader->currentSize = 0;
                        }
                        fUploader->buf[fUploader->currentSize++] = c;
                        continue;
                    }
                    fHeader.concat(c);
                    if (c == '\n')
                    {
                        // if the byte is a newline character
                        // if the current line is blank, you got two newline characters in a row.
                        // that's the end of the client HTTP request, so send a response:
                        if (fRequest.length() == 0)
                        {
                            if (fHeader.startsWith("POST /"))
                            {
                                fUploaderPage = getPost();
                                if (fUploaderPage == nullptr)
                                {
                                    fClients[i].println("HTTP/1.0 404 Not Found");
                                    fClients[i].println("Content-type:text/html");
                                    fClients[i].println("Connection: close");
                                    fClients[i].println();
                                    break;
                                }
                                else
                                {
                                    int offs = fHeader.indexOf("\nContent-Length: ");
                                    unsigned contentLength = 0;
                                    if (offs > 0)
                                    {
                                        int pos1 = fHeader.indexOf('\n', offs+1);
                                        int pos2 = fHeader.lastIndexOf(' ', pos1);
                                        if (pos1 != 0 && pos2 != 0)
                                        {
                                            contentLength = fHeader.substring(pos1+1, pos2).toInt();
                                        }
                                    }
                                    fUploader = new WUploader;
                                    fUploader->status = UPLOAD_FILE_START;
                                    fUploader->filename = "filename.txt";
                                    fUploader->name = "name.txt";
                                    fUploader->type = "type.txt";
                                    fUploader->fileSize = contentLength;
                                    fUploader->receivedSize = 0;
                                    fUploader->currentSize = 0;
                                    fUploaderPage->callUploader(*fUploader);
                                }
                            }
                            else if (fHeader.startsWith("GET /robots.txt"))
                            {
                                Stream& out = fClients[i];
                                //PSRamBufferedPrintStream out(fClients[i]);
                                // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                                // and a content-type so the client knows what's coming, then a blank line:
                                out.println("HTTP/1.0 200 OK");
                                out.println("Content-type:text/html");
                                out.println("Connection: close");
                                out.println();

                                // The HTTP response ends with another blank line
                                out.println();
                                out.flush();
                                // Break out of the while loop
                                fHeader = "";
                                fClients[i].stop();
                                break;
                            }
                            else if (fHeader.startsWith("GET /"))
                            {
                                // Stream& out = fClients[i];
                                RamBufferedPrintStream out(fClients[i]);
                                // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                                // and a content-type so the client knows what's coming, then a blank line:
                                handleGetRequest(out);

                                // The HTTP response ends with another blank line
                                out.println();
                                out.flush();
                                // Break out of the while loop
                                fHeader = "";
                                fClients[i].stop();
                                break;
                            }
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
    bool fEnabled = true;
    bool fStarted = false;
    String fHeader;
    String fRequest;
    bool fHandlePost;
    const WPage* fPages;
    const WPage* fUploaderPage = nullptr;
    WUploader* fUploader = nullptr;
    void (*fConnectedCallback)() = nullptr;
    void (*fWiFiActiveCallback)(bool ap) = nullptr;

    const WPage& getPage() const
    {
        String url = "/";
        if (fHeader.startsWith("GET /"))
        {
            int pos1 = fHeader.indexOf('\n');
            int pos2 = fHeader.lastIndexOf(' ', pos1);
            if (pos1 != -1 && pos2 != -1)
            {
                url = fHeader.substring(4, pos2);
                if ((pos1 = url.indexOf('?')) != -1)
                {
                    url = url.substring(0, pos1);
                }
            }
        }
        for (unsigned i = 0; i < numPages; i++)
        {
            if (fPages[i].getURL() == url && fPages[i].isGet())
            {
                return fPages[i];
            }
        }
        return fPages[0];
    }

    const WPage* getPost() const
    {
        String url = "/";
        if (fHeader.startsWith("POST /"))
        {
            int pos1 = fHeader.indexOf('\n');
            int pos2 = fHeader.lastIndexOf(' ', pos1);
            if (pos1 != -1 && pos2 != -1)
            {
                url = fHeader.substring(5, pos2);
            }
        }
        for (unsigned i = 0; i < numPages; i++)
        {
            if (fPages[i].getURL() == url && !fPages[i].isGet())
            {
                return &fPages[i];
            }
        }
        return nullptr;
    }

    void handleGetRequest(Print &out)
    {
        const WPage &page = getPage();
        page.handleGetRequest(out, fHeader);
    }
};


#endif