/*******************************************************************************
	Main Source File

	Company:
		Microchip Technology Inc.

	File Name:
		main.c

	Summary:
		This file contains the "main" function for a project.

	Description:
		This file contains the "main" function for a project.  The
		"main" function calls the "SYS_Initialize" function to initialize the state
		machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "device.h"                     // SYS function prototypes

#include "lib/usb.h"


// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

usb_descriptor_device_registers_t usb::EPDESCTBL[2];


usb::usb_descriptor_device usb::DESCRIPTOR_DEVICE = {
	.bDeviceClass = 0x02,
	.bDeviceSubclass = 0x02,
	.bDeviceProtocol = 0x00,
	.bMaxPacketSize = 8,
	.idVendor = 0x04d8,
	.idProduct = 0x0001,
	.bcdDevice = 0x0001,
	.iManufacturer = 0,
	.iProduct = 0,
	.iSerialNumber = 0,
	.bNumConfigurations = 1
};

/* Clock distribution
 * 
 * OSC16M @ 4MHz
 * |
 * |--> GCLK0 @ 4MHz 
 * |    |
 * |     `-> MCLK @ 4MHz
 * |
 *  `-> GCLK1 @ 500KHz
 *      |
 *       `-> GCLK_ADC @ 500KHz
 * 
 * DFLL48M @ 48MHz
 * |
 *  `-> GCLK2 @ 48MHz
 *      |
 *       `-> GCLK_USB @ 48MHz
 */


int main() {
	uint32_t calibration = *((uint32_t*) 0x00806020);

	// PM config
	PM_REGS->PM_PLCFG = PM_PLCFG_PLSEL_PL2; // Enter PL2
	while (!(PM_REGS->PM_INTFLAG & PM_INTFLAG_PLRDY_Msk)); // Wait for the transition to complete

	// OSCCTRL config
	OSCCTRL_REGS->OSCCTRL_OSC16MCTRL = OSCCTRL_OSC16MCTRL_ENABLE(1) // Enable OSC16M
					| OSCCTRL_OSC16MCTRL_FSEL_12; // Set frequency to 12MHz
	OSCCTRL_REGS->OSCCTRL_DFLLVAL = OSCCTRL_DFLLVAL_COARSE((calibration >> 26u) & 0x3f)
					| OSCCTRL_DFLLVAL_FINE(128); // Load calibration value
	OSCCTRL_REGS->OSCCTRL_DFLLCTRL = OSCCTRL_DFLLCTRL_ENABLE(1) // Enable DFLL48M
					| OSCCTRL_DFLLCTRL_MODE(0); // Run in open-loop mode

	// GLCK config
	GCLK_REGS->GCLK_GENCTRL[1] = GCLK_GENCTRL_GENEN(1) // Enable GCLK 1
					| GCLK_GENCTRL_SRC_OSC16M // Set OSC16M as a source
					| GCLK_GENCTRL_DIVSEL_DIV2 // Set division mode (2^(x+1))
					| GCLK_GENCTRL_DIV(2); // Divide by 8 (2^(2+1))
	GCLK_REGS->GCLK_PCHCTRL[30] = GCLK_PCHCTRL_CHEN(1) // Enable ADC clock
					| GCLK_PCHCTRL_GEN_GCLK1; //Set GCLK1 as a clock source

	GCLK_REGS->GCLK_GENCTRL[2] = GCLK_GENCTRL_GENEN(1) // Enable GCLK 2
					| GCLK_GENCTRL_SRC_DFLL48M // Set DFLL48M as a source
					//| GCLK_GENCTRL_DIV(48)  // Temporarily divide by 48
					| GCLK_GENCTRL_OE(1); // Enable clock output
	GCLK_REGS->GCLK_PCHCTRL[4] = GCLK_PCHCTRL_CHEN(1) // Enable USB clock
					| GCLK_PCHCTRL_GEN_GCLK2; //Set GCLK2 as a clock source

	// NVIC config
	__DMB();
	__enable_irq();
	NVIC_SetPriority(USB_IRQn, 3);
	NVIC_EnableIRQ(USB_IRQn);

	// SUPC config
	SUPC_REGS->SUPC_VREF = SUPC_VREF_TSEN(1) // Enable temperature sensor
					| SUPC_VREF_SEL_1V0; // Set 1.0V as a reference

	// ADC config
	ADC_REGS->ADC_REFCTRL = ADC_REFCTRL_REFSEL_INTREF; // Set ADC reference voltage
	ADC_REGS->ADC_INPUTCTRL = ADC_INPUTCTRL_MUXNEG_GND // Set GND as negative input
					| ADC_INPUTCTRL_MUXPOS_TEMP; // Set temperature sensor as positive input
	ADC_REGS->ADC_INTENSET = ADC_INTFLAG_RESRDY(1); // Enable result ready interrupt
	//ADC_REGS->ADC_CTRLA = ADC_CTRLA_ENABLE(1);  // Enable ADC

	// PORT config
	PORT_REGS->GROUP[0].PORT_DIRSET = 0x3;
	PORT_REGS->GROUP[0].PORT_PINCFG[24] = PORT_PINCFG_PMUXEN(1); // Enable mux on pin 24
	PORT_REGS->GROUP[0].PORT_PINCFG[25] = PORT_PINCFG_PMUXEN(1); // Enable mux on pin 25
	PORT_REGS->GROUP[0].PORT_PMUX[12] = PORT_PMUX_PMUXE_G // Mux pin 24 to USB
					| PORT_PMUX_PMUXO_G; // Mux pin 25 to USB
	PORT_REGS->GROUP[0].PORT_PINCFG[16] = PORT_PINCFG_PMUXEN(1); // Enable mux on pin 16
	PORT_REGS->GROUP[0].PORT_PMUX[8] = PORT_PMUX_PMUXE_H; // Mux pin 16 to GCLK
	PORT_REGS->GROUP[0].PORT_PINCFG[23] = PORT_PINCFG_PMUXEN(1); // Enable mux on pin 22
	PORT_REGS->GROUP[0].PORT_PMUX[11] = PORT_PMUX_PMUXO_G; // Mux pin 22 to USB SOF
	
	usb::init();


	while (true) {
		//ADC_REGS->ADC_SWTRIG = ADC_SWTRIG_START(1);  // Start conversion
		//while (!(ADC_REGS->ADC_INTFLAG & ADC_INTFLAG_RESRDY_Msk));  // Wait for ADC result
		// Result will be available in ADC_RESULT
		__WFI();
	}

	return (EXIT_FAILURE);
}


/*******************************************************************************
 End of File
 */

