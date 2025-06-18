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

//	pSensor->state = MCP23008_IDLE;
//	pSensor->regAddress = 0;
//	pSensor->connected = 0;
//
//
//  char t;
//  char addr = 0x80 | (pSensor->address << 1);
//
//  printf ("init power sensor: %d\n", pSensor->address);
//
//
//  data3[0] = 0x00; // cfg - reset
//  data3[1] = 0x80;
//  data3[2] = 0x00;
//  t = HAL_I2C_Master_Transmit(pSensor->hi2c, addr , data3, 3, 100);
////  printf ("write t: %d\n", t);
//
//
//
//  data3[0] = 0x00; // cfg - config
//  data3[1] = 0x45;
//  data3[2] = 0x27;
//  t = HAL_I2C_Master_Transmit(pSensor->hi2c, addr, data3, 3, 100);
//  printf ("write t: %d\n", t);
// */
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


//  uint16_t addr = 0x40 | (ic->hw_address << 1);
//  uint16_t readAddr = 0x41 | (ic->hw_address << 1);
////  char s;
////
////
////
//  HAL_I2C_Master_Transmit(ic->hi2c, addr, &reg, 1, 100);
//  retval = HAL_I2C_Master_Receive(ic->hi2c, readAddr, MCP23008_recvBuffer, 1, 100);
////  s = recvBuffer[1];
////  recvBuffer[1] = recvBuffer[0];
////  recvBuffer[0] = s;
////
////
////  return *((int16_t *)recvBuffer);
////
  return (uint16_t)retval;
}

//==============================================================================
int16_t MCP23008_Write_Reg(MCP23008_t *ic, uint8_t reg)
{
  uint16_t retval;
  uint16_t addr = 0x40 | (ic->addr << 1);
  uint16_t readAddr = 0x41 | (ic->addr << 1);
//  char s;
//
//
//
  HAL_I2C_Master_Transmit(ic->pInstance, addr, &reg, 1, 100);
  retval = HAL_I2C_Master_Receive(ic->pInstance, readAddr, MCP23008_recvBuffer, 2, 100);

  return retval;
}

//==============================================================================
void MCP23008_printRegs(MCP23008_t *ic)
{
//
//  char addr = 0x80 | (pSensor->address << 1);
//  char readAddr = 0x81 | (pSensor->address << 1);
//  uint8_t regAddress = 0;
//
//  char t;
//
//  int16_t *v = (int16_t *)recvBuffer;
//  char s;
//
//
////  printf ("sensor %d regs:---------\n", pSensor->address);
////  t = HAL_I2C_Master_Transmit(pSensor->hi2c, addr, &regAddress, 1, 100);
////  t = HAL_I2C_Master_Receive(pSensor->hi2c, readAddr, recvBuffer, 2, 100);
// // printf ("cfg: %02X %02X\n", recvBuffer[0], recvBuffer[1]);
//
//  regAddress = 0;
//  t = HAL_I2C_Master_Transmit(pSensor->hi2c, addr, &regAddress, 1, 100);
//  t = HAL_I2C_Master_Receive(pSensor->hi2c, readAddr, recvBuffer, 2, 100);
//  printf ("mask: %02X %02X\n", recvBuffer[0], recvBuffer[1]);
//
//
//  regAddress = 1;
//  t = HAL_I2C_Master_Transmit(pSensor->hi2c, addr, &regAddress, 1, 100);
//  t = HAL_I2C_Master_Receive(pSensor->hi2c, readAddr, recvBuffer, 2, 100);
//  s = recvBuffer[1];
//  recvBuffer[1] = recvBuffer[0];
//  recvBuffer[0] = s;
//  printf ("shunt voltage: %02X %02X  %d  %f mV\n", recvBuffer[0], recvBuffer[1], *v, (float)*v * .0025);
//
//  regAddress = 2;
//  t = HAL_I2C_Master_Transmit(pSensor->hi2c, addr, &regAddress, 1, 100);
//  t = HAL_I2C_Master_Receive(pSensor->hi2c, readAddr, recvBuffer, 2, 100);
//  s = recvBuffer[1];
//  recvBuffer[1] = recvBuffer[0];
//  recvBuffer[0] = s;
//  printf ("bus voltage: %02X %02X  %d  %f V\n", recvBuffer[0], recvBuffer[1], *v, (float)*v * 0.00125);
//
//
//  regAddress = 3;
//  t = HAL_I2C_Master_Transmit(pSensor->hi2c, addr, &regAddress, 1, 100);
//  t = HAL_I2C_Master_Receive(pSensor->hi2c, readAddr, recvBuffer, 2, 100);
//  s = recvBuffer[1];
//  recvBuffer[1] = recvBuffer[0];
//  recvBuffer[0] = s;
//  printf ("power: %02X %02X  %d  %f\n", recvBuffer[0], recvBuffer[1], *v, (float)*v * 0.0025);
//
//
//  regAddress = 4;
//  t = HAL_I2C_Master_Transmit(pSensor->hi2c, addr, &regAddress, 1, 100);
//  t = HAL_I2C_Master_Receive(pSensor->hi2c, readAddr, recvBuffer, 2, 100);
//  s = recvBuffer[1];
//  recvBuffer[1] = recvBuffer[0];
//  recvBuffer[0] = s;
//
//  printf ("current: %02X %02X  %d  %f\n", recvBuffer[0], recvBuffer[1], *v, (float)*v * 0.0001);
//
//
}


