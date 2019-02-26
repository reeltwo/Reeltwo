// Copyright (C) 2013 Andy Kipp <kipp.andrew@gmail.com> 
// 
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef ROBOTEQCONTROLLER_H
#define ROBOTEQCONTROLLER_H

#include <Stream.h>

/**
  * \ingroup Core
  *
  * \class RoboteQController
  *
  * \brief Communicate with a Roboteq controller
  */
class RoboteQController
{
public:
    enum
    {
        kROBOTEQ_OK                  = 0,
        kROBOTEQ_TIMEOUT             = -1,
        kROBOTEQ_ERROR               = -2,
        kROBOTEQ_BAD_COMMAND         = -3,
        kROBOTEQ_BAD_RESPONSE        = -4,
        kROBOTEQ_BUFFER_OVER         = -5,

        kROBOTEQ_FAULT_OVERHEAT      = 0x01,
        kROBOTEQ_FAULT_OVERVOLTAGE   = 0x02,
        kROBOTEQ_FAULT_UNDERVOLTAGE  = 0x04,
        kROBOTEQ_FAULT_SHORT         = 0x08,
        kROBOTEQ_FAULT_ESTOP         = 0x10,
        kROBOTEQ_FAULT_SCRIPT        = 0x20,
        kROBOTEQ_FAULT_MOSFET        = 0x40,
        kROBOTEQ_FAULT_CONFIG        = 0x80,

        kROBOTEQ_STATUS_SERIAL       = 0x01,
        kROBOTEQ_STATUS_PULSE        = 0x02,
        kROBOTEQ_STATUS_ANALOG       = 0x04,
        kROBOTEQ_STATUS_POWER_OFF    = 0x08,
        kROBOTEQ_STATUS_STALL        = 0x10,
        kROBOTEQ_STATUS_LIMIT        = 0x20,
        kROBOTEQ_SCRIPT_RUN          = 0x80
    };

    /**
      * \brief Constructor
      */
    RoboteQController(Stream *serial) :
        fSerial(serial),
        fTimeout()
    {
    }

    /**
      * Check if controller is connected
      *
      * \returns kROBOTEQ_OK if connected
      */
    int isConnected(void)
    {
        if (fSerial == NULL) 
            return kROBOTEQ_ERROR;

        uint8_t inByte = 0;
        uint32_t startTime = millis();

        // send QRY
        fSerial->write(kROBOTEQ_QUERY_CHAR);
        fSerial->flush();

        while (millis() - startTime < (uint32_t)fTimeout)
        {
            // wait for ACK
            if (fSerial->available() > 0)
            {
                inByte = m_Serial->read();
                if (inByte == kROBOTEQ_ACK_CHAR)
                    return kROBOTEQ_OK;
            }
        }
        return kROBOTEQ_TIMEOUT;
    }

    /**
      * Send motor power command (!G)
      *
      * \param ch channel
      * \param p power level (-1000, 1000)
      * \returns kROBOTEQ_OK if successful 
      */
    int commandMotorPower(uint8_t ch, int16_t p)
    {
        char command[kROBOTEQ_COMMAND_BUFFER_SIZE];
        sprintf(command, "!G %02d %d\r", ch, p);
        return sendCommand(command);
    }

    /**
      * Send emergency stop command (!EX)
      * note: you have to reset the controller after this sending command
      *
      * \returns kROBOTEQ_OK if successful
      */
    int commandEmergencyStop(void)
    {
        char command[kROBOTEQ_COMMAND_BUFFER_SIZE];
        sprintf(command, "!EX\r");
        return sendCommand(command);
    }

    /**
      * Query controller firmware
      *
      * \returns kROBOTEQ_OK if successful
      */
    int queryFirmware(char *buf, size_t bufSize)
    {
        // Query: ?FID
        // Response: FID=<firmware>
        memset(buf, NULL, bufSize);
        return sendQuery("?FID\r", (uint8_t*)buf, 100);
    }

    /**
      * Query battery amps
      * 
      * \returns battery amps * 10
      */
    int queryBatteryAmps(void)
    {
        // Query: ?BA 
        // Response: BA=<ch1*10>:<ch2*10>
        int ch1, ch2;
        char buffer[kROBOTEQ_BUFFER_SIZE];
        int res;

        // Send Query
        if ((res = sendQuery("?BA\r", (uint8_t*)buffer, kROBOTEQ_BUFFER_SIZE)) < 0) 
            return res;
        if (res < 4)
            return kROBOTEQ_BAD_RESPONSE;

        // Parse Response
        if (sscanf((char*)buffer, "BA=%i:%i", &ch1, &ch2) < 2) {
            return kROBOTEQ_BAD_RESPONSE;
        }

        // Return total amps (ch1 + ch2)
        return ch1+ch2;
    }

