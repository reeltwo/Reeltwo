#if defined(ESP32)

#include "PSControllerInternal.h"

/** Size of the output report buffer for the Dualshock and Navigation controllers */
#define PS3_REPORT_BUFFER_SIZE 48

#define PS4_REPORT_BUFFER_SIZE 77

#define PS3_DEVICE_NAME "PS Host"
#define PS3_SERVER_NAME "PS_SERVER"

#define ESP_BD_ADDR_HEX_STR        "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx"
#define ESP_BD_ADDR_HEX_ARR(addr)   addr[0],  addr[1],  addr[2],  addr[3],  addr[4],  addr[5]
#define ESP_BD_ADDR_HEX_PTR(addr)  &addr[0], &addr[1], &addr[2], &addr[3], &addr[4], &addr[5]

//////////////////////////////////////////////////////////////////////////////////

enum
{
    kHIDCMD_code_set_report   = 0x50,
    kHIDCMD_code_type_output  = 0x02,
    kHIDCMD_code_type_feature = 0x03,

    kHIDCMD_identifier_ps3_enable  = 0xf4,
    kHIDCMD_identifier_ps3_control = 0x01,

    kHIDCMD_identifier_ps4_enable  = 0xf4,
    kHIDCMD_identifier_ps4_control = 0x11
};

struct hid_ps3cmd_t
{
    uint8_t code;
    uint8_t identifier;
    uint8_t data[PS3_REPORT_BUFFER_SIZE];
};

struct hid_ps4cmd_t
{
    uint8_t code;
    uint8_t identifier;
    uint8_t data[PS4_REPORT_BUFFER_SIZE];
};

struct PS3Command
{
    int player;
    uint8_t lintensity;
    uint8_t lduration;
    uint8_t rintensity;
    uint8_t rduration;
};

struct PS4Command
{
    uint8_t smallRumble;
    uint8_t largeRumble;
    uint8_t r, g, b; // RGB
     // Time to flash bright (255 = 2.5 seconds)
    uint8_t flashOn;
    // Time to flash dark (255 = 2.5 seconds)
    uint8_t flashOff;
};

//////////////////////////////////////////////////////////////////

static PSController* sController[10];

class PSController::priv
{
public:
    static PSController* getPS3(uint16_t l2cap_id)
    {
        for (int i = 0; i < sizeof(sController)/sizeof(sController[0]); i++)
        {
            PSController* ctrl = sController[i];
            if (ctrl != nullptr && (ctrl->fHIDC == l2cap_id || ctrl->fHIDI == l2cap_id))
            {
                return ctrl;
            }
        }
        return nullptr;
    }

    static void sendPS3HID(uint16_t l2cap_cid, hid_ps3cmd_t* hid_cmd, uint8_t len)
    {
        // Serial.println("sendPS3HID");
        BT_HDR* p_buf = (BT_HDR *)osi_malloc(BT_DEFAULT_BUFFER_SIZE);
        if (p_buf != nullptr)
        {
            p_buf->len = len + sizeof(*hid_cmd) - sizeof(hid_cmd->data);
            p_buf->offset = L2CAP_MIN_OFFSET;

            memcpy((uint8_t*)(p_buf + 1) + p_buf->offset, (uint8_t*)hid_cmd, p_buf->len);

            /*uint8_t result =*/ L2CA_DataWrite(l2cap_cid, p_buf);

            // if (result == L2CAP_DW_SUCCESS)
            //     Serial.println("success");

            // else if (result == L2CAP_DW_CONGESTED)
            //     Serial.println("congested");

            // else if (result == L2CAP_DW_FAILED)
            //     Serial.println("failed");
        }
    }

    static void sendPS4HID(uint16_t l2cap_cid, hid_ps4cmd_t* hid_cmd, uint8_t len)
    {
        // Serial.println("sendPS4HID");
        BT_HDR* p_buf = (BT_HDR *)osi_malloc(BT_DEFAULT_BUFFER_SIZE);
        if (p_buf != nullptr)
        {
            p_buf->len = len + sizeof(*hid_cmd) - sizeof(hid_cmd->data);
            p_buf->offset = L2CAP_MIN_OFFSET;

            memcpy((uint8_t*)(p_buf + 1) + p_buf->offset, (uint8_t*)hid_cmd, p_buf->len);

            /*uint8_t result =*/ L2CA_DataWrite(l2cap_cid, p_buf);

            // if (result == L2CAP_DW_SUCCESS)
            //     Serial.println("success");

            // else if (result == L2CAP_DW_CONGESTED)
            //     Serial.println("congested");

            // else if (result == L2CAP_DW_FAILED)
            //     Serial.println("failed");
        }
    }

