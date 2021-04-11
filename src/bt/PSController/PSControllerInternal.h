#ifndef PSControllerInternal_h
#define PSControllerInternal_h

#include "sdkconfig.h"

/** Check if the project is configured properly */
#ifndef ARDUINO_ARCH_ESP32

#ifndef CONFIG_BT_ENABLED
#error "The ESP32-PS3 module requires the Bluetooth component to be enabled in the project's menuconfig"
#endif

#ifndef CONFIG_BLUEDROID_ENABLED
#error "The ESP32-PS3 module requires Bluedroid to be enabled in the project's menuconfig"
#endif

#ifndef CONFIG_CLASSIC_BT_ENABLED
#error "The ESP32-PS3 module requires Classic Bluetooth to be enabled in the project's menuconfig"
#endif

#ifndef CONFIG_BT_SPP_ENABLED
#error "The ESP32-PS3 module requires Classic Bluetooth's SPP to be enabled in the project's menuconfig"
#endif

/** Check the configured blueooth mode */
#ifdef CONFIG_BTDM_CONTROLLER_MODE_BTDM
#define BT_MODE ESP_BT_MODE_BTDM
#elif defined CONFIG_BTDM_CONTROLLER_MODE_BR_EDR_ONLY
#define BT_MODE ESP_BT_MODE_CLASSIC_BT
#else
#error "The selected Bluetooth controller mode is not supported by the ESP32-PS3 module"
#endif

#endif // ARDUINO_ARCH_ESP32

/* Detect ESP-IDF releases */
#if __has_include("esp_idf_version.h")
#include <esp_idf_version.h>

#else

/* Detect Arduino releases */
#if __has_include("core_version.h")
#include <core_version.h>
#endif

/* Arduino releases using IDF v3.2.3 */
#if defined(ARDUINO_ESP32_RELEASE_1_0_4) || defined(ARDUINO_ESP32_RELEASE_1_0_3)
#define ESP_IDF_VERSION_MAJOR 3
#define ESP_IDF_VERSION_MINOR 2
#define ESP_IDF_VERSION_PATCH 3
#endif

/* Arduino releases using IDF v3.2.2 */
#if defined(ARDUINO_ESP32_RELEASE_1_0_3) || defined(ARDUINO_ESP32_RELEASE_1_0_2) || defined(ARDUINO_ESP32_RELEASE_1_0_1) || defined(ARDUINO_ESP32_RELEASE_1_0_0)
#define ESP_IDF_VERSION_MAJOR 3
#define ESP_IDF_VERSION_MINOR 2
#define ESP_IDF_VERSION_PATCH 2
#endif

// Macro to convert IDF version number into an integer
#define ESP_IDF_VERSION_VAL(major, minor, patch) ((major << 16) | (minor << 8) | (patch))

// Current IDF version, as an integer
#define ESP_IDF_VERSION  ESP_IDF_VERSION_VAL(ESP_IDF_VERSION_MAJOR, \
                                             ESP_IDF_VERSION_MINOR, \
                                             ESP_IDF_VERSION_PATCH)

#endif // __has_include("esp_idf_version.h")

extern "C" {
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_spp_api.h"
#include "stack/gap_api.h"
#include "stack/bt_types.h"
#include "stack/l2c_api.h"
#include "stack/btm_api.h"
#include "osi/allocator.h"
}

#include "PSController.h"

#endif
