#ifndef SMI_H
#define SMI_H

#include <stdint.h>

#define MDIO_PIN GPIO_Pin_14
#define MDC_PIN GPIO_Pin_15
#define PHY_RESET_PIN GPIO_Pin_15

#define START		0b01
#define WRITE		0b01
#define READ		0b10
#define GAP		0b10
#define TURNAROUND 	0b11

void smi_init();
/**
 * @brief Toggelt den MDC Pin einmal
 */
void smi_clk_toggle();

/**
 * @brief Sendet einen Wert über's SMI
 *
 * @param val:      zu sendender Wert
 * @param len:      Länge des zu sendenden Werts in Bits
 */
void smi_send_val(int val, int len);

/**
 * @brief Liest einen Wert über SMI
 *
 * @param len       Länge des zu empfangenen Werts
 *
 * @return          Empfangenen Wert
 */
uint16_t smi_get_val(int len);

/**
 * @brief Sendet ein Kommando über SMI
 *
 * @param cmd           zu sendendes Kommando
 * @param phy_addr      Adresse des PHYs
 * @param phy_reg_addr  Adresse des PHY Registers
 */
void smi_send_cmd(uint8_t cmd, uint8_t phy_addr, uint16_t phy_reg_addr);

/**
 * @brief Führt ein Read Kommando über SMI aus
 *
 * @param phy_addr      SMI Adresse des PHYs
 * @param phy_reg_addr  Adresse des PHY Registers
 *
 * @return              Wert des PHY Registers
 */
uint16_t smi_read(uint8_t phy_addr, uint8_t phy_reg_addr);

/**
 * @brief Führt ein Write Kommando über SMI
 *
 * @param phy_addr      SMI Adresse des PHYs
 * @param phy_reg_addr  Adresse des PHY Registers
 * @param phy_reg_val   Wert zum Schreiben
 */
void smi_write(uint8_t phy_addr, uint16_t phy_reg_addr, uint16_t phy_reg_val);

#endif
