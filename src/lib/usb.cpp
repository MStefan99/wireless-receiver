#include "usb.h"

using namespace usb;


static void controlHandler();
static void deviceRequestHandler();
static void enableEndpoints(uint8_t configurationNumber);
static void endpoint1Handler();


#define MIN(a, b) ((a < b)? (a) : (b))


#define BMREQUESTTYPE_DPTD(bmRequestType) ((bmRequestType & 0x80) >> 7)
#define BMREQUESTTYPE_TYPE(bmRequestType) ((bmRequestType & 0x60) >> 5)
#define BMREQUESTTYPE_RECIPIENT(bmRequestType) (bmRequestType & 0x1f)

#define WVALUE_TYPE(wValue) ((wValue & 0xff00) >> 8)
#define WVALUE_IDX(wValue) (wValue & 0xff)
#define WVALUE_ADD(wValue) (wValue & 0x7f)


enum class REQ_RECIPIENT {
	DEVICE = 0x00,
	INTERFACE = 0x01,
	ENDPOINT = 0x02
};


enum class DEVICE_REQ {
	GET_STATUS = 0x00,
	CLEAR_FEATURE = 0x01,
	SET_FEATURE = 0x03,
	SET_ADDRESS = 0x05,
	GET_DESCRIPTOR = 0x06,
	SET_DESCRIPTOR = 0x07,
	GET_CONFIGURATION = 0x08,
	SET_CONFIGURATION = 0x09
};


enum class INTERFACE_REQ {
	GET_STATUS = 0x00,
	CLEAR_FEATURE = 0x01,
	SET_FEATURE = 0x03,
	GET_INTERFACE = 0x0A,
	SET_INTERFACE = 0x11
};


enum class ENDPOINT_REQ {
	GET_STATUS = 0x00,
	CLEAR_FEATURE = 0x01,
	SET_FEATURE = 0x03,
	SYNCH_FRAME = 0x12
};


enum class DESCRIPTOR_TYPE {
	DEVICE = 0x1,
	CONFIGURATION = 0x2,
	STRING = 0x3,
	INTERFACE = 0x4,
	ENDPOINT = 0x5,
	DEVICE_QUALIFIER = 0x6
};


typedef struct {
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
} usb_device_endpoint0_request;


usb_descriptor_device_registers_t usb::EPDESCTBL[2];
usb_device_endpoint0_request EP0REQ;

static uint8_t outBuf1[8];


extern "C" {


	void USB_Handler() {
		if (USB_REGS->DEVICE.DEVICE_ENDPOINT[0].USB_EPINTFLAG) {
			controlHandler();
		}
	}
}


static void controlHandler() {
	if (USB_REGS->DEVICE.USB_INTFLAG & USB_DEVICE_INTFLAG_EORST_Msk) { // Process USB reset
		USB_REGS->DEVICE.USB_INTFLAG = USB_DEVICE_INTFLAG_EORST(1); // Clear pending interrupt
		USB_REGS->DEVICE.DEVICE_ENDPOINT[0].USB_EPINTENSET = USB_DEVICE_EPINTENSET_RXSTP(1) // Enable endpoint interrupt
						| USB_DEVICE_EPINTENSET_TRCPT0(1) // Enable OUT endpoint interrupt
						| USB_DEVICE_EPINTENSET_TRCPT1(1); // Enable IN endpoint interrupt
		EPDESCTBL[0].DEVICE_DESC_BANK[0].USB_ADDR = (uint32_t) & EP0REQ;
	}

	if (USB_REGS->DEVICE.DEVICE_ENDPOINT[0].USB_EPINTFLAG & USB_DEVICE_EPINTFLAG_RXSTP_Msk) { // Process SETUP
		USB_REGS->DEVICE.DEVICE_ENDPOINT[0].USB_EPINTFLAG = USB_DEVICE_EPINTFLAG_RXSTP(1); // Clear pending interrupt
		uint8_t recipient = BMREQUESTTYPE_RECIPIENT(EP0REQ.bmRequestType);
		switch (recipient) {
			case (uint8_t)REQ_RECIPIENT::DEVICE:
				deviceRequestHandler();
				break;
		}
		USB_REGS->DEVICE.DEVICE_ENDPOINT[0].USB_EPSTATUSSET = USB_DEVICE_EPSTATUS_BK1RDY(1);
		USB_REGS->DEVICE.DEVICE_ENDPOINT[0].USB_EPSTATUSCLR = USB_DEVICE_EPSTATUS_BK0RDY(1);
	}

	if (USB_REGS->DEVICE.DEVICE_ENDPOINT[0].USB_EPINTFLAG & USB_DEVICE_EPINTFLAG_TRCPT0_Msk) {
		USB_REGS->DEVICE.DEVICE_ENDPOINT[0].USB_EPINTFLAG = USB_DEVICE_EPINTFLAG_TRCPT0(1); // Clear pending interrupt
		USB_REGS->DEVICE.DEVICE_ENDPOINT[0].USB_EPSTATUSSET = USB_DEVICE_EPSTATUS_BK1RDY(1);
	}

	if (USB_REGS->DEVICE.DEVICE_ENDPOINT[0].USB_EPINTFLAG & USB_DEVICE_EPINTFLAG_TRCPT1_Msk) {
		USB_REGS->DEVICE.DEVICE_ENDPOINT[0].USB_EPINTFLAG = USB_DEVICE_EPINTFLAG_TRCPT1(1); // Clear pending interrupt
		if (USB_REGS->DEVICE.USB_DADD && !(USB_REGS->DEVICE.USB_DADD & USB_DEVICE_DADD_ADDEN_Msk)) {
			USB_REGS->DEVICE.USB_DADD |= USB_DEVICE_DADD_ADDEN(1);
		}
	}
}


