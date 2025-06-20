/*
 * mcp23008.c
 *
 *  Created on: 22 июня 2022 г.
 *      Author: lukashinvn
 */


#include "mcp23008.h"

#include <stdio.h>

uint8_t MCP23008_data[3];

uint8_t MCP23008_recvBuffer[2];

MCP23008_t	ic_mcp23008;

//==============================================================================
void MCP23008_init(void)
{
	ic_mcp23008.pInstance = MCP23008_I2C_INSTANCE;
	ic_mcp23008.addr = MCP23008_I2C_HW_ADDR | 0x20;	// из даташита
}

//==============================================================================
int16_t MCP23008_Read_Reg(MCP23008_t *ic, uint8_t reg)
{
  u8_t retval = 0;

  u8_t addrRead = (ic->addr << 1) & 0xFE;
  u8_t addrWrite = (ic->addr << 1) | 0x01;

  u8_t opcd = (ic->addr) << 1 | 0x21;

  HAL_I2C_Master_Transmit(ic->pInstance, addrWrite, &opcd, 1, 100);
  HAL_I2C_Master_Transmit(ic->pInstance, addrRead, &reg, 1, 100);
  HAL_I2C_Master_Receive(ic->pInstance, addrRead, &retval, 1, 100);
  return (uint16_t)retval;
}

//==============================================================================
int16_t MCP23008_Write_Reg(MCP23008_t *ic, uint8_t reg)
{
  uint16_t retval;
  uint16_t addr = 0x40 | (ic->addr << 1);
  uint16_t readAddr = 0x41 | (ic->addr << 1);

  HAL_I2C_Master_Transmit(ic->pInstance, addr, &reg, 1, 100);
  retval = HAL_I2C_Master_Receive(ic->pInstance, readAddr, MCP23008_recvBuffer, 2, 100);

  return retval;
}

//==============================================================================
void MCP23008_printRegs(MCP23008_t *ic)
{
}


//==============================================================================
void MCP23008_process(MCP23008_t *ic)
{
//  // в ИЛ-114 убрано чтение датчиков через DMA, для надёжности, поскольку датчиков всего 6 и
//  // на этой шине I2C еще установлен расширитель GPIO
}

//==============================================================================
