/*
 * Note: this code isn't working :)
 *
 * Resources : 
 * FIDO U2F HID spec: https://fidoalliance.org/specs/fido-u2f-v1.0-ps-20141009/fido-u2f-hid-protocol-ps-20141009.html
 * PluggableUSB API from Arduino: https://github.com/arduino/Arduino/wiki/PluggableUSB-and-PluggableHID-howto
 * Existing U2F implementation on a Teensy: https://github.com/yohanes/teensy-u2f/blob/master/usb_desc.h
 * MIDIUSB from Arduino: https://github.com/arduino-libraries/MIDIUSB
 * device discovery implemented by libu2fhost : https://github.com/Yubico/libu2f-host/blob/master/u2f-host/devs.c#L395
*/

/*
 * Simple dump of if descriptors : sudo usbhid-dump
 *
 * Debugging communication between U2F and PC : using libu2f-host (as root!)
 * - curl 'https://demo.yubico.com/wsapi/u2f/enroll?username=jas&password=foo' > foo
 * - u2f-host --debug -aregister < foo
 *
 * Packet analyser for USB : wireshark, after modprobe usbmon
 *
 */

#include "U2F_HID.h"


// Magic values from https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/u2fd/u2fhid.cc

static const uint8_t _hidReportDescriptor[] PROGMEM = {
    0x06, 0xD0, 0xF1, /* Usage Page (FIDO Alliance), FIDO_USAGE_PAGE */
    0x09, 0x01,       /* Usage (U2F HID Auth. Device) FIDO_USAGE_U2FHID */
    0xA1, 0x01,       /* Collection (Application), HID_APPLICATION */
    0x09, 0x20,       /*  Usage (Input Report Data), FIDO_USAGE_DATA_IN */
    0x15, 0x00,       /*  Logical Minimum (0) */
    0x26, 0xFF, 0x00, /*  Logical Maximum (255) */
    0x75, 0x08,       /*  Report Size (8) */
    0x95, 0x40,       /*  Report Count (64), HID_INPUT_REPORT_BYTES */
    0x81, 0x02,       /*  Input (Data, Var, Abs), Usage */
    0x09, 0x21,       /*  Usage (Output Report Data), FIDO_USAGE_DATA_OUT */
    0x15, 0x00,       /*  Logical Minimum (0) */
    0x26, 0xFF, 0x00, /*  Logical Maximum (255) */
    0x75, 0x08,       /*  Report Size (8) */
    0x95, 0x40,       /*  Report Count (64), HID_OUTPUT_REPORT_BYTES */
    0x91, 0x02,       /*  Output (Data, Var, Abs), Usage */
    0xC0              /* End Collection */
};

U2F_HID_ U2F_HID;

U2F_HID_::U2F_HID_(void) : PluggableUSBModule(2, 2, epType) {
	epType[0] = EP_TYPE_INTERRUPT_OUT; // TX, OUT from the point of view of the host
	epType[1] = EP_TYPE_INTERRUPT_IN; // RX, IN from the point of view of the host
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

int U2F_HID_::SendReport(void* data) {
	return USB_Send(ENDPOINT_IN, data, HID_TX_SIZE);
}

int U2F_HID_::RecvRaw(void *data) {
	return USB_Recv(ENDPOINT_OUT, data, HID_RX_SIZE);
}