    /**
      * Query battery amps
      * 
      * \param ch channel
      * \returns battery amps * 10
      */
    int queryBatteryAmps(uint8_t ch);
    {
        // Query: ?BA [ch]
        // Response: BA=<ch*10>
        int amps;
        char command[kROBOTEQ_COMMAND_BUFFER_SIZE];
        char buffer[kROBOTEQ_BUFFER_SIZE];
        int res;

        // Build Query Command
        sprintf(command, "?BA %i\r", ch);

        // Send Query
        if ((res = this->sendQuery(command, (uint8_t*)buffer, kROBOTEQ_BUFFER_SIZE)) < 0) 
            return res;
        if (res < 4)
            return kROBOTEQ_BAD_RESPONSE;

        // Parse Response
        if (sscanf((char*)buffer, "BA=%i", &amps) < 1) {
            return kROBOTEQ_BAD_RESPONSE;
        }

        return amps;
    }

    /**
      * Query battery voltage
      * 
      * \returns battery voltage * 10
      */ 
    int queryBatteryVoltage(void)
    {
        // Query: ?V 2 (2 = main battery voltage)
        // Response: V=<voltage>*10
        uint8_t buffer[kROBOTEQ_BUFFER_SIZE];
        int voltage = -1; 
        memset(buffer, NULL, kROBOTEQ_BUFFER_SIZE);
        int res;
        if ((res = this->sendQuery("?V 2\r", (uint8_t*)buffer, kROBOTEQ_BUFFER_SIZE)) < 0) 
            return res;
        if (res < 4)
            return kROBOTEQ_BAD_RESPONSE;
        // Parse Response
        if (sscanf((char*)buffer, "V=%i", &voltage) != 1)
            return kROBOTEQ_BAD_RESPONSE;
        return voltage; 
    }

    /**
      * Query motor voltage
      * 
      * \returns motor voltage * 10
      */
    int queryMotorVoltage(void)
    {
        // Query: ?V 1 (1 = main motor voltage)
        // Response: V=<voltage>*10
        uint8_t buffer[kROBOTEQ_BUFFER_SIZE];
        int voltage = -1; 
        memset(buffer, NULL, kROBOTEQ_BUFFER_SIZE);
        int res;
        if ((res = this->sendQuery("?V 1\r", (uint8_t*)buffer, kROBOTEQ_BUFFER_SIZE)) < 0) 
            return res;
        if (res < 4)
            return kROBOTEQ_BAD_RESPONSE;
        // Parse Response
        if (sscanf((char*)buffer, "V=%i", &voltage) != 1)
            return kROBOTEQ_BAD_RESPONSE;
        return voltage; 
    }

    /**
      * Query the motor power command
      * 
      * \param ch channel
      * \returns motor power
      */
    int queryMotorPower(uint8_t ch)
    {
        // Query: ?M [ch]
        // Response: M=<motor power>
        int p;
        char command[kROBOTEQ_COMMAND_BUFFER_SIZE];
        char buffer[kROBOTEQ_BUFFER_SIZE];
        int res;

        // Build Query Command
        sprintf(command, "?M %i\r", ch);

        // Send Query
        if ((res = this->sendQuery("?BA\r", (uint8_t*)buffer, kROBOTEQ_BUFFER_SIZE)) < 0) 
            return res;
        if (res < 4)
            return kROBOTEQ_BAD_RESPONSE;

        // Parse Response
        if (sscanf((char*)buffer, "M=%i", &p) < 1) {
            return kROBOTEQ_BAD_RESPONSE;
        }
        return p; 

    }

    /**
      * Query fault flags
      */
    int queryFaultFlag(void)
    {
        // Query: ?FF
        // Response: FF=<status>
        int fault = -1;
        uint8_t buffer[kROBOTEQ_BUFFER_SIZE];
        memset(buffer, NULL, kROBOTEQ_BUFFER_SIZE);
        int res = 0;
        if ((res = this->sendQuery("?FF\r", (uint8_t*)buffer, kROBOTEQ_BUFFER_SIZE)) < 0)
            return res;
        if (res < 4) 
            return kROBOTEQ_BAD_RESPONSE;
        // Parse Response
        if (sscanf((char*)buffer, "FF=%i", &fault) < 1)
            return kROBOTEQ_BAD_RESPONSE;
        return fault;
    }

    /**
      * Query status flags
      */
    int queryStatusFlag(void)
    {
        // Query: ?FS
        // Response: FS=<status>
        uint8_t buffer[kROBOTEQ_BUFFER_SIZE];
        int status = -1;
        memset(buffer, NULL, kROBOTEQ_BUFFER_SIZE);
        int res;
        if ((res = this->sendQuery("?FS\r", (uint8_t*)buffer, kROBOTEQ_BUFFER_SIZE)) < 0) 
            return res;
        if (res < 4)
            return kROBOTEQ_BAD_RESPONSE;
        // Parse Response
        if (sscanf((char*)buffer, "FS=%i", &status) < 1)
            return kROBOTEQ_BAD_RESPONSE;
        return status;  
    }

