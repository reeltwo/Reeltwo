#ifndef CH559USBPort_h
#define CH559USBPort_h

#include "ReelTwo.h"

class CH559USBPort
{
public:
    CH559USBPort(HardwareSerial& serial) :
        fSerial(&serial)
    {
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
                DEBUG_PRINTLN("No COMMAND Received");
                for (uint8_t i = 0; i < fRXPos; i ++ )
                {
                    DEBUG_PRINT_HEX(fUARTRxBuff[i]);
                    DEBUG_PRINT(" ");
                }
                DEBUG_PRINTLN();
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
                DEBUG_PRINT("Device Connected on port");
                DEBUG_PRINTLN(device);
                break;
            case MSG_TYPE_DISCONNECTED:
                DEBUG_PRINT("Device Disconnected on port");
                DEBUG_PRINTLN(device);
                break;
            case MSG_TYPE_ERROR:
                DEBUG_PRINT("Device Error ");
                DEBUG_PRINT(device);
                DEBUG_PRINT(" on port ");
                DEBUG_PRINTLN(devType);
                break;
            case MSG_TYPE_DEVICE_POLL:
                DEBUG_PRINT("Device HID Data from port: ");
                DEBUG_PRINT(device);
                DEBUG_PRINT(" , Length: ");
                DEBUG_PRINT(cmdLength);
                DEBUG_PRINT(" , Type: ");
                DEBUG_PRINT (deviceType[devType]);
                DEBUG_PRINT(" , ID: ");
                for (int j = 0; j < 4; j++)
                {
                    DEBUG_PRINT("0x");
                    DEBUG_PRINT_HEX(msgbuffer[j + 7]);
                    DEBUG_PRINT(" ");
                }
                DEBUG_PRINT(" ,  ");
                for (int j = 0; j < cmdLength; j++)
                {
                    DEBUG_PRINT("0x");
                    DEBUG_PRINT_HEX(msgbuffer[j + 11]);
                    DEBUG_PRINT(" ");
                }
                DEBUG_PRINTLN();
                break;
            case MSG_TYPE_DEVICE_STRING:
                DEBUG_PRINT("Device String port ");
                DEBUG_PRINT(devType);
                DEBUG_PRINT(" Name: ");
                for (int j = 0; j < cmdLength; j++)
                    DEBUG_PRINT((char)msgbuffer[j + 11]);
                DEBUG_PRINTLN();
                break;
            case MSG_TYPE_DEVICE_INFO:
                DEBUG_PRINT("Device info from port");
                DEBUG_PRINT(device);
                DEBUG_PRINT(", Descriptor: ");
                for (int j = 0; j < cmdLength; j++)
                {
                    DEBUG_PRINT("0x");
                    DEBUG_PRINT_HEX(msgbuffer[j + 11]);
                    DEBUG_PRINT(" ");
                }
                DEBUG_PRINTLN();
                break;
            case MSG_TYPE_HID_INFO:
                DEBUG_PRINT("HID info from port");
                DEBUG_PRINT(device);
                DEBUG_PRINT(", Descriptor: ");
                for (int j = 0; j < cmdLength; j++)
                {
                    DEBUG_PRINT("0x");
                    DEBUG_PRINT_HEX(msgbuffer[j + 11]);
                    DEBUG_PRINT(" ");
                }
                DEBUG_PRINTLN();
                break;
            case MSG_TYPE_STARTUP:
                DEBUG_PRINTLN("USB host ready");
                break;
            case MSG_TYPE_BTADDRESS:
                DEBUG_PRINTLN("BT ADDRESS: ");
                for (int j = 0; j < cmdLength; j++)
                {
                    DEBUG_PRINT("0x");
                    if (j < sizeof(fBTAddress))
                        fBTAddress[j] = msgbuffer[j + 11];
                    DEBUG_PRINT_HEX(fBTAddress[j]);
                    DEBUG_PRINT(" ");
                }
                fBTReceived = true;
                break;
        }
    }

private:
    Stream* fSerial;
    uint8_t fUARTRxBuff[1024];
    unsigned fRXPos = 0;
    unsigned fCmdLength = 0;
    uint8_t fCmdType = 0;
    uint8_t fBTAddress[6];
    bool fBTReceived = false;
};
#endif
