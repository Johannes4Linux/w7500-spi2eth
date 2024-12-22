#include "smi.h"
#include "timer.h"
#include "W7500x_gpio.h"
#include "W7500x_ssp.h"

void smi_init()
{
	SSP_InitTypeDef SSPx_InitStructure;

	/* SPI configuration */
	SSP_StructInit(&SSPx_InitStructure);
	SSPx_InitStructure.SSP_DataSize = SSP_DataSize_16b;
	SSPx_InitStructure.SSP_Mode  = SSP_Mode_Master;
	SSP_Init(SSP0, &SSPx_InitStructure);
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
	uint16_t smi;
	smi = (START << 14) | (READ << 12) | (phy_addr << 7) | (phy_reg_addr << 2);

	/* Empty buffer */
	while (SSP_GetFlagStatus(SSP0, SSP_FLAG_RNE));
	SSP_ReceiveData(SSP0);

	SSP0->DR = 0xffff;
	SSP0->DR = 0xffff;
	SSP0->DR = smi;
	SSP0->DR = 0xffff;

	while (SSP_GetFlagStatus(SSP0, SSP_FLAG_BSY));
	while (!SSP_GetFlagStatus(SSP0, SSP_FLAG_RNE));

	for (int i=0; i<3; i++)
		SSP_ReceiveData(SSP0);

	while (!SSP_GetFlagStatus(SSP0, SSP_FLAG_RNE));
	return SSP_ReceiveData(SSP0);
}

/**
 * @brief Schreibt ein standard SMI Register (0-0x1f)
 *
 * @param phy_addr      SMI Adresse des PHYs
 * @param phy_reg_addr  Adresse des PHY Registers
 * @param phy_reg_val   Wert zum Schreiben
 */
void smi_write(uint8_t phy_addr, uint16_t phy_reg_addr, uint16_t phy_reg_val) {
	uint16_t smi;
	smi = (START << 14) | (WRITE << 12) | (phy_addr << 7) | (phy_reg_addr << 2);

	/* Empty buffer */
	while (SSP_GetFlagStatus(SSP0, SSP_FLAG_RNE));
	SSP_ReceiveData(SSP0);

	SSP0->DR = 0xffff;
	SSP0->DR = 0xffff;
	SSP0->DR = smi;
	SSP0->DR = phy_reg_val;

	while (SSP_GetFlagStatus(SSP0, SSP_FLAG_BSY));
	while (!SSP_GetFlagStatus(SSP0, SSP_FLAG_RNE));

	for (int i=0; i<4; i++)
		SSP_ReceiveData(SSP0);
}