    static void sendCommandPS3(uint16_t l2cap_id, PS3Command& cmd)
    {
        enum
        {
            kLED1 = 1 << 1,
            kLED2 = 1 << 2,
            kLED3 = 1 << 3,
            kLED4 = 1 << 4
        };
        bool led1 = false;
        bool led2 = false;
        bool led3 = false;
        bool led4 = false;

        // Serial.println("sendCommandPS3");
        int player = cmd.player;
        if ((led4 = player >= 4) != 0)
            player -= 4;
        if ((led3 = player >= 3) != 0)
            player -= 3;
        if ((led2 = player >= 2) != 0)
            player -= 2;
        if ((led1 = player >= 1) != 0)
            player -= 1;

        enum
        {
            kIndex_rumble_right_duration = 1,
            kIndex_rumble_right_intensity = 2,
            kIndex_rumble_left_duration = 3,
            kIndex_rumble_left_intensity = 4,

            kIndex_leds = 9,
            kIndex_led4_arguments = 10,
            kIndex_led3_arguments = 15,
            kIndex_led2_arguments = 20,
            kIndex_led1_arguments = 25
        };
        const uint8_t args[] = { 0xff, 0x27, 0x10, 0x00, 0x32 };

        hid_ps3cmd_t hid_cmd;
        uint16_t len = sizeof(hid_cmd.data);
        memset(&hid_cmd, '\0', sizeof(hid_cmd));

        hid_cmd.code = kHIDCMD_code_set_report | kHIDCMD_code_type_output;
        hid_cmd.identifier = kHIDCMD_identifier_ps3_control;

        hid_cmd.data[kIndex_rumble_right_duration]  = cmd.rduration;
        hid_cmd.data[kIndex_rumble_right_intensity] = cmd.rintensity;
        hid_cmd.data[kIndex_rumble_left_duration]   = cmd.lduration;
        hid_cmd.data[kIndex_rumble_left_intensity]  = cmd.lintensity;

        hid_cmd.data[kIndex_leds] = 0;
        if (led1)
            hid_cmd.data[kIndex_leds] |= kLED1;
        if (led2)
            hid_cmd.data[kIndex_leds] |= kLED2;
        if (led3)
            hid_cmd.data[kIndex_leds] |= kLED3;
        if (led4)
            hid_cmd.data[kIndex_leds] |= kLED4;

        if (led1)
            memcpy(&hid_cmd.data[kIndex_led1_arguments], args, sizeof(args));
        if (led2)
            memcpy(&hid_cmd.data[kIndex_led2_arguments], args, sizeof(args));
        if (led3)
            memcpy(&hid_cmd.data[kIndex_led3_arguments], args, sizeof(args));
        if (led4)
            memcpy(&hid_cmd.data[kIndex_led4_arguments], args, sizeof(args));

        sendPS3HID(l2cap_id, &hid_cmd, len);
    }

    static void sendCommandPS4(uint16_t l2cap_id, PS4Command& cmd)
    {
        // Serial.println("sendCommandPS4");
        enum
        {
            kIndex_small_rumble = 5,
            kIndex_large_rumble = 6,

            kIndex_red = 7,
            kIndex_green = 8,
            kIndex_blue = 9,

            kIndex_flash_on_time = 10,
            kIndex_flash_off_time = 11
        };

        hid_ps4cmd_t hid_cmd;
        uint16_t len = sizeof(hid_cmd.data);
        memset(&hid_cmd, '\0', sizeof(hid_cmd));
        hid_cmd.data[0] = 0x80;
        hid_cmd.data[1] = 0x00;
        hid_cmd.data[2] = 0xFF;

        hid_cmd.code = kHIDCMD_code_set_report | kHIDCMD_code_type_output;
        hid_cmd.identifier = kHIDCMD_identifier_ps4_control;

        // Serial.println(cmd.smallRumble);
        // Serial.println(cmd.largeRumble);
        // Serial.println(cmd.r);
        // Serial.println(cmd.g);
        // Serial.println(cmd.b);
        // Serial.println(cmd.flashOn);
        // Serial.println(cmd.flashOff);
        hid_cmd.data[kIndex_small_rumble] = cmd.smallRumble;
        hid_cmd.data[kIndex_large_rumble] = cmd.largeRumble;

        hid_cmd.data[kIndex_red] = cmd.r;
        hid_cmd.data[kIndex_green] = cmd.g;
        hid_cmd.data[kIndex_blue] = cmd.b;

        hid_cmd.data[kIndex_flash_on_time] = cmd.flashOn;
        hid_cmd.data[kIndex_flash_off_time] = cmd.flashOff;

        sendPS4HID(l2cap_id, &hid_cmd, len);
    }

