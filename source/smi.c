#include "smi.h"
#include "timer.h"
#include "W7500x_gpio.h"

void smi_init()
{
	/* Initialisiere MDC und MDIO */
	GPIO_InitTypeDef GPIO_InitDef;  
	GPIO_StructInit(&GPIO_InitDef); // init the structure	
	GPIO_InitDef.GPIO_Pin = MDC_PIN | MDIO_PIN;
	GPIO_InitDef.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOB, &GPIO_InitDef);

	PAD_AFConfig(PAD_PB, MDIO_PIN, PAD_AF1);  
	PAD_AFConfig(PAD_PB, MDC_PIN, PAD_AF1);  

	/* Initialisiere PHY Reset Pin */
	GPIO_InitDef.GPIO_Pin = PHY_RESET_PIN;
	GPIO_Init(GPIOA, &GPIO_InitDef);

	PAD_AFConfig(PAD_PA, PHY_RESET_PIN, PAD_AF1);
	GPIO_WriteBit(GPIOA, PHY_RESET_PIN, 1);
}

/**
 * @brief Toggelt den MDC Pin einmal
 */
void smi_clk_toggle() {
		GPIO_WriteBit(GPIOB, MDC_PIN, 1);
//		delay_us(5);
		GPIO_WriteBit(GPIOB, MDC_PIN, 0);
//		delay_us(5);
}
/**
 * @brief Gibt die übergebenen Daten am SMI Bus aus
 *
 * @param data     zu sendende Daten
 * @param len      Länge der Daten in Bits
 */
void smi_output(uint32_t data, int len) {
	while(len--) {
		/* Daten rausschreiben... */
		GPIO_WriteBit(GPIOB, MDIO_PIN, (data  & (1<<len)) > 0);
		/* Und die Clk einmal toggeln */
		smi_clk_toggle();
	}
}

/**
 * @brief Liest ein DWord vom SMI Bus ein
 *
 * @return gelesene Daten
 */
uint16_t smi_input() {
	uint16_t i=16, data=0;

	while(i--) {
		data <<= 1;
		/* Erst die Clk einmal toggeln */
		smi_clk_toggle();
		/* Dann die Daten einlesen */
		data |= GPIO_ReadInputDataBit(GPIOB, MDIO_PIN);
	}
	return data;
}

/**
 * @brief Liest ein standard SMI Register (0-0x1f)
 *
 * @param phy_addr      SMI Adresse des PHYs
 * @param phy_reg_addr  Adresse des PHY Registers
 *
 * @return              Wert des PHY Registers
 */
uint16_t smi_read(uint8_t phy_addr, uint8_t phy_reg_addr) {
	uint16_t phy_reg_val;

	GPIOB->OUTENSET |= MDIO_PIN;

	/* Preamble -> Toggle MDC 32 times while MDIO is high */
	GPIO_WriteBit(GPIOB, MDIO_PIN, 1);
	for(int i=0; i<32; i++)
		smi_clk_toggle();
	/**
	 * TODO:
	 * Instead of doing this manually, use smi_output to send the preamble
	 */

	smi_output(START, 2);
	smi_output(READ, 2);
	smi_output(phy_addr, 5);
	smi_output(phy_reg_addr, 5);

	/* Turnaround */
	GPIOB->OUTENCLR = MDIO_PIN;
//	delay_us(1);
	smi_clk_toggle();

	/* Daten Einlesen */
	phy_reg_val = smi_input();

	/* IDLE */
	GPIOB->OUTENSET |= MDIO_PIN;
	smi_clk_toggle();

	return phy_reg_val;
}

/**
 * @brief Schreibt ein standard SMI Register (0-0x1f)
 *
 * @param phy_addr      SMI Adresse des PHYs
 * @param phy_reg_addr  Adresse des PHY Registers
 * @param phy_reg_val   Wert zum Schreiben
 */
void smi_write(uint8_t phy_addr, uint16_t phy_reg_addr, uint16_t phy_reg_val) {
	GPIOB->OUTENSET |= MDIO_PIN;

	/* Preamble -> Toggle MDC 32 times while MDIO is high */
	GPIO_WriteBit(GPIOB, MDIO_PIN, 1);
	for(int i=0; i<32; i++)
		smi_clk_toggle();
	/**
	 * TODO:
	 * Instead of doing this manually, use smi_output to send the preamble
	 */

	smi_output(START, 2);
	smi_output(WRITE, 2);
	smi_output(phy_addr, 5);
	smi_output(phy_reg_addr, 5);

	/* Turnaround */
	smi_output(GAP, 2);

	/* Daten Einlesen */
	smi_output(phy_reg_val, 16);

	/* IDLE */
	GPIOB->OUTENSET |= MDIO_PIN;
	smi_clk_toggle();
}
