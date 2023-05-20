#ifndef CommandScreenHandlerSMQ_h
#define CommandScreenHandlerSMQ_h

#include "ReelTwo.h"

class CommandScreenHandlerSMQ: public CommandScreenHandler
{
public:
    CommandScreenHandlerSMQ()
    {
        resetDisplay();
    }

    inline void invertDisplay(bool invert)
    {
        fInvert = invert;
    }

    inline void clearDisplay()
    {
        fClearDisplay = true;
    }

    inline void setTextSize(int siz)
    {
        fTextSize = siz;
    }

    void drawTextCentered(String text)
    {
        fCentered = true;
        fString = text;
    }

    void setCursor(uint8_t x, uint8_t y)
    {
        fX = x;
        fY = y;
    }

    void print(String text)
    {
        fString += text;
    }

    void println(unsigned val)
    {
        fString += String("\n") + String(val);
    }

    void println(String text)
    {
        fString += String("\n") + text;
    }

    void display()
    {
        if (SMQ::sendTopic("LCD", "Remote"))
        {
            SMQ::send_uint8("x", fX);
            SMQ::send_uint8("y", fY);
            SMQ::send_boolean("invert", fInvert);
            SMQ::send_boolean("centered", fCentered);
            SMQ::send_uint8("size", fTextSize);
            SMQ::send_string("text", fString.c_str());
            SMQ::sendEnd();
            resetDisplay();
        }
        else
        {
            /* Request display() be called again */
            setNeedsRedisplay();
        }
    }

    void remoteDialEvent(long newValue, long oldValue)
    {
        fNewDialValue = newValue;
    }

    void remoteButtonEvent(uint8_t id, bool pressed, bool repeat)
    {
        fButtonID = id;
        fButtonPressed = pressed;
        fButtonRepeat = repeat;
    }

    void remoteActive()
    {
        setEnabled(true);
        fRemoteActive = true;
    }

    virtual void sleepDevice() override
    {
    }

    virtual bool handleEvent()
    {
        bool ret = false;
        if (fRemoteActive)
        {
            restoreScreen();
            switchToScreen(kMainScreen);
            fRemoteActive = false;
        }
        CommandScreen* currentScr = current();
        if (currentScr != nullptr)
        {
            if (fDialValue != fNewDialValue || fButtonID != 0)
            {
                // Reset the screen blanking timer if the encoder changed
                // or if any of the buttons are currently pressed
                ret = true;
            }
            if (fDialValue != fNewDialValue)
            {
                currentScr->buttonDial(fNewDialValue, fDialValue);
                fDialValue = fNewDialValue;
            }
            if (fButtonID)
            {
                if (fButtonID == 1 && fButtonPressed)
                    currentScr->buttonUpPressed();
                if (fButtonID == 2 && fButtonPressed)
                    currentScr->buttonLeftPressed();
                if (fButtonID == 3 && fButtonPressed)
                    currentScr->buttonDownPressed();
                if (fButtonID == 4 && fButtonPressed)
                    currentScr->buttonRightPressed();
                if (fButtonID == 5 && fButtonPressed)
                    currentScr->buttonInPressed();
                if (fLastScreenID == currentScr->ID())
                {
                    if (fButtonID == 1 && !fButtonPressed)
                        currentScr->buttonUpReleased();
                    if (fButtonID == 2 && !fButtonPressed)
                        currentScr->buttonLeftReleased();
                    if (fButtonID == 3 && !fButtonPressed)
                        currentScr->buttonDownReleased();
                    if (fButtonID == 4 && !fButtonPressed)
                        currentScr->buttonRightReleased();
                    if (fButtonID == 5 && !fButtonPressed)
                        currentScr->buttonInReleased();
                }
                fLastScreenID = currentScr->ID();
            }
            else if (fLastScreenID == currentScr->ID())
            {
                if (fButtonID == 1 && fButtonPressed && fButtonRepeat)
                {
                    currentScr->buttonUpPressed(true);
                }
                else if (fButtonID == 2 && fButtonPressed && fButtonRepeat)
                {
                    currentScr->buttonLeftPressed(true);
                }
                else if (fButtonID == 3 && fButtonPressed && fButtonRepeat)
                {
                    currentScr->buttonDownPressed(true);
                }
                else if (fButtonID == 4 && fButtonPressed && fButtonRepeat)
                {
                    currentScr->buttonRightPressed(true);
                }
                else if (fButtonID == 5 && fButtonPressed && fButtonRepeat)
                {
                    currentScr->buttonInPressed(true);
                }
            }
        }
        fButtonID = 0;
        fButtonPressed = false;
        fButtonRepeat = false;
        fDialValue = fNewDialValue;
        return ret;
    }

private:
    long        fDialValue = 0;
    long        fNewDialValue = 0;
    uint8_t     fButtonID = 0;
    bool        fRemoteActive = false;
    bool        fButtonPressed = false;
    bool        fButtonRepeat = false;
    ScreenID    fLastScreenID = kInvalid;
    bool        fInvert;
    bool        fClearDisplay;
    bool        fCentered;
    String      fString;
    uint8_t     fX;
    uint8_t     fY;
    uint8_t     fTextSize;

    void resetDisplay()
    {
        fInvert = false;
        fClearDisplay = false;
        fTextSize = 0;
        fCentered = false;
        fString = "";
        fX = 0;
        fY = 0;
    }
};
#endif