    static void connect_ind_cback(BD_ADDR bd_addr, uint16_t l2cap_cid, uint16_t psm, uint8_t l2cap_id)
    {
        // char mac[18];
        //const uint8_t* addr = esp_bt_dev_get_address();

        bool found = false;
        // First check if the address is in the allowed list
        // Serial.print("PS BT CONNECTION FROM:");
        // for (int i = 0; i < 6; i++)
        // {
        //     if (i > 0)
        //         Serial.print(":");
        //     Serial.print(bd_addr[i],HEX);
        // }
        // Serial.println();
        // First check to see if there are any reserved slots for this controller
        for (int i = 0; i < sizeof(sController)/sizeof(sController[0]); i++)
        {
            PSController* ctrl = sController[i];
            if (ctrl != nullptr && memcmp(bd_addr, ctrl->fBDAddr, sizeof(ctrl->fBDAddr)) == 0)
            {
                if (ctrl->fHIDC == 0)
                {
                    ctrl->fHIDC = l2cap_cid;
                }
                else if (ctrl->fHIDI == 0)
                {
                    ctrl->fHIDI = l2cap_cid;
                }
                found = true;
                // Serial.println("FOUND MATCH?");
                break;
            }
        }
        // Serial.println(found);
        if (!found)
        {
            static uint8_t sZeroAddr[6];
            // See if any free slots for this controller
            for (int i = 0; i < sizeof(sController)/sizeof(sController[0]); i++)
            {
                PSController* ctrl = sController[i];
                if (ctrl != nullptr && memcmp(ctrl->fBDAddr, sZeroAddr, sizeof(sZeroAddr)) == 0)
                {
                    // Assign slot to this controller
                    memcpy(ctrl->fBDAddr, bd_addr, sizeof(ctrl->fBDAddr));
                    if (ctrl->fHIDC == 0)
                    {
                        ctrl->fHIDC = l2cap_cid;
                    }
                    else if (ctrl->fHIDI == 0)
                    {
                        ctrl->fHIDI = l2cap_cid;
                    }
                    found = true;
                    break;
                }
            }
            // if (found)
            // {
            //     Serial.println("FOUND FREE SLOT");
            // }
            // else
            // {
            //     Serial.println("NO FREE SLOT");
            // }
        }
        // Serial.println("WTF");
        if (found)
        {
            /* Send connection pending response to the L2CAP layer. */
            L2CA_CONNECT_RSP (bd_addr, l2cap_id, l2cap_cid, L2CAP_CONN_PENDING, L2CAP_CONN_PENDING, nullptr, nullptr);

            /* Send response to the L2CAP layer. */
            L2CA_CONNECT_RSP (bd_addr, l2cap_id, l2cap_cid, L2CAP_CONN_OK, L2CAP_CONN_OK, nullptr, nullptr);

            /* Send a Configuration Request. */
            static tL2CAP_CFG_INFO sConfigInfo;
            L2CA_CONFIG_REQ (l2cap_cid, &sConfigInfo);
        }
    }

    static void connect_cfm_cback(uint16_t l2cap_cid, uint16_t result)
    {
        /* Not needed */
    }

    static void config_cfm_cback(uint16_t l2cap_cid, tL2CAP_CFG_INFO *p_cfg)
    {
        // Serial.println("config_cfm_cback");
        PSController* ctrl = getPS3(l2cap_cid);

        if (ctrl != nullptr && l2cap_cid == ctrl->fHIDI)
        {
            switch (ctrl->fType)
            {
                case kPS3:
                case kPS3Nav:
                {
                    const uint8_t payload[] = { 0x42, 0x03, 0x00, 0x00 };

                    hid_ps3cmd_t hid_cmd;
                    hid_cmd.code = kHIDCMD_code_set_report | kHIDCMD_code_type_feature;
                    hid_cmd.identifier = kHIDCMD_identifier_ps3_enable;
                    memcpy(hid_cmd.data, payload, sizeof(payload));
                    sendPS3HID(ctrl->fHIDC, &hid_cmd, sizeof(payload));
                    break;
                }
                case kPS4:
                {
                    const uint8_t payload[] = { 0x43, 0x02 };

                    // Serial.println("PS4 Controller");
                    hid_ps4cmd_t hid_cmd;
                    hid_cmd.code = kHIDCMD_code_set_report | kHIDCMD_code_type_feature;
                    hid_cmd.identifier = kHIDCMD_identifier_ps4_enable;
                    memcpy(hid_cmd.data, payload, sizeof(payload));
                    sendPS4HID(ctrl->fHIDC, &hid_cmd, sizeof(payload));

                    PS4Command cmd = {};
                    cmd.r = 32;
                    cmd.g = 32;
                    cmd.b = 64;
                    sendCommandPS4(ctrl->fHIDC, cmd);
                    break;
                }
            }
            ctrl->fCongested = false;
            ctrl->fConnecting = true;
        }
    }

    static void config_ind_cback(uint16_t l2cap_cid, tL2CAP_CFG_INFO *p_cfg)
    {
        // Serial.println("config_ind_cback");
        p_cfg->result = L2CAP_CFG_OK;

        L2CA_CONFIG_RSP(l2cap_cid, p_cfg);
    }

    static void disconnect_ind_cback(uint16_t l2cap_cid, bool ack_needed)
    {
        // Serial.println("disconnect_ind_cback");
        if (ack_needed)
        {
            /* send L2CAP disconnect response */
            L2CA_DISCONNECT_RSP(l2cap_cid);
        }
        PSController* ctrl = getPS3(l2cap_cid);
        if (ctrl != nullptr)
        {
            if (l2cap_cid == ctrl->fHIDC)
                ctrl->fHIDC = 0;
            if (l2cap_cid == ctrl->fHIDI)
            {
                ctrl->fHIDI = 0;
                if (ctrl->fConnected)
                {
                    ctrl->fConnected = false;
                    ctrl->fCongested = false;
                    ctrl->onDisconnect();
                }
            }
        }
    }

