#ifndef CommandScreen_h
#define CommandScreen_h

#include "ReelTwo.h"

#ifdef USE_SCREEN_DEBUG
#define MENU_SCREEN_DEBUG_PRINTLN(s) DEBUG_PRINTLN(s)
#define MENU_SCREEN_DEBUG_PRINT(s) DEBUG_PRINT(s)
#define MENU_SCREEN_DEBUG_PRINTLN_HEX(s) DEBUG_PRINTLN_HEX(s)
#define MENU_SCREEN_DEBUG_PRINT_HEX(s) DEBUG_PRINT_HEX(s)
#define MENU_SCREEN_DEBUG_FLUSH(s) DEBUG_FLUSH()
#else
#define MENU_SCREEN_DEBUG_PRINTLN(s) 
#define MENU_SCREEN_DEBUG_PRINT(s) 
#define MENU_SCREEN_DEBUG_PRINTLN_HEX(s) 
#define MENU_SCREEN_DEBUG_PRINT_HEX(s) 
#define MENU_SCREEN_DEBUG_FLUSH(s) 
#endif

const typedef struct SerialCommand_t {
    char label[30];
    char cmd[15];
} SerialCommand;

class CommandMenu
{
public:
    CommandMenu(SerialCommand* cmds, uint8_t siz) :
        fCommands(cmds),
        fSize(siz),
        fValue(0)
    {
    }

    SerialCommand* getSerialCommand(int index)
    {
        return &(fCommands[index % fSize]);
    }

    const char* getButtonLabel(int index, char* buf, size_t maxSize)
    {
        strncpy_P(buf, getSerialCommand(index)->label, maxSize);
        if (strlen(buf) > 9)
        {
            char *ix = buf;
            int n = 0;
            while ((ix = strchr(ix, ' ')) != nullptr)
            {
                *ix++ = '\n';
                n++;
            }
        }
        return buf;
    }

    const char* getLabel(int index, char* buf, size_t maxSize)
    {
        strncpy_P(buf, getSerialCommand(index)->label, maxSize);
        return buf;
    }

    const char* getCommand(int index, char* buf, size_t maxSize)
    {
        strncpy_P(buf, getSerialCommand(index)->cmd, maxSize);
        size_t len = strlen(buf);
        if (len > 0)
        {
            maxSize = (len + 1 < maxSize) ? len : maxSize-2;
            buf[maxSize] = '\r';
            buf[maxSize+1] = '\0';
        }
        return buf;
    }

    uint8_t getSize()
    {
        return fSize;
    }

    void draw(int x, int y)
    {
        if (fSize == 0)
            return;
        // if ((GD.inputs.track_tag & 0xff) == kTAG_SLIDER)
        //     fValue = GD.inputs.track_val;

        // GD.SaveContext();

        // // Setup slider and track
        // GD.Tag(kTAG_SLIDER);
        // GD.cmd_slider(455, 20, 15, 220, 0, fValue, 0xFFFF);
        // GD.cmd_track(455, 20, 15, 220, kTAG_SLIDER);

        // // Draw menu buttons
        // char buf[32];
        // int lineSize = (0xFFFF / (fSize / 3)) * 2;
        // for (int gridy = 0; gridy < 5; gridy++)
        // {
        //     for (int gridx = 0; gridx < 3; gridx++)
        //     {
        //         int tag = (gridy + fValue / lineSize) * 3 + gridx;
        //         if (tag >= fSize)
        //             break;
        //         getButtonLabel(tag, buf, sizeof(buf));
        //         if (buf[0] == '\0')
        //             continue;
        //         GD.Tag(tag+1);
        //         GD.ColorRGB(0xf7931e);
        //         GD.cmd_fgcolor((currentSelection() == tag+1) ? 0x75715e : 0x171812);
        //         GD.cmd_button(x + (90*gridx), y + (50*gridy), 80, 40, 16, 0, buf);
        //     }
        // }
        // GD.RestoreContext();
    }

    inline uint8_t getSelected()
    {
        return fSelected;
    }

    bool handleSelection(uint8_t selection)
    {
        if (selection > 0 && selection < fSize)
        {
            fSelected = selection-1;
            return true;
        }
        return false;
    }

    void changeCommands(SerialCommand* newCommands, uint8_t newSize)
    {
        fCommands = newCommands;
        fSize = newSize;
        fValue = 0;
    }

private:
    SerialCommand *fCommands;
    uint8_t fSize;
    uint16_t fValue;
    uint8_t fSelected;
};

class CommandScreen;

class CommandScreenHandler
{
public:
    CommandScreenHandler(uint32_t screenBlankDelay = 15000) :
        fScreenBlankDelay(screenBlankDelay)
    {}

    CommandScreen* current()
    {
        return fCurrentScreen;
    }

    ScreenID currentID();
    CommandScreen* findScreen(ScreenID id);

