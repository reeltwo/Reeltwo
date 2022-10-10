#ifndef REELTWOSMQ_H
#define REELTWOSMQ_H
#define USE_SMQ

#ifdef ReelTwo_h
#error ReelTwoSMQ.h must be included before ReelTwo.h
#endif

#include "ReelTwo.h"

#ifdef USE_SMQDEBUG
#define SMQ_DEBUG_PRINTLN(s) DEBUG_PRINTLN(s)
#define SMQ_DEBUG_PRINT(s) DEBUG_PRINT(s)
#define SMQ_DEBUG_PRINTLN_HEX(s) DEBUG_PRINTLN_HEX(s)
#define SMQ_DEBUG_PRINT_HEX(s) DEBUG_PRINT_HEX(s)
#define SMQ_DEBUG_FLUSH(s) DEBUG_FLUSH()
#else
#define SMQ_DEBUG_PRINTLN(s) 
#define SMQ_DEBUG_PRINT(s) 
#define SMQ_DEBUG_PRINTLN_HEX(s) 
#define SMQ_DEBUG_PRINT_HEX(s) 
#define SMQ_DEBUG_FLUSH(s) 
#endif

#define REELTWO_READY() \
{ \
    if (!sSMQREADY) \
    { \
        _REELTWO_READY_; \
        SMQ_SERIAL.begin(DEFAULT_BAUD_RATE); \
        sSMQ = &SMQ_SERIAL; \
        SMQ_DEBUG_PRINTLN("Waiting for agent"); \
        SMQ::ready(); \
    } \
}

static Stream *sSMQ;
static bool sSMQREADY;

typedef uint16_t msg_id;
//typedef uint32_t smq_id;
typedef uint16_t smq_id;

/**
  * \ingroup Core
  *
  * \class SMQ
  *
  * \brief Serial Message Queue
  *
  * Serial Message Queue implements a buffer-free CRC checked binary serial protocol to connect to a
  * host computer that will transmit publish/subscribe requests via ZeroMQ.
  */
class SMQ
{
public:
    static void ready()
    {
        while (!sSMQ->available())
             ;
        for (;;)
        {
            int val = read_int8();
            if (val == 'A')
            {
                sSMQREADY = true;
                break;
            }
            DEBUG_PRINTLN((char)val);
        }
        Message::sendSubscriberList();
    }

    static void process()
    {
        while (sSMQ->available())
        {
            int op = sSMQ->read();
            if (op == 'D')
            {
                char ack = 'R';
                sSMQ->write(&ack, sizeof(ack));
                sSMQ->flush();
                DEBUG_PRINTLN("Subscriber::process");
                Message::process();
                break;
            }
            else if (op == 'A')
            {
                /* acknowledge */
                sSMQREADY = true;
            }
            else if (op != -1)
            {
                //Serial.print("<== ");
                //Serial.println(op, HEX);
            }
            else
            {
                //Serial.print(".");
                break;
            }
        }        
    }

    static void send_string(const char* str)
    {
        uint8_t delim = 0x00;
        send_raw_bytes(&delim, sizeof(delim));

        size_t len = strlen(str);
        send_data(&len, sizeof(len));
        send_data(str, len);
    }

    static void send_string(PROGMEMString str)
    {
        uint8_t delim = 0x00;
        send_raw_bytes(&delim, sizeof(delim));

        size_t len = strlen_P((const char*)str);
        send_data(&len, sizeof(len));
        send_data(str, len);
    }

    static void send_string_id(const msg_id id)
    {
        uint8_t delim = 0x01;
        send_raw_bytes(&delim, sizeof(delim));
        send_raw_bytes(&id, sizeof(id));
    }

    static void send_string_hash(const char* str)
    {
        uint8_t delim = 0x01;
        send_raw_bytes(&delim, sizeof(delim));

        uint16_t crc = 0xFFFF;
        size_t len = strlen(str);
        const uint8_t* b = (uint8_t*)str;
        const uint8_t* buf_end = (uint8_t*)str + len;
        while (b < buf_end)
        {
            crc = update_crc(crc, *b++);
        }
        crc = ~crc;
        send_raw_bytes(&crc, sizeof(crc));
    }

    static bool send_ready()
    {
        return sSMQREADY;
    }

    static bool sendTopic(const smq_id id)
    {
        send_start(id);
        return true;
    }

    static bool sendTopic(PROGMEMString str)
    {
        send_start(str);
        return true;
    }

    static bool sendTopic(const char* str)
    {
        send_start(str);
        return true;
    }

    static void send_start(const smq_id id)
    {
        sSMQREADY = false;
        send_string_id(id);
    }

    static void send_start(PROGMEMString str)
    {
        sSMQREADY = false;
        send_string(str);
    }

    static void send_start(const char* str)
    {
        sSMQREADY = false;
        send_string(str);
    }

    static void send_start_hash(const char* str)
    {
        send_string_hash(str);
    }

    static void send_string(const msg_id id, const char* val)
    {
        send_string_id(id);
        send_string(val);
    }

    static void send_string(PROGMEMString key, const char* val)
    {
        send_string(key);
        send_string(val);
    }

