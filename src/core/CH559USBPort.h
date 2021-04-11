class CH559USBPort
{
public:
    CH559USBPort() :
        fSerial(NULL)
    {
    }

    void begin(HardwareSerial& serial, uint8_t rxPin, uint8_t txPin)
    {
        fSerial = &serial;
        fSerial->begin(1000000, SERIAL_8N1, rxPin, txPin);
    }

    void process()
    {
        while (fSerial != NULL && fSerial->available())
        {
            delay(2);
            fUARTRxBuff[fRXPos] = fSerial->read();
            // DEBUG_PRINT_HEX(fUARTRxBuff[fRXPos]);
            if (fRXPos == 0 && fUARTRxBuff[fRXPos] == 0xFE)
            {
                fCmdType = 1;
            }
            else if (fRXPos == 1 && fCmdType == 1)
            {
                fCmdLength = fUARTRxBuff[fRXPos];
            }
            else if (fRXPos == 2 && fCmdType == 1)
            {
                fCmdLength += (fUARTRxBuff[fRXPos] << 8);
                //printf( "Length: %i\n", fCmdLength);//Only for Debug
            }
            else if (fCmdType == 0 && fUARTRxBuff[fRXPos] == '\n')
            {
                printf("No COMMAND Received\n");
                for (uint8_t i = 0; i < fRXPos; i ++ )
                {
                    printf( "0x%02X ", fUARTRxBuff[i]);
                }
                printf("\n");
                fRXPos = 0;
                fCmdType = 0;
            }
            if ((fRXPos > 0 && fRXPos == fCmdLength + 11 && fCmdType) || (fRXPos > 1024))
            {
                filterCommand(fCmdLength, fUARTRxBuff);
                fRXPos = 0;
                fCmdType = 0;
            }
            else
            {
                fRXPos++;
            }
        }
        fRXPos = 0;
    }

    const uint8_t* getBTAddress()
    {
        return (fBTReceived) ? fBTAddress : NULL;
    }

private:
    enum
    {
        MSG_TYPE_CONNECTED      = 0x01,
        MSG_TYPE_DISCONNECTED   = 0x02,
        MSG_TYPE_ERROR          = 0x03,
        MSG_TYPE_DEVICE_POLL    = 0x04,
        MSG_TYPE_DEVICE_STRING  = 0x05,
        MSG_TYPE_DEVICE_INFO    = 0x06,
        MSG_TYPE_HID_INFO       = 0x07,
        MSG_TYPE_STARTUP        = 0x08,
        MSG_TYPE_BTADDRESS      = 0x09
    };

    void filterCommand(int buffLength, unsigned char *msgbuffer)
    {
        static const char* deviceType[] = {"UNKNOWN", "POINTER", "MOUSE", "RESERVED", "JOYSTICK", "GAMEPAD", "KEYBOARD", "KEYPAD", "MULTI_AXIS", "SYSTEM"};

        int cmdLength = buffLength;
        unsigned char msgType = msgbuffer[3];
        unsigned char devType = msgbuffer[4];
        unsigned char device = msgbuffer[5];
        unsigned char endpoint = msgbuffer[6];
        unsigned char idVendorL = msgbuffer[7];
        unsigned char idVendorH = msgbuffer[8];
        unsigned char idProductL = msgbuffer[9];
        unsigned char idProductH = msgbuffer[10];
        switch (msgType)
        {
            case MSG_TYPE_CONNECTED:
                Serial.print("Device Connected on port");
                Serial.println(device);
                break;
            case MSG_TYPE_DISCONNECTED:
                Serial.print("Device Disconnected on port");
                Serial.println(device);
                break;
            case MSG_TYPE_ERROR:
                Serial.print("Device Error ");
                Serial.print(device);
                Serial.print(" on port ");
                Serial.println(devType);
                break;
            case MSG_TYPE_DEVICE_POLL:
                Serial.print("Device HID Data from port: ");
                Serial.print(device);
                Serial.print(" , Length: ");
                Serial.print(cmdLength);
                Serial.print(" , Type: ");
                Serial.print (deviceType[devType]);
                Serial.print(" , ID: ");
                for (int j = 0; j < 4; j++)
                {
                    Serial.print("0x");
                    Serial.print(msgbuffer[j + 7], HEX);
                    Serial.print(" ");
                }
                Serial.print(" ,  ");
                for (int j = 0; j < cmdLength; j++)
                {
                    Serial.print("0x");
                    Serial.print(msgbuffer[j + 11], HEX);
                    Serial.print(" ");
                }
                Serial.println();
                break;
            case MSG_TYPE_DEVICE_STRING:
                Serial.print("Device String port ");
                Serial.print(devType);
                Serial.print(" Name: ");
                for (int j = 0; j < cmdLength; j++)
                    Serial.write(msgbuffer[j + 11]);
                Serial.println();
                break;
            case MSG_TYPE_DEVICE_INFO:
                Serial.print("Device info from port");
                Serial.print(device);
                Serial.print(", Descriptor: ");
                for (int j = 0; j < cmdLength; j++)
                {
                    Serial.print("0x");
                    Serial.print(msgbuffer[j + 11], HEX);
                    Serial.print(" ");
                }
                Serial.println();
                break;
            case MSG_TYPE_HID_INFO:
                Serial.print("HID info from port");
                Serial.print(device);
                Serial.print(", Descriptor: ");
                for (int j = 0; j < cmdLength; j++)
                {
                    Serial.print("0x");
                    Serial.print(msgbuffer[j + 11], HEX);
                    Serial.print(" ");
                }
                Serial.println();
                break;
            case MSG_TYPE_STARTUP:
                Serial.println("USB host ready");
                break;
            case MSG_TYPE_BTADDRESS:
                Serial.println("BT ADDRESS: ");
                for (int j = 0; j < cmdLength; j++)
                {
                    Serial.print("0x");
                    if (j < sizeof(fBTAddress))
                        fBTAddress[j] = msgbuffer[j + 11];
                    Serial.print(fBTAddress[j], HEX);
                    Serial.print(" ");
                }
                fBTReceived = true;
                break;
        }
    }

private:
    HardwareSerial* fSerial;
    uint8_t fUARTRxBuff[1024];
    unsigned fRXPos = 0;
    unsigned fCmdLength = 0;
    uint8_t fCmdType = 0;
    uint8_t fBTAddress[6];
    bool fBTReceived = false;
};
