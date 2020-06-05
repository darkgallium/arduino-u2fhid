#ifndef U2F_HID_H
#define U2F_HID_H

#include <Arduino.h>
#include "PluggableUSB.h"

#define ENDPOINT_OUT pluggedEndpoint
#define ENDPOINT_IN ((uint8_t) (pluggedEndpoint+1))

#define HID_TX_SIZE 0x40
#define HID_RX_SIZE 0x40

#define HID_USAGE_PAGE 0xf1d0
#define HID_USAGE 0x01

// HID 'Driver'

#define HID_GET_REPORT        0x01
#define HID_GET_IDLE          0x02
#define HID_GET_PROTOCOL      0x03
#define HID_SET_REPORT        0x09
#define HID_SET_IDLE          0x0A
#define HID_SET_PROTOCOL      0x0B

#define HID_HID_DESCRIPTOR_TYPE         0x21
#define HID_REPORT_DESCRIPTOR_TYPE      0x22
#define HID_PHYSICAL_DESCRIPTOR_TYPE    0x23

#define HID_SUBCLASS_NONE 0
#define HID_PROTOCOL_NONE 0
#define HID_REPORT_PROTOCOL	1

#define HID_REPORT_TYPE_INPUT   1
#define HID_REPORT_TYPE_OUTPUT  2
#define HID_REPORT_TYPE_FEATURE 3

#define D_HIDREPORT(length) { 9, 0x21, 0x01, 0x01, 0, 1, 0x22, lowByte(length), highByte(length) }

typedef struct
{
  uint8_t len;      // 9
  uint8_t dtype;    // 0x21
  uint8_t addr;
  uint8_t versionL; // 0x101
  uint8_t versionH; // 0x101
  uint8_t country;
  uint8_t desctype; // 0x22 report
  uint8_t descLenL;
  uint8_t descLenH;
} HIDDescDescriptor;

typedef struct 
{
  InterfaceDescriptor hid;
  HIDDescDescriptor   desc;
  EndpointDescriptor  out;
  EndpointDescriptor  in;
} HIDDescriptor;

class U2F_HID_ : public PluggableUSBModule
{

protected:
  int getInterface(uint8_t* interfaceNum);
  int getDescriptor(USBSetup& setup);
  bool setup(USBSetup& setup);
  uint8_t getShortName(char* name);

public:
  U2F_HID_(void);
  int begin(void);
  int SendReport(void* data);
  int RecvRaw(void *data);

private:
  uint8_t epType[2];
  uint16_t descriptorSize;
  uint8_t protocol;
  uint8_t idle;
};

extern U2F_HID_ U2F_HID;

// Constants and structs from https://fidoalliance.org/specs/fido-u2f-v1.2-ps-20170411/inc/u2f_hid.h

#define HID_RPT_SIZE            64      // Default size of raw HID report
#define CID_BROADCAST           0xffffffff // Broadcast channel id
#define TYPE_MASK               0x80    // Frame type mask 
#define TYPE_INIT               0x80    // Initial frame identifier
#define TYPE_CONT               0x00    // Continuation frame identifier

typedef struct {
  uint32_t cid;                        // Channel identifier
  union {
    uint8_t type;                      // Frame type - b7 defines type
    struct {
      uint8_t cmd;                     // Command - b7 set
      uint8_t bcnth;                   // Message byte count - high part
      uint8_t bcntl;                   // Message byte count - low part
      uint8_t data[HID_RPT_SIZE - 7];  // Data payload
    } init;
    struct {
      uint8_t seq;                     // Sequence number - b7 cleared
      uint8_t data[HID_RPT_SIZE - 5];  // Data payload
    } cont;
  };
} U2FHID_FRAME;

#define FRAME_TYPE(f) ((f).type & TYPE_MASK)
#define FRAME_CMD(f)  ((f).init.cmd & ~TYPE_MASK)
#define MSG_LEN(f)    ((f).init.bcnth*256 + (f).init.bcntl)
#define FRAME_SEQ(f)  ((f).cont.seq & ~TYPE_MASK)

// HID usage- and usage-page definitions

#define FIDO_USAGE_PAGE         0xf1d0  // FIDO alliance HID usage page
#define FIDO_USAGE_U2FHID       0x01    // U2FHID usage for top-level collection
#define FIDO_USAGE_DATA_IN      0x20    // Raw IN data report
#define FIDO_USAGE_DATA_OUT     0x21    // Raw OUT data report
        
