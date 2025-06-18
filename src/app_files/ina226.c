/*
 * ina226.c
 *
 *  Created on: 24 янв. 2021 г.
 *      Author: zhuchenkovao
 */



#include "ina226.h"

#include <stdio.h>

/// @brief      Задержка после перезапуска INA226 (мс)
#define INA_RESET_DELAY 0


uint8_t data3[3];

uint8_t recvBuffer[2];


//==============================================================================
void T_INA226_init(T_INA226 *pSensor)
{



  pSensor->chipID = 0;
  pSensor->P = 0;
  pSensor->V = 0;
  pSensor->C = 0;


  pSensor->state = ST_INA226_IDLE;
  pSensor->regAddress = 0;
  pSensor->connected = 0;


  char t;
  char addr = 0x80 | (pSensor->address << 1);

  printf ("init power sensor: %d\n", pSensor->address);


  data3[0] = 0x00; // cfg - reset
  data3[1] = 0x80;
  data3[2] = 0x00;
  t = HAL_I2C_Master_Transmit(pSensor->hi2c, addr , data3, 3, 100);
//  printf ("write t: %d\n", t);

  HAL_Delay(INA_RESET_DELAY);

  data3[0] = 0x00; // cfg - config
  data3[1] = 0x45;
  data3[2] = 0x27;
  t = HAL_I2C_Master_Transmit(pSensor->hi2c, addr, data3, 3, 100);
//  printf ("write t: %d\n", t);
  // */



  /*// рассчет значений для R=10mOhm, Imax= 6,5536A, Wlimit=40W
  // 28V * 1.428A = 40W limit
  // 40W / 0.005 W/bit = 8000 (1F40h)
  data3[0] = 0x07; // limit
  data3[1] = 0x1F;
  data3[2] = 0x40;
  t = HAL_I2C_Master_Transmit(pSensor->hi2c, addr, data3, 3, 100);
  printf ("write t: %d\n", t);

  // Current_LSB = 6,5536/32768 = 0,0002 A/bit
  // cal = 0,00512 / ( 0,0002 A/div * 0,01 Ohm) = 2560 (A00h)
  // power_LSB = 0,0002 * 25 = 0.005 W/bit

  data3[0] = 0x05; // cal
  data3[1] = 0x0A;// 0x0A;
  data3[2] = 0x00;
  t = HAL_I2C_Master_Transmit(pSensor->hi2c, addr, data3, 3, 100);
  printf ("write t: %d\n", t);


  data3[0] = 0x06; // mask
  data3[1] = 0x08; // POL = 1, APOL = 1, LEN = 1
  data3[2] = 0x03;
  t = HAL_I2C_Master_Transmit(pSensor->hi2c, addr, data3, 3, 100);
  printf ("write t: %d\n", t);
*/


  // рассчет значений для R=10mOhm, Imax= 3.2768A, Wlimit=40W
  // 28V * 1.428A = 40W limit
  // 40W / 0.0025 W/bit = 16000 (3E80h)
  data3[0] = 0x07; // limit
  data3[1] = 0x3E;
  data3[2] = 0x80;
  t = HAL_I2C_Master_Transmit(pSensor->hi2c, addr, data3, 3, 100);
//  printf ("write t: %d\n", t);

  // Current_LSB = 3,2768/32768 = 0,0001 A/bit
  // cal = 0,00512 / ( 0,0001 A/div * 0,01 Ohm) = 5120 (1400h)
  // power_LSB = 0,0001 * 25 = 0.0025 W/bit

  data3[0] = 0x05; // cal
  data3[1] = 0x0F;
  data3[2] = 0xE0;
  t = HAL_I2C_Master_Transmit(pSensor->hi2c, addr, data3, 3, 100);
 // printf ("write t: %d\n", t);


  data3[0] = 0x06; // mask
  data3[1] = 0x08; // POL = 1, APOL = 0, LEN = 1
  data3[2] = 0x01;
  t = HAL_I2C_Master_Transmit(pSensor->hi2c, addr, data3, 3, 100);
 // printf ("write t: %d\n", t);




}

