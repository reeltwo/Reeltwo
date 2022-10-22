#ifndef REELTWOSMQ32_H
#define REELTWOSMQ32_H
#define USE_SMQ
#define USE_SMQ32

#ifndef SMQ_HOSTNAME
#define SMQ_HOSTNAME "Generic"
#endif

#ifdef ReelTwo_h
#error ReelTwoSMQ32.h must be included before ReelTwo.h
#endif

#include "ReelTwo.h"
#include <esp_now.h>
#include <WiFi.h>

#ifdef USE_SMQDEBUG
#define SMQ_DEBUG_PRINTLN(s) DEBUG_PRINTLN(s)
#define SMQ_DEBUG_PRINT(s) DEBUG_PRINT(s)
#define SMQ_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__)
#define SMQ_DEBUG_PRINTLN_HEX(s) DEBUG_PRINTLN_HEX(s)
#define SMQ_DEBUG_PRINT_HEX(s) DEBUG_PRINT_HEX(s)
#define SMQ_DEBUG_FLUSH(s) DEBUG_FLUSH()
#else
#define SMQ_DEBUG_PRINTLN(s) 
#define SMQ_DEBUG_PRINT(s) 
#define SMQ_DEBUG_PRINTF(...)
#define SMQ_DEBUG_PRINTLN_HEX(s) 
#define SMQ_DEBUG_PRINT_HEX(s) 
#define SMQ_DEBUG_FLUSH(s) 
#endif

#define SMQ_ADDR_HEX_STR        "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx"
#define SMQ_ADDR_HEX_ARR(addr)   addr[0],  addr[1],  addr[2],  addr[3],  addr[4],  addr[5]
#define SMQ_ADDR_HEX_PTR(addr)  &addr[0], &addr[1], &addr[2], &addr[3], &addr[4], &addr[5]

#define SMQ_MAX_KEYLEN ESP_NOW_KEY_LEN

#define SMQ_BEACON_BROADCAST_INTERVAL 1000
#define SMQ_HOST_LOST_TIMEOUT 10000

#define SMQ_MAX_PAIRED_HOSTS 20
#define SMQ_MINIMUM_KEY_LEN 8

// Pairing times out after 2 minutes
#define SMQ_PAIRING_TIMEOUT 2*60*1000

// static const char* PMK_KEY_STR = "PLEASE_CHANGE_ME";

typedef uint16_t msg_id;
typedef uint32_t smq_id;

#define WIFI_CHANNEL 1
#define QUEUE_SIZE 10
#define MAX_MSG_SIZE 250
#define SMQ_MAX_HOST_NAME 13

struct SMQAddress
{
    uint8_t fData[6];

    bool equals(uint8_t addr[6])
    {
        return (memcmp(addr, fData, sizeof(fData)) == 0);
    }

    bool equals(SMQAddress& addr)
    {
        return equals(addr.fData);
    }

    String toString()
    {
        char macaddr[6*3+1];
        snprintf(macaddr, sizeof(macaddr), "%02X:%02X:%02X:%02X:%02X:%02X",
            fData[0], fData[1], fData[2], fData[3], fData[4], fData[5]);
        return macaddr;
    }
};

struct SMQLMK
{
    uint8_t fData[16];

    bool equals(uint8_t addr[16])
    {
        return (memcmp(addr, fData, sizeof(fData)) == 0);
    }

    bool equals(SMQLMK& addr)
    {
        return equals(addr.fData);
    }

    String toString()
    {
        char str[16*3+1];
        uint8_t* p = fData;
        snprintf(str, sizeof(str), "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
            p[0], p[1], p[2], p[3], p[4], p[5],
            p[6], p[7], p[8], p[9], p[10], p[11],
            p[12], p[13], p[14], p[15]);
        return str;
    }
};

struct SMQAddressKey
{
    SMQAddress fAddr;
    SMQLMK fLMK;
};

struct SMQHost
{
    char fName[SMQ_MAX_HOST_NAME];
    uint8_t fCount;
    SMQAddress fAddr;
    SMQLMK fLMK;
    uint32_t fLastSeen;
    bool fPaired;
    SMQHost* fNext;
    SMQHost* fPrev;
    smq_id fTopics[];

    String getHostName()
    {
        return fName;
    }

    String getHostAddress()
    {
        char macaddr[6*3+1];
        uint8_t* p = fAddr.fData;
        snprintf(macaddr, sizeof(macaddr), "%02X:%02X:%02X:%02X:%02X:%02X",
            p[0], p[1], p[2], p[3], p[4], p[5]);
        return macaddr;
    }

    String getHostKey()
    {
        return fLMK.toString();
    }

    SMQAddress* getAddress(SMQAddress* addr)
    {
        memcpy(addr, &fAddr, sizeof(*addr));
        return addr;
    }

    bool hasTopic(smq_id topicID)
    {
        for (unsigned i = 0; i < fCount; i++)
        {
            if (fTopics[i] == topicID)
                return true;
        }
        return false;
    }

    bool hasTopic(const char* topic)
    {
        return hasTopic(WSID32(topic));
    }
};