    /**
      * Query encoder speed in RPM
      *
      * \param ch channel
      * \returns rpm
      */
    int queryEncoderSpeed(uint8_t ch)
    {
        // Query: ?S [ch]
        // Response: S=[speed]
        int speed;
        char command[kROBOTEQ_COMMAND_BUFFER_SIZE];
        char buffer[kROBOTEQ_BUFFER_SIZE];
        int res;

        // Build Query Command
        sprintf(command, "?S %i\r", ch);

        // Send Query
        if ((res = this->sendQuery(command, (uint8_t*)buffer, kROBOTEQ_BUFFER_SIZE)) < 0)
            return res;
        if (res < 3)
            return kROBOTEQ_BAD_RESPONSE;

        // Parse Response
        if (sscanf((char*)buffer, "S=%i", &speed) < 1) {
            return kROBOTEQ_BAD_RESPONSE;
        }
        return speed;
    }

    /**
      * Set encoder pulse per rotation
      *
      * \param ch channel
      * \param ppr pulese per rotation
      *
      * \returns kROBOTEQ_OK if successful
      */
    int setEncoderPulsePerRotation(uint8_t ch, uint16_t ppr)
    {
        char command[kROBOTEQ_COMMAND_BUFFER_SIZE];
        sprintf(command, "^EPPR %02d %d\r", ch, ppr);
        return sendCommand(command);
    }

    /**
      * Set motor amp limit
      *
      * \param ch channel
      * \param a amps level (x10)
      *
      * \returns kROBOTEQ_OK if successful
      */
    int setMotorAmpLimit(uint8_t ch, uint16_t a)
    {
        char command[kROBOTEQ_COMMAND_BUFFER_SIZE];
        sprintf(command, "^ALIM %i %i", ch, a);
        return sendCommand(command);
    }

    /**
      * Load controller configuration
      *
      * \returns kROBOTEQ_OK if successful
      */
    int loadConfiguration(void)
    {
        char command[kROBOTEQ_COMMAND_BUFFER_SIZE];
        sprintf(command, "%%EELD\r");
        return sendCommand(command);
    }

    /**
      * Save controller configuration
      *
      * \returns kROBOTEQ_OK if successful
      */
    int saveConfiguration(void)
    {
        char command[kROBOTEQ_COMMAND_BUFFER_SIZE];
        sprintf(command, "%%EESAV\r");
        return this->sendCommand(command);
    }

    /**
      * Set timeout
      */
    inline void setTimeout(uint16_t timeout)
    {
        fTimeout = timeout;
    }

private:
    enum
    {
        kROBOTEQ_DEFAULT_TIMEOUT     = 1000,
        kROBOTEQ_BUFFER_SIZE         = 64,
        kROBOTEQ_COMMAND_BUFFER_SIZE = 20,

        kROBOTEQ_QUERY_CHAR          = 0x05,
        kROBOTEQ_ACK_CHAR            = 0x06
    };

    inline int sendQuery(const char *query, uint8_t *buf, size_t bufSize)
    {
        return sendQuery(query, strlen(query), buf, bufSize);            
    }

    int sendQuery(const char *query, size_t querySize, uint8_t *buf, size_t bufSize)
    {
        if (fSerial == NULL) 
            return kROBOTEQ_ERROR;

        // Write Query to Serial
        fSerial->write((uint8_t*)query, querySize);
        fSerial->flush();

        // Read Serial until timeout or newline
        int res = readResponse(buf, bufSize);
        return res;
    }

    inline int sendCommand(const char *command)
    {
        return sendCommand(command, strlen(command));            
    }

    int sendCommand(const char *command, size_t commandSize)
    {
        if (fSerial == NULL) 
            return kROBOTEQ_ERROR;

        // Write Command to Serial
        fSerial->write((uint8_t*)command, commandSize);
        fSerial->flush();

        uint8_t buffer[kROBOTEQ_BUFFER_SIZE];
        int res = 0;

        // Read Serial until timeout or newline
        if ((res = this->readResponse((uint8_t*)buffer, kROBOTEQ_BUFFER_SIZE)) < 0)
            return res;

        if (res < 1)
            return kROBOTEQ_BAD_RESPONSE; 

        // Check Command Status
        if (strncmp((char*)buffer, "+", 1) == 0)
            return kROBOTEQ_OK;
        return kROBOTEQ_BAD_COMMAND;
    }

    int readResponse(uint8_t *buf, size_t bufSize)
    {
        uint8_t inByte;
        size_t index = 0;
        uint32_t startTime = millis();  
        while (millis() - startTime < fTimeout)
        {
            if (m_Serial->available() > 0)
            {
                inByte = m_Serial->read();
                buf[index++] = inByte;
                if (index > bufferSize)
                {
                    // out of buffer space
                    return kROBOTEQ_BUFFER_OVER;
                }
                if (inByte == 0x0D)
                    return index;
            }
        }
        return kROBOTEQ_TIMEOUT;
    }

    Stream*  fSerial;
    uint16_t fTimeout;

    static char* chomp(char* s)
    {
        int end = strlen(s) - 1;
        if (end >= 0 && (s[end] == '\n' || s[end] == '\r'))
            s[end] = '\0';
        return s;
    }
};

#endif
