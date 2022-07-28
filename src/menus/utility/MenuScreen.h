///////////////////////////////////////////////////////////////////////////////
//
// Utility base class for menu selection screens
// 
///////////////////////////////////////////////////////////////////////////////

class MenuScreen : public CommandScreen
{
public:
    MenuScreen(ScreenID id, const char** menu, uint8_t siz) :
        CommandScreen(sDisplay, id),
        fMenu(menu),
        fMenuCount(siz)
    {}

    virtual void init() override
    {
        fCurrentDisplayItem = -1;
    }

    virtual void render() override
    {
        if (fCurrentItem != fCurrentDisplayItem)
        {
            sDisplay.invertDisplay(false);
            sDisplay.clearDisplay();
            sDisplay.setTextSize(2);
            sDisplay.setCursor(0, 0);
            sDisplay.println(fMenu[fCurrentItem]);
            sDisplay.display();
            fCurrentDisplayItem = fCurrentItem;
        }
    }

    virtual void buttonUpPressed(bool repeat) override
    {
        fCurrentItem = (fCurrentItem > 0) ? fCurrentItem - 1 : fMenuCount - 1;
    }

    virtual void buttonDownPressed(bool repeat) override
    {
        fCurrentItem = (fCurrentItem + 1 < fMenuCount) ? fCurrentItem + 1 : 0;
    }

    virtual void buttonLeftReleased() override
    {
        popScreen();
    }

    virtual void buttonRightReleased() override
    {
        buttonInReleased();
    }

protected:
    const char** fMenu;
    uint8_t fMenuCount;
    int16_t fCurrentItem = 0;
    int16_t fCurrentDisplayItem = -1;
};

