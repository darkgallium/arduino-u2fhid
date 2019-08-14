/*
 * Resources : 
 * FIDO U2F HID spec: https://fidoalliance.org/specs/fido-u2f-v1.0-ps-20141009/fido-u2f-hid-protocol-ps-20141009.html
 * RawHID from NicoHood: https://github.com/NicoHood/HID/blob/master/src/SingleReport/RawHID.cpp
 * PluggableUSB API from Arduino: https://github.com/arduino/Arduino/wiki/PluggableUSB-and-PluggableHID-howto
 * Existing U2F implementation on a Teensy: https://github.com/yohanes/teensy-u2f/blob/master/usb_desc.h
 * MIDIUSB from Arduino: https://github.com/arduino-libraries/MIDIUSB
*/


#include "U2F_HID.h"

static const uint8_t _hidReportDescriptor[] PROGMEM = {
        0x06, lowByte(HID_USAGE_PAGE), highByte(HID_USAGE_PAGE),
        0x0A, lowByte(HID_USAGE), highByte(HID_USAGE),

        0xA1, 0x01,                  /* Collection 0x01 */
        0x75, 0x08,                  /* report size = 8 bits */
        0x15, 0x00,                  /* logical minimum = 0 */
        0x26, 0xFF, 0x00,            /* logical maximum = 255 */

        0x95, HID_TX_SIZE,        /* report count TX */
        0x09, 0x01,                  /* usage */
        0x81, 0x02,                  /* Input (array) */

        0x95, HID_RX_SIZE,        /* report count RX */
        0x09, 0x02,                  /* usage */
        0x91, 0x02,                  /* Output (array) */
        0xC0                         /* end collection */	
};

U2F_HID_ U2F_HID;

U2F_HID_::U2F_HID_(void) : PluggableUSBModule(2, 2, epType) {
	epType[0] = EP_TYPE_INTERRUPT_OUT;
	epType[1] = EP_TYPE_INTERRUPT_IN;
	PluggableUSB().plug(this);
}

int U2F_HID_::begin(void) {
	return 0;
}

int U2F_HID_::getInterface(uint8_t* interfaceCount) {
	*interfaceCount += 1;

	HIDDescriptor hidInterface = {
		D_INTERFACE(pluggedInterface, 2, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
		D_HIDREPORT(sizeof(_hidReportDescriptor)),
		D_ENDPOINT(USB_ENDPOINT_OUT(ENDPOINT_OUT), USB_ENDPOINT_TYPE_INTERRUPT, HID_TX_SIZE, 0x01),
		D_ENDPOINT(USB_ENDPOINT_IN(ENDPOINT_IN), USB_ENDPOINT_TYPE_INTERRUPT, HID_RX_SIZE, 0x01)
	};

	return USB_SendControl(0, &hidInterface, sizeof(hidInterface));

}

int U2F_HID_::getDescriptor(USBSetup& setup)
{
        // Check if this is a HID Class Descriptor request
        if (setup.bmRequestType != REQUEST_DEVICETOHOST_STANDARD_INTERFACE) { return 0; }
        if (setup.wValueH != HID_REPORT_DESCRIPTOR_TYPE) { return 0; }

        // In a HID Class Descriptor wIndex cointains the interface number
        if (setup.wIndex != pluggedInterface) { return 0; }

        // Reset the protocol on reenumeration. Normally the host should not assume the state of the protocol
        // due to the USB specs, but Windows and Linux just assumes its in report mode.
        protocol = HID_REPORT_PROTOCOL;

        return USB_SendControl(TRANSFER_PGM, _hidReportDescriptor, sizeof(_hidReportDescriptor));
}

bool U2F_HID_::setup(USBSetup& setup) {
	return 0;
}

uint8_t U2F_HID_::getShortName(char* name) {
	memcpy(name, "U2FHID", 6);
	return 6;
}

int U2F_HID_::SendReport(uint8_t id, const void* data, int len)
{
	uint8_t p[64];
	p[0] = id;
	memcpy(&p[1], data, len);
	return USB_Send(pluggedEndpoint, p, len+1);
}
