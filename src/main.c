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
#include "device.h"                // SYS function prototypes


// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main() {
	// PM config
	PM_REGS->PM_PLCFG = PM_PLCFG_PLSEL_PL2;  // Enter PL2
	while (!(PM_REGS->PM_INTFLAG & PM_INTFLAG_PLRDY_Msk));  // Wait for the transition to complete
	PM_REGS->PM_INTFLAG = PM_INTFLAG_PLRDY(1);  // Clear the flag


	// GLCK config
	GCLK_REGS->GCLK_GENCTRL[1] = GCLK_GENCTRL_GENEN(1)  // Enable GCLK 1
					| GCLK_GENCTRL_SRC_OSC16M  // Set OSC16M as a source
					| GCLK_GENCTRL_DIVSEL_DIV2  // Set division mode (2^(x+1))
					| GCLK_GENCTRL_DIV(2);  // Divide by 8 (2^(2+1))
	GCLK_REGS->GCLK_PCHCTRL[30] = GCLK_PCHCTRL_CHEN(1)  // Enable ADC clock
					| GCLK_PCHCTRL_GEN_GCLK1;  //Set GCLK1 as a clock source
	GCLK_REGS->GCLK_PCHCTRL[4] = GCLK_PCHCTRL_CHEN(1)  // Enable USB clock
					| GCLK_PCHCTRL_GEN_GCLK0;  //Set GCLK0 as a clock source


	// SUPC config
	SUPC_REGS->SUPC_VREF = SUPC_VREF_TSEN(1)  // Enable temperature sensor
					| SUPC_VREF_SEL_1V0;  // Set 1.0V as a reference


	// ADC config
	ADC_REGS->ADC_REFCTRL = ADC_REFCTRL_REFSEL_INTREF;  // Set ADC reference voltage
	ADC_REGS->ADC_INPUTCTRL = ADC_INPUTCTRL_MUXNEG_GND  // Set GND as negative input
					| ADC_INPUTCTRL_MUXPOS_TEMP;  // Set temperature sensor as positive input
	ADC_REGS->ADC_CTRLA = ADC_CTRLA_ENABLE(1);  // Enable ADC
	
	// PORT config
	PORT_REGS->GROUP[0].PORT_PINCFG[24] = PORT_PINCFG_PMUXEN(1);  // Enable mux on pin 24
	PORT_REGS->GROUP[0].PORT_PINCFG[25] = PORT_PINCFG_PMUXEN(1);  // Enable mux on pin 25
	PORT_REGS->GROUP[0].PORT_PMUX[12] = PORT_PMUX_PMUXE_G  // Mux pin 24 to USB
					| PORT_PMUX_PMUXO_G;  // Mux pin 25 to USB
	
	// USB config
	uint32_t calibration = *((uint32_t*)0x00806020);
	USB_REGS->DEVICE.USB_PADCAL = USB_PADCAL_TRANSN(calibration >> 13u & 0x1f)
					| USB_PADCAL_TRANSP(calibration >> 18u & 0x1f)
					| USB_PADCAL_TRIM(calibration >> 23u & 0x7);  // USB pad calibration
	USB_REGS->DEVICE.USB_CTRLA = USB_CTRLA_ENABLE(1)  // Enable USB
					| USB_CTRLA_MODE_DEVICE;  // Enable in device mode
	USB_REGS->DEVICE.USB_CTRLB = USB_DEVICE_CTRLB_DETACH(0);  // Attach to host


	while (true) {
		ADC_REGS->ADC_SWTRIG = ADC_SWTRIG_START(1);  // Start conversion
		while (!(ADC_REGS->ADC_INTFLAG & ADC_INTFLAG_RESRDY_Msk));  // Wait for ADC result
		// Result will be available in ADC_RESULT
		__WFI();
	}

	return (EXIT_FAILURE);
}


/*******************************************************************************
 End of File
 */

