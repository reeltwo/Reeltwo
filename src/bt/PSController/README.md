# PSController
Use a PS3 or PS4 controller with an esp32

This is heavily modified version of these two repos:
	https://github.com/jvpernis/esp32-ps3
	https://github.com/aed3/PS4-esp32

This repo can be downloaded as a zip file and imported into the Arduino IDE as a library.
The instructions on how to do this and the library for PS3 controllers can be found here: https://github.com/jvpernis/esp32-ps3/issues/3#issuecomment-517141523

### Display Bluetooth address ###

The example sketches in this libary all demonstrate initializing the libary using a custom Bluetooth MAC address. However, instead of hardcoding the MAC address like this in your sketch, you might want to simply read the ESP32's MAC address so that you can write it to the PS3 controller.

Luckily, this can be accomplished by a simple sketch using `Ps3.getAddress()`:

```c

#include <PSController.h>

void setup()
{
    Serial.begin(115200);
    PSController::startListening();

    String address = PSController::getDeviceAddress();
    Serial.println(address);
}

```

## Pairing the PS3/PS4 Controller:
When a PS controller is 'paired' to a Playstation console, it just means that it has stored the console's Bluetooth MAC address, which is the only device the controller will connect to. Usually, this pairing happens when you connect the controller to the PS4 console using a USB cable, and press the PS button. This initiates writing the console's MAC address to the controller.

Therefore, if you want to connect your PS3/PS4 controller to the ESP32, you either need to figure out what the Bluetooth MAC address of your Playstation console is and set the ESP32's address to it, or change the MAC address stored in the PS4 controller.

Whichever path you choose, you're going to need a tool to read and/or write the currently paired MAC address from the PS3/PS4 controller. I used [SixaxisPairTool](https://dancingpixelstudios.com/sixaxis-controller/sixaxispairtool/) for this, but you can try using [sixaxispairer](https://github.com/user-none/sixaxispairer) as well, if open source is important to you.

If you opted to change the ESP32's MAC address, you'll need to include the ip address in the ```PSController::startListening()``` function during within the ```setup()``` Arduino function like below where ```03:03:03:03:03:03``` is the MAC address:
```
void setup()
{
    Serial.begin(115200);
    PSController::startListening("03:03:03:03:03:03");
    Serial.println("Ready.");
}
```
