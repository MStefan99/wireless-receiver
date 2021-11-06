#include "device.h"

#include "lib/usb.h"

#define NVMTEMP ((uint32_t*)0x00806030)


/* Clock distribution
 * 
 * OSC16M @ 8MHz
 * |
 * |--> GCLK0 @ 8MHz 
 * |    |
 * |     `-> MCLK @ 8MHz
 * |
 *  `-> GCLK1 @ 250KHz
 *      |
 *       `-> GCLK_ADC @ 250KHz
 * 
 * DFLL48M @ 48MHz
 * |
 *  `-> GCLK2 @ 48MHz
 *      |
 *       `-> GCLK_USB @ 48MHz
 */


extern "C" {


	void SysTick_Handler() {
		// Nothing to do here yet
	}
}


int main() {
	uint32_t calibration = *((uint32_t*)0x00806020);

	// PM config
	PM_REGS->PM_PLCFG = PM_PLCFG_PLSEL_PL2; // Enter PL2
	while (!(PM_REGS->PM_INTFLAG & PM_INTFLAG_PLRDY_Msk)); // Wait for the transition to complete

	// OSCCTRL config
	OSCCTRL_REGS->OSCCTRL_OSC16MCTRL = OSCCTRL_OSC16MCTRL_ENABLE(1) // Enable OSC16M
					| OSCCTRL_OSC16MCTRL_FSEL_8; // Set frequency to 8MHz
	OSCCTRL_REGS->OSCCTRL_DFLLVAL = OSCCTRL_DFLLVAL_COARSE((calibration >> 26u) & 0x3f)
					| OSCCTRL_DFLLVAL_FINE(108); // Load calibration value
	OSCCTRL_REGS->OSCCTRL_DFLLCTRL = OSCCTRL_DFLLCTRL_ENABLE(1) // Enable DFLL48M
					| OSCCTRL_DFLLCTRL_MODE(0); // Run in open-loop mode

	// GLCK config
	GCLK_REGS->GCLK_GENCTRL[1] = GCLK_GENCTRL_GENEN(1) // Enable GCLK 1
					| GCLK_GENCTRL_SRC_OSC16M // Set OSC16M as a source
					| GCLK_GENCTRL_DIVSEL_DIV2 // Set division mode (2^(x+1))
					| GCLK_GENCTRL_DIV(3); // Divide by 16 (2^(3+1))
	GCLK_REGS->GCLK_PCHCTRL[30] = GCLK_PCHCTRL_CHEN(1) // Enable ADC clock
					| GCLK_PCHCTRL_GEN_GCLK1; //Set GCLK1 as a clock source

	GCLK_REGS->GCLK_GENCTRL[2] = GCLK_GENCTRL_GENEN(1) // Enable GCLK 2
					| GCLK_GENCTRL_SRC_DFLL48M // Set DFLL48M as a source
					| GCLK_GENCTRL_OE(1); // Enable clock output
	GCLK_REGS->GCLK_PCHCTRL[4] = GCLK_PCHCTRL_CHEN(1) // Enable USB clock
					| GCLK_PCHCTRL_GEN_GCLK2; //Set GCLK2 as a clock source

	// NVIC config
	__DMB();
	__enable_irq();
	NVIC_SetPriority(USB_IRQn, 3);
	NVIC_EnableIRQ(USB_IRQn);
	NVIC_EnableIRQ(SysTick_IRQn);

	// SysTick config
	SysTick->CTRL = 0;
	SysTick->LOAD = 8000 - 1;
	SysTick->CTRL = SysTick_CTRL_TICKINT_Msk
					| SysTick_CTRL_CLKSOURCE_Msk
					| SysTick_CTRL_ENABLE_Msk;

	// SUPC config
	SUPC_REGS->SUPC_VREF = SUPC_VREF_TSEN(1) // Enable temperature sensor
					| SUPC_VREF_SEL_1V0; // Set 1.0V as a reference

	// ADC config
	ADC_REGS->ADC_REFCTRL = ADC_REFCTRL_REFSEL_INTREF; // Set ADC reference voltage
	ADC_REGS->ADC_INPUTCTRL = ADC_INPUTCTRL_MUXNEG_GND // Set GND as negative input
					| ADC_INPUTCTRL_MUXPOS_TEMP; // Set temperature sensor as positive input
	ADC_REGS->ADC_INTENSET = ADC_INTFLAG_RESRDY(1); // Enable result ready interrupt
	ADC_REGS->ADC_CTRLA = ADC_CTRLA_ENABLE(1); // Enable ADC

	// PORT config
	PORT_REGS->GROUP[0].PORT_DIRSET = 0x3 | 0x1 << 16u;
	PORT_REGS->GROUP[0].PORT_PINCFG[24] = PORT_PINCFG_PMUXEN(1); // Enable mux on pin 24
	PORT_REGS->GROUP[0].PORT_PINCFG[25] = PORT_PINCFG_PMUXEN(1); // Enable mux on pin 25
	PORT_REGS->GROUP[0].PORT_PMUX[12] = PORT_PMUX_PMUXE_G // Mux pin 24 to USB
					| PORT_PMUX_PMUXO_G; // Mux pin 25 to USB

	// USB config
	usb::init();

	// Temperature calibration values
	uint8_t tempR = NVMTEMP[0] & 0xff;
	uint16_t adcR = (NVMTEMP[1] & 0xfff00) >> 8u;
	uint8_t tempH = (NVMTEMP[0] & 0xff0000) >> 12u;
	uint16_t adcH = (NVMTEMP[1] & 0xfff00000) >> 20u;

	while (true) {
		ADC_REGS->ADC_SWTRIG = ADC_SWTRIG_START(1); // Start conversion
		while (!(ADC_REGS->ADC_INTFLAG & ADC_INTFLAG_RESRDY_Msk)) {
			__WFI();
		} // Wait for ADC result
		uint16_t temperature = tempR + ((ADC_REGS->ADC_RESULT - adcR) * (tempH - tempR) / 
						(adcH - adcR));
		usb::write((uint8_t*) & temperature, 2);
		__WFI();
	}

	return 1;
}
