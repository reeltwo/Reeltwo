#ifndef SerialConsoleController_h
#define SerialConsoleController_h

#include "ReelTwo.h"
#include "JoystickController.h"

#ifndef SERIAL_CONSOLE_LONG_PRESS_TIME
#define SERIAL_CONSOLE_LONG_PRESS_TIME      3000
#endif

class SerialConsoleController : public JoystickController
{
public:
    enum {
        kHome = -50,
        kPageUp = -53,
        kPageDown = -54,
        kEnd = -70
    };

    SerialConsoleController(Stream &serial) :
        fSerial(&serial)
    {
        memset(&state, '\0', sizeof(state));
        memset(&event, '\0', sizeof(event));
        memset(&longpress, '\0', sizeof(longpress));
        fConnecting = true;
        fConnected = false;
    }

    uint16_t y;
    uint16_t x;
    uint16_t w1;
    uint16_t w2;
    bool button[5];
    struct LongPress {
        uint32_t pressTime;
        bool longPress;
    };
    struct {
        LongPress l3;
        LongPress triangle;
        LongPress circle;
        LongPress cross;
        LongPress square;
    } longpress;
    uint32_t lastPacket;

    inline float getSpeed()
    {
        return fSpeed;
    }

    inline void setSpeed(float speed)
    {
        fSpeed = max(min(speed, 1.0f), 0.0f);
    }

    inline void increaseSpeed()
    {
        setSpeed(fSpeed + 0.1);
    }

    inline void decreaseSpeed()
    {
        setSpeed(fSpeed - 0.1);
    }

    void update()
    {
        Event evt = {};
        State prev = state;
        state.analog.stick.lx = map(x, 0, 1024, 127, -128);
        state.analog.stick.ly = map(y, 0, 1024, 127, -128);
        state.analog.stick.rx = state.analog.stick.lx;
        state.analog.stick.ry = state.analog.stick.ry;
        state.analog.button.l1 = map(w2, 0, 1024, 255, 0);
        state.analog.button.l2 = map(w1, 0, 1024, 255, 0);
        state.analog.button.r1 = state.analog.button.l1;
        state.analog.button.r2 = state.analog.button.l2;
        state.button.triangle = button[0];
        state.button.circle = button[1];
        state.button.cross = button[2];
        state.button.square = button[3];
        state.button.l3 = button[4];

        // for (int i = 0; i < SizeOfArray(button); i++)
        // {
        //     if (button[i])
        //     {
        //         CONSOLE_SERIAL.print("BUTTON_DOWN ");
        //         CONSOLE_SERIAL.println(i);
        //     }
        // }
    #define CHECK_BUTTON_DOWN(b) evt.button_down.b = (!prev.button.b && state.button.b)
        CHECK_BUTTON_DOWN(l3);
        CHECK_BUTTON_DOWN(triangle);
        CHECK_BUTTON_DOWN(circle);
        CHECK_BUTTON_DOWN(cross);
        CHECK_BUTTON_DOWN(square);
    #define CHECK_BUTTON_UP(b) evt.button_up.b = (prev.button.b && !state.button.b)
        CHECK_BUTTON_UP(l3);
        CHECK_BUTTON_UP(triangle);
        CHECK_BUTTON_UP(circle);
        CHECK_BUTTON_UP(cross);
        CHECK_BUTTON_UP(square);
    #define CHECK_BUTTON_LONGPRESS(b) \
    { \
        evt.long_button_up.b = false; \
        if (evt.button_down.b) \
        { \
            longpress.b.pressTime = millis(); \
            longpress.b.longPress = false; \
        } \
        else if (evt.button_up.b) \
        { \
            longpress.b.pressTime = 0; \
            if (longpress.b.longPress) \
                evt.button_up.b = false; \
            longpress.b.longPress = false; \
        } \
        else if (longpress.b.pressTime != 0 && state.button.b) \
        { \
            if (longpress.b.pressTime + SERIAL_CONSOLE_LONG_PRESS_TIME < millis()) \
            { \
                longpress.b.pressTime = 0; \
                longpress.b.longPress = true; \
                evt.long_button_up.b = true; \
            } \
        } \
    }
        CHECK_BUTTON_LONGPRESS(l3);
        CHECK_BUTTON_LONGPRESS(triangle);
        CHECK_BUTTON_LONGPRESS(circle);
        CHECK_BUTTON_LONGPRESS(cross);
        CHECK_BUTTON_LONGPRESS(square);

        /* Analog events */
        evt.analog_changed.stick.lx  = state.analog.stick.lx - prev.analog.stick.lx;
        evt.analog_changed.stick.ly  = state.analog.stick.ly - prev.analog.stick.ly;
        evt.analog_changed.button.l1 = state.analog.button.l1 - prev.analog.button.l1;
        evt.analog_changed.button.l2 = state.analog.button.l2 - prev.analog.button.l2;
        if (fConnecting)
        {
            fConnecting = false;
            fConnected = true;
            onConnect();
        }
        if (fConnected)
        {
            event = evt;
            notify();
        }
    }

