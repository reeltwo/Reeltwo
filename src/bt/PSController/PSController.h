#ifndef PSController_h
#define PSController_h

#if !defined(ESP32)
#error Only supports ESP32
#endif

#include <inttypes.h>
#include "JoystickController.h"
#include "Arduino.h"


class PSController : public JoystickController
{
public:
    enum Type
    {
        kPS3,
        kPS3Nav,
        kPS4
    };

    PSController(const char* mac, Type type = kPS3Nav);
    PSController() : PSController(nullptr) {}

    static bool startListening(const char* mac = nullptr);
    static bool startListening(String& mac) { return startListening(mac.c_str()); }
    static bool stopListening();

    static String getDeviceAddress();

    void setPlayer(int player);
    void setLED(uint8_t r, uint8_t g, uint8_t b);
    void setRumble(float leftIntensity, int leftDuration, float rightIntensity, int rightDuration);
    void setRumble(float intensity, int duration = -1)
    {
        setRumble(intensity, duration, intensity, duration);
    }
    virtual void disconnect() override;

    void setType(Type type);
    void setMacAddress(const char* mac);

    class priv;
private:
    friend class L2CAP;
    void parsePacket(uint8_t* packet);

    Type fType;
    int fPlayer;
    uint16_t fHIDC;
    uint16_t fHIDI;
    uint8_t fBDAddr[6];
    State fState;
};

#endif
