#include "usb.h"


usb::usb_descriptor_device usb::DESCRIPTOR_DEVICE = {
	.bLength = 18,
	.bDescriptorType = 0x01,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0x00,
	.bDeviceSubclass = 0x00,
	.bDeviceProtocol = 0x00,
	.bMaxPacketSize = 8,
	.idVendor = 0x04d8,
	.idProduct = 0x000a,
	.bcdDevice = 0x000a,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 0,
	.bNumConfigurations = 1
};


usb::usb_descriptor_endpoint endpoint1 = {
	.bLength = 7,
	.bDescriptorType = 0x05,
	.bEndpointAddress = 0x01,
	.bmAttributes = 0x3,
	.wMaxPacketSize = 8,
	.bInterval = 200
};


usb::usb_descriptor_endpoint endpoint2 = {
	.bLength = 7,
	.bDescriptorType = 0x05,
	.bEndpointAddress = 0x81,
	.bmAttributes = 0x3,
	.wMaxPacketSize = 8,
	.bInterval = 200
};


usb::usb_descriptor_interface interface0 = {
	.bLength = 9,
	.bDescriptorType = 0x04,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = 0xff,
	.bInterfaceSubclass = 0xff,
	.bInterfaceProtocol = 0,
	.iInterface = 0,
	.ENDPOINTS =
	{endpoint1, endpoint2}
};


usb::usb_descriptor_configuration usb::DESCRIPTOR_CONFIGURATION[] = {
	{
		.bLength = 9,
		.bDescritptorType = 0x02,
		.wTotalLength = 32,
		.bNumInterfaces = 1,
		.bConfigurationValue = 1,
		.iConfiguration = 0,
		.bmAttributes = 0x80,
		.bMaxPower = 75,
		.INTERFACES =
		{interface0}
	}
};


usb::usb_descriptor_string usb::DESCRIPTOR_STRING[] = {
	{
		.bLength = 4,
		.bDescriptorType = 0x03,
		.bString =
		{
			0x0409
		}
	},
	{
		50,
		0x03,
		u"Mishanya Technology Inc."
	},
	{
		58,
		0x03,
		u"Sub-GHz Wireless transceiver"
	}
};
