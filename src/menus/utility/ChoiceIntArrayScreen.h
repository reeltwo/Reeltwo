///////////////////////////////////////////////////////////////////////////////
//
// Utility base class for screens selecting between several integer values
// 
///////////////////////////////////////////////////////////////////////////////

class ChoiceIntArrayScreen : public CommandScreen
{
public:
    ChoiceIntArrayScreen(ScreenID id, unsigned* values, uint8_t siz) :
        CommandScreen(sDisplay, id),
        fValues(values),
        fNumValues(siz)
    {}

    virtual void init()
    {
        fDisplayValue = ~0u;
        fStartValue = getValue();
    }

    virtual void buttonUpPressed(bool repeat) override
    {
        unsigned val = getValueToIndex(getValue());
        if (val > 0)
            setValue(fValues[val-1]);
    }

    virtual void buttonLeftPressed(bool repeat) override
    {
        popScreen();
        saveValue(fStartValue);
    }

    virtual void buttonDownPressed(bool repeat) override
    {
        unsigned val = getValueToIndex(getValue());
        if (val+1 < fNumValues)
        {
            setValue(fValues[val+1]);
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
    unsigned* fValues;
    uint8_t fNumValues;

    virtual unsigned getValueToIndex(unsigned value)
    {
        for (unsigned i = 0; i < fNumValues; i++)
        {
            if (fValues[i] == value)
                return i;
        }
        return 0;
    }

    virtual unsigned getValue() = 0;

    virtual void setValue(unsigned newValue) = 0;

    virtual void saveValue(unsigned newValue) = 0;
};
