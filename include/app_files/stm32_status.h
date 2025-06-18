/*
 * stm32_status.h
 *
 *  Created on: Nov 15, 2019
 *      Author: zhuchenkovao
 */

// немасштабируемый модуль


#ifndef INC_STM32_STATUS_H_
#define INC_STM32_STATUS_H_


#include        "stdint.h"

#include        "timers_hw2.h"


// информация по микроконтроллеру, АЦП, измерение уровня напряжения батарейки подключенной к Stm32 и др.

typedef struct SStm32Status {

  char                hardFault;                 // признак перезагрузки контроллера в результате серьёзной ошибки

  char                stm32flashUpdateStatus;    // признак-статус обновления прошивки контроллера (нужно для лога)

  char                flashCleared;              // признак отчистки флэш-памяти (нужно для лога)

  char                restart;                   // признак перезагрузки контроллера (нужно для лога)

  char                readOutEnabled;            //
  uint32_t            debugEnabled;              //

  uint32_t            rcccsr;              //

  //uint32_t            dbgcr;              //
  //uint32_t            dbgapb1;              //
  //uint32_t            dbgapb2;              //


  char                backupDataNotValid;        // признак отключения батарейки (нужно для лога)
  uint8_t             lastOffDT[8];              // последнее время выключения (сохраняется в backup регистре)

  char                startupDTSet;
  uint8_t             startupDT[8];              // время загрузки (включения М/К)

  uint32_t            restartCount;              // количество перезапусков М/К

  char                vBatCaptured;              // для службы лога и стостояния, если равно 2, то значени можно контролировать
  TTimer              tmrVbatCapture;            // таймер считывания Vbat, Vbat считывается реже чем все остальные каналы
  unsigned int        adcValues[10];             // данные АЦП1      0 - LMT87, 1 - TempSensorStm, 2 - Vrefint, 3 - Vbat
  uint16_t            voltages[10];              // данные АЦП1


  char                UID[3][5];                 // уникальные UID для каждого М/К



  float               vdda;                      // напряжение питания аналоговой части
  float               vBat;                      // напряжение батарейки


  float               tempLM94022_1;             // температура с датчика 1
  float               tempLM94022_2;             // температура с датчика 2
  float               tempStm;                   // температура с датчика stm32

} TStm32Status;


extern   TStm32Status    stm32State;  //


void  STM32_init (void);
void  STM32_readBackupData (void);
void  STM32_setStartupDT ( uint8_t *DTData);
//void  STM32_writeOffTime (uint8_t *DTData);
void  STM32_setSheduleEraseW25Q32 (void);
void  STM32_setSheduleUpdateFlashStm32 (uint32_t size);
uint32_t  STM32_getLastLogNum (void);
uint32_t  STM32_getStm32UpdateFlashSize (void);
void  STM32_process (void);


#endif /* INC_STM32_STATUS_H_ */
