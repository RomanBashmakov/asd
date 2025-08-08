/*
 * ina226.h
 *
 *  Created on: 24 янв. 2021 г.
 *      Author: zhuchenkovao
 */

#ifndef INC_APP_FILES_INA226_H_
#define INC_APP_FILES_INA226_H_



#include "stm32f7xx_hal.h"


enum INA226_States {
  ST_INA226_IDLE = 0,
  //ST_INA226_BUSY,
  ST_INA226_READ_COMPLETE,
  ST_INA226_READ_IDREG_START,
  //ST_INA226_READ_IDREG,
  ST_INA226_READ_VREG_START,
  //ST_INA226_READ_VREG,
  ST_INA226_READ_CREG_START,
  //ST_INA226_READ_CREG,
  ST_INA226_READ_PREG_START,
  //ST_INA226_READ_PREG,
  ST_INA226_WRITE_CFGREG_START,
  ST_INA226_WRITE_CFGREG,


};






typedef struct S_INA226 {
  char        address;
  I2C_HandleTypeDef       *hi2c;

  uint8_t             state;

  uint8_t             regAddress;     // for read regs address;


  char                connected;

  uint16_t            chipID;
  int16_t             P;
  int16_t             V;
  int16_t             C;

  uint8_t             regData[2];       // for read regs data;



} T_INA226;



void T_INA226_init(T_INA226 *pSensor);
void T_INA226_distortAlert(T_INA226 *pSensor);
void T_INA226_AlertOff(T_INA226 *pSensor);
void T_INA226_AlertOn(T_INA226 *pSensor);
int16_t T_INA226_getReg(T_INA226 *pSensor, uint8_t regAddr);
void T_INA226_printRegs(T_INA226 *pSensor);

float T_INA226_getPower(T_INA226 *pSensor);
float T_INA226_getVoltage(T_INA226 *pSensor);
float T_INA226_getCurrent(T_INA226 *pSensor);

void T_INA226_process(T_INA226 *pSensor);
void T_INA226_setState(T_INA226 *pSensor, uint8_t newState);



#endif /* INC_APP_FILES_INA226_H_ */