    static void disconnect_cfm_cback(uint16_t l2cap_cid, uint16_t result)
    {
        // Serial.println("disconnect_cfm_cback");
        L2CA_DISCONNECT_REQ(l2cap_cid);
        PSController* ctrl = getPS3(l2cap_cid);
        if (ctrl != nullptr)
        {
            if (l2cap_cid == ctrl->fHIDC)
                ctrl->fHIDC = 0;
            if (l2cap_cid == ctrl->fHIDI)
            {
                ctrl->fHIDI = 0;
                if (ctrl->fConnected)
                {
                    ctrl->fConnected = false;
                    ctrl->fCongested = false;
                    ctrl->onDisconnect();
                }
            }
        }
    }

    static void data_ind_cback(uint16_t l2cap_cid, BT_HDR *p_buf)
    {
        // Serial.println("data_ind_cback");
        PSController* ctrl = getPS3(l2cap_cid);
        if (ctrl != nullptr)
        {
            switch (ctrl->fType)
            {
                case kPS3:
                case kPS3Nav:
                    if (p_buf->len == 50)
                    {
                        ctrl->parsePacket(p_buf->data);
                    }
                    else
                    {
                        // Serial.println("data_ind_cback len="+String(p_buf->len));
                    }
                    break;
                case kPS4:
                    if (p_buf->len == 79)
                    {
                        ctrl->parsePacket(p_buf->data);
                    }
                    else
                    {
                        // Serial.println("data_ind_cback len="+String(p_buf->len));
                    }
                    break;
            }
        }
        osi_free(p_buf);
    }

    static void congest_cback (uint16_t l2cap_cid, bool congested)
    {
        PSController* ctrl = getPS3(l2cap_cid);
        if (ctrl != nullptr)
            ctrl->fCongested = true;
    }

    static void init_service(const char *name, uint16_t psm, uint8_t security_id)
    {
        static const tL2CAP_APPL_INFO dyn_info = {
            connect_ind_cback,
            connect_cfm_cback,
            nullptr,
            config_ind_cback,
            config_cfm_cback,
            disconnect_ind_cback,
            disconnect_cfm_cback,
            nullptr,
            data_ind_cback,
            congest_cback,
            nullptr
        };
        /* Register the PSM for incoming connections */
        if (!L2CA_Register(psm, (tL2CAP_APPL_INFO *) &dyn_info))
            return;

        /* Register with the Security Manager for our specific security level (none) */
        if (!BTM_SetSecurityLevel(false, name, security_id, 0, psm, 0, 0))
            return;
    }

    static void deinit_service(const char *name, uint16_t psm)
    {
        /* Deregister the PSM from incoming connections */
        L2CA_Deregister(psm);
    }

    static void spp_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
    {
        if (event == ESP_SPP_INIT_EVT)
        {
            esp_bt_dev_set_device_name(PS3_DEVICE_NAME);

        #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 0, 0)
            esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);
        #else
            esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE);
        #endif

            esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, PS3_SERVER_NAME);
        }
    }
};

///////////////////////////////////////////////////////////////////////