    unsigned getNumScreens();
    CommandScreen* getScreenAt(unsigned index);

    inline void setScreenBlankDelay(uint32_t millis)
    {
        fScreenBlankDelay = millis;
    }

    inline uint32_t getScreenSleepDuration()
    {
        return (isSleeping()) ? millis() - fLastMillis : 0;
    }

    void switchToScreen(ScreenID id, bool popStack = true)
    {
        CommandScreen* scr = findScreen(id);
        if (scr != nullptr)
        {
            fCurrentScreen = scr;
        }
        else
        {
            MENU_SCREEN_DEBUG_PRINTLN("SCREEN NOT FOUND: "+String(id));
        }
        if (popStack)
            fScreenIDSP = 0;
    }

    void pushScreen(ScreenID id)
    {
        if (fScreenIDSP < SizeOfArray(fScreenIDStack))
        {
            fScreenIDStack[fScreenIDSP++] = currentID();
            switchToScreen(id, false);
        }
        else
        {
            MENU_SCREEN_DEBUG_PRINTLN("STACK TOO DEEP");
        }
    }

    void popScreen()
    {
        if (fScreenIDSP > 0)
        {
            ScreenID id = fScreenIDStack[--fScreenIDSP];
            switchToScreen(id, false);
        }
    }

    void blankScreen()
    {
        if ((fScreenState & 0x1) == 0)
        {
            fScreenState |= 0x1;
            sleepDevice();
        }
    }

    void restoreScreen();

    bool isSleeping() const
    {
        return (fScreenState & 0x1) == 0x1;
    }

    void process();

    uint32_t elapsed()
    {
        return millis() - fScreenStartMillis;
    }

    inline bool isEnabled() const { return fEnabled; }
    void setEnabled(bool enabled)
    {
        fEnabled = enabled;
    }

    inline bool needsRedisplay()
    {
        if (fRedisplay)
        {
            fRedisplay = false;
            return true;
        }
        return false;
    }

    inline void setNeedsRedisplay()
    {
        fRedisplay = true;
    }

protected:
    friend class CommandScreen;
    CommandScreen* fHead = nullptr;
    CommandScreen* fTail = nullptr;
    CommandScreen* fCurrentScreen = nullptr;
    uint8_t fScreenState = 0;
    uint32_t fScreenStartMillis = 0;
    uint32_t fLastMillis = 0;
    CommandScreen* fLastScreen = nullptr;
    uint32_t fScreenBlankDelay;
    bool fScreenTouched = false;
    bool fEnabled = false;
    uint8_t fScreenIDSP = 0;
    bool fRedisplay = false;
    ScreenID fScreenIDStack[5];

    virtual void clearContext() {}
    virtual void wakeDevice() {}
    virtual void sleepDevice() {}
    virtual void saveContext() {}
    virtual void restoreContext() {}
    virtual bool isTouching() { return false; }
    virtual void swapDevice() {}
    virtual uint8_t currentSelection() { return 0; }
    virtual bool handleEvent() { return false; }

    void resetBlankingTimer()
    {
        fLastMillis = millis();
    }
    void append(CommandScreen* screen);
};

class CommandScreen
{
public:
    CommandScreen(CommandScreenHandler &handler, ScreenID id, SerialCommand* cmds = nullptr, uint8_t siz = 0) :
        fID(id),
        fMenu(cmds, siz),
        fLastTag(0),
        fHandler(handler),
        fNext(nullptr)
    {
        handler.append(this);
    }

    inline ScreenID ID() const
    {
        return fID;
    }

    inline uint8_t getSelected() const
    {
        return fLastTag;
    }

    virtual void init() {}
    virtual void exit() {}
    virtual void handleSelection(uint8_t selection) { UNUSED_ARG(selection) }
    virtual void render() = 0;
    virtual bool handleEvent() { return false; }
    virtual bool isActive() { return false; }
    virtual bool isStatus() { return false; }
    void switchToScreen(ScreenID id)
    {
        fHandler.switchToScreen(id);
    }
    void pushScreen(ScreenID id)
    {
        fHandler.pushScreen(id);
    }
    void popScreen()
    {
        fHandler.popScreen();
    }
    void restoreScreen()
    {
        fHandler.restoreScreen();
    }
    inline unsigned getKeyRepeatRate() const
    {
        return fKeyRepeatRateMS;
    }
    void setKeyRepeatRate(unsigned ms)
    {
        fKeyRepeatRateMS = ms;
    }
    virtual void buttonUpPressed(bool repeat = false) {}
    virtual void buttonLeftPressed(bool repeat = false) {}
    virtual void buttonDownPressed(bool repeat = false) {}
    virtual void buttonRightPressed(bool repeat = false) {}
    virtual void buttonInPressed(bool repeat = false) {}
    virtual void buttonUpReleased() {}
    virtual void buttonLeftReleased() {}
    virtual void buttonDownReleased() {}
    virtual void buttonRightReleased() {}
    virtual void buttonInReleased() {}
    virtual void buttonDial(long newValue, long oldValue = 0)
    {
        if (newValue > oldValue)
        {
            buttonUpPressed();
        }
        else
        {
            buttonDownPressed();
        }
    }

protected:
    static void toggleMaskBit(uint8_t &mask, uint8_t bit)
    {
        if ((mask & bit) == 0)
            mask |= bit;
        else
            mask &= ~bit;
    }
    static void toggleMaskBit(uint16_t &mask, uint16_t bit)
    {
        if ((mask & bit) == 0)
            mask |= bit;
        else
            mask &= ~bit;
    }
    void clearSelection()
    {
        fLastTag = 0;
    }
    inline bool hasMenu()
    {
        return (fMenu.getSize() != 0);
    }
    ScreenID fID;
    CommandMenu fMenu;
    uint8_t fLastTag;
    unsigned fKeyRepeatRateMS = 0;

private:
    friend class CommandScreenHandler;
    CommandScreenHandler &fHandler;
    CommandScreen* fNext;
};