//==============================================================================
void T_INA226_distortAlert(T_INA226 *pSensor)
{
  char addr = 0x80 | (pSensor->address << 1);
  data3[0] = 0x06; // mask
  data3[1] = 0x08; // POL = 1, APOL = 0, LEN = 1
  data3[2] = 0x01;
  HAL_I2C_Master_Transmit(pSensor->hi2c, addr, data3, 3, 100);

  data3[0] = 0x06; // mask
  data3[1] = 0x08; // POL = 1, APOL = 1, LEN = 1
  data3[2] = 0x03;
  HAL_I2C_Master_Transmit(pSensor->hi2c, addr, data3, 3, 100);

}

//выкл сигнала ALERT на INA226 (связанно с выключением транзистора)
//==============================================================================
void T_INA226_AlertOff(T_INA226 *pSensor)
{
  char addr = 0x80 | (pSensor->address << 1);
  data3[0] = 0x06; // mask
  data3[1] = 0x08; // POL = 1, APOL = 0, LEN = 1
  data3[2] = 0x01;
  HAL_I2C_Master_Transmit(pSensor->hi2c, addr, data3, 3, 100);
}



//выкл сигнала ALERT на INA226 (связанно с включением транзистора)
void T_INA226_AlertOn(T_INA226 *pSensor)
{
  char addr = 0x80 | (pSensor->address << 1);
  data3[0] = 0x06; // mask
  data3[1] = 0x08; // POL = 1, APOL = 1, LEN = 1
  data3[2] = 0x03;
  HAL_I2C_Master_Transmit(pSensor->hi2c, addr, data3, 3, 100);

}




//==============================================================================
int16_t T_INA226_getReg(T_INA226 *pSensor, uint8_t regAddr)
{
  char addr = 0x80 | (pSensor->address << 1);
  char readAddr = 0x81 | (pSensor->address << 1);
  char s;



  HAL_I2C_Master_Transmit(pSensor->hi2c, addr, &regAddr, 1, 100);
  HAL_I2C_Master_Receive(pSensor->hi2c, readAddr, recvBuffer, 2, 100);
  s = recvBuffer[1];
  recvBuffer[1] = recvBuffer[0];
  recvBuffer[0] = s;


  return *((int16_t *)recvBuffer);

}