    void connect()
    {
        fEmulationActive = true;
    }

    virtual void disconnect() override
    {
        fEmulationActive = false;
    }

    inline bool isEmulationActive() const
    {
        return fEmulationActive;
    }

protected:
    Stream* fSerial = nullptr;
    uint32_t fLastTime = 0;
    float fSpeed = 0.5;
    bool fEmulationActive = false;

    static uint16_t updateDirection(float speed)
    {
        return 512 + 512 * speed;
    }

    char readCharBlocking()
    {
        while (!fSerial->available())
            ;
        return fSerial->read();
    }

    int read()
    {
        int ret = 0;
        if (fEmulationActive && fSerial->available())
        {
            int ch = fSerial->read();
            switch (ch)
            {
                case 27:
                    switch (readCharBlocking())
                    {
                        case 79:
                            switch (readCharBlocking())
                            {
                                case 80:
                                    /* F1 */
                                    break;
                                case 81:
                                    /* F2 */
                                    break;
                                case 82:
                                    /* F3 */
                                    break;
                                case 84:
                                    /* F4 */
                                    break;
                            }
                            break;
                        case 91:
                            switch (readCharBlocking())
                            {
                                case 65:
                                    /* Up arrow */
                                    y = updateDirection(-fSpeed);//512 - 256;
                                    w1 = updateDirection(0);
                                    update();
                                    break;
                                case 66:
                                    /* Down arrow */
                                    y = updateDirection(fSpeed);//512 + 256;
                                    w1 = updateDirection(0);
                                    update();
                                    break;
                                case 67:
                                    /* Right arrow */
                                    x = updateDirection(fSpeed); //512 + 256;
                                    w1 = updateDirection(0);
                                    update();
                                    break;
                                case 68:
                                    /* Left arrow */
                                    x = updateDirection(-fSpeed);//512 - 256;
                                    w1 = updateDirection(0); //512;
                                    update();
                                    break;
                                case 50:
                                    switch (fSerial->read())
                                    {
                                        case 126:
                                            /* Home */
                                            ret = kHome;
                                            break;
                                    }
                                    break;
                                case 53:
                                    switch (readCharBlocking())
                                    {
                                        case 126:
                                            /* Page up */
                                            ret = kPageUp;
                                            break;
                                    }
                                    break;
                                case 54:
                                    switch (readCharBlocking())
                                    {
                                        case 126:
                                            /* Page down */
                                            ret = kPageDown;
                                            break;
                                    }
                                    break;
                                case 70:
                                    /* End */
                                    ret = kEnd;
                                    break;
                            }
                            break;
                    }
                    break;
                default:
                    ret = ch;
                    break;
            }
            fLastTime = millis();
        }
        else if (fLastTime + 500 < millis())
        {
            if (x != 512 || y != 512)
            {
                x = updateDirection(0);//512;
                y = updateDirection(0);//512;
                update();
                // DEBUG_PRINTLN("IDLE");
            }
        }
        return ret;
    }
};
#endif