void PSController::parsePacket(uint8_t* packet)
{
    Event evt = {};
    State prev = fState;
    switch (fType)
    {
    #define CHECK_FLAG(mask) ((raw & mask) != 0)
        case kPS3:
        case kPS3Nav:
        {
            enum
            {
                kIndex_buttons_raw = 12,

                kIndex_analog_stick_lx = 16,
                kIndex_analog_stick_ly = 17,
                kIndex_analog_stick_rx = 18,
                kIndex_analog_stick_ry = 19,

                kIndex_analog_button_up = 24,
                kIndex_analog_button_right = 25,
                kIndex_analog_button_down = 26,
                kIndex_analog_button_left = 27,

                kIndex_analog_button_l2 = 28,
                kIndex_analog_button_r2 = 29,
                kIndex_analog_button_l1 = 30,
                kIndex_analog_button_r1 = 31,

                kIndex_analog_button_triangle = 32,
                kIndex_analog_button_circle = 33,
                kIndex_analog_button_cross = 34,
                kIndex_analog_button_square = 35,

                kIndex_status = 39,

                kIndex_sensor_accelerometer_x = 51,
                kIndex_sensor_accelerometer_y = 53,
                kIndex_sensor_accelerometer_z = 55,
                kIndex_sensor_gyroscope_z = 57
            };
            enum
            {
                kBMask_select   = 1 << 0,
                kBMask_l3       = 1 << 1,
                kBMask_r3       = 1 << 2,
                kBMask_start    = 1 << 3,

                kBMask_up       = 1 << 4,
                kBMask_right    = 1 << 5,
                kBMask_down     = 1 << 6,
                kBMask_left     = 1 << 7,

                kBMask_l2       = 1 << 8,
                kBMask_r2       = 1 << 9,
                kBMask_l1       = 1 << 10,
                kBMask_r1       = 1 << 11,

                kBMask_triangle = 1 << 12,
                kBMask_circle   = 1 << 13,
                kBMask_cross    = 1 << 14,
                kBMask_square   = 1 << 15,

                kBMask_ps       = 1 << 16
            };

            enum
            {
                kStatusMask_rumbling = 0x02,
                kStatusMask_bluetooth = 0x04
            };

            const uint8_t kAnalogOffset = 0x80;
            const uint16_t kSensorOffset = 0x200;
            uint32_t raw = *((uint32_t*)&packet[kIndex_buttons_raw]);

            fState.button.select   = CHECK_FLAG(kBMask_select);
            fState.button.l3       = CHECK_FLAG(kBMask_l3);
            fState.button.start    = CHECK_FLAG(kBMask_start);

            fState.button.up       = CHECK_FLAG(kBMask_up);
            fState.button.right    = CHECK_FLAG(kBMask_right);
            fState.button.down     = CHECK_FLAG(kBMask_down);
            fState.button.left     = CHECK_FLAG(kBMask_left);

            fState.button.upright  = (fState.button.up && fState.button.right);
            fState.button.upleft   = (fState.button.up && fState.button.left);
            fState.button.downright= (fState.button.down && fState.button.right);
            fState.button.downleft = (fState.button.down && fState.button.left);

            fState.button.l1       = CHECK_FLAG(kBMask_l1);
            fState.button.l2       = CHECK_FLAG(kBMask_l2);

            fState.button.triangle = CHECK_FLAG(kBMask_triangle);
            fState.button.circle   = CHECK_FLAG(kBMask_circle);
            fState.button.cross    = CHECK_FLAG(kBMask_cross);
            fState.button.square   = CHECK_FLAG(kBMask_square);

            fState.button.ps       = CHECK_FLAG(kBMask_ps);

            fState.analog.stick.lx = packet[kIndex_analog_stick_lx] - kAnalogOffset;
            fState.analog.stick.ly = packet[kIndex_analog_stick_ly] - kAnalogOffset;
            fState.analog.button.l1 = packet[kIndex_analog_button_l1];
            fState.analog.button.l2 = packet[kIndex_analog_button_l2];
            if (fType == kPS3Nav)
            {
                // right-side same as left side
                fState.button.r1 = fState.button.l1;
                fState.button.r2 = fState.button.l2;
                fState.button.r3 = fState.button.l3;
                fState.analog.stick.rx = fState.analog.stick.lx;
                fState.analog.stick.ry = fState.analog.stick.ly;
                fState.analog.button.r1 = fState.analog.button.l1;
                fState.analog.button.r2 = fState.analog.button.l2;
            }
            else
            {
                fState.button.r1       = CHECK_FLAG(kBMask_r1);
                fState.button.r2       = CHECK_FLAG(kBMask_r2);
                fState.button.r3       = CHECK_FLAG(kBMask_r3);
                fState.analog.stick.rx = packet[kIndex_analog_stick_rx] - kAnalogOffset;
                fState.analog.stick.ry = packet[kIndex_analog_stick_ry] - kAnalogOffset;
                fState.analog.button.r1 = packet[kIndex_analog_button_r1];
                fState.analog.button.r2 = packet[kIndex_analog_button_r2];
            }

            fState.analog.button.up       = packet[kIndex_analog_button_up];
            fState.analog.button.right    = packet[kIndex_analog_button_right];
            fState.analog.button.down     = packet[kIndex_analog_button_down];
            fState.analog.button.left     = packet[kIndex_analog_button_left];

            fState.analog.button.triangle = packet[kIndex_analog_button_triangle];
            fState.analog.button.circle   = packet[kIndex_analog_button_circle];
            fState.analog.button.cross    = packet[kIndex_analog_button_cross];
            fState.analog.button.square   = packet[kIndex_analog_button_square];

            fState.sensor.accelerometer.x = (uint16_t(packet[kIndex_sensor_accelerometer_x]) << 8) + packet[kIndex_sensor_accelerometer_x+1] - kSensorOffset;
            fState.sensor.accelerometer.y = (uint16_t(packet[kIndex_sensor_accelerometer_y]) << 8) + packet[kIndex_sensor_accelerometer_y+1] - kSensorOffset;
            fState.sensor.accelerometer.z = (uint16_t(packet[kIndex_sensor_accelerometer_z]) << 8) + packet[kIndex_sensor_accelerometer_z+1] - kSensorOffset;
            fState.sensor.gyroscope.z     = (uint16_t(packet[kIndex_sensor_gyroscope_z])     << 8) + packet[kIndex_sensor_gyroscope_z+1]     - kSensorOffset;

            fState.status.battery    =  (BatteryStatus)packet[kIndex_status+1];
            fState.status.charging   =  (fState.status.battery == kCharging);
            fState.status.connection = (packet[kIndex_status+2] & kStatusMask_bluetooth) ? kBluetooth : kUSB;
            fState.status.rumbling   = (packet[kIndex_status+2] & kStatusMask_rumbling) ? false: true;
            break;
        }
        case kPS4:
        {
            enum
            {
                kIndex_buttons = 17,

                kIndex_analog_stick_lx = 13,
                kIndex_analog_stick_ly = 14,
                kIndex_analog_stick_rx = 15,
                kIndex_analog_stick_ry = 16,

                kIndex_analog_button_l2 = 20,
                kIndex_analog_button_r2 = 21,

                kIndex_status = 42
            };

            enum
            {
                kBMask_up       = 0,
                kBMask_right    = 1 << 1,
                kBMask_down     = 1 << 2,
                kBMask_left     = (1 << 1) + (1 << 2),

                kBMask_upright  = 1,
                kBMask_upleft   = 1 + (1 << 1) + (1 << 2),
                kBMask_downright= 1 + (1 << 1),
                kBMask_downleft = 1 + (1 << 2),

                kBMask_arrows   = 0xf,

                kBMask_square   = 1 << 4,
                kBMask_cross    = 1 << 5,
                kBMask_circle   = 1 << 6,
                kBMask_triangle = 1 << 7,

                kBMask_l1       = 1 << 8,
                kBMask_r1       = 1 << 9,
                kBMask_l2       = 1 << 10,
                kBMask_r2       = 1 << 11,

                kBMask_share    = 1 << 12,
                kBMask_options  = 1 << 13,

                kBMask_l3       = 1 << 14,
                kBMask_r3       = 1 << 15,

                kBMask_ps       = 1 << 16,
                kBMask_touchpad = 1 << 17
            };

            enum
            {
                kStatusMask_battery  = 0xf,
                kStatusMask_charging = 1 << 4,
                kStatusMask_audio    = 1 << 5,
                kStatusMask_mic      = 1 << 6
            };
            const uint8_t kAnalogOffset = 0x80;
            uint32_t raw = *((uint32_t*)&packet[kIndex_buttons]);
            uint8_t arrows_only = (raw & kBMask_arrows);

            fState.button.options  = CHECK_FLAG(kBMask_options);
            fState.button.l3       = CHECK_FLAG(kBMask_l3);
            fState.button.r3       = CHECK_FLAG(kBMask_r3);
            fState.button.share    = CHECK_FLAG(kBMask_share);

            fState.button.up       = (arrows_only == kBMask_up);
            fState.button.right    = (arrows_only == kBMask_right);
            fState.button.down     = (arrows_only == kBMask_down);
            fState.button.left     = (arrows_only == kBMask_left);

            fState.button.upright  = (arrows_only == kBMask_upright);
            fState.button.upleft   = (arrows_only == kBMask_upleft);
            fState.button.downright= (arrows_only == kBMask_downright);
            fState.button.downleft = (arrows_only == kBMask_downleft);

            fState.button.l2       = CHECK_FLAG(kBMask_l2);
            fState.button.r2       = CHECK_FLAG(kBMask_r2);
            fState.button.l1       = CHECK_FLAG(kBMask_l1);
            fState.button.r1       = CHECK_FLAG(kBMask_r1);

            fState.button.triangle = CHECK_FLAG(kBMask_triangle);
            fState.button.circle   = CHECK_FLAG(kBMask_circle);
            fState.button.cross    = CHECK_FLAG(kBMask_cross);
            fState.button.square   = CHECK_FLAG(kBMask_square);

            fState.button.ps       = CHECK_FLAG(kBMask_ps);
            fState.button.touchpad = CHECK_FLAG(kBMask_touchpad);

            fState.analog.stick.lx = packet[kIndex_analog_stick_lx] - kAnalogOffset;
            fState.analog.stick.ly = -packet[kIndex_analog_stick_ly] + kAnalogOffset - 1;
            fState.analog.stick.rx = packet[kIndex_analog_stick_rx] - kAnalogOffset;
            fState.analog.stick.ry = -packet[kIndex_analog_stick_ry] + kAnalogOffset - 1;

            fState.analog.button.l2 = packet[kIndex_analog_button_l2];
            fState.analog.button.r2 = packet[kIndex_analog_button_r2];

            fState.status.battery  = (BatteryStatus)(packet[kIndex_status] & kStatusMask_battery);
            fState.status.charging = ((packet[kIndex_status] & kStatusMask_charging) != 0);
            fState.status.audio    = ((packet[kIndex_status] & kStatusMask_audio) != 0);
            fState.status.mic      = ((packet[kIndex_status] & kStatusMask_mic) != 0);
            break;
        }
    }
    /* Button down events */
    #define CHECK_BUTTON_DOWN(b) evt.button_down.b = (!prev.button.b && fState.button.b)
    CHECK_BUTTON_DOWN(select);
    CHECK_BUTTON_DOWN(l3);
    CHECK_BUTTON_DOWN(r3);
    CHECK_BUTTON_DOWN(start);

    CHECK_BUTTON_DOWN(up);
    CHECK_BUTTON_DOWN(right);
    CHECK_BUTTON_DOWN(down);
    CHECK_BUTTON_DOWN(left);

    CHECK_BUTTON_DOWN(upright);
    CHECK_BUTTON_DOWN(upleft);
    CHECK_BUTTON_DOWN(downright);
    CHECK_BUTTON_DOWN(downleft);

    CHECK_BUTTON_DOWN(l2);
    CHECK_BUTTON_DOWN(r2);
    CHECK_BUTTON_DOWN(l1);
    CHECK_BUTTON_DOWN(r1);

    CHECK_BUTTON_DOWN(triangle);
    CHECK_BUTTON_DOWN(circle);
    CHECK_BUTTON_DOWN(cross);
    CHECK_BUTTON_DOWN(square);

    CHECK_BUTTON_DOWN(ps);
    CHECK_BUTTON_DOWN(share);
    CHECK_BUTTON_DOWN(options);
    CHECK_BUTTON_DOWN(touchpad);
    #undef CHECK_BUTTON_DOWN

    /* Button up events */
    #define CHECK_BUTTON_UP(b) evt.button_up.b = (prev.button.b && !fState.button.b)
    CHECK_BUTTON_UP(select);
    CHECK_BUTTON_UP(l3);
    CHECK_BUTTON_UP(r3);
    CHECK_BUTTON_UP(start);

    CHECK_BUTTON_UP(up);
    CHECK_BUTTON_UP(right);
    CHECK_BUTTON_UP(down);
    CHECK_BUTTON_UP(left);

    CHECK_BUTTON_UP(upright);
    CHECK_BUTTON_UP(upleft);
    CHECK_BUTTON_UP(downright);
    CHECK_BUTTON_UP(downleft);

    CHECK_BUTTON_UP(l2);
    CHECK_BUTTON_UP(r2);
    CHECK_BUTTON_UP(l1);
    CHECK_BUTTON_UP(r1);

    CHECK_BUTTON_UP(triangle);
    CHECK_BUTTON_UP(circle);
    CHECK_BUTTON_UP(cross);
    CHECK_BUTTON_UP(square);

    CHECK_BUTTON_UP(ps);
    CHECK_BUTTON_UP(share);
    CHECK_BUTTON_UP(options);
    CHECK_BUTTON_UP(touchpad);
    #undef CHECK_BUTTON_UP

    /* Analog events */
    evt.analog_changed.stick.lx        = fState.analog.stick.lx - prev.analog.stick.lx;
    evt.analog_changed.stick.ly        = fState.analog.stick.ly - prev.analog.stick.ly;
    evt.analog_changed.stick.rx        = fState.analog.stick.rx - prev.analog.stick.rx;
    evt.analog_changed.stick.ry        = fState.analog.stick.ry - prev.analog.stick.ry;

    evt.analog_changed.button.up       = fState.analog.button.up    - prev.analog.button.up;
    evt.analog_changed.button.right    = fState.analog.button.right - prev.analog.button.right;
    evt.analog_changed.button.down     = fState.analog.button.down  - prev.analog.button.down;
    evt.analog_changed.button.left     = fState.analog.button.left  - prev.analog.button.left;

    evt.analog_changed.button.l2       = fState.analog.button.l2 - prev.analog.button.l2;
    evt.analog_changed.button.r2       = fState.analog.button.r2 - prev.analog.button.r2;
    evt.analog_changed.button.l1       = fState.analog.button.l1 - prev.analog.button.l1;
    evt.analog_changed.button.r1       = fState.analog.button.r1 - prev.analog.button.r1;

    evt.analog_changed.button.triangle = fState.analog.button.triangle - prev.analog.button.triangle;
    evt.analog_changed.button.circle   = fState.analog.button.circle   - prev.analog.button.circle;
    evt.analog_changed.button.cross    = fState.analog.button.cross    - prev.analog.button.cross;
    evt.analog_changed.button.square   = fState.analog.button.square   - prev.analog.button.square;

#undef CHECK_FLAG

    if (fConnected)
    {
        state = fState;
        event = evt;
        notify();
    }
    else if (fConnecting)
    {
        fConnecting = false;
        fConnected = true;
        setPlayer(fPlayer);
        onConnect();
    }

}