static void deviceRequestHandler() {
	uint8_t descType = WVALUE_TYPE(EP0REQ.wValue);
	switch (EP0REQ.bRequest) {
		case (uint8_t)DEVICE_REQ::GET_DESCRIPTOR:
			switch (descType) {
				case (uint8_t)DESCRIPTOR_TYPE::DEVICE:
					EPDESCTBL[0].DEVICE_DESC_BANK[1].USB_ADDR = (uint32_t) & DESCRIPTOR_DEVICE;
					EPDESCTBL[0].DEVICE_DESC_BANK[1].USB_PCKSIZE = DESCRIPTOR_DEVICE.bLength
									| USB_DEVICE_PCKSIZE_AUTO_ZLP(1);
					break;
				case (uint8_t)DESCRIPTOR_TYPE::CONFIGURATION:
					EPDESCTBL[0].DEVICE_DESC_BANK[1].USB_ADDR = (uint32_t) & DESCRIPTOR_CONFIGURATION + WVALUE_IDX(EP0REQ.wValue);
					EPDESCTBL[0].DEVICE_DESC_BANK[1].USB_PCKSIZE =
									MIN(DESCRIPTOR_CONFIGURATION[WVALUE_IDX(EP0REQ.wValue)].wTotalLength, EP0REQ.wLength)
									| USB_DEVICE_PCKSIZE_AUTO_ZLP(1);
					break;
				case (uint8_t)DESCRIPTOR_TYPE::STRING:
				{
					uint8_t idx = WVALUE_IDX(EP0REQ.wValue);
					if (idx <= 2) {
						EPDESCTBL[0].DEVICE_DESC_BANK[1].USB_ADDR = (uint32_t) & (DESCRIPTOR_STRING[idx]);
						EPDESCTBL[0].DEVICE_DESC_BANK[1].USB_PCKSIZE = DESCRIPTOR_STRING[idx].bLength
										| USB_DEVICE_PCKSIZE_AUTO_ZLP(1);
					} else {
						USB_REGS->DEVICE.DEVICE_ENDPOINT[0].USB_EPSTATUSSET = USB_DEVICE_EPSTATUSCLR_STALLRQ1(1);
					}
				}
					break;
				case (uint8_t)DESCRIPTOR_TYPE::DEVICE_QUALIFIER:
					USB_REGS->DEVICE.DEVICE_ENDPOINT[0].USB_EPSTATUSSET = USB_DEVICE_EPSTATUSCLR_STALLRQ1(1);
					break;
			}
			USB_REGS->DEVICE.DEVICE_ENDPOINT[0].USB_EPSTATUSSET = USB_DEVICE_EPSTATUS_BK1RDY(1);
			break;
		case (uint8_t)DEVICE_REQ::SET_ADDRESS:
			USB_REGS->DEVICE.USB_DADD = WVALUE_ADD(EP0REQ.wValue);
			EPDESCTBL[0].DEVICE_DESC_BANK[1].USB_PCKSIZE = USB_DEVICE_PCKSIZE_AUTO_ZLP(1);
			break;
		case (uint8_t)DEVICE_REQ::SET_CONFIGURATION:
			EPDESCTBL[0].DEVICE_DESC_BANK[1].USB_PCKSIZE = USB_DEVICE_PCKSIZE_AUTO_ZLP(1);
			enableEndpoints(EP0REQ.wValue);
			break;
	}
}


static void enableEndpoints(uint8_t configurationNumber) {
	USB_REGS->DEVICE.DEVICE_ENDPOINT[1].USB_EPCFG = USB_DEVICE_EPCFG_EPTYPE0(0x4) // Configure endpoint 1 as interrupt out
					| USB_DEVICE_EPCFG_EPTYPE1(0x4); // Configure endpoint 1 as interrupt in
	EPDESCTBL[1].DEVICE_DESC_BANK[0].USB_ADDR = (uint32_t) & outBuf1;
}


void usb::init() {
	uint32_t calibration = *((uint32_t*)0x00806020);

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


void usb::write(const uint8_t* data, uint8_t len) {
	EPDESCTBL[1].DEVICE_DESC_BANK[1].USB_ADDR = (uint32_t)data;
	EPDESCTBL[1].DEVICE_DESC_BANK[1].USB_PCKSIZE = len | USB_DEVICE_PCKSIZE_AUTO_ZLP(1);
	USB_REGS->DEVICE.DEVICE_ENDPOINT[1].USB_EPSTATUSSET = USB_DEVICE_EPSTATUS_BK1RDY(1);
}


void usb::read(uint8_t* data, uint8_t len) {
	memcpy(data, outBuf1, MIN(len, sizeof (outBuf1)));
	USB_REGS->DEVICE.DEVICE_ENDPOINT[1].USB_EPSTATUSCLR = USB_DEVICE_EPSTATUS_BK0RDY(1);
}
