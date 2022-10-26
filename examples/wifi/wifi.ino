#define USE_DEBUG

#include <ReelTwo.h>
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include "wifi/WifiAccess.h"
#include "wifi/WifiWebServer.h"

#define WIFI_AP_NAME                    "ReeltwoWiFi"
#define WIFI_AP_PASSPHRASE              "Astromech"

#define INTEGER_RANGE_LOW 1
#define INTEGER_RANGE_HIGH 255

WifiAccess wifiAccess;
TaskHandle_t wifiTask;

int slidervalue = 50;
bool checkedvalue = true;
bool rangeenabled = false;
String wifiName = WIFI_AP_NAME;
String wifiPass = WIFI_AP_PASSPHRASE;
String items[] = {
    "Item #0",
    "Item #1",
    "Item #2",
    "Item #3",
    "Item #4",
    "Item #5",
    "Item #6",
    "Item #7",
    "Item #8"
};
int selectedItem = 0;
int integerRange = 42;

// Single web page "ReelTwo WiFi". With five buttons.
WElement mainContents[] = {
    W1("ReelTwo WiFi"),
    // Start aligning horizontally
    WHorizontalAlign(),
    WButton("Hello", "hello", []() {
        DEBUG_PRINTLN("Hello");
    }),
    WButton("World", "world", []() {
        DEBUG_PRINTLN("World");
    }),

    // Divider line
    WHR(),

    // Start aligning vertically
    WVerticalAlign(),

  	// Slide value range 0 - 100 value stored in 'slidervalue'
    WSlider("My Slider", "slidervalue", 0, 100,
        []()->int { return slidervalue; },
        [](int val) {
        	slidervalue = val;
        	DEBUG_PRINT("slider: "); DEBUG_PRINTLN(val);
        }
    ),

  	// Check box on/off value stored in 'checkedvalue'
    WCheckbox("Sample Checkbox", "checkedvalue",
        []() { return checkedvalue; },
        [](bool val) {
        	checkedvalue = val;
        	DEBUG_PRINT("checkbox: "); DEBUG_PRINTLN(val);
        }
    ),

    // Text field value stored in 'wifiName'
    WTextField("WiFi:", "wifi",
        []()->String { return wifiName; },
        [](String val) {
        	wifiName = val;
        	DEBUG_PRINT("SSID: "); DEBUG_PRINTLN(val);
        }
    ),

    // Password text field value stored in 'wifiPass'
    WPassword("Password:", "password",
        []()->String { return wifiPass; },
        [](String val) {
        	wifiPass = val;
        	DEBUG_PRINT("Password: "); DEBUG_PRINTLN(val);
        }
    ),

  	// Check box on/off value stored in 'rangeenabled'. Reload content after action.
    WCheckboxReload("Enable Range", "rangeenabled",
        []() { return rangeenabled; },
        [](bool val) {
        	rangeenabled = val;
        	DEBUG_PRINT("rangeenabled: "); DEBUG_PRINTLN(val);
        }
    ),

    // Integer text field number range 1 - 255 value stored in 'integerRange'
    WTextFieldIntegerRange("Range:", "range",
        INTEGER_RANGE_LOW, INTEGER_RANGE_HIGH,
        []()->String { return String(integerRange); },
        [](String val) {
        	int ival = val.toInt();
        	if (ival >= INTEGER_RANGE_LOW && ival <= INTEGER_RANGE_HIGH) {
        		integerRange = ival;
	        	DEBUG_PRINT("Range: "); DEBUG_PRINTLN(ival);
        	}
        },
        // Return true if enabled or not
        []() {
        	return (rangeenabled);
        }
    ),

    // Popup allows selection from array of strings
    WSelect("Select Item", "selected",
        items, SizeOfArray(items),
        []() { return selectedItem; },
        [](int val) {
            selectedItem = val;
        	DEBUG_PRINT("Selected: "); DEBUG_PRINTLN(val);
        }
    ),

    // Start aligning horizontally
    WHorizontalAlign(),

    // WButtonReload will reload the content after the action
    WButtonReload("Next", "next", []() {
    	if (selectedItem < SizeOfArray(items)-1)
    		selectedItem++;
    	DEBUG_PRINT("Selected: "); DEBUG_PRINTLN(selectedItem);
    }),
    WButtonReload("Prev", "prev", []() {
    	if (selectedItem > 0)
    		selectedItem--;
    	DEBUG_PRINT("Selected: "); DEBUG_PRINTLN(selectedItem);
    }),

    // Start aligning vertically
    WVerticalAlign(),

    // Print out the status of all the variables
    WButton("Done", "done", []() {
		DEBUG_PRINT("slider:   "); DEBUG_PRINTLN(slidervalue);
		DEBUG_PRINT("checkbox: "); DEBUG_PRINTLN(checkedvalue);
		DEBUG_PRINT("ssid:     "); DEBUG_PRINTLN(wifiName);
		DEBUG_PRINT("password: "); DEBUG_PRINTLN(wifiPass);
		DEBUG_PRINT("enabled:  "); DEBUG_PRINTLN(rangeenabled);
		DEBUG_PRINT("range:    "); DEBUG_PRINTLN(integerRange);
		DEBUG_PRINT("selected: "); DEBUG_PRINTLN(selectedItem);
    }),
};

WPage pages[] = {
    WPage("/", mainContents, SizeOfArray(mainContents)),
};

WifiWebServer<10,SizeOfArray(pages)> webServer(pages, wifiAccess);

// WiFi Task runs on other core
void wifiLoopTask(void* /*ignore*/)
{
    for (;;)
    {
        webServer.handle();
        vTaskDelay(1);
    }
}

void setup()
{
    REELTWO_READY();

    // Early initialization here:
    // I2C, pinMode, Serial, etc

    // Call setup() in all subclasses of SetupEvent
    SetupEvent::ready();

    // Additional initialization code here

    wifiAccess.setNetworkCredentials(
        WIFI_AP_NAME,
        WIFI_AP_PASSPHRASE,
        true, /* WiFi Access Point */
        true  /* WiFi Enabled */);

    wifiAccess.notifyWifiConnected([](WifiAccess &wifi) {
    	// WiFi is active
        Serial.print("Connect to http://"); Serial.println(wifi.getIPAddress());
    });

    xTaskCreatePinnedToCore(
          wifiLoopTask,
          "WiFi",
          5000,    // 5K stack size
          nullptr,
          1,
          &wifiTask,
          0);
}

void loop()
{
	// Call animate() in all subclasses of AnimatedEvent
    AnimatedEvent::process();

    // Additional action performed each time throught he loop
}