//////////////////////////////////////////////////////////////////////////////////////

PSController::PSController(const char* mac, Type type) :
    fType(type),
    fHIDC(0),
    fHIDI(0)
{
    static int sPlayerCount;
    fPlayer = ++sPlayerCount;
    memset(fBDAddr, '\0', sizeof(fBDAddr));
    if (mac != nullptr)
        setMacAddress(mac);
    for (int i = 0; i < sizeof(sController)/sizeof(sController[0]); i++)
    {
        if (sController[i] == nullptr)
        {
            sController[i] = this;
            break;
        }
    }
    memset(&fState, '\0', sizeof(fState));
}

void PSController::setType(Type type)
{
    fType = type;
}

void PSController::setMacAddress(const char* mac)
{
    sscanf(mac, ESP_BD_ADDR_HEX_STR, ESP_BD_ADDR_HEX_PTR(fBDAddr));
}

bool PSController::startListening(const char* mac)
{
    if (mac != nullptr)
    {
        esp_bd_addr_t addr;

        if (sscanf(mac, ESP_BD_ADDR_HEX_STR, ESP_BD_ADDR_HEX_PTR(addr)) != ESP_BD_ADDR_LEN)
            return false;

        // The bluetooth MAC address is derived from the base MAC address
        // https://docs.espressif.com/projects/esp-idf/en/stable/api-reference/system/system.html#mac-address
        uint8_t base_mac[6];
        memcpy(base_mac, addr, 6);
        base_mac[5] -= 2;
        esp_base_mac_addr_set(base_mac);
    }
    if (!btStarted() && !btStart())
        return false;

    esp_bluedroid_status_t bt_state = esp_bluedroid_get_status();
    if (bt_state == ESP_BLUEDROID_STATUS_UNINITIALIZED)
    {
        if (esp_bluedroid_init())
            return false;
    }

    if (bt_state != ESP_BLUEDROID_STATUS_ENABLED && esp_bluedroid_enable())
        return false;

    esp_err_t ret;

#if 0//ndef ARDUINO_ARCH_ESP32
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK)
        return false;

    if ((ret = esp_bt_controller_enable(BT_MODE)) != ESP_OK)
        return false;

    if ((ret = esp_bluedroid_init()) != ESP_OK)
        return false;
    
    if ((ret = esp_bluedroid_enable()) != ESP_OK)
        return false;
