/*
 * flash_W25Q32.h
 *
 *  Created on: Nov 15, 2019
 *      Author: zhuchenkovao
 */

// немасштабируемый модуль

#ifndef INC_FLASH_W25Q32_H_
#define INC_FLASH_W25Q32_H_

#include "stm32f7xx_hal.h"

#include "timers_hw2.h"


#define   W25Q32_MEMORY_START_ADDR          0
#define   W25Q32_MEMORY_END_ADDR            0x3FFFFF
#define   W25Q32_MEMORY_SIZE                0x400000             //  ‭4194304‬ байт



#define   W25Q32_TXRX_BUFFER_SIZE       100



typedef struct SFlashW25Q32 {

  char                          timeoutError;        // ошибка таймаута произошедшая во время ожидания готовности статусных битов SUS или BUSY

  TTimer                        tmrCheckConnection;  // таймер проверки работоспособности w25q32
  char                          connected;           // признак связи с контроллером

  char                          deviceIDCorrect;
  uint8_t                       deviceID[3];

  uint8_t                       deviceUniqueID[8];

  GPIO_TypeDef                  *GPIO_PORT_cs;
  uint16_t                      GPIO_PIN_cs;
  SPI_HandleTypeDef             *hspi;

  uint8_t                       txBuffer[W25Q32_TXRX_BUFFER_SIZE];
  uint8_t                       rxBuffer[W25Q32_TXRX_BUFFER_SIZE];


} TFlashW25Q32;




extern   TFlashW25Q32    flashW25;               // дескритор объекта - флеш память



void FLASH_W25Q32_init (SPI_HandleTypeDef  *hspi, GPIO_TypeDef *GPIO_PORT_cs, uint16_t GPIO_PIN_cs);
void FLASH_W25Q32_process (void);
void FLASH_W25Q32_checkID (void);
void FLASH_W25Q32_printSector (uint16_t sectorNum);
void FLASH_W25Q32_readData (uint32_t addr, uint8_t* data, uint16_t size);
char FLASH_W25Q32_checkEmpty (uint32_t addr, uint16_t size);
void FLASH_W25Q32_writeData (uint32_t addr, uint8_t* data, uint16_t size);
void FLASH_W25Q32_eraseSector (uint32_t sectorAddr);
void FLASH_W25Q32_eraseChip (void);  //



#endif /* INC_FLASH_W25Q32_H_ */
