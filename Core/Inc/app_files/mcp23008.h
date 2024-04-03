/*
 * mcp23008.h
 *
 *  Created on: 22 июня 2022 г.
 *      Author: lukashinvn
 */

#ifndef INC_APP_FILES_MCP23008_H_
#define INC_APP_FILES_MCP23008_H_

#define	MCP23008_I2C_INSTANCE	&hi2c1  /* задано аппаратно в схеме */
#define MCP23008_I2C_HW_ADDR    0x00  /* задано аппаратно в схеме */
#define MCP23008_I2C_HW_ADDR_WIDTH  0x03  /* ширина адресного пространства (число бит) hw-адреса */


#include "stm32f7xx_hal.h"

#include "i2c.h"

//==============================================================================

enum MCP23008_Regs {
	  regIODIR 		= 0x00,
	  regIPOL		 = 0x01,
	  regGPINTEN	= 0x02,
	  regDEFVAL		= 0x03,
	  regINTCON		= 0x04,
	  regIOCON		= 0x05,
	  regGPPU		= 0x06,
	  regINTF		= 0x07,
	  regINTCAP		= 0x08,
	  regGPIO		= 0x09,
	  regOLAT		= 0x0A,
};


typedef struct {
	I2C_HandleTypeDef 	*pInstance;
	uint16_t    		addr;
	char                GPIOstate;
} MCP23008_t;

//==============================================================================

void MCP23008_init(void);
int16_t MCP23008_Read_Reg(MCP23008_t *ic, uint8_t reg);
int16_t MCP23008_Write_Reg(MCP23008_t *ic, uint8_t reg);
void MCP23008_printRegs(MCP23008_t *ic);
void MCP23008_process(MCP23008_t *ic);

#endif /* INC_APP_FILES_MCP23008_H_ */
