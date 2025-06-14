#include "W7500x_uart.h"
#include "W7500x_gpio.h"
#include "W7500x_wztoe.h"
#include "W7500x_ssp.h"
#include "smi.h"
#include "timer.h"
#include "print.h"

void recv_u16_spi(uint16_t *val)
{
	uint8_t *ptr = (uint8_t *) val;

	for (int i=1; i>=0; i--) {
		/* Wait for byte to arrive */
		while (!SSP_GetFlagStatus(SSP1, SSP_FLAG_RNE));
		/* Read in byte */
		ptr[i] = (uint8_t) SSP1->DR;
	}
}

void send_u16_spi(uint16_t *val)
{
	uint8_t *ptr = (uint8_t *) val;

	for (int i=1; i>=0; i--) {
		/* Wait for space in TX Buffer */
		while (!SSP_GetFlagStatus(SSP1, SSP_FLAG_TNF));
		/* Read in byte */
		SSP1->DR = (uint8_t) ptr[i];
	}
}

uint8_t PHY_ADDR = 0;

#define IDLE 0x0
#define SET_SMI_OP 0x1
#define GET_SMI 0x2
#define SET_SMI 0x3
#define SET_MAC 0x4
#define GET_MAC 0x5
#define SEND_FRAME 0x6
#define RECV_FRAME 0x7
#define GET_IRQ 0x8
#define RESET_PHY 0x9
#define CHECK_ALIVE 0xA
#define IRQ_TEST 0xB

#define RAW_SOCKET_NR 0
#define S0_TXMEM_BASE 0x46020000
#define S0_RXMEM_BASE 0x46030000

#ifdef SURF5
#define IRQ_PIN GPIO_Pin_9
#define RESET_PIN GPIO_Pin_5
#else
#define IRQ_PIN GPIO_Pin_15
#define RESET_PIN GPIO_Pin_5
#endif

#define SET_IRQ   GPIO_WriteBit(GPIOA, IRQ_PIN, 0)
#define RESET_IRQ GPIO_WriteBit(GPIOA, IRQ_PIN, 1)

#define SET_RESET   GPIO_WriteBit(GPIOA, RESET_PIN, 0)
#define UNSET_RESET GPIO_WriteBit(GPIOA, RESET_PIN, 1)

