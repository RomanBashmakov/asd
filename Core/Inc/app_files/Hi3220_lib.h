/*
 * Hi3220_lib.h
 *
 *  Created on: Jun 17, 2020
 *      Author: zhuchenkovao
 */

// Hi-3220 lib for HAL Stm32 (in blocking mode no DMA)



#ifndef INC_HI3220_LIB_H_
#define INC_HI3220_LIB_H_


#include "main.h"

#include "device_3220.h"

#include "arinc429.h"
#include "utils.h"

#define HI_SPI_CS_DWN	  HAL_GPIO_WritePin(hi->pGPIO->PORT_cs, hi->pGPIO->PIN_cs, GPIO_PIN_RESET)
#define HI_SPI_CS_UP	  HAL_GPIO_WritePin(hi->pGPIO->PORT_cs, hi->pGPIO->PIN_cs, GPIO_PIN_SET)

#define HI_RUN_UP	  HAL_GPIO_WritePin(hi->pGPIO->PORT_run, hi->pGPIO->PIN_run, GPIO_PIN_SET)
#define HI_RUN_DWN	  HAL_GPIO_WritePin(hi->pGPIO->PORT_run, hi->pGPIO->PIN_run, GPIO_PIN_RESET)


typedef struct SHI3220PinStruct {

  GPIO_TypeDef                  *PORT_mrst;
  uint16_t                      PIN_mrst;

  GPIO_TypeDef                  *PORT_ready;
  uint16_t                      PIN_ready;

  GPIO_TypeDef                  *PORT_run;
  uint16_t                      PIN_run;


  GPIO_TypeDef                  *PORT_int;
  uint16_t                      PIN_int;

  GPIO_TypeDef                  *PORT_ack;
  uint16_t                      PIN_ack;


  GPIO_TypeDef                  *PORT_cs;
  uint16_t                      PIN_cs;




} THI3220PinStruct;


typedef struct SHI3220 {

  THI3220PinStruct *pGPIO;

  SPI_HandleTypeDef             *phspi;



} THI3220;

//==============================================================================

///*--- 125 ----------------------*/
//typedef struct {	// Time in BCD
//    u32_t label      :8;
//    u32_t sdi        :2;
//    u32_t minutesS   :4;
//    u32_t minutesU   :4;
//    u32_t minutesT   :4;
//    u32_t hoursU     :4;
//    u32_t hoursT     :3;
//    u32_t matrix     :2;
//    u32_t parity     :1;
//} A429_word125_t;
//
//union W125_t {
//    A429_word125_t  str;
//    u32_t           word429;
//};
//
///*--- 150 ----------------------*/
//typedef struct {	// Time in binary
//    u32_t label     :8;
//    u32_t secondS   :3;
//    u32_t second    :6;
//    u32_t minute    :6;
//    u32_t hour      :5;
//    u32_t nc		:1;
//    u32_t matrix    :2;
//    u32_t parity    :1;
//} A429_word150_t;
//
//union W150_t {
//    A429_word150_t  str;
//    u32_t           word429;
//};
//
///*--- 260 ----------------------*/
//typedef struct {	// Date in BCD
//    u32_t label     :8;
//    u32_t sdi       :2;
//    u32_t yearU     :4;
//    u32_t yearT     :4;
//    u32_t monthU    :4;
//    u32_t monthT    :1;
//    u32_t dayU      :4;
//    u32_t dayT      :2;
//    u32_t matrix    :2;
//    u32_t parity    :1;
//} A429_word260_t;
//
//union W260_t {
//    A429_word260_t  str;
//    u32_t           word429;
//};
//
//
///*------------------------------*/

//==============================================================================

int HI3220_init(THI3220 *hi, THI3220PinStruct *ps, SPI_HandleTypeDef *hspi);

void HI3220_softwareReset(THI3220 *hi, uint16_t mode);


uint8_t HI3220_readReg(THI3220 *hi, uint16_t addr);
void HI3220_writeReg(THI3220 *hi, uint16_t addr, uint8_t data);

void HI3220_writeMCR(THI3220 *hi, uint8_t data);

void HI3220_writeMAP(THI3220 *hi, uint16_t addr);

void HI3220_writeByteAtAddr (THI3220 *hi, uint16_t addr, uint8_t byte);
uint8_t HI3220_readByteAtAddr (THI3220 *hi, uint16_t addr);

uint8_t HI3220_readAtMAP(THI3220 *hi);
void HI3220_writeAtMAP (THI3220 *hi, uint8_t data);

void HI3220_readFIFO (THI3220 *hi, unsigned short FIFOChan, unsigned char *bufferRX, unsigned short count);
void HI3220_transmitDirect (THI3220 *hi, unsigned short chan, u8_t *pAW_buf, unsigned short count);
void HI3220_transmitDirect_ZH_LV(THI3220 *hi, unsigned short chan, unsigned char *bufferTX, unsigned short count);


void ARINC_LoadDescriptors(THI3220 *hi,
    unsigned short tx_chan,      // Transmit channel 0-7 (for block pointer)
    unsigned short seq,              // Sequence start location 0-255) into descriptor table
    unsigned char *descriptorBuffer, // pointer descriptor buffer
    unsigned char mesgCnt);           // number of descriptor(messages) to load





#endif /* INC_HI3220_LIB_H_ */