#endif

    if ((ret = esp_spp_register_callback(priv::spp_callback)) != ESP_OK)
        return false;

    if ((ret = esp_spp_init(ESP_SPP_MODE_CB)) != ESP_OK)
        return false;

    priv::init_service("PS-HIDC", BT_PSM_HIDC, BTM_SEC_SERVICE_FIRST_EMPTY  );
    priv::init_service("PS-HIDI", BT_PSM_HIDI, BTM_SEC_SERVICE_FIRST_EMPTY+1);
    return true;

}

bool PSController::stopListening()
{
    priv::deinit_service("PS-HIDC", BT_PSM_HIDC);
    priv::deinit_service("PS-HIDI", BT_PSM_HIDI);

    esp_err_t ret;
    if ((ret = esp_spp_deinit()) != ESP_OK)
        return false;

#if 0//ndef ARDUINO_ARCH_ESP32
    if ((ret = esp_bluedroid_disable()) != ESP_OK)
        return false;

    if ((ret = esp_bluedroid_deinit()) != ESP_OK)
        return false;

    if ((ret = esp_bt_controller_disable()) != ESP_OK)
        return false;

    if ((ret = esp_bt_controller_deinit()) != ESP_OK)
        return false;
#endif
    return true;
}

void PSController::disconnect()
{
    if (fHIDC != 0)
    {
        priv::disconnect_cfm_cback(fHIDC, 0);
        fHIDC = 0;
    }
    if (fHIDI != 0)
    {
        priv::disconnect_cfm_cback(fHIDI, 0);
        fHIDI = 0;
    }
}

