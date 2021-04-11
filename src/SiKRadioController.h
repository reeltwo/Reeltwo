#ifndef SiKRadioController_h
#define SiKRadioController_h

#include "JoystickController.h"

#ifdef USE_RADIO
#include <OSCMessage.h>
#include <SLIPEncodedSerial.h>

class SiKRadioController : public JoystickController
{
public:
    SiKRadioController(HardwareSerial &serial) :
        fSLIP(serial)
    {
        memset(&state, '\0', sizeof(state));
        memset(&event, '\0', sizeof(event));
        // serial.setTimeout(100);
    }

    void start()
    {
        xTaskCreatePinnedToCore(
              joyLoopTask,
              "joy",
              4000,
              this,
              1,
              &fTask,
              0);
    }

private:
    SLIPEncodedSerial fSLIP;
    TaskHandle_t fTask;

    void loop()
    {
        uint32_t lastEvent = 0;
        fConnecting = true;
        for (;;)
        {
            OSCMessage msg;
            if (fSLIP.available())
            {
                while (!fSLIP.endofPacket())
                {
                    while (fSLIP.available())
                    {
                        msg.fill(fSLIP.read());
                        if (fConnecting)
                        {
                            Serial.println("RADIO CONNECTED");
                            fConnecting = false;
                        }
                    }
                }
            }
            if (!msg.hasError())
            {
                state.analog.stick.lx = map(msg.getInt(0), 0, 1024, 127, -128);
                state.analog.stick.ly = map(msg.getInt(1), 0, 1024, 127, -128);
                state.analog.button.l2 = 255;
                int32_t b = msg.getInt(2);
                state.button.left =  ((b & (1<<1)) != 0);
                state.button.up =    ((b & (1<<2)) != 0);
                state.button.right = ((b & (1<<3)) != 0);
                state.button.down =  ((b & (1<<4)) != 0);
                state.button.select= ((b & (1<<0)) != 0);
                fConnected = true;
                lastEvent = millis();
                // Serial.println(String(state.analog.stick.lx)+","+String(state.analog.stick.ly));
            }
            if (fConnected && lastEvent + 200 < millis())
            {
                Serial.println("NO RADIO DATA for 200ms");
                fConnected = false;
            }
            vTaskDelay(1);
        }
    }

    static void joyLoopTask(void* arg)
    {
        RadioController* ctrl = (RadioController*)arg;
        ctrl->loop();
    }

    static int map(int x, int in_min, int in_max, int out_min, int out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }
};
#endif

#endif