ScreenID CommandScreenHandler::currentID()
{
    return (fCurrentScreen != nullptr) ? fCurrentScreen->fID : kInvalid;
}

CommandScreen* CommandScreenHandler::findScreen(ScreenID id)
{
    for (CommandScreen* scr = fHead; scr != nullptr; scr = scr->fNext)
    {
        if (scr->fID == id)
            return scr;
    }
    return nullptr;
}

unsigned CommandScreenHandler::getNumScreens()
{
    unsigned count = 0;
    for (CommandScreen* scr = fHead; scr != nullptr; scr = scr->fNext)
        count++;
    return count;
}

CommandScreen* CommandScreenHandler::getScreenAt(unsigned index)
{
    unsigned i = 0;
    for (CommandScreen* scr = fHead; scr != nullptr; scr = scr->fNext, i++)
    {
        if (i == index)
            return scr;
    }
    return nullptr;
}

void CommandScreenHandler::restoreScreen()
{
    resetBlankingTimer();
    if ((fScreenState & 0x1) == 0x1)
    {
        fScreenState &= ~0x1;
        wakeDevice();
        if (fCurrentScreen != nullptr)
            fCurrentScreen->init();
    }
}

void CommandScreenHandler::process()
{
    if (!fEnabled)
        return;
    if (fCurrentScreen == nullptr)
        fCurrentScreen = findScreen(ScreenID(0));
    clearContext();
    CommandScreen* current = fCurrentScreen;
    if (fLastScreen != current)
    {
        if (fLastScreen != nullptr)
        {
            fLastScreen->exit();
        }
        // Screen was changed
        if (current != nullptr)
        {
            MENU_SCREEN_DEBUG_PRINT("SCREEN: "); MENU_SCREEN_DEBUG_PRINTLN(current->fID);
            current->fLastTag = 0;
            current->init();
        }
        resetBlankingTimer();
        fScreenStartMillis = fLastMillis;
    }
    if (current != nullptr && current->isActive())
    {
        restoreScreen();
    }
    else if ((fScreenState & 0x1) == 0 && fScreenBlankDelay > 0 && fLastMillis + fScreenBlankDelay < millis())
    {
        blankScreen();
    }
    if (current != nullptr && (fScreenState & 0x1) == 0)
    {
        if (isTouching())
            resetBlankingTimer();

        saveContext();
        current->render();
        restoreContext();

        current->fMenu.draw(180, 10);
    }
    else
    {
        if (isTouching())
        {
            if (!fScreenTouched)
                fScreenTouched = true;
        }
        else if (fScreenTouched)
        {
            fScreenTouched = false;
            restoreScreen();
        }
    }
    swapDevice();
    if (current != nullptr)
    {
        if (handleEvent())
        {
            restoreScreen();
        }
    }
    if (currentSelection() == 0 && current->fLastTag != 0)
    {
        current->handleSelection(current->fLastTag);
        if (current->fLastTag > 0)
        {
            if (current->fMenu.handleSelection(current->fLastTag))
            {
                // char buf[SERIAL_BUFFER_SIZE];
                // const char* cmd = current->fMenu.getCommand(
                //     current->fMenu.getSelected(), buf, sizeof(buf));
                // if (*cmd != '\0')
                //     current->sendSerialCommand(cmd);
            }
        }
        current->fLastTag = 0;
    }
    else
    {
        if (current->fLastTag != 0 && isTouching())
        {
            // ignore roaming fingers
        }
        else
        {
            current->fLastTag = currentSelection();
        }
    }
    fLastScreen = current;
}

void CommandScreenHandler::append(CommandScreen* screen)
{
    if (fHead == nullptr)
        fHead = screen;
    if (fTail != nullptr)
        fTail->fNext = screen;
    fTail = screen;
}
#endif