String PSController::getDeviceAddress()
{
    String address = "";

    if (btStarted()) {
        char mac[18];
        const uint8_t* addr = esp_bt_dev_get_address();

        sprintf(mac, ESP_BD_ADDR_STR, ESP_BD_ADDR_HEX_ARR(addr));

        address = String(mac);
    }

    return address;
}

void PSController::setPlayer(int player)
{
    fPlayer = player;
    if (!fHIDC)
        return;
    switch (fType)
    {
        case kPS3:
        case kPS3Nav:
        {
            PS3Command cmd = {};
            cmd.player = fPlayer;
            priv::sendCommandPS3(fHIDC, cmd);
            break;
        }
        case kPS4:
        {
            PS4Command cmd = {};
            cmd.r = 32;
            cmd.g = 32;
            cmd.b = 64;
            priv::sendCommandPS4(fHIDC, cmd);
            break;
        }
    }
}

void PSController::setLED(uint8_t r, uint8_t g, uint8_t b)
{
    if (!fHIDC)
        return;
    if (fType == kPS4)
    {
        PS4Command cmd = {};
        cmd.r = r;
        cmd.g = g;
        cmd.b = b;
        priv::sendCommandPS4(fHIDC, cmd);
    }
}

void PSController::setRumble(float leftIntensity, int leftDuration, float rightIntensity, int rightDuration)
{
    if (!fHIDC)
        return;

    switch (fType)
    {
        case kPS3:
        case kPS3Nav:
        {
            const float int_min = 0.0;
            const float int_max = 100.0;

            const int dur_min = 0;
            const int dur_max = 5000;

            PS3Command cmd = {};
            cmd.lintensity = map(constrain(leftIntensity, int_min, int_max), int_min, int_max, 0, 255);
            cmd.lduration = (leftDuration != -1) ? map(constrain(leftDuration, dur_min, dur_max), dur_min, dur_max, 0, 254) : 255;
            cmd.rintensity = map(constrain(rightIntensity, int_min, int_max), int_min, int_max, 0, 255);
            cmd.rduration = (rightDuration != -1) ? map(constrain(rightDuration, dur_min, dur_max), dur_min, dur_max, 0, 254) : 255;

            cmd.player = fPlayer;
            priv::sendCommandPS3(fHIDC, cmd);
            break;
        }
        case kPS4:
        {
            break;
        }
    }
}
#endif