    static void send_string(PROGMEMString key, PROGMEMString val)
    {
        send_string(key);
        send_string(val);
    }

    static void send_string(const char* key, const char* val)
    {
        send_string(key);
        send_string(val);
    }

    static void send_int8(const msg_id id, int8_t val)
    {
        uint8_t delim = 0x02;
        send_string_id(id);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_int8(PROGMEMString key, int8_t val)
    {
        uint8_t delim = 0x02;
        send_string(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_int8(const char* key, int8_t val)
    {
        uint8_t delim = 0x02;
        send_string(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_int16(const msg_id id, int16_t val)
    {
        uint8_t delim = 0x03;
        send_string_id(id);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_int16(PROGMEMString key, int16_t val)
    {
        uint8_t delim = 0x03;
        send_string(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_int16(const char* key, int16_t val)
    {
        uint8_t delim = 0x03;
        send_string(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_int32(const msg_id id, int32_t val)
    {
        uint8_t delim = 0x04;
        send_string_id(id);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_int32(const char* key, int32_t val)
    {
        uint8_t delim = 0x04;
        send_string(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_int32(PROGMEMString key, int32_t val)
    {
        uint8_t delim = 0x04;
        send_string(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_uint8(const msg_id id, uint8_t val)
    {
        uint8_t delim = 0x05;
        send_string_id(id);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_uint8(PROGMEMString key, uint8_t val)
    {
        uint8_t delim = 0x05;
        send_string(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_uint8(const char* key, uint8_t val)
    {
        uint8_t delim = 0x05;
        send_string(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_uint16(const msg_id id, uint16_t val)
    {
        uint8_t delim = 0x06;
        send_string_id(id);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_uint16(PROGMEMString key, uint16_t val)
    {
        uint8_t delim = 0x06;
        send_string(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_uint16(const char* key, uint16_t val)
    {
        uint8_t delim = 0x06;
        send_string(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_uint32(const msg_id id, uint32_t val)
    {
        uint8_t delim = 0x07;
        send_string_id(id);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_uint32(PROGMEMString key, uint32_t val)
    {
        uint8_t delim = 0x07;
        send_string(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_uint32(const char* key, uint32_t val)
    {
        uint8_t delim = 0x07;
        send_string(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_float(const msg_id id, float val)
    {
        uint8_t delim = 0x08;
        send_string_id(id);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_float(PROGMEMString key, float val)
    {
        uint8_t delim = 0x08;
        send_string(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_float(const char* key, float val)
    {
        uint8_t delim = 0x08;
        send_string(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_double(const msg_id id, double val)
    {
        uint8_t delim = 0x09;
        send_string_id(id);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_double(PROGMEMString key, double val)
    {
        uint8_t delim = 0x09;
        send_string(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_double(const char* key, double val)
    {
        uint8_t delim = 0x09;
        send_string(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_boolean(const msg_id id, bool val)
    {
        uint8_t delim = (val) ? 0x0A : 0x0B;
        send_string_id(id);
        send_raw_bytes(&delim, sizeof(delim));
    }

    static void send_boolean(PROGMEMString key, bool val)
    {
        uint8_t delim = (val) ? 0x0A : 0x0B;
        send_string(key);
        send_raw_bytes(&delim, sizeof(delim));
    }

    static void send_boolean(const char* key, bool val)
    {
        uint8_t delim = (val) ? 0x0A : 0x0B;
        send_string(key);
        send_raw_bytes(&delim, sizeof(delim));
    }

    static void send_null(const msg_id id)
    {
        uint8_t delim = 0x0C;
        send_string_id(id);
        send_raw_bytes(&delim, sizeof(delim));
    }

    static void send_null(PROGMEMString key)
    {
        uint8_t delim = 0x0C;
        send_string(key);
        send_raw_bytes(&delim, sizeof(delim));
    }

    static void send_null(const char* key)
    {
        uint8_t delim = 0x0C;
        send_string(key);
        send_raw_bytes(&delim, sizeof(delim));
    }

    // static void send_buffer(const void* buf, size_t len)
    // {
    //     uint8_t delim = 0x0D;
    //     send_string(key);
    //     send_raw_bytes(&delim, sizeof(delim));
    //     send_data(&len, sizeof(len));
    //     send_data(buf, len);
    // }

    static void send_end()
    {
        uint8_t delim = 0xFF;
        send_raw_bytes(&delim, sizeof(delim));
    }

    class Message;
    typedef void (*MessageHandler)(Message& msg);

    /**
      * \struct Message
      *
      * \brief Encapsulate an SMQ topic message provided by the MCU
      *
      * \code
      *     SMQMESSAGE("HoloMovie", {
      *         char movieName[13];
      *         frontHolo.play(msg.get_string(MSGID("name"), movieName, sizeof(movieName)));
      *     }),
      *     SMQMESSAGE("FLD", {
      *         FLD.selectEffect(msg.get_integer(MSGID("state")));
      *     }),
      *     SMQMESSAGE("RLD", {
      *         RLD.selectEffect(msg.get_integer(MSGID("state")));
      *     }),
      *     SMQMESSAGE("LD", {
      *         // Multicast message
      *         long state = msg.get_integer(MSGID("state"));
      *         FLD.selectEffect(state);
      *         RLD.selectEffect(state);
      *     }),
      *     SMQMESSAGE("Holo", {
      *         // Multicast message
      *         char cmdBuffer[13];
      *         const char* cmd = msg.get_string(MSGID("cmd"), cmdBuffer, sizeof(cmdBuffer));
      *         frontHolo.handleCommand(cmd);
      *         rearHolo.handleCommand(cmd);
      *         topHolo.handleCommand(cmd);
      *         radarEye.handleCommand(cmd);
      *     }),
      *     SMQMESSAGE("JAWA", {
      *         char* cmdBuffer = jawaCommander.getBuffer();
      *         size_t cmdBufferSize = jawaCommander.getBufferSize();
      *         const char* cmd = msg.get_string(MSGID("cmd"), cmdBuffer, cmdBufferSize);
      *         jawaCommander.process('\r');
      *     }),
      *     SMQMESSAGE("Periscope", {
      *         // Periscope message
      *         char cmdBuffer[13];
      *         int cmd = msg.get_integer(MSGID("cmd"));
      *         switch (cmd)
      *         {
      *             case WSID16("up"):
      *                 periscope.up();
      *                 break;
      *             case WSID16("down"):
      *                 periscope.down();
      *                 break;
      *             case WSID16("searchcw"):
      *                 periscope.searchLightCW();
      *                 break;
      *             case WSID16("searchccw"):
      *                 periscope.searchLightCCW();
      *                 break;
      *             case WSID16("randomfast"):
      *                 periscope.randomFast();
      *                 break;
      *             case WSID16("randomslow"):
      *                 periscope.randomSlow();
      *                 break;
      *             case WSID16("faceforward"):
      *                 periscope.faceForward();
      *                 break;
      *         }
      *     }),
      *     SMQMESSAGE("ServoDispatch", {
      *         byte num = msg.get_integer(MSGID("num"));
      *         if (num < servoDispatch.getNumServos())
      *         {
      *             int32_t curPos = servoDispatch.currentPos(num);
      *             uint32_t startDelay = msg.get_integer(MSGID("startDelay"));
      *             uint32_t moveTime = msg.get_integer(MSGID("moveTime"));
      *             int32_t startPos = msg.get_integer(MSGID("startPos"));
      *             int32_t endPos = msg.get_integer(MSGID("endPos"));
      *             int32_t relPos = msg.get_integer(MSGID("relPos"));
      *             if (startPos == -1)
      *                 startPos = curPos;
      *             if (relPos > 0)
      *                 endPos = curPos + relPos;
      *             servoDispatch.moveTo(num, startDelay, moveTime, startPos, endPos);
      *         }
      *     });
      * \endcode
      */

    /// \private
    class Message
    {
    public:
        Message(smq_id topic, MessageHandler handler) :
            fTopic(topic),
            fHandler(handler)
        {
            fNext = *tail();
            *tail() = this;
        }

        long get_integer(const msg_id keyID)
        {
            if (find_key(keyID))
            {
                return get_integer_worker();
            }
            fMType = -1;
            return 0;
        }

        long get_integer(const char* key)
        {
            if (key == NULL || find_key(key))
            {
                return get_integer_worker();
            }
            fMType = -1;
            return 0;
        }

        long get_integer(PROGMEMString key)
        {
            if (key == NULL || find_key(key))
            {
                return get_integer_worker();
            }
            fMType = -1;
            return 0;
        }

        double get_double(const msg_id keyID)
        {
            if (find_key(keyID))
            {
                return get_double_worker();
            }
            fMType = -1;
            return 0;
        }

        double get_double(const char* key)
        {
            if (key == NULL || find_key(key))
            {
                return get_double_worker();
            }
            fMType = -1;
            return 0;
        }

        double get_double(PROGMEMString key)
        {
            if (key == NULL || find_key(key))
            {
                return get_double_worker();
            }
            fMType = -1;
            return 0;
        }

        const char* get_string(const msg_id keyID, char* buffer, size_t maxlen)
        {
            if (find_key(keyID))
            {
                return get_string_worker(buffer, maxlen);
            }
            buffer[0] = '\0';
            fMType = -1;
            return NULL;
        }

        const char* get_string(const char* key, char* buffer, size_t maxlen)
        {
            if (key == NULL || find_key(key))
            {
                return get_string_worker(buffer, maxlen);
            }
            buffer[0] = '\0';
            fMType = -1;
            return NULL;

        }

        const char* get_string(PROGMEMString key, char* buffer, size_t maxlen)
        {
            if (key == NULL || find_key(key))
            {
                return get_string_worker(buffer, maxlen);
            }
            buffer[0] = '\0';
            fMType = -1;
            return NULL;
        }

        bool get_boolean(const msg_id key)
        {
            return (bool)get_integer(key);
        }
        bool get_boolean(const char* key)
        {
            return (bool)get_integer(key);
        }
        bool get_boolean(PROGMEMString key)
        {
            return (bool)get_integer(key);
        }
        int8_t get_int8(const msg_id key)
        {
            return (int8_t)get_integer(key);
        }
        int8_t get_int8(const char* key)
        {
            return (int8_t)get_integer(key);
        }
        int8_t get_int8(PROGMEMString key)
        {
            return (int8_t)get_integer(key);
        }
        int16_t get_int16(const msg_id key)
        {
            return (int16_t)get_integer(key);
        }
        int16_t get_int16(const char* key)
        {
            return (int16_t)get_integer(key);
        }
        int16_t get_int16(PROGMEMString key)
        {
            return (int16_t)get_integer(key);
        }
        int32_t get_int32(const msg_id key)
        {
            return (int16_t)get_integer(key);
        }
        int32_t get_int32(const char* key)
        {
            return (int16_t)get_integer(key);
        }
        int32_t get_int32(PROGMEMString key)
        {
            return (int16_t)get_integer(key);
        }
        uint8_t get_uint8(const msg_id key)
        {
            return (uint8_t)get_integer(key);
        }
        uint8_t get_uint8(const char* key)
        {
            return (uint8_t)get_integer(key);
        }
        uint8_t get_uint8(PROGMEMString key)
        {
            return (uint8_t)get_integer(key);
        }
        uint16_t get_uint16(const uint16_t key)
        {
            return (uint16_t)get_integer(key);
        }
        uint16_t get_uint16(const char* key)
        {
            return (uint16_t)get_integer(key);
        }
        uint16_t get_uint16(PROGMEMString key)
        {
            return (uint16_t)get_integer(key);
        }
        uint32_t get_uint32(const msg_id key)
        {
            return (uint32_t)get_integer(key);
        }
        uint32_t get_uint32(const char* key)
        {
            return (uint32_t)get_integer(key);
        }
        uint32_t get_uint32(PROGMEMString key)
        {
            return (uint32_t)get_integer(key);
        }
        float get_float(const msg_id key)
        {
            return (float)get_double(key);
        }
        float get_float(const char* key)
        {
            return (float)get_double(key);
        }
        float get_float(PROGMEMString key)
        {
            return (float)get_double(key);
        }

        void end()
        {
            while (!fEOM)
            {
                find_key(static_cast<const char*>(NULL));
            }
        }

    private:
        bool find_key(const msg_id keyID)
        {
            while (!fEOM)
            {
                bool match = false;
                uint8_t t = read_uint8();
                switch (t)
                {
                    case 0x00:
                    {
                        // String
                        DEBUG_PRINTLN("[KEYSTRING]");
                        uint16_t lencrc = read_uint16();
                        uint16_t len = read_uint16();
                        // TODO verify lencrc
                        UNUSED_ARG(lencrc);

                        uint16_t crc = read_uint16();
                        uint16_t recrc = 0;
                        while (len-- > 0)
                        {
                            int8_t ch = read_int8();
                            recrc = update_crc(recrc, ch);
                        }
                        if (recrc == crc)
                        {
                            match = (crc == keyID);
                        }
                        else
                        {
                            DEBUG_PRINT("crc=");
                            DEBUG_PRINT_HEX(crc);
                            DEBUG_PRINT(" expected=");
                            DEBUG_PRINT_HEX(recrc);
                            DEBUG_PRINTLN("CRC BAD4");
                            DEBUG_FLUSH();
                        }
                        break;
                    }
                    case 0x01:
                    {
                        // Hash
                        DEBUG_PRINTLN("[KEYCRC]");
                        uint16_t crc = read_uint16();
                        match = (keyID == crc);
                        break;
                    }
                    case 0xFF:
                        // EOM
                        fEOM = true;
                        return false;
                    default:
                        DEBUG_PRINTLN("BAD MSG");
                        return false;
                }
                fMType = read_uint8();
                if (match)
                    return true;

                // skip value
                get_integer_worker();
            }
            return false;
        }

        bool find_key(const char* key)
        {
            unsigned keylen = (key != NULL) ? strlen(key) : 0;
            while (!fEOM)
            {
                bool match = false;
                uint8_t t = read_uint8();
                switch (t)
                {
                    case 0x00:
                    {
                        // String
                        DEBUG_PRINTLN("[KEYSTRING]");
                        uint16_t lencrc = read_uint16();
                        uint16_t len = read_uint16();
                        // TODO verify lencrc
                        UNUSED_ARG(lencrc);

                        uint16_t crc = read_uint16();
                        uint16_t recrc = 0;
                        const char* k = key;
                        match = (keylen == len);
                        while (len-- > 0)
                        {
                            int8_t ch = read_int8();
                            DEBUG_PRINT((char)ch);
                            DEBUG_PRINT(" - ");
                            if (k != NULL)
                            {
                                DEBUG_PRINTLN((char)*k);
                            }
                            recrc = update_crc(recrc, ch);
                            if (match && *k++ != ch)
                                match = false;
                        }
                        DEBUG_PRINT("pmatch=");
                        DEBUG_PRINTLN(match);
                        if (recrc != crc)
                        {
                            DEBUG_PRINT("crc=");
                            DEBUG_PRINT_HEX(crc);
                            DEBUG_PRINT(" expected=");
                            DEBUG_PRINT_HEX(recrc);
                            DEBUG_PRINTLN("CRC BAD4");
                            DEBUG_FLUSH();
                        }
                        break;
                    }
                    case 0x01:
                    {
                        // Hash
                        DEBUG_PRINTLN("[KEYCRC]");
                        uint16_t crc = read_uint16();
                        uint16_t keycrc = 0xFFFF;
                        const uint8_t* b = (uint8_t*)key;
                        const uint8_t* buf_end = (uint8_t*)key + keylen;
                        while (b < buf_end)
                        {
                            keycrc = update_crc(keycrc, *b++);
                        }
                        match = (keylen != 0 && keycrc == crc);
                        break;
                    }
                    case 0xFF:
                        // EOM
                        fEOM = true;
                        return false;
                    default:
                        DEBUG_PRINTLN("BAD MSG");
                        return false;
                }
                fMType = read_uint8();
                if (match)
                    return true;

                // skip value
                get_integer_worker();
            }
            return false;
        }

        bool find_key(PROGMEMString keyP)
        {
            const char* key = (const char*)keyP;
            unsigned keylen = (key != NULL) ? strlen_P(key) : 0;
            while (!fEOM)
            {
                bool match = false;
                uint8_t t = read_uint8();
                switch (t)
                {
                    case 0x00:
                    {
                        // String
                        DEBUG_PRINTLN("[KEYSTRING]");
                        uint16_t lencrc = read_uint16();
                        uint16_t len = read_uint16();
                        // TODO verify lencrc
                        UNUSED_ARG(lencrc);

                        uint16_t crc = read_uint16();
                        uint16_t recrc = 0;
                        const char* k = key;
                        match = (keylen == len);
                        while (len-- > 0)
                        {
                            int8_t ch = read_int8();
                            DEBUG_PRINT((char)ch);
                            DEBUG_PRINT(" - ");
                            if (k != NULL)
                            {
                                DEBUG_PRINTLN((char)*k);
                            }
                            recrc = update_crc(recrc, ch);
                            if (match && pgm_read_byte(k++) != ch)
                                match = false;
                        }
                        DEBUG_PRINT("[match=");
                        DEBUG_PRINTLN(match);
                        if (recrc != crc)
                        {
                            DEBUG_PRINT("crc=");
                            DEBUG_PRINT_HEX(crc);
                            DEBUG_PRINT(" expected=");
                            DEBUG_PRINT_HEX(recrc);
                            DEBUG_PRINTLN("CRC BAD4");
                            DEBUG_FLUSH();
                        }
                        break;
                    }
                    case 0x01:
                    {
                        // Hash
                        DEBUG_PRINTLN("[KEYCRC]");
                        uint16_t crc = read_uint16();
                        uint16_t keycrc = 0xFFFF;
                        const uint8_t* b = (uint8_t*)key;
                        const uint8_t* buf_end = (uint8_t*)key + keylen;
                        while (b < buf_end)
                        {
                            keycrc = update_crc(keycrc, pgm_read_byte(b++));
                        }
                        match = (keylen != 0 && keycrc == crc);
                        break;
                    }
                    case 0xFF:
                        // EOM
                        fEOM = true;
                        return false;
                    default:
                        DEBUG_PRINTLN("BAD MSG");
                        return false;
                }
                fMType = read_uint8();
                if (match)
                    return true;

                // skip value
                get_integer_worker();
            }
            return false;
        }

        long get_integer_worker()
        {
            long ret = 0;
            switch (fMType)
            {
                case 0x00:
                {
                    // Skip String
                    uint16_t lencrc = read_uint16();
                    uint16_t len = read_uint16();
                    // TODO verify lencrc
                    UNUSED_ARG(lencrc);

                    uint16_t crc = read_uint16();
                    uint16_t recrc = 0;
                    while (len-- > 0)
                    {
                        uint8_t ch = read_uint8();
                        recrc = update_crc(recrc, ch);
                    }
                    if (recrc != crc)
                    {
                        DEBUG_PRINTLN("CRC BAD1");
                    }
                    break;
                }
                case 0x01:
                {
                    // Skip Hash
                    uint16_t crc = read_uint16();
                    UNUSED_ARG(crc);
                    break;
                }
                case 0x02:
                {
                    // int8
                    uint16_t crc = read_uint16();
                    int8_t val = read_int8();
                    uint16_t recrc = 0;
                    for (unsigned i = 0; i < sizeof(val); i++)
                    {
                        recrc = update_crc(recrc, ((uint8_t*)&val)[i]);
                    }
                    if (recrc == crc)
                        ret = val;
                    break;
                }
                case 0x03:
                {
                    // int16
                    uint16_t crc = read_uint16();
                    int16_t val = read_int16();
                    uint16_t recrc = 0;
                    for (unsigned i = 0; i < sizeof(val); i++)
                    {
                        recrc = update_crc(recrc, ((uint8_t*)&val)[i]);
                    }
                    if (recrc == crc)
                        ret = val;
                    break;
                }
                case 0x04:
                {
                    // int32
                    uint16_t crc = read_uint16();
                    int32_t val = read_int32();
                    uint16_t recrc = 0;
                    for (unsigned i = 0; i < sizeof(val); i++)
                    {
                        recrc = update_crc(recrc, ((uint8_t*)&val)[i]);
                    }
                    if (recrc == crc)
                        ret = val;
                    break;
                }
                case 0x05:
                {
                    // uint8
                    uint16_t crc = read_uint16();
                    uint8_t val = read_uint8();
                    uint16_t recrc = 0;
                    for (unsigned i = 0; i < sizeof(val); i++)
                    {
                        recrc = update_crc(recrc, ((uint8_t*)&val)[i]);
                    }
                    if (recrc == crc)
                        ret = val;
                    break;
                }
                case 0x06:
                {
                    // uint16
                    uint16_t crc = read_uint16();
                    uint16_t val = read_uint16();
                    uint16_t recrc = 0;
                    for (unsigned i = 0; i < sizeof(val); i++)
                    {
                        recrc = update_crc(recrc, ((uint8_t*)&val)[i]);
                    }
                    if (recrc == crc)
                        ret = val;
                    break;
                }
                case 0x07:
                {
                    // uint32
                    uint16_t crc = read_uint16();
                    uint32_t val = read_uint32();
                    uint16_t recrc = 0;
                    for (unsigned i = 0; i < sizeof(val); i++)
                    {
                        recrc = update_crc(recrc, ((uint8_t*)&val)[i]);
                    }
                    if (recrc == crc)
                        ret = val;
                    break;
                }
                case 0x08:
                {
                    // uint32
                    uint16_t crc = read_uint16();
                    float val = read_float();
                    uint16_t recrc = 0;
                    for (unsigned i = 0; i < sizeof(val); i++)
                    {
                        recrc = update_crc(recrc, ((uint8_t*)&val)[i]);
                    }
                    if (recrc == crc)
                        ret = (long)val;
                    break;
                }
                case 0x09:
                {
                    // double
                    uint16_t crc = read_uint16();
                    double val = read_double();
                    uint16_t recrc = 0;
                    for (unsigned i = 0; i < sizeof(val); i++)
                    {
                        recrc = update_crc(recrc, ((uint8_t*)&val)[i]);
                    }
                    if (recrc == crc)
                        ret = (long)val;
                    break;
                }
                case 0x0A:
                    // true
                    ret = true;
                    break;
                case 0x0B:
                    // false
                    ret = false;
                    break;
                case 0x0C:
                    // null
                    ret = 0;
                    break;
                case 0x0D:
                    // buffer
                    break;
                case 0xFF:
                    // EOM
                    fEOM = true;
                    break;
            }
            return ret;
        }

        double get_double_worker()
        {
            double ret = 0;
            switch (fMType)
            {
                case 0x00:
                {
                    // Skip String
                    uint16_t lencrc = read_uint16();
                    uint16_t len = read_uint16();
                    // TODO verify lencrc
                    UNUSED_ARG(lencrc);

                    uint16_t crc = read_uint16();
                    uint16_t recrc = 0;
                    while (len-- > 0)
                    {
                        uint8_t ch = read_uint8();
                        recrc = update_crc(recrc, ch);
                    }
                    if (recrc != crc)
                    {
                        DEBUG_PRINTLN("CRC BAD2");
                    }
                    break;
                }
                case 0x01:
                {
                    // Skip Hash
                    uint16_t crc = read_uint16();
                    UNUSED_ARG(crc)
                    break;
                }
                case 0x02:
                {
                    // int8
                    uint16_t crc = read_uint16();
                    int8_t val = read_int8();
                    uint16_t recrc = 0;
                    for (unsigned i = 0; i < sizeof(val); i++)
                    {
                        recrc = update_crc(recrc, ((uint8_t*)&val)[i]);
                    }
                    if (recrc == crc)
                        ret = val;
                    break;
                }
                case 0x03:
                {
                    // int16
                    uint16_t crc = read_uint16();
                    int16_t val = read_int16();
                    uint16_t recrc = 0;
                    for (unsigned i = 0; i < sizeof(val); i++)
                    {
                        recrc = update_crc(recrc, ((uint8_t*)&val)[i]);
                    }
                    if (recrc == crc)
                        ret = val;
                    break;
                }
                case 0x04:
                {
                    // int32
                    uint16_t crc = read_uint16();
                    int32_t val = read_int32();
                    uint16_t recrc = 0;
                    for (unsigned i = 0; i < sizeof(val); i++)
                    {
                        recrc = update_crc(recrc, ((uint8_t*)&val)[i]);
                    }
                    if (recrc == crc)
                        ret = val;
                    break;
                }
                case 0x05:
                {
                    // uint8
                    uint16_t crc = read_uint16();
                    uint8_t val = read_uint8();
                    uint16_t recrc = 0;
                    for (unsigned i = 0; i < sizeof(val); i++)
                    {
                        recrc = update_crc(recrc, ((uint8_t*)&val)[i]);
                    }
                    if (recrc == crc)
                        ret = val;
                    break;
                }
                case 0x06:
                {
                    // uint16
                    uint16_t crc = read_uint16();
                    uint16_t val = read_uint16();
                    uint16_t recrc = 0;
                    for (unsigned i = 0; i < sizeof(val); i++)
                    {
                        recrc = update_crc(recrc, ((uint8_t*)&val)[i]);
                    }
                    if (recrc == crc)
                        ret = val;
                    break;
                }
                case 0x07:
                {
                    // uint32
                    uint16_t crc = read_uint16();
                    uint32_t val = read_uint32();
                    uint16_t recrc = 0;
                    for (unsigned i = 0; i < sizeof(val); i++)
                    {
                        recrc = update_crc(recrc, ((uint8_t*)&val)[i]);
                    }
                    if (recrc == crc)
                        ret = val;
                    break;
                }
                case 0x08:
                {
                    // uint32
                    uint16_t crc = read_uint16();
                    float val = read_float();
                    uint16_t recrc = 0;
                    for (unsigned i = 0; i < sizeof(val); i++)
                    {
                        recrc = update_crc(recrc, ((uint8_t*)&val)[i]);
                    }
                    if (recrc == crc)
                        ret = val;
                    break;
                }
                case 0x09:
                {
                    // double
                    uint16_t crc = read_uint16();
                    double val = read_double();
                    uint16_t recrc = 0;
                    for (unsigned i = 0; i < sizeof(val); i++)
                    {
                        recrc = update_crc(recrc, ((uint8_t*)&val)[i]);
                    }
                    if (recrc == crc)
                        ret = val;
                    break;
                }
                case 0x0A:
                    // true
                    ret = true;
                    break;
                case 0x0B:
                    // false
                    ret = false;
                    break;
                case 0x0C:
                    // null
                    ret = 0;
                    break;
                case 0x0D:
                    // buffer
                    break;
                case 0xFF:
                    // EOM
                    fEOM = true;
                    break;
            }
            return ret;
        }

        const char* get_string_worker(char* buffer, size_t maxlen)
        {
            const char* ret = NULL;
            switch (fMType)
            {
                case 0x00:
                {
                    // Skip String
                    uint16_t lencrc = read_uint16();
                    uint16_t len = read_uint16();
                    // TODO verify lencrc
                    UNUSED_ARG(lencrc);

                    uint16_t crc = read_uint16();
                    uint16_t recrc = 0;
                    char* b = buffer;

                    // DEBUG_PRINT("crc : "); DEBUG_PRINTLN_HEX(crc);
                    // DEBUG_PRINT("len : "); DEBUG_PRINTLN(len);
                    while (len-- > 0)
                    {
                        uint8_t ch = read_uint8();
                        if (maxlen > 0)
                        {
                            *b = ch;
                            if (maxlen - 1 > 0)
                                b++;
                            *b = '\0';
                            // DEBUG_PRINT("str : "); DEBUG_PRINTLN(buffer);
                            maxlen--;
                        }
                        recrc = update_crc(recrc, ch);
                    }
                    // DEBUG_PRINT("recrc : "); DEBUG_PRINTLN_HEX(recrc);
                    if (recrc == crc)
                    {
                        ret = buffer;
                    }
                    else
                    {
                        buffer[0] = '\0';
                        DEBUG_PRINTLN("CRC BAD3");
                    }
                    break;
                }
                default:
                {
                    // skip value
                    get_integer_worker();
                    break;
                }
            }
            return ret;
        }

        static void process()
        {
            smq_id recvTopicID = (sizeof(smq_id) == sizeof(uint32_t)) ? read_uint32() : read_uint16();
            DEBUG_PRINT("PROCESS: "); DEBUG_PRINTLN_HEX(recvTopicID);
            for (Message* msg = *tail(); msg != NULL; msg = msg->fNext)
            {
                // smq_id topicID = (sizeof(smq_id) == sizeof(uint32_t)) ?
                //     pgm_read_dword(&msg->fTopic) : pgm_read_word(&msg->fTopic);
                smq_id topicID = msg->fTopic;
                if (recvTopicID == topicID)
                {
                    msg->fMType = -1;
                    msg->fEOM = false;
                    // DEBUG_PRINTLN("CALLING MSG");
                    // DEBUG_PRINT("fHandler=");
                    // DEBUG_PRINTLN_HEX((size_t)topics->fHandler);
                    msg->fHandler(*msg);
                    msg->end();
                    break;
                }
            }
        }

        static void sendSubscriberList()
        {
            unsigned count = 0;
            for (Message* msg = *tail(); msg != NULL; msg = msg->fNext)
                count++;
            if (count != 0)
            {
                send_string_hash("subscribers");
                DEBUG_PRINTLN("subscribers = "); DEBUG_PRINTLN(count);
                send_raw_bytes(&count, sizeof(count));
                for (Message* msg = *tail(); msg != NULL; msg = msg->fNext)
                {
                    // smq_id topicID = (sizeof(smq_id) == sizeof(uint32_t)) ?
                    //     pgm_read_dword(&msg->fTopic) : pgm_read_word(&msg->fTopic);
                    smq_id topicID = msg->fTopic;
                    DEBUG_PRINT("  ["); DEBUG_PRINTLN_HEX(topicID);
                    send_raw_bytes(&topicID, sizeof(topicID));
                }
                send_end();
                sSMQ->flush();
            }
        }

        smq_id fTopic;
        MessageHandler fHandler;
        Message* fNext;
        bool fEOM = true;
        int fMType = -1;

        static Message** tail()
        {
            static Message* sTail;
            return &sTail;
        }
        friend class SMQ;
    };

private:
    static inline uint16_t update_crc(uint16_t crc, const uint8_t data)
    {
        // crc-16 poly 0x8005 (x^16 + x^15 + x^2 + 1)
        static const PROGMEM uint16_t crc16_table[256] =
        {
            0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241, 0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
            0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40, 0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
            0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40, 0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
            0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641, 0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
            0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240, 0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
            0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41, 0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
            0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41, 0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
            0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640, 0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
            0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240, 0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
            0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41, 0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
            0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41, 0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
            0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640, 0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
            0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241, 0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
            0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40, 0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
            0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40, 0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
            0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641, 0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
        };
        return (crc >> 8) ^ pgm_read_word_near(&crc16_table[(crc ^ data) & 0xFF]);
    }

    static uint16_t calc_crc(const void* buf, size_t len, uint16_t crc)
    {
        const uint8_t* b = (uint8_t*)buf;
        const uint8_t* buf_end = (uint8_t*)buf + len;
        while (b < buf_end)
        {
            crc = update_crc(crc, *b++);
        }
        return crc;
    }

    static void send_raw_bytes(const void* buf, size_t len)
    {
        sSMQ->write((uint8_t*)buf, len);
        sSMQ->flush();
    }

    static void send_data(const void* buf, uint16_t len)
    {
        uint16_t crc = 0;
        const uint8_t* b = (uint8_t*)buf;
        const uint8_t* buf_end = (uint8_t*)buf + len;
        while (b < buf_end)
        {
            crc = update_crc(crc, *b++);
        }
        send_raw_bytes(&crc, sizeof(crc));
        send_raw_bytes(buf, len);
    }

    static void send_data(PROGMEMString pbuf, uint16_t len)
    {
        uint16_t crc = 0;
        const uint8_t* pb = (uint8_t*)pbuf;
        const uint8_t* pbuf_end = (uint8_t*)pbuf + len;
        while (pb < pbuf_end)
        {
            crc = update_crc(crc, pgm_read_byte(pb++));
        }
        send_raw_bytes(&crc, sizeof(crc));
        pb = (uint8_t*)pbuf;
        pbuf_end = (uint8_t*)pbuf + len;
        while (pb < pbuf_end)
        {
            byte b = pgm_read_byte(pb++);
            sSMQ->write((uint8_t*)&b, 1);
        }
        sSMQ->flush();
    }

    static int read(void* buf, size_t len)
    {
        char* b = (char*)buf;
        char* b_end = b + len;
        while (b < b_end)
        {
            while (!sSMQ->available())
                ;
            int v = sSMQ->read();
            if (v != -1)
                *b++ = v;
        }
        int cnt = b - (char*)buf;
        if (cnt > 0)
        {
            DEBUG_PRINT(cnt); DEBUG_PRINT("<== [0x");
            for (int i = 0; i < cnt; i++)
            {
                DEBUG_PRINT_HEX(((uint8_t*)buf)[i]);
                DEBUG_PRINT(" ");
            }
            DEBUG_PRINTLN("]");
            DEBUG_FLUSH();
        }
        return cnt;
    }

    static int8_t read_int8()
    {
        int8_t val;
        read(&val, sizeof(val));
        return val;
    }

    static int16_t read_int16()
    {
        int16_t val;
        read(&val, sizeof(val));
        return val;
    }

    static int32_t read_int32()
    {
        int32_t val;
        read(&val, sizeof(val));
        return val;
    }

    static uint8_t read_uint8()
    {
        uint8_t val;
        read(&val, sizeof(val));
        return val;
    }

    static uint16_t read_uint16()
    {
        uint16_t val;
        read(&val, sizeof(val));
        return val;
    }

    static uint32_t read_uint32()
    {
        uint32_t val;
        read(&val, sizeof(val));
        return val;
    }

    static float read_float()
    {
        float val;
        read(&val, sizeof(val));
        return val;
    }

    static double read_double()
    {
        double val;
        read(&val, sizeof(val));
        return val;
    }

    static uint8_t* read_buffer(uint8_t* buf, uint16_t len, uint16_t bufsize)
    {
        uint8_t* p = buf;
        uint8_t* pend = p + min(len, bufsize);
        while (p < pend)
        {
            while (!sSMQ->available())
                ;
            *p++ = sSMQ->read();
        }
        if (len > bufsize)
        {
            /* Skip data that is bigger than our buffer */
            len = len - bufsize;
            while (len--)
                read_uint8();
            /* message was truncated and is probably not valid */
            return NULL;
        }
        return buf;
    }
};

typedef void (*SMQMessageHandler)(class SMQ::Message& msg);

#define SMQMSG_FUNC_DECL(topic) \
  static void SMQHandler_##topic(SMQ::Message& msg)
#define SMQMESSAGE(topic, handler) \
  SMQMSG_FUNC_DECL(topic); \
  SMQ::Message SMQMSG_##topic(SMQID(#topic), SMQHandler_##topic); \
  SMQMSG_FUNC_DECL(topic) { UNUSED_ARG(msg) handler }

#endif
