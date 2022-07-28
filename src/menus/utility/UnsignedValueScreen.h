///////////////////////////////////////////////////////////////////////////////
//
// Utility base class for screens specifying an unsigned integer
// from min - max value
// 
///////////////////////////////////////////////////////////////////////////////

class UnsignedValueScreen : public CommandScreen
{
public:
    UnsignedValueScreen(ScreenID id, unsigned maxValue = ~0u, unsigned minValue = 1) :
        CommandScreen(sDisplay, id),
        fMaxValue(maxValue),
        fMinValue(minValue)
    {}

    virtual void init()
    {
        fDisplayValue = ~0u;
        fStartValue = getValue();
    }

    virtual void buttonUpPressed(bool repeat) override
    {
        unsigned val = getValue();
        if (val > fMinValue)
        {
            setValue(val-1);
        }
    }

    virtual void buttonLeftPressed(bool repeat) override
    {
        popScreen();
        setValue(fStartValue);
    }

    virtual void buttonDownPressed(bool repeat) override
    {
        unsigned val = getValue();
        if (val < fMaxValue)
        {
            setValue(val+1);
        }
    }

    virtual void buttonInReleased() override
    {
        if (fDisplayValue != ~0u)
        {
            saveValue(fDisplayValue);
            popScreen();
            if (fDisplayValue != fStartValue)
            {
                pushScreen(kSettingsUpdatedScreen);
            }
        }
        else
        {
            popScreen();
        }
    }

    virtual void render() override
    {
        unsigned value = getValue();
        if (value != fDisplayValue)
        {
            sDisplay.invertDisplay(value != fStartValue);
            sDisplay.clearDisplay();
            sDisplay.setTextSize(4);
            sDisplay.setCursor(5, 0);
            sDisplay.println(value);
            sDisplay.display();
            fDisplayValue = value;
        }
    }

protected:
    unsigned fDisplayValue = ~0u;
    unsigned fStartValue;
    unsigned fMaxValue;
    unsigned fMinValue;

    virtual unsigned getValue() = 0;
    virtual void setValue(unsigned newValue) = 0;
    virtual void saveValue(unsigned newValue) = 0;
};

