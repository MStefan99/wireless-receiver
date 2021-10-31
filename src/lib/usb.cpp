#include "device.h"
#include "usb.h"

using namespace usb;


enum class USB_DEVICE_REQ {
	GET_STATUS = 0x00,
	CLEAR_FEATURE = 0x01,
	SET_FEATURE = 0x03,
	SET_ADDRESS = 0x05,
	GET_DESCRIPTOR = 0x06,
	SET_DESCRIPTOR = 0x07,
	GET_CONFIGURATION = 0x08,
	SET_CONFIGURATION = 0x09
};


enum class USB_INTERFACE_REQ {
	GET_STATUS = 0x00,
	CLEAR_FEATURE = 0x01,
	SET_FEATURE = 0x03,
	GET_INTERFACE = 0x0A,
	SET_INTERFACE = 0x11
};


enum class USB_EP_REQ {
	GET_STATUS = 0x00,
	CLEAR_FEATURE = 0x01,
	SET_FEATURE = 0x03,
	SYNCH_FRAME = 0x12
};


typedef struct {
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
} usb_device_endpoint0_request;


usb_device_endpoint0_request EP0REQ;


extern "C" {


	void USB_Handler() {
		if (USB_REGS->DEVICE.USB_INTFLAG & USB_DEVICE_INTFLAG_EORST_Msk) {
			USB_REGS->DEVICE.DEVICE_ENDPOINT[0].USB_EPINTENSET = USB_DEVICE_EPINTENSET_RXSTP(1); // Enable endpoint interrupt
			EPDESCTBL[0].DEVICE_DESC_BANK[1].USB_ADDR = (uint32_t) & DESCRIPTOR_DEVICE;
			EPDESCTBL[0].DEVICE_DESC_BANK[0].USB_ADDR = (uint32_t) & EP0REQ;
			EPDESCTBL[0].DEVICE_DESC_BANK[0].USB_PCKSIZE = USB_DEVICE_PCKSIZE_BYTE_COUNT(8);
		}

		if (USB_REGS->DEVICE.DEVICE_ENDPOINT[0].USB_EPINTFLAG & USB_DEVICE_EPINTFLAG_RXSTP_Msk) {

			EPDESCTBL[0].DEVICE_DESC_BANK[1].USB_PCKSIZE = USB_DEVICE_PCKSIZE_BYTE_COUNT(18);
			USB_REGS->DEVICE.DEVICE_ENDPOINT[0].USB_EPSTATUSSET = USB_DEVICE_EPSTATUS_BK1RDY(1);
			USB_REGS->DEVICE.DEVICE_ENDPOINT[0].USB_EPSTATUSCLR = USB_DEVICE_EPSTATUS_BK0RDY(1);
		}
	}
}


void usb::init() {
	uint32_t calibration = *((uint32_t*) 0x00806020);
	
	USB_REGS->DEVICE.USB_PADCAL = USB_PADCAL_TRANSN(calibration >> 13u & 0x1f)
					| USB_PADCAL_TRANSP(calibration >> 18u & 0x1f)
					| USB_PADCAL_TRIM(calibration >> 23u & 0x7); // USB pad calibration
	USB_REGS->DEVICE.USB_CTRLA = USB_CTRLA_ENABLE(1) // Enable USB
					| USB_CTRLA_MODE_DEVICE; // Enable in device mode
	USB_REGS->DEVICE.DEVICE_ENDPOINT[0].USB_EPCFG = USB_DEVICE_EPCFG_EPTYPE0(0x1) // Configure endpoint 0 as setup out
					| USB_DEVICE_EPCFG_EPTYPE1(0x1); // Configure endpoint 0 as setup in
	USB_REGS->DEVICE.USB_DESCADD = (uint32_t) & EPDESCTBL; // Setting endpoint descriptor address
	USB_REGS->DEVICE.USB_CTRLB = USB_DEVICE_CTRLB_DETACH(0); // Attach to host
	USB_REGS->DEVICE.USB_INTENSET = USB_DEVICE_INTENSET_EORST(1); // Enable end-of-reset interrupt
}
