/*
 * KSZ9567S_spi.h
 *
 *  Created on: Sep 11, 2021
 *      Author: zhuchenkovao
 */

#ifndef INC_APP_FILES_KSZ9567S_SPI_H_
#define INC_APP_FILES_KSZ9567S_SPI_H_


#include "stm32f7xx_hal.h"

typedef struct {

  GPIO_TypeDef*         GPIO_port;
  uint16_t              GPIO_pin;
  SPI_HandleTypeDef     *spi;



} T_KSZ9567S_SPI;


char KSZ9567_SPI_RegReadByte(T_KSZ9567S_SPI *hub, uint16_t addr);
void KSZ9567_SPI_RegWriteByte(T_KSZ9567S_SPI *hub, uint16_t addr, uint8_t byte);

uint16_t KSZ9567_SPI_SgmiiRegRead(T_KSZ9567S_SPI *hub, uint32_t addr);
void KSZ9567_SPI_SgmiiRegWrite(T_KSZ9567S_SPI *hub, uint32_t addr, uint16_t data);



#endif /* INC_APP_FILES_KSZ9567S_SPI_H_ */