// General constants    

#define U2FHID_IF_VERSION       2       // Current interface implementation version
#define U2FHID_TRANS_TIMEOUT    3000    // Default message timeout in ms

// U2FHID native commands

#define U2FHID_PING         (TYPE_INIT | 0x01)  // Echo data through local processor only
#define U2FHID_MSG          (TYPE_INIT | 0x03)  // Send U2F message frame
#define U2FHID_LOCK         (TYPE_INIT | 0x04)  // Send lock channel command
#define U2FHID_INIT         (TYPE_INIT | 0x06)  // Channel initialization
#define U2FHID_WINK         (TYPE_INIT | 0x08)  // Send device identification wink
#define U2FHID_SYNC         (TYPE_INIT | 0x3c)  // Protocol resync command

#define INIT_NONCE_SIZE         8       // Size of channel initialization challenge
#define CAPFLAG_WINK            0x01    // Device supports WINK command

typedef struct {
  uint8_t nonce[INIT_NONCE_SIZE];       // Client application nonce
} U2FHID_INIT_REQ;

typedef struct {
  uint8_t nonce[INIT_NONCE_SIZE];       // Client application nonce
  uint32_t cid;                         // Channel identifier  
  uint8_t versionInterface;             // Interface version
  uint8_t versionMajor;                 // Major version number
  uint8_t versionMinor;                 // Minor version number
  uint8_t versionBuild;                 // Build version number
  uint8_t capFlags;                     // Capabilities flags  
} U2FHID_INIT_RESP;

// U2FHID_SYNC command defines

typedef struct {
  uint8_t nonce;                        // Client application nonce
} U2FHID_SYNC_REQ;

typedef struct {
  uint8_t nonce;                        // Client application nonce
} U2FHID_SYNC_RESP;

// https://github.com/google/tock-on-titan/blob/master/userspace/u2f_app/include/u2f.h

typedef struct {
        uint8_t ins;
        uint8_t p1;
        uint8_t p2;
        uint16_t len;
        uint8_t *data;
} APDU;

#define U2F_SW_NO_ERROR                 0x9000
#define U2F_SW_INS_NOT_SUPPORTED        0x6D00
#define U2F_SW_CONDITIONS_NOT_SATISFIED 0x6985
#define U2F_SW_WRONG_DATA               0x6A80

#define MAX_ECDSA_SIG_SIZE  72  // asn1 DER format
#define MAX_KH_SIZE  128  // key handle
#define MAX_CERT_SIZE  2048  // attestation certificate

#define U2F_APPID_SIZE  32
#define U2F_NONCE_SIZE  32

// U2Fv2 instructions

#define U2F_INS_REGISTER  0x01
#define U2F_INS_AUTHENTICATE  0x02
#define U2F_INS_VERSION  0x03

// U2F_REGISTER instruction defines

#define U2F_REGISTER_ID  0x05  // magic constant
#define U2F_REGISTER_HASH_ID  0x00  // magic constant

typedef struct {
  uint8_t nonce[U2F_NONCE_SIZE];
  uint8_t appId[U2F_APPID_SIZE];
} U2F_REGISTER_REQ;

typedef struct {
  uint8_t registerId;
  uint8_t pubKey[65];
  uint8_t keyHandleLen;
  uint8_t keyHandle[32];
  uint8_t attestationCert[MAX_CERT_SIZE];
  uint8_t signature[MAX_ECDSA_SIG_SIZE];
} U2F_REGISTER_RESP;

// U2F_AUTHENTICATE instruction defines

// Authentication parameter byte
#define U2F_AUTH_ENFORCE  0x03  // Require user presence
#define U2F_AUTH_CHECK_ONLY  0x07  // Test but do not consume

typedef struct {
  uint8_t nonce[U2F_NONCE_SIZE];
  uint8_t appId[U2F_APPID_SIZE];
  uint8_t keyHandleLen;
  uint8_t keyHandle[MAX_KH_SIZE];
} U2F_AUTHENTICATE_REQ;

// Flags values
#define U2F_TOUCHED  0x01
#define U2F_ALTERNATE_INTERFACE  0x02
#define U2F_CTR_SIZE 4

typedef struct {
  uint8_t flags;
  uint8_t ctr[U2F_CTR_SIZE];
  uint8_t sig[MAX_ECDSA_SIG_SIZE];
} U2F_AUTHENTICATE_RESP;

#endif