struct SMQRecvMsg
{
    uint8_t fAddr[6];
    uint8_t fSize;
    uint8_t fData[MAX_MSG_SIZE];
};
static bool sSMQInited = false;
static SMQAddress sSMQFromAddr;
static unsigned sSMQPairedHostsCount;
static SMQAddressKey sSMQPairedHosts[SMQ_MAX_PAIRED_HOSTS];
static char sSMQPairingMode = false;
static uint32_t sSMQPairingTimeOut = 0;
static bool sClearToSend = false;
static uint8_t sSendBuffer[MAX_MSG_SIZE];
static uint8_t* sSendPtr = sSendBuffer;
static SMQLMK sSMQLMK;
static uint8_t sBroadcastMAC[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static QueueHandle_t sRecvQueue;
static uint8_t* sReadPtr = nullptr;
static int sReadLen = 0;
static char sHostName[SMQ_MAX_HOST_NAME];
static SMQHost* sHostHead;
static SMQHost* sHostTail;
static uint32_t sKeyHash;
static uint8_t* sSendAddr = nullptr;
static void (*sPairingEvent)(SMQHost* host);
static void (*sDiscoverEvent)(SMQHost* host);
static void (*sLostEvent)(SMQHost* host);
static smq_id sSMQBEACON_ID = STRID("BEACON");
static smq_id sSMQPAIRING_ID = STRID("PAIRING");

#define REELTWO_READY() _REELTWO_READY_

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
    static void startPairing()
    {
        if (addBroadcastPeer())
        {
            sSMQPairingMode = true;
            sSMQPairingTimeOut = millis() + SMQ_PAIRING_TIMEOUT;
        }
    }

    static bool isPairing()
    {
        return (sSMQPairingMode == true);
    }

    static void stopPairing()
    {
        if (sSMQPairingMode)
        {
            sSMQPairingMode++;
            sSMQPairingTimeOut = millis() + 2000;
        }
    }

    static int masterKeyExchange(SMQLMK* remoteKey)
    {
        static SMQLMK zeroAddress;
        SMQLMK lmk;
        getLocalMasterKey(&lmk);
        if (lmk.equals(zeroAddress) == remoteKey->equals(zeroAddress))
        {
            if (lmk.equals(zeroAddress))
            {
                // Error condition neither device has a master key
                printf("No master key. One device must have a master key to pair.\n");
                return -1;
            }
            else if (!lmk.equals(*remoteKey))
            {
                // Error condition two different master keys cannot pair
                printf("Two different master keys. Reset one device to factory defaults and try again.\n");
                return -1;
            }
            else
            {
                // One of the devices has a master key continue
            }
        }
        if (!lmk.equals(zeroAddress))
        {
            printf("WE HAVE A MASTER KEY\n");
            // We have a master key
            *remoteKey = lmk;
            return 0;
        }
        // Remote device has master key
        printf("REMOTE HAS MASTER KEY\n");
        lmk = *remoteKey;
        setLocalMasterKey(&lmk);
        return 1;
    }

    static void addPairedHosts(unsigned numHosts, SMQAddressKey* hosts)
    {
        numHosts = min(numHosts, unsigned(SMQ_MAX_PAIRED_HOSTS));
        sSMQPairedHostsCount = numHosts;
        memcpy(sSMQPairedHosts, hosts, numHosts * sizeof(sSMQPairedHosts[0]));
        if (!addPairedPeers())
        {
            SMQ_DEBUG_PRINTLN("Failed to add paired peers");
        }
        SMQ_DEBUG_PRINTLN("Paired Hosts:");
        for (int i = 0; i < sSMQPairedHostsCount; i++)
        {
            SMQ_DEBUG_PRINT(sSMQPairedHosts[i].fAddr.toString());
            SMQ_DEBUG_PRINT(" - ");
            SMQ_DEBUG_PRINTLN(sSMQPairedHosts[i].fLMK.toString());
        }
    }

    static bool addPairedHost(SMQAddress* addr, SMQLMK* lmk = nullptr)
    {
        // Make sure host doesn't exist already
        for (unsigned i = 0; i < sSMQPairedHostsCount; i++)
        {
            if (memcmp(&sSMQPairedHosts[i].fAddr, addr, sizeof(*addr)) == 0)
                return false;
        }
        if (sSMQPairedHostsCount+1 < SMQ_MAX_PAIRED_HOSTS)
        {
            SMQAddressKey host;
            host.fAddr = *addr;
            host.fLMK = *lmk;
            sSMQPairedHosts[sSMQPairedHostsCount++] = host;
            return true;
        }
        return false;
    }

    static unsigned getPairedHostCount()
    {
        return sSMQPairedHostsCount;
    }

    static unsigned getPairedHosts(SMQAddressKey* hosts, unsigned maxCount)
    {
        maxCount = min(maxCount, sSMQPairedHostsCount);
        memcpy(hosts, sSMQPairedHosts, maxCount * sizeof(sSMQPairedHosts[0]));
        return maxCount;
    }

    static void setHostPairingCallback(void (*callback)(SMQHost* host))
    {
        sPairingEvent = callback;
    }

    static void setHostDiscoveryCallback(void (*callback)(SMQHost* host))
    {
        sDiscoverEvent = callback;
    }

    static void setHostLostCallback(void (*callback)(SMQHost* host))
    {
        sLostEvent = callback;
    }

    static bool clearToSend()
    {
        return sClearToSend;
    }

    static String getAddress()
    {
        return WiFi.macAddress();
    }

    static bool init(String hostName, String key)
    {
        return init(hostName.c_str(), key.c_str());
    }

    static bool init(const char* hostName = nullptr, const char* key = nullptr)
    {
        // Ensure minimum key length
        if (key != nullptr && strlen(key) < SMQ_MINIMUM_KEY_LEN)
        {
            SMQ_DEBUG_PRINTLN("Key too short");
            return false;
        }
        if (WiFi.getMode() != WIFI_STA)
        {
            printf("CHANGING TO WIFI_STA MODE\n");
            // Puts ESP in STATION MODE
            WiFi.mode(WIFI_STA);
        }
        if (WiFi.getMode() != WIFI_STA)
        {
            SMQ_DEBUG_PRINTLN("WiFi must be in station mode");
            return false;
        }
        if (esp_now_init() != ESP_OK)
        {
            SMQ_DEBUG_PRINTLN("Failed to initalize SMQ32");
            return false;
        }
        memset(sHostName, '\0', sizeof(sHostName));
        snprintf(sHostName, sizeof(sHostName)-1, "%s", (hostName != nullptr) ? hostName : SMQ_HOSTNAME);
        sKeyHash = (key != nullptr && *key != '\0') ? WSID32(key) : 0;
        sRecvQueue = xQueueCreate(QUEUE_SIZE, sizeof(SMQRecvMsg));
        union
        {
            SMQLMK key;
            uint32_t crc[4];
        } master;
        master.crc[0] = WSID32(key);
        master.crc[1] = WSID32(key+1);
        master.crc[2] = WSID32(key+2);
        master.crc[3] = WSID32(key+3);
        printf("MASTER: %s\n", master.key.toString().c_str());
        esp_now_set_pmk(master.key.fData);

        // Set up callback
        esp_err_t status = esp_now_register_recv_cb(msg_recv_cb);
        if (ESP_OK != status)
        {
            SMQ_DEBUG_PRINTLN("Could not register callback");
            // handle_error(status);
            return false;
        }

        status = esp_now_register_send_cb(msg_send_cb);
        if (ESP_OK != status)
        {
            SMQ_DEBUG_PRINTLN("Could not register send callback");
            // handle_error(status);
            return false;
        }
        sClearToSend = true;
        sSMQInited = true;
        return true;
    }

    static bool addBroadcastPeer()
    {
        // add broadcast peer if not already added
        if (!esp_now_is_peer_exist(sBroadcastMAC))
        {
            esp_now_peer_info_t peer_info;
            peer_info.channel = WIFI_CHANNEL;
            memcpy(peer_info.peer_addr, sBroadcastMAC, sizeof(sBroadcastMAC));
            peer_info.ifidx = WIFI_IF_STA;
            peer_info.encrypt = false;
            esp_err_t status = esp_now_add_peer(&peer_info);
            if (ESP_OK != status)
            {
                SMQ_DEBUG_PRINTLN("Could not add peer");
                // handle_error(status);
                return false;
            }
        }
        return true;
    }

    static void createLocalMasterKey(SMQLMK* key)
    {
        esp_fill_random(key->fData, sizeof(key->fData));
    }

    static void setLocalMasterKey(SMQLMK* key)
    {
        sSMQLMK = *key;
    }

    static void getLocalMasterKey(SMQLMK* key)
    {
        *key = sSMQLMK;
    }

    static void removeBroadcastPeer()
    {
        // Delete broadcast peer if it exists ignore if not
        esp_now_del_peer(sBroadcastMAC);
    }

    static void process()
    {
        static uint32_t sLastBeacon;
        SMQRecvMsg msg;
        if (!sSMQInited)
            return;
        while (xQueueReceive(sRecvQueue, &msg, 0))
        {
            sReadPtr = msg.fData;
            sReadLen = msg.fSize;
            // printf("sReadLen: %d *sReadPtr=0x%02X\n", sReadLen, *sReadPtr);
            if (sReadLen > 1 && *sReadPtr++ == 0x0E)
            {
                sReadLen--;
                // printf("FROM : %02X:%02X:%02X:%02X:%02X:%02X\n",
                //     msg.fAddr[0], msg.fAddr[1], msg.fAddr[2],
                //     msg.fAddr[3], msg.fAddr[4], msg.fAddr[5]);
                Message::process(&msg);
            }
        }
        if (sLastBeacon + SMQ_BEACON_BROADCAST_INTERVAL < millis())
        {
            if (sSMQPairingMode && sSMQPairingTimeOut < millis())
            {
                removeBroadcastPeer();
                sSMQPairingMode = false;
                // sSMQPairingMode will be 2 if successful
                if (sSMQPairingMode == true && sPairingEvent)
                {
                    sPairingEvent(nullptr);
                }
            }
            if (SMQ::clearToSend())
            {
                if (sSMQPairingMode)
                {
                    Message::sendPair();
                }
                else if (sSMQPairedHostsCount != 0)
                {
                    Message::sendBeacon();
                }
                sLastBeacon = millis();
            }
            pruneHostList();
        }
    }

    static void sendString(String str)
    {
        send_string(str.c_str());
    }

    static void send_string(const char* str)
    {
        uint8_t delim = 0x00;
        send_raw_bytes(&delim, sizeof(delim));

        uint16_t len = strlen(str);
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

    static void send_topic_id(const smq_id id)
    {
        uint8_t delim = 0x0E;
        send_raw_bytes(&delim, sizeof(delim));
        send_raw_bytes(&id, sizeof(id));
    }

    static void send_topic_hash(const char* str)
    {
        send_topic_id(WSID32(str));
    }

    static void send_string_hash(const char* str)
    {
        uint8_t delim = 0x01;
        send_raw_bytes(&delim, sizeof(delim));

        uint16_t crc = 0xFFFF;
        uint16_t len = strlen(str);
        const uint8_t* b = (uint8_t*)str;
        const uint8_t* buf_end = (uint8_t*)str + len;
        while (b < buf_end)
        {
            crc = update_crc(crc, *b++);
        }
        crc = ~crc;
        send_raw_bytes(&crc, sizeof(crc));
    }

    static void send_string_hash(PROGMEMString str)
    {
        uint8_t delim = 0x01;
        send_raw_bytes(&delim, sizeof(delim));

        uint16_t crc = 0xFFFF;
        uint16_t len = strlen_P((const char*)str);
        const uint8_t* b = (uint8_t*)str;
        const uint8_t* buf_end = (uint8_t*)str + len;
        while (b < buf_end)
        {
            crc = update_crc(crc, pgm_read_byte(b++));
        }
        send_raw_bytes(&crc, sizeof(crc));
    }

    static void send_start(const smq_id id)
    {
        send_topic_id(id);
        send_raw_bytes(&sKeyHash, sizeof(sKeyHash));
    }

    static void send_start(const char* str)
    {
        send_topic_hash(str);
        send_raw_bytes(&sKeyHash, sizeof(sKeyHash));
    }

    static void send_start(PROGMEMString str)
    {
        send_topic_hash(String(str).c_str());
        send_raw_bytes(&sKeyHash, sizeof(sKeyHash));
    }

    static void send_string(const msg_id id, const char* val)
    {
        send_string_id(id);
        send_string(val);
    }

    static void send_string(const char* key, const char* val)
    {
        send_string_hash(key);
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

    static void sendString(const char* key, const char* val)
    {
        send_string(key, val);
    }

    static void sendString(String key, String val)
    {
        send_string(key.c_str(), val.c_str());
    }

    static void sendString(const char* key, String val)
    {
        send_string(key, val.c_str());
    }

    static void sendString(String key, const char* val)
    {
        send_string(key.c_str(), val);
    }

    static void send_int8(const msg_id id, int8_t val)
    {
        uint8_t delim = 0x02;
        send_string_id(id);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_int8(const char* key, int8_t val)
    {
        uint8_t delim = 0x02;
        send_string_hash(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_int8(PROGMEMString key, int8_t val)
    {
        uint8_t delim = 0x02;
        send_string_hash(key);
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

    static void send_int16(const char* key, int16_t val)
    {
        uint8_t delim = 0x03;
        send_string_hash(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_int16(PROGMEMString key, int16_t val)
    {
        uint8_t delim = 0x03;
        send_string_hash(key);
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
        send_string_hash(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_int32(PROGMEMString key, int32_t val)
    {
        uint8_t delim = 0x04;
        send_string_hash(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_long(const msg_id id, long val)
    {
        send_int32(id, val);
    }

    static void send_long(const char* key, long val)
    {
        send_int32(key, val);
    }

    static void send_uint8(const msg_id id, uint8_t val)
    {
        uint8_t delim = 0x05;
        send_string_id(id);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_uint8(const char* key, uint8_t val)
    {
        uint8_t delim = 0x05;
        send_string_hash(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_uint8(PROGMEMString key, uint8_t val)
    {
        uint8_t delim = 0x05;
        send_string_hash(key);
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

    static void send_uint16(const char* key, uint16_t val)
    {
        uint8_t delim = 0x06;
        send_string_hash(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_uint16(PROGMEMString key, uint16_t val)
    {
        uint8_t delim = 0x06;
        send_string_hash(key);
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

    static void send_uint32(const char* key, uint32_t val)
    {
        uint8_t delim = 0x07;
        send_string(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_uint32(PROGMEMString key, uint32_t val)
    {
        uint8_t delim = 0x07;
        send_string_hash(key);
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

    static void send_float(const char* key, float val)
    {
        uint8_t delim = 0x08;
        send_string_hash(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_float(PROGMEMString key, float val)
    {
        uint8_t delim = 0x08;
        send_string_hash(key);
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

    static void send_double(const char* key, double val)
    {
        uint8_t delim = 0x09;
        send_string_hash(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_double(PROGMEMString key, double val)
    {
        uint8_t delim = 0x09;
        send_string_hash(key);
        send_raw_bytes(&delim, sizeof(delim));
        send_data(&val, sizeof(val));
    }

    static void send_boolean(const msg_id id, bool val)
    {
        uint8_t delim = (val) ? 0x0A : 0x0B;
        send_string_id(id);
        send_raw_bytes(&delim, sizeof(delim));
    }

    static void send_boolean(const char* key, bool val)
    {
        uint8_t delim = (val) ? 0x0A : 0x0B;
        send_string_hash(key);
        send_raw_bytes(&delim, sizeof(delim));
    }

    static void send_boolean(PROGMEMString key, bool val)
    {
        uint8_t delim = (val) ? 0x0A : 0x0B;
        send_string_hash(key);
        send_raw_bytes(&delim, sizeof(delim));
    }

    static void send_null(const msg_id id)
    {
        uint8_t delim = 0x0C;
        send_string_id(id);
        send_raw_bytes(&delim, sizeof(delim));
    }

    static void send_null(const char* key)
    {
        uint8_t delim = 0x0C;
        send_string_hash(key);
        send_raw_bytes(&delim, sizeof(delim));
    }

    static void send_null(PROGMEMString key)
    {
        uint8_t delim = 0x0C;
        send_string_hash(key);
        send_raw_bytes(&delim, sizeof(delim));
    }

    // static void send_buffer(const void* buf, uint16_t len)
    // {
    //     uint8_t delim = 0x0D;
    //     send_string(key);
    //     send_raw_bytes(&delim, sizeof(delim));
    //     send_data(&len, sizeof(len));
    //     send_data(buf, len);
    // }

    static void clearAllPeers()
    {
        esp_now_peer_info_t peer;
        esp_err_t e = esp_now_fetch_peer(true, &peer);
        while (e == ESP_OK)
        {
            uint8_t addr[8];
            memcpy(addr, peer.peer_addr, sizeof(addr));
            e = esp_now_fetch_peer(false, &peer);
            esp_now_del_peer(addr);
        }
        removeBroadcastPeer();
    }

    static bool sendTopic(String topic, String host)
    {
        return sendTopic(topic.c_str(), (host.length() != 0) ? host.c_str() : nullptr);
    }

    static bool sendTopic(PROGMEMString topic)
    {
        return sendTopic(String(topic).c_str());
    }

    // host can either be host name or host mac address
    static bool sendTopic(const char* topic, const char* hostNameAddr = nullptr)
    {
        // printf("sendTopic %s\n", topic);
        sSendAddr = nullptr;
        if (!sSMQInited || !clearToSend())
        {
            SMQ_DEBUG_PRINT("FAILED TO SEND TOPIC: "); SMQ_DEBUG_PRINTLN(topic);
            return false;
        }
        clearAllPeers();

        bool searchHostMac = false;
        uint8_t addr[6];
        if (hostNameAddr != nullptr)
        {
            if (sscanf(hostNameAddr, SMQ_ADDR_HEX_STR, SMQ_ADDR_HEX_PTR(addr)) == sizeof(addr))
            {
                searchHostMac = true;
            }
        }

        bool found = false;
        smq_id topicID = WSID32(topic);
        for (SMQHost* host = sHostHead; host != nullptr; host = host->fNext)
        {
            // If host name is specified and does not match advance to next host
            if (hostNameAddr != nullptr)
            {
                if (!searchHostMac)
                {
                    if (strcmp(hostNameAddr, host->fName) != 0)
                        continue;
                }
                else if (memcmp(addr, &host->fAddr, sizeof(addr)) != 0)
                {
                    continue;
                }
            }
            for (unsigned i = 0; i < host->fCount; i++)
            {
                if (host->fTopics[i] == topicID)
                {
                    esp_now_peer_info_t peer_info;
                    memset(&peer_info, '\0', sizeof(peer_info));
                    peer_info.channel = WIFI_CHANNEL;
                    memcpy(peer_info.peer_addr, &host->fAddr, sizeof(host->fAddr));
                    peer_info.ifidx = WIFI_IF_STA;
                    peer_info.encrypt = true;
                    memcpy(peer_info.lmk, host->fLMK.fData, sizeof(host->fLMK.fData));
                    SMQ_DEBUG_PRINTF("ADD %s PEER: %s [LMK:%s]\n", host->getHostName().c_str(), host->getHostAddress().c_str(), host->getHostKey().c_str());
                    if (esp_now_add_peer(&peer_info) == ESP_OK)
                    {
                        if (!found)
                        {
                            sSendAddr = host->fAddr.fData;
                            found = true;
                        }
                        else
                        {
                            sSendAddr = nullptr;
                        }
                    }
                    else
                    {
                        SMQ_DEBUG_PRINTF("Failed to register peer\n");
                    }
                }
            }
        }
        if (found)
        {
            SMQ::send_start(topic);
            return true;
        }
        return false;
    }

    static SMQAddress messageSender()
    {
        return sSMQFromAddr;
    }

    static bool broadcastTopic(smq_id topic)
    {
        sSendAddr = sBroadcastMAC;
        if (clearToSend())
        {
            clearAllPeers();
            if (addBroadcastPeer())
            {
                SMQ::send_start(topic);
                return true;
            }
        }
        return false;
    }

    static bool broadcastPairedTopic(smq_id topic)
    {
        sSendAddr = nullptr;
        if (clearToSend())
        {
            clearAllPeers();
            if (addPairedPeers())
            {
                SMQ::send_start(topic);
                return true;
            }
        }
        return false;
    }

    static bool addPairedPeers()
    {
        clearAllPeers();
        for (unsigned i = 0; i < sSMQPairedHostsCount; i++)
        {
            // printf("Listening to [%d] %s\n", i, sSMQPairedHosts[i].toString().c_str());
            esp_now_peer_info_t peer_info;
            memset(&peer_info, '\0', sizeof(peer_info));
            peer_info.channel = WIFI_CHANNEL;
            SMQAddressKey* host = &sSMQPairedHosts[i];
            // uint8_t* fAddr = sSMQPairedHosts[i].fAddr;
            // printf("PAIR %02X:%02X:%02X:%02X:%02X:%02X sizeof=%d\n",
            //     fAddr[0], fAddr[1], fAddr[2], fAddr[3], fAddr[4], fAddr[5],
            //     sizeof(sSMQPairedHosts[i]));
            memcpy(peer_info.peer_addr, &host->fAddr, sizeof(host->fAddr));
            peer_info.ifidx = WIFI_IF_STA;
            peer_info.encrypt = true;
            memcpy(peer_info.lmk, host->fLMK.fData, sizeof(host->fLMK.fData));
            SMQ_DEBUG_PRINTF("ADD PEER: %s [LMK:%s]\n", host->fAddr.toString().c_str(), host->fLMK.toString().c_str());
            esp_err_t status = esp_now_add_peer(&peer_info);
            if (ESP_OK != status)
            {
                SMQ_DEBUG_PRINTF("esp_now_add_peer status=%d\n", status);
                SMQ_DEBUG_PRINTLN("Could not add peer");
                // handle_error(status);
                return false;
            }
        }
        return true;
    }

    static void send_end()
    {
        uint8_t delim = 0xFF;
        send_raw_bytes(&delim, sizeof(delim));

        // printf(" - len=%d\n", sSendPtr - sSendBuffer);
        sClearToSend = false;
        esp_err_t status = esp_now_send(sSendAddr, sSendBuffer, sSendPtr - sSendBuffer);
        switch (status)
        {
            case ESP_ERR_ESPNOW_NOT_INIT:
                SMQ_DEBUG_PRINTLN("Not init");
                break;
            case ESP_ERR_ESPNOW_ARG:
                SMQ_DEBUG_PRINTLN("Invalid arg");
                break;
            case ESP_ERR_ESPNOW_INTERNAL:
                SMQ_DEBUG_PRINTLN("Internal error");
                break;
            case ESP_ERR_ESPNOW_NO_MEM:
                SMQ_DEBUG_PRINTLN("No mem");
                break;
            case ESP_ERR_ESPNOW_NOT_FOUND:
                SMQ_DEBUG_PRINTLN("Not found");
                break;
            case ESP_ERR_ESPNOW_IF:
                SMQ_DEBUG_PRINTLN("Interface mismatch");
                break;
            case ESP_OK:
                break;
        }
        if (status != ESP_OK)
        {
            SMQ_DEBUG_PRINTF("esp_now_send: %d\n", status);
            sClearToSend = true;
        }
        sSendPtr = sSendBuffer;
    }

    static void sendEnd()
    {
        send_end();
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
            if (key == NULL || find_key(WSID16(key)))
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

        String getString(const msg_id keyID)
        {
            char buf[250];
            return get_string(keyID, buf, sizeof(buf));
        }


        String getString(const char* key)
        {
            char buf[250];
            return get_string(key, buf, sizeof(buf));
        }

        String getString(String key)
        {
            return getString(key.c_str());
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
            if (find_key(WSID16(key)))
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
                        // DEBUG_PRINTLN("[KEYSTRING]");
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
                        // else
                        // {
                        //     DEBUG_PRINT("crc=");
                        //     DEBUG_PRINT_HEX(crc);
                        //     DEBUG_PRINT(" expected=");
                        //     DEBUG_PRINT_HEX(recrc);
                        //     DEBUG_PRINTLN("CRC BAD4");
                        //     DEBUG_FLUSH();
                        // }
                        break;
                    }
                    case 0x01:
                    {
                        // Hash
                        // DEBUG_PRINTLN("[KEYCRC]");
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
                        // DEBUG_PRINTLN("CRC BAD3");
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

        static void process(SMQRecvMsg* smsg)
        {
            smq_id recvTopicID = read_uint32();
            // printf("recvTopicID: 0x%08X\n", recvTopicID);
            uint32_t keyHash = read_uint32();
            if (keyHash == sKeyHash)
            {
                // PAIR is like BEACON just unencrypted
                if (recvTopicID == sSMQPAIRING_ID && !sSMQPairingMode)
                {
                    SMQ_DEBUG_PRINTLN("Received PAIRING but not in PAIRING mode. Ignored.");
                }
                else if (recvTopicID == sSMQBEACON_ID || recvTopicID == sSMQPAIRING_ID)
                {
                    char name[SMQ_MAX_HOST_NAME];
                    read_buffer((uint8_t*)name, sizeof(name), sizeof(name));
                    name[sizeof(name)-1] = '\0';

                    SMQLMK key;
                    uint8_t count = read_uint8();
                    SMQHost* host = getHost(smsg->fAddr, count);
                    if (recvTopicID == sSMQPAIRING_ID)
                    {
                        read_buffer(key.fData, sizeof(key.fData), sizeof(key.fData));
                        SMQ_DEBUG_PRINTF("RECEIVED KEY: %s\n", key.toString().c_str());
                        if (host != nullptr)
                        {
                            memcpy(host->fLMK.fData, &key, sizeof(key));
                        }
                    }
                    // if (recvTopicID == sSMQBEACON_ID)
                    // {
                    //     printf("BEACON [0x%08X]\n", recvTopicID);
                    // }
                    // else if (recvTopicID == sSMQPAIRING_ID)
                    // {
                    //     printf("PAIRING [0x%08X]\n", recvTopicID);
                    // }
                    if (host != nullptr)
                    {
                        // Check if host name has been set and if not initialize
                        // printf("FOUND HOST \"%s\" paired=%d\n", host->fName, host->fPaired);
                        if (strcmp(host->fName, name) != 0 ||
                            (recvTopicID == sSMQBEACON_ID && host->fPaired) ||
                            (recvTopicID == sSMQPAIRING_ID && !host->fPaired))
                        {
                            // Need to initialize host entry
                            strcpy(host->fName, name);
                            host->fCount = count;
                            host->fPaired = false;
                            // printf("TOPIC COUNT=%d\n", count);
                            for (unsigned i = 0; i < count; i++)
                            {
                                smq_id topic;
                                read_buffer((uint8_t*)&topic, sizeof(topic), sizeof(topic));
                                // printf("READING [0x%08X]\n", topic);
                                host->fTopics[i] = topic;
                            }
                            if (recvTopicID == sSMQPAIRING_ID && sPairingEvent != nullptr)
                            {
                                if (sSMQPairingMode)
                                {
                                    // Ensure we send at least one PAIR event
                                    Message::sendPair();
                                    sPairingEvent(host);
                                    host->fPaired = true;
                                    // pruneHost(host, false);
                                }
                            }
                            else if (recvTopicID == sSMQBEACON_ID && sDiscoverEvent != nullptr)
                            {
                                sDiscoverEvent(host);
                            }
                        }
                        host->fLastSeen = millis();
                    }
                    else
                    {
                        printf("HOST NOT FOUND: %s\n", name);
                    }
                }
                else
                {
                    // printf("recvTopicID: 0x%08X\n", recvTopicID);
                    memcpy(sSMQFromAddr.fData, smsg->fAddr, sizeof(smsg->fAddr));
                    for (Message* msg = *tail(); msg != NULL; msg = msg->fNext)
                    {
                        // smq_id topicID = (sizeof(smq_id) == sizeof(uint32_t)) ?
                        //     pgm_read_dword(&msg->fTopic) : pgm_read_word(&msg->fTopic);
                        smq_id topicID = msg->fTopic;
                        if (recvTopicID == topicID)
                        {
                            // SMQ_DEBUG_PRINT("PROCESS: "); SMQ_DEBUG_PRINTLN_HEX(recvTopicID);
                            msg->fMType = -1;
                            msg->fEOM = false;
                            // DEBUG_PRINTLN("CALLING MSG");
                            // DEBUG_PRINT("fHandler=");
                            // DEBUG_PRINTLN_HEX((size_t)msg->fHandler);
                            msg->fHandler(*msg);
                            msg->end();
                            break;
                        }
                    }
                }
            }
        }

        static void sendPair()
        {
            uint8_t count = 0;
            for (Message* msg = *tail(); msg != NULL; msg = msg->fNext)
                count++;
            if (broadcastTopic(sSMQPAIRING_ID))
            {
                send_raw_bytes(&sHostName, sizeof(sHostName));
                send_raw_bytes(&count, sizeof(count));
                send_raw_bytes(&sSMQLMK, sizeof(sSMQLMK));
                for (Message* msg = *tail(); msg != NULL; msg = msg->fNext)
                {
                    smq_id topicID = msg->fTopic;
                    send_raw_bytes(&topicID, sizeof(topicID));
                }
                send_end();
            }
            else
            {
                SMQ_DEBUG_PRINTLN("FAILED TO SEND PAIRING");
            }
        }

        static void sendBeacon()
        {
            uint8_t count = 0;
            for (Message* msg = *tail(); msg != NULL; msg = msg->fNext)
                count++;
            if (broadcastPairedTopic(sSMQBEACON_ID))
            {
                send_raw_bytes(&sHostName, sizeof(sHostName));
                send_raw_bytes(&count, sizeof(count));
                for (Message* msg = *tail(); msg != NULL; msg = msg->fNext)
                {
                    smq_id topicID = msg->fTopic;
                    send_raw_bytes(&topicID, sizeof(topicID));
                }
                send_end();
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
    static SMQHost* getHost(const uint8_t* mac_addr, uint8_t topicCount = 0)
    {
        SMQHost* host;
        for (host = sHostHead; host != nullptr; host = host->fNext)
        {
            if (memcmp(mac_addr, &host->fAddr, sizeof(host->fAddr)) == 0)
            {
                // Return found host
                return host;
            }
        }
        // Need to create a new host entry
        size_t siz = sizeof(SMQHost)+sizeof(host->fTopics[0])*topicCount;
        host = (SMQHost*)malloc(siz);
        if (host != nullptr)
        {
            memset(host, '\0', siz);
            memcpy(&host->fAddr, mac_addr, sizeof(host->fAddr));
            if (sHostHead == nullptr)
            {
                sHostHead = host;
            }
            else
            {
                sHostTail->fNext = host;
            }
            host->fPrev = sHostTail;
            sHostTail = host;
        }
        // Lookup the LMK for this host
        for (unsigned i = 0; i < sSMQPairedHostsCount; i++)
        {
            SMQAddressKey* hostKey = &sSMQPairedHosts[i];
            if (hostKey->fAddr.equals(host->fAddr))
            {
                host->fLMK = hostKey->fLMK;
            }
        }
        return host;
    }

    static void pruneHost(SMQHost* host, bool notify = true)
    {
        if (notify)
            sLostEvent(host);

        if (host->fPrev != nullptr)
        {
            host->fPrev->fNext = host->fNext;
        }
        else if (sHostHead == host)
        {
            sHostHead = host->fNext;
        }
        if (host->fNext != nullptr)
        {
            host->fNext->fPrev = host->fPrev;
        }
        else if (host == sHostTail)
        {
            sHostTail = host->fPrev;
        }
        if (host != nullptr)
        {
            host->fPrev = host->fNext = nullptr;
        }
        free(host);
    }

    static void pruneHostList()
    {
        uint32_t now = millis();
        SMQHost* host = sHostHead;
        while (host != nullptr)
        {
            SMQHost* next = host->fNext;
            if (host->fLastSeen + SMQ_HOST_LOST_TIMEOUT < now)
            {
                SMQHost* lostHost = host;
                pruneHost(lostHost);
            }
            host = next;
        }
    }

    static void msg_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len)
    {
        SMQRecvMsg msg;
        if (len < sizeof(msg.fData))
        {
            memcpy(msg.fAddr, mac_addr, sizeof(msg.fAddr));
            memcpy(msg.fData, data, len);
            msg.fSize = len;
            // Dont wait if queue is full. Message will be lost
            if (xQueueSend(sRecvQueue, &msg, 0) != pdPASS)
            {
                SMQ_DEBUG_PRINTLN("[SMQ] xQueueSend FAILED");
            }
        }
    }

    static void msg_send_cb(const uint8_t* mac, esp_now_send_status_t sendStatus)
    {
        switch (sendStatus)
        {
            case ESP_NOW_SEND_SUCCESS:
                // Serial.println("Send success");
                break;

            case ESP_NOW_SEND_FAIL:
                SMQ_DEBUG_PRINTLN("[SMQ] esp_now_send FAILED");
                break;

            default:
                break;
        }
        sClearToSend = true;
        addPairedPeers();
    }

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
        // Truncate on overflow
        if (sSendPtr + len >= &sSendBuffer[sizeof(sSendBuffer)])
        {
            printf("SMQ TRUNCATE\n");
            len = 0;
        }
        // for (int i = 0; i < len; i++)
        // {
        //     printf("%02X ", ((uint8_t*)buf)[i]);
        // }
        memcpy(sSendPtr, buf, len);
        sSendPtr += len;
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
            send_raw_bytes(&b, sizeof(b));
        }
    }

    static int read(void* buf, size_t len)
    {
        char* b = (char*)buf;
        char* b_end = b + len;
        while (b < b_end)
        {
            if (sReadLen <= 0)
                break;
            *b++ = *sReadPtr++;
            sReadLen--;
        }
        int cnt = b - (char*)buf;
        // if (cnt > 0)
        // {
        //     DEBUG_PRINT(cnt); DEBUG_PRINT("<== [0x");
        //     for (int i = 0; i < cnt; i++)
        //     {
        //         DEBUG_PRINT_HEX(((uint8_t*)buf)[i]);
        //         DEBUG_PRINT(" ");
        //     }
        //     DEBUG_PRINTLN("]");
        //     DEBUG_FLUSH();
        // }
        return cnt;
    }

    static int8_t read_int8()
    {
        int8_t val = 0;
        read(&val, sizeof(val));
        return val;
    }

    static int16_t read_int16()
    {
        int16_t val = 0;
        read(&val, sizeof(val));
        return val;
    }

    static int32_t read_int32()
    {
        int32_t val = 0;
        read(&val, sizeof(val));
        return val;
    }

    static uint8_t read_uint8()
    {
        uint8_t val = 0;
        read(&val, sizeof(val));
        return val;
    }

    static uint16_t read_uint16()
    {
        uint16_t val = 0;
        read(&val, sizeof(val));
        return val;
    }

    static uint32_t read_uint32()
    {
        uint32_t val = 0;
        read(&val, sizeof(val));
        return val;
    }

    static float read_float()
    {
        float val = 0;
        read(&val, sizeof(val));
        return val;
    }

    static double read_double()
    {
        double val = 0;
        read(&val, sizeof(val));
        return val;
    }

    static uint8_t* read_buffer(uint8_t* buf, uint16_t len, uint16_t bufsize)
    {
        uint8_t* p = buf;
        uint8_t* pend = p + min(len, bufsize);
        while (p < pend)
        {
            if (sReadLen <= 0)
                break;
            *p++ = *sReadPtr++;
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
SMQ sSMQ;

typedef void (*SMQMessageHandler)(class SMQ::Message& msg);

#define SMQMSG_FUNC_DECL(topic) \
  static void SMQHandler_##topic(SMQ::Message& msg)
#define SMQMESSAGE(topic, handler) \
  SMQMSG_FUNC_DECL(topic); \
  SMQ::Message SMQMSG_##topic(STRID(#topic), SMQHandler_##topic); \
  SMQMSG_FUNC_DECL(topic) { UNUSED_ARG(msg) handler }

#endif
