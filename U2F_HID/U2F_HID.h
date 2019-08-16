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
  int SendReport(uint8_t id, const void* data, int len);
  int RecvRaw(void *data);

private:
  uint8_t epType[2];
  uint16_t descriptorSize;
  uint8_t protocol;
  uint8_t idle;
};

extern U2F_HID_ U2F_HID;

#endif