int main()
{
	volatile uint8_t cmd, irq;
	uint16_t spi_val;
	uint8_t mac_addr[6] = {0x04, 0x04, 0x04, 0x04, 0x04, 0x04}; 
	volatile uint8_t phy_addr, phy_reg, phy_rw;
	volatile uint16_t phy_val;
	GPIO_InitTypeDef GPIO_InitDef;
	SSP_InitTypeDef SSP1_InitStructure;
	uint16_t len;
	uint32_t ptr;

	/*System clock configuration*/
	SystemInit();    

	/* SPI configuration */
	SSP_StructInit(&SSP1_InitStructure);
	SSP1_InitStructure.SSP_DataSize = SSP_DataSize_8b;
	SSP1_InitStructure.SSP_Mode  = SSP_Mode_Slave;
	SSP1_InitStructure.SSP_CPOL  = SSP_CPOL_High;
	SSP1_InitStructure.SSP_CPHA  = SSP_CPHA_2Edge;
	SSP_Init(SSP1, &SSP1_InitStructure);

	/* Init GPIOs for IRQ outputs */
	GPIO_InitDef.GPIO_Pin = (IRQ_PIN);
	GPIO_InitDef.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitDef.GPIO_Pad = Px_PCR_DS_HIGH;
	GPIO_Init(GPIOA, &GPIO_InitDef);
	PAD_AFConfig(PAD_PA, IRQ_PIN, PAD_AF1);

	/* High is IRQ not active */
	RESET_IRQ;

	GPIO_Configuration(GPIOA, RESET_PIN, GPIO_Mode_OUT, PAD_AF1);
	/* Release Reset */
	SET_RESET;
	delay_ms(1);
	UNSET_RESET;
	/* Set PA_00_AFR to Alternate function 1: GPIOA_05 */
	*((volatile uint32_t *) 0x41002014) = 1;

	/* UART2 configuration*/
	S_UART_Init(115200);

	smi_init();

	*(volatile uint32_t *)(0x41003068) = 0x64; //TXD0 - set PAD strengh and pull-up
	*(volatile uint32_t *)(0x4100306C) = 0x64; //TXD1 - set PAD strengh and pull-up
	*(volatile uint32_t *)(0x41003070) = 0x64; //TXD2 - set PAD strengh and pull-up
	*(volatile uint32_t *)(0x41003074) = 0x64; //TXD3 - set PAD strengh and pull-up
	*(volatile uint32_t *)(0x41003050) = 0x64; //TXE  - set PAD strengh and pull-up
	
//	print("Search for a PHY... ");
//	while (smi_read(PHY_ADDR, 0) == 0xffff) {
//		/* Increment the PHY address */
//		PHY_ADDR = (PHY_ADDR + 1) & 0x1f;
//	}
//	print("Done!\n\rFound a PHY at address %d\n\r", PHY_ADDR);
//

	/* Set maximum TX and RX Buffer sizes */
	setSn_TXBUF_SIZE(RAW_SOCKET_NR, 16);
	setSn_RXBUF_SIZE(RAW_SOCKET_NR, 16);

	setSHAR(mac_addr);

	setSn_MR(RAW_SOCKET_NR, Sn_MR_MACRAW);
	setSn_CR(RAW_SOCKET_NR, Sn_CR_OPEN);
	print("Init is done!\n\r");

	while (1) {
		if (SSP_GetFlagStatus(SSP1, SSP_FLAG_RNE)) {
			cmd = (uint8_t) SSP1->DR;
			switch (cmd) {
				case GET_SMI:
					/* Send back two bytes with SMI Data register */
					send_u16_spi((uint16_t *) &phy_val);
					cmd = IDLE;
//					print("GET_SMI: %x\n\r", phy_val);
					break;
				case SET_SMI_OP:
					/* Receive parameters */
					recv_u16_spi(&spi_val);

					/* Parse parameters */
					phy_reg = spi_val & 0x1f;
					phy_addr = (spi_val >> 5) & 0x1f;
					phy_rw = (spi_val >> 10) & 1;

					/* Do access */
					if (phy_rw == 1) 
						smi_write(phy_addr, phy_reg, phy_val);
					else
						phy_val = smi_read(phy_addr, phy_reg);
//					print("SET_SMI_OP spi_val: %x,  phy_addr: %x, reg_addr: %x, rw %d\n\r", spi_val, phy_addr, phy_reg, phy_rw);
					cmd = IDLE;
					break;
				case SET_SMI:
					/* Receive new value for SMI Data register */
					recv_u16_spi((uint16_t *) &phy_val);
					cmd = IDLE;
					//print("SET_SMI %x\n\r", phy_val);
					break;
				case SET_MAC:
					/* Receive new MAC address */
					for (int i=0; i<6; i++) {
						while (!SSP_GetFlagStatus(SSP1, SSP_FLAG_RNE));
						mac_addr[i] = (uint8_t) SSP1->DR;
					}
					/* Set new MAC Addr */
					setSHAR(mac_addr);
					break;
				case GET_MAC:
					/* Send MAC addr */
					for (int i=0; i<6; i++) {
						while (!SSP_GetFlagStatus(SSP1, SSP_FLAG_TNF));
						SSP1->DR = (uint8_t) mac_addr[i];
					}
					break;
				case SEND_FRAME:
					/* Receive size of frame */
					recv_u16_spi(&len);
					ptr = getSn_TX_WR(RAW_SOCKET_NR);
					for (int i=0; i<len; i++) {
						while (!SSP_GetFlagStatus(SSP1, SSP_FLAG_RNE));
						*(volatile uint8_t *)(S0_TXMEM_BASE + (ptr++ & 0xFFFF)) = (uint8_t) SSP1->DR;
					}
					//print("SEND_FRAME_ Len: %d\n\r", len);
					setSn_TX_WR(RAW_SOCKET_NR, ptr);
					setSn_CR(RAW_SOCKET_NR, Sn_CR_SEND);
//					print("Sent\n\r");
					break;
				case RECV_FRAME:
					/* First send length of recv. frame */
					len = getSn_RX_RSR(RAW_SOCKET_NR);
//					print("%d bytes in queue\n\r", len);
					if (len == 0) {
						send_u16_spi(&len);
						break;
					}

					ptr = getSn_RX_RD(RAW_SOCKET_NR);
					uint8_t *lptr = (uint8_t*) &len;
					for (int i=0; i<2; i++) {
						while (!SSP_GetFlagStatus(SSP1, SSP_FLAG_TNF));
						SSP1->DR = lptr[1-i] = *(volatile uint8_t *)(S0_RXMEM_BASE +(ptr++ & 0xFFFF));
					}
					len -= 2;
//					print("head %d bytes\n\r", len);
					setSn_CR(RAW_SOCKET_NR, Sn_CR_RECV);
					while (getSn_CR(RAW_SOCKET_NR));

					/* Send frame data to SPI master */
					for (int i=0; i<len; i++) {
						while (!SSP_GetFlagStatus(SSP1, SSP_FLAG_TNF));
						SSP1->DR = *(volatile uint8_t *)(S0_RXMEM_BASE +(ptr++ & 0xFFFF));

					}
					setSn_RX_RD(RAW_SOCKET_NR, ptr);

					setSn_CR(RAW_SOCKET_NR, Sn_CR_RECV);
					while (getSn_CR(RAW_SOCKET_NR));

//					print("Recv. %d bytes\n\r", len);
					break;
				case GET_IRQ:
					/* Send back Pending Interrupts */
					while (!SSP_GetFlagStatus(SSP1, SSP_FLAG_TNF));
					SSP1->DR = (uint8_t) irq;
					/* Clear IRQs */
					setSn_ICR(RAW_SOCKET_NR, irq);
					/* Reset GPIOs */
					RESET_IRQ;
					//print("GET_IRQ: %x\n\r", irq);
					break;
				case RESET_PHY:
					SET_RESET;
					delay_ms(1);
					UNSET_RESET;
					break;
				case CHECK_ALIVE:
					/* Send back Pending Interrupts */
					while (!SSP_GetFlagStatus(SSP1, SSP_FLAG_TNF));
					SSP1->DR = (uint8_t) 0x73;
					break;
				case IRQ_TEST:
					SET_IRQ;
					break;
				default:
					break;
			}
		}

		/* Check for IRQs */
		irq = getSn_IR(RAW_SOCKET_NR) & 0x14;
		if (irq & Sn_IR_SENDOK)
			SET_IRQ;
		if (irq & Sn_IR_RECV)
			SET_IRQ;
	}
}