//==============================================================================
void T_INA226_printRegs(T_INA226 *pSensor)
{

  char addr = 0x80 | (pSensor->address << 1);
  char readAddr = 0x81 | (pSensor->address << 1);
  uint8_t regAddress = 0;

  char t;

  int16_t *v = (int16_t *)recvBuffer;
  char s;


//  printf ("sensor %d regs:---------\n", pSensor->address);
//  t = HAL_I2C_Master_Transmit(pSensor->hi2c, addr, &regAddress, 1, 100);
//  t = HAL_I2C_Master_Receive(pSensor->hi2c, readAddr, recvBuffer, 2, 100);
 // printf ("cfg: %02X %02X\n", recvBuffer[0], recvBuffer[1]);

  regAddress = 0;
  t = HAL_I2C_Master_Transmit(pSensor->hi2c, addr, &regAddress, 1, 100);
  t = HAL_I2C_Master_Receive(pSensor->hi2c, readAddr, recvBuffer, 2, 100);
  printf ("mask: %02X %02X\n", recvBuffer[0], recvBuffer[1]);


  regAddress = 1;
  t = HAL_I2C_Master_Transmit(pSensor->hi2c, addr, &regAddress, 1, 100);
  t = HAL_I2C_Master_Receive(pSensor->hi2c, readAddr, recvBuffer, 2, 100);
  s = recvBuffer[1];
  recvBuffer[1] = recvBuffer[0];
  recvBuffer[0] = s;
  printf ("shunt voltage: %02X %02X  %d  %f mV\n", recvBuffer[0], recvBuffer[1], *v, (float)*v * .0025);

  regAddress = 2;
  t = HAL_I2C_Master_Transmit(pSensor->hi2c, addr, &regAddress, 1, 100);
  t = HAL_I2C_Master_Receive(pSensor->hi2c, readAddr, recvBuffer, 2, 100);
  s = recvBuffer[1];
  recvBuffer[1] = recvBuffer[0];
  recvBuffer[0] = s;
  printf ("bus voltage: %02X %02X  %d  %f V\n", recvBuffer[0], recvBuffer[1], *v, (float)*v * 0.00125);


  regAddress = 3;
  t = HAL_I2C_Master_Transmit(pSensor->hi2c, addr, &regAddress, 1, 100);
  t = HAL_I2C_Master_Receive(pSensor->hi2c, readAddr, recvBuffer, 2, 100);
  s = recvBuffer[1];
  recvBuffer[1] = recvBuffer[0];
  recvBuffer[0] = s;
  printf ("power: %02X %02X  %d  %f\n", recvBuffer[0], recvBuffer[1], *v, (float)*v * 0.0025);


  regAddress = 4;
  t = HAL_I2C_Master_Transmit(pSensor->hi2c, addr, &regAddress, 1, 100);
  t = HAL_I2C_Master_Receive(pSensor->hi2c, readAddr, recvBuffer, 2, 100);
  s = recvBuffer[1];
  recvBuffer[1] = recvBuffer[0];
  recvBuffer[0] = s;

  printf ("current: %02X %02X  %d  %f\n", recvBuffer[0], recvBuffer[1], *v, (float)*v * 0.0001);


}




//==============================================================================
float T_INA226_getPower(T_INA226 *pSensor)
{
  if (pSensor)
    return (float)pSensor->P * 0.0025;
  else return 0;
}


//==============================================================================
float T_INA226_getVoltage(T_INA226 *pSensor)
{
  if (pSensor)
    return (float)pSensor->V * 0.00125;
  else return 0;

}


//==============================================================================
float T_INA226_getCurrent(T_INA226 *pSensor)
{
  if (pSensor)
    return (float)pSensor->C * 0.0001;
  else return 0;


}