//==============================================================================
void MCP23008_process(MCP23008_t *ic)
{
//
//  // в ИЛ-114 убрано чтение датчиков через DMA, для надёжности, поскольку датчиков всего 6 и
//  // на этой шине I2C еще установлен расширитель GPIO
//  switch (pSensor->state){
//    case MCP23008_READ_IDREG_START:
//      if (pSensor->hi2c->State == HAL_I2C_STATE_READY){
//        pSensor->regAddress = 0xFE;
//
//
//        HAL_I2C_Master_Transmit (pSensor->hi2c, (uint16_t)(0x80 | (pSensor->address << 1)), &pSensor->regAddress, 1, 100);
//        HAL_I2C_Master_Receive  (pSensor->hi2c, (uint16_t)(0x81 | (pSensor->address << 1)), pSensor->regData, 2, 100);
//
//
//        int16_t *v = (int16_t *)pSensor->regData;
//        pSensor->chipID = *v;
//        if (pSensor->chipID == (uint16_t)0x4954) pSensor->connected = 1;
//        else pSensor->connected = 0;
//
//
//        pSensor->state = MCP23008_READ_VREG_START;
//
//
//      }
//      break;
//    /*case MCP23008_READ_IDREG:
//      if (pSensor->hi2c->State == HAL_I2C_STATE_READY){
//        HAL_I2C_Master_Receive_DMA (pSensor->hi2c, (uint16_t)(0x81 | (pSensor->address << 1)), pSensor->regData, 2);
//
//        pSensor->state = MCP23008_READ_VREG_START;
//      }
//
//      break;*/
//    case MCP23008_READ_VREG_START:
//      if (pSensor->hi2c->State == HAL_I2C_STATE_READY){
//
//        pSensor->regAddress = 0x02;
//        HAL_I2C_Master_Transmit (pSensor->hi2c, (uint16_t)(0x80 | (pSensor->address << 1)), &pSensor->regAddress, 1, 100);
//        HAL_I2C_Master_Receive (pSensor->hi2c, (uint16_t)(0x81 | (pSensor->address << 1)), pSensor->regData, 2, 100);
//
//
//        int16_t *v = (int16_t *)pSensor->regData;
//        char s;
//        s = pSensor->regData[1];
//        pSensor->regData[1] = pSensor->regData[0];
//        pSensor->regData[0] = s;
//
//        pSensor->V = *v;
//
//
//        pSensor->state = MCP23008_READ_CREG_START;
//
//
//      }
//      break;
///*    case MCP23008_READ_VREG:
//      if (pSensor->hi2c->State == HAL_I2C_STATE_READY){
//        HAL_I2C_Master_Receive_DMA (pSensor->hi2c, (uint16_t)(0x81 | (pSensor->address << 1)), pSensor->regData, 2);
//
//        pSensor->state = MCP23008_READ_CREG_START;
//      }
//
//      break;*/
//    case MCP23008_READ_CREG_START:
//      if (pSensor->hi2c->State == HAL_I2C_STATE_READY){
//
//
//        pSensor->regAddress = 0x04;
//        HAL_I2C_Master_Transmit (pSensor->hi2c, (uint16_t)(0x80 | (pSensor->address << 1)), &pSensor->regAddress, 1, 100);
//        HAL_I2C_Master_Receive (pSensor->hi2c, (uint16_t)(0x81 | (pSensor->address << 1)), pSensor->regData, 2, 100);
//
//
//        int16_t *v = (int16_t *)pSensor->regData;
//        char s;
//        s = pSensor->regData[1];
//        pSensor->regData[1] = pSensor->regData[0];
//        pSensor->regData[0] = s;
//
//        pSensor->C = *v;
//
//
//        pSensor->state = MCP23008_READ_PREG_START;
//
//
//      }
//      break;
///*    case MCP23008_READ_CREG:
//      if (pSensor->hi2c->State == HAL_I2C_STATE_READY){
//        HAL_I2C_Master_Receive_DMA (pSensor->hi2c, (uint16_t)(0x81 | (pSensor->address << 1)), pSensor->regData, 2);
//
//        pSensor->state = MCP23008_READ_PREG_START;
//      }
//
//      break;*/
//    case MCP23008_READ_PREG_START:
//      if (pSensor->hi2c->State == HAL_I2C_STATE_READY){
//
//
//        pSensor->regAddress = 0x03;
//        HAL_I2C_Master_Transmit (pSensor->hi2c, (uint16_t)(0x80 | (pSensor->address << 1)), &pSensor->regAddress, 1, 100);
//        HAL_I2C_Master_Receive (pSensor->hi2c, (uint16_t)(0x81 | (pSensor->address << 1)), pSensor->regData, 2, 100);
//
//        int16_t *v = (int16_t *)pSensor->regData;
//        char s;
//        s = pSensor->regData[1];
//        pSensor->regData[1] = pSensor->regData[0];
//        pSensor->regData[0] = s;
//
//        pSensor->P = *v;
//
//
//        pSensor->state = MCP23008_READ_COMPLETE;
//
//
//      }
//      break;
//    /*case MCP23008_READ_PREG:
//      if (pSensor->hi2c->State == HAL_I2C_STATE_READY){
//        HAL_I2C_Master_Receive_DMA (pSensor->hi2c, (uint16_t)(0x81 | (pSensor->address << 1)), pSensor->regData, 2);
//
//        pSensor->state = MCP23008_BUSY;
//      }
//
//      break;
//    case MCP23008_BUSY:
//      if (pSensor->hi2c->State == HAL_I2C_STATE_READY){
//
//
//
//        pSensor->state = MCP23008_READ_COMPLETE;
//      }
//
//      break;*/
//    case MCP23008_IDLE:
//
//      pSensor->state = MCP23008_READ_IDREG_START;
//      break;
//
//  }
//
}

//==============================================================================