//==============================================================================
void T_INA226_process(T_INA226 *pSensor)
{

  // в ИЛ-114 убрано чтение датчиков через DMA, для надёжности, поскольку датчиков всего 6 и
  // на этой шине I2C еще установлен расширитель GPIO
  switch (pSensor->state){
    case ST_INA226_READ_IDREG_START:
      if (pSensor->hi2c->State == HAL_I2C_STATE_READY){
        pSensor->regAddress = 0xFE;


        HAL_I2C_Master_Transmit (pSensor->hi2c, (uint16_t)(0x80 | (pSensor->address << 1)), &pSensor->regAddress, 1, 100);
        HAL_I2C_Master_Receive  (pSensor->hi2c, (uint16_t)(0x81 | (pSensor->address << 1)), pSensor->regData, 2, 100);


        int16_t *v = (int16_t *)pSensor->regData;
        pSensor->chipID = *v;
        if (pSensor->chipID == (uint16_t)0x4954) pSensor->connected = 1;
        else pSensor->connected = 0;


        pSensor->state = ST_INA226_READ_VREG_START;


      }
      break;
    /*case ST_INA226_READ_IDREG:
      if (pSensor->hi2c->State == HAL_I2C_STATE_READY){
        HAL_I2C_Master_Receive_DMA (pSensor->hi2c, (uint16_t)(0x81 | (pSensor->address << 1)), pSensor->regData, 2);

        pSensor->state = ST_INA226_READ_VREG_START;
      }

      break;*/
    case ST_INA226_READ_VREG_START:
      if (pSensor->hi2c->State == HAL_I2C_STATE_READY){

        pSensor->regAddress = 0x02;
        HAL_I2C_Master_Transmit (pSensor->hi2c, (uint16_t)(0x80 | (pSensor->address << 1)), &pSensor->regAddress, 1, 100);
        HAL_I2C_Master_Receive (pSensor->hi2c, (uint16_t)(0x81 | (pSensor->address << 1)), pSensor->regData, 2, 100);


        int16_t *v = (int16_t *)pSensor->regData;
        char s;
        s = pSensor->regData[1];
        pSensor->regData[1] = pSensor->regData[0];
        pSensor->regData[0] = s;

        pSensor->V = *v;


        pSensor->state = ST_INA226_READ_CREG_START;


      }
      break;
/*    case ST_INA226_READ_VREG:
      if (pSensor->hi2c->State == HAL_I2C_STATE_READY){
        HAL_I2C_Master_Receive_DMA (pSensor->hi2c, (uint16_t)(0x81 | (pSensor->address << 1)), pSensor->regData, 2);

        pSensor->state = ST_INA226_READ_CREG_START;
      }

      break;*/
    case ST_INA226_READ_CREG_START:
      if (pSensor->hi2c->State == HAL_I2C_STATE_READY){


        pSensor->regAddress = 0x04;
        HAL_I2C_Master_Transmit (pSensor->hi2c, (uint16_t)(0x80 | (pSensor->address << 1)), &pSensor->regAddress, 1, 100);
        HAL_I2C_Master_Receive (pSensor->hi2c, (uint16_t)(0x81 | (pSensor->address << 1)), pSensor->regData, 2, 100);


        int16_t *v = (int16_t *)pSensor->regData;
        char s;
        s = pSensor->regData[1];
        pSensor->regData[1] = pSensor->regData[0];
        pSensor->regData[0] = s;

        pSensor->C = *v;


        pSensor->state = ST_INA226_READ_PREG_START;


      }
      break;
/*    case ST_INA226_READ_CREG:
      if (pSensor->hi2c->State == HAL_I2C_STATE_READY){
        HAL_I2C_Master_Receive_DMA (pSensor->hi2c, (uint16_t)(0x81 | (pSensor->address << 1)), pSensor->regData, 2);

        pSensor->state = ST_INA226_READ_PREG_START;
      }

      break;*/
    case ST_INA226_READ_PREG_START:
      if (pSensor->hi2c->State == HAL_I2C_STATE_READY){


        pSensor->regAddress = 0x03;
        HAL_I2C_Master_Transmit (pSensor->hi2c, (uint16_t)(0x80 | (pSensor->address << 1)), &pSensor->regAddress, 1, 100);
        HAL_I2C_Master_Receive (pSensor->hi2c, (uint16_t)(0x81 | (pSensor->address << 1)), pSensor->regData, 2, 100);

        int16_t *v = (int16_t *)pSensor->regData;
        char s;
        s = pSensor->regData[1];
        pSensor->regData[1] = pSensor->regData[0];
        pSensor->regData[0] = s;

        pSensor->P = *v;


        pSensor->state = ST_INA226_READ_COMPLETE;


      }
      break;
    /*case ST_INA226_READ_PREG:
      if (pSensor->hi2c->State == HAL_I2C_STATE_READY){
        HAL_I2C_Master_Receive_DMA (pSensor->hi2c, (uint16_t)(0x81 | (pSensor->address << 1)), pSensor->regData, 2);

        pSensor->state = ST_INA226_BUSY;
      }

      break;
    case ST_INA226_BUSY:
      if (pSensor->hi2c->State == HAL_I2C_STATE_READY){



        pSensor->state = ST_INA226_READ_COMPLETE;
      }

      break;*/
    case ST_INA226_IDLE:

      pSensor->state = ST_INA226_READ_IDREG_START;
      break;

  }

}

//==============================================================================
void T_INA226_setState(T_INA226 *pSensor, uint8_t newState)
{
  pSensor->state = newState;

}
