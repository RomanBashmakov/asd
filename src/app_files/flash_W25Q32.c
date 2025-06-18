/*
 * flash_W25Q32.c
 *
 *  Created on: 17 нояб. 2019 г.
 *      Author: zhuchenkovao
 */

// немасштабируемый модуль


#include "flash_W25Q32.h"

#include <stdio.h>




#define   W25Q32_S1_BUSY           0x01
#define   W25Q32_S1_WEL            0x02
#define   W25Q32_S2_SUS            0x80


// переменные
TFlashW25Q32    flashW25;               // флеш память



// приватные прототипы функций
void FLASH_W25Q32_transmitReceive (uint16_t size);




//===============================================================================

void FLASH_W25Q32_init (SPI_HandleTypeDef  *hspi, GPIO_TypeDef *GPIO_PORT_cs, uint16_t GPIO_PIN_cs)
{

  printf ("flash init:\n");



  flashW25.hspi = hspi;
  flashW25.GPIO_PORT_cs = GPIO_PORT_cs;
  flashW25.GPIO_PIN_cs = GPIO_PIN_cs;

  flashW25.deviceIDCorrect = 0;
  flashW25.timeoutError = 0;

  flashW25.connected = 0x7F;
  TIMERS_setTimer(&flashW25.tmrCheckConnection, 3000);

  HAL_GPIO_WritePin(flashW25.GPIO_PORT_cs, flashW25.GPIO_PIN_cs, GPIO_PIN_SET);


  for (int i = 0; i < W25Q32_TXRX_BUFFER_SIZE; i++) flashW25.txBuffer[i] = 0;
  for (int i = 0; i < W25Q32_TXRX_BUFFER_SIZE; i++) flashW25.rxBuffer[i] = 0;


  // get JEDEC ID
/*  flashW25.txBuffer[0] = 0x9F;
  FLASH_W25Q32_transmitReceive (4);

  // проверка JEDEC ID для  W25Q32FV (SPI mode), должно быть: 0xEF 0x40 0x16
  if (flashW25.rxBuffer[1] == (uint8_t)0xEF &&
      flashW25.rxBuffer[2] == (uint8_t)0x40 &&
      flashW25.rxBuffer[3] == (uint8_t)0x16)
  {
    flashW25.deviceIDCorrect = 1;
  }*/

  FLASH_W25Q32_checkID ();


  printf("flash JEDEC ID: ");
  for (int i = 1; i < 4; i++){
    printf(" %02X", flashW25.rxBuffer[i]);
  }
  printf(" device ID Correct: %d\n", flashW25.deviceIDCorrect);



  // get device UID
  flashW25.txBuffer[0] = 0x4B;
  FLASH_W25Q32_transmitReceive (13);




  for (int i = 0, j = 12; i < 8; i++, j--){
    flashW25.deviceUniqueID[i] = flashW25.rxBuffer[j];
  }

  printf("device UID: ");
  for (int i = 7; i >= 0; i--){
    printf(" %02X", flashW25.deviceUniqueID[i]);
  }
  printf("\n");





  // reset W25Q32FV:
  // чтение статусных регистров для контроля занятости и приостановки
  char  busy = 0, sus = 0;

  //flashW25.txBuffer[0] = 0x06;    //
 // FLASH_W25Q32_transmitReceive (1);  //



  // ожидание готовности флэш-памяти
  TTimer  tmr;
  TIMERS_setTimer(&tmr, 60000);   // возможно идет стирание всего чипа, поэтому таймер таймаута 60 сек
  do {
    // чтение статусного регистра 1
    flashW25.txBuffer[0] = 0x05;
    FLASH_W25Q32_transmitReceive (2);
    busy = flashW25.rxBuffer[1] & W25Q32_S1_BUSY;

    // чтение статусного регистра 2
    flashW25.txBuffer[0] = 0x15;
    FLASH_W25Q32_transmitReceive (2);
    sus = flashW25.rxBuffer[1] & W25Q32_S2_SUS;


    if ( TIMERS_expired(tmr) ){   // проверка таймаута, если если бит не установился в течении 60 сек то выходим
      flashW25.timeoutError = 1;
      break;
    }


  } while (sus & busy);


  // после ожидания sus и busy
  printf("reset W25Q32\n");
  // отправка команд перезагрузки
  flashW25.txBuffer[0] = 0x66;
  FLASH_W25Q32_transmitReceive (1);
  flashW25.txBuffer[0] = 0x99;
  FLASH_W25Q32_transmitReceive (1);






  // чтение статусного регистра 1
  /*flashW25.txBuffer[0] = 0x05;
  FLASH_W25Q32_transmitReceive (2);
  for (int i = 0; i < 2; i++){
    printf(" %02X", flashW25.rxBuffer[i]);
  }
  printf("\n");

  // чтение статусного регистра 2
  flashW25.txBuffer[0] = 0x15;
  FLASH_W25Q32_transmitReceive (2);

  for (int i = 0; i < 2; i++){
    printf(" %02X", flashW25.rxBuffer[i]);
  }
  printf("\n");   //  */


  // запись в статусные регистры нулевых значений
  flashW25.txBuffer[0] = 0x50;          // write enable
  FLASH_W25Q32_transmitReceive (1);

  // write S1 0x00
  flashW25.txBuffer[0] = 0x01;
  flashW25.txBuffer[1] = 0x00;
  FLASH_W25Q32_transmitReceive (2);

  flashW25.txBuffer[0] = 0x50;          // write enable
  FLASH_W25Q32_transmitReceive (1);


  // write S2 0x00
  flashW25.txBuffer[0] = 0x31;
  flashW25.txBuffer[1] = 0x00;
  FLASH_W25Q32_transmitReceive (2);


/*  printf("read status\n");
  // чтение статусного регистра 1
  flashW25.txBuffer[0] = 0x05;
  FLASH_W25Q32_transmitReceive (2);
  for (int i = 0; i < 2; i++){
    printf(" %02X", flashW25.rxBuffer[i]);
  }
  printf("\n");

  // чтение статусного регистра 2
  flashW25.txBuffer[0] = 0x15;
  FLASH_W25Q32_transmitReceive (2);

  for (int i = 0; i < 2; i++){
    printf(" %02X", flashW25.rxBuffer[i]);
  }
  printf("\n");
*/



/*

  flashW25.txBuffer[0] = 0x06;          // write enable
  FLASH_W25Q32_transmitReceive (1);

  // запись
  flashW25.txBuffer[0] = 0x02;
  flashW25.txBuffer[1] = 0x3F;
  flashW25.txBuffer[2] = 0xFF;
  flashW25.txBuffer[3] = 0x00;
  flashW25.txBuffer[4] = 0xA4;
  flashW25.txBuffer[5] = 0xA5;
  flashW25.txBuffer[6] = 0x67;
  flashW25.txBuffer[7] = 0x71;
  FLASH_W25Q32_transmitReceive (8);

 // printf("read s1\n");

  do {
  // чтение статусного регистра 1 после записи
    flashW25.txBuffer[0] = 0x05;
    FLASH_W25Q32_transmitReceive (2);
    printf(" %02X", flashW25.rxBuffer[1]);

  } while (flashW25.rxBuffer[1] & W25Q32_S1_BUSY);
  printf("\n");
// */

 /* // чтение
  flashW25.txBuffer[0] = 0x03;
  flashW25.txBuffer[1] = 0x3F;
  flashW25.txBuffer[2] = 0xFF;
  flashW25.txBuffer[3] = 0xFD;
  FLASH_W25Q32_transmitReceive (100);



  for (int i = 0; i < 100; i++){
    printf(" %02X", flashW25.rxBuffer[i]);
  }
  printf("\n");


*/



 // while (1);




}





//============================================================================
void FLASH_W25Q32_process ()
{

  if (TIMERS_expired (flashW25.tmrCheckConnection) ){
    TIMERS_setTimer (&flashW25.tmrCheckConnection, 3000);

    FLASH_W25Q32_checkID ();

    if (flashW25.deviceIDCorrect){
      flashW25.connected = 1;
    }else{
      flashW25.connected = 0;
    }


  }

}


//=============================================================================
void FLASH_W25Q32_checkID (void)
{
  // get JEDEC ID
  flashW25.txBuffer[0] = 0x9F;
  FLASH_W25Q32_transmitReceive (4);

  // проверка JEDEC ID для  W25Q32FV (SPI mode), должно быть: 0xEF 0x40 0x16
  if (flashW25.rxBuffer[1] == (uint8_t)0xEF &&
      flashW25.rxBuffer[2] == (uint8_t)0x40 &&
      flashW25.rxBuffer[3] == (uint8_t)0x16)
  {
    flashW25.deviceIDCorrect = 1;
  }else {
    flashW25.deviceIDCorrect = 0;
  }
}


//============================================================================
// прием-передача без DMA
void FLASH_W25Q32_transmitReceive (uint16_t size)
{
  HAL_GPIO_WritePin(flashW25.GPIO_PORT_cs, flashW25.GPIO_PIN_cs, GPIO_PIN_RESET);
  HAL_SPI_TransmitReceive(flashW25.hspi, flashW25.txBuffer, flashW25.rxBuffer, size, 100);
  HAL_GPIO_WritePin(flashW25.GPIO_PORT_cs, flashW25.GPIO_PIN_cs, GPIO_PIN_SET);
}


//============================================================================
// отладочный вывод сектора 4Кб
void FLASH_W25Q32_printSector (uint16_t sectorNum)
{
  if (sectorNum > 0x400) sectorNum = 0x400;  // ASSERT
  uint32_t sectorAddr = sectorNum * 0x1000;

  uint8_t data[16];

  printf("sector %lX data:\n", sectorAddr);
  for (int addr = 0; addr < 0x1000; addr += 0x10){
    FLASH_W25Q32_readData(sectorAddr + addr, data, 16);

    printf(" %lX:  ", sectorAddr + addr);
    for (int i = 0; i < 16; i++){  // debug output
      printf(" %02X", data[i]);
    }
    printf("\n");
    HAL_Delay(2);
  }

}

//============================================================================
// чтение данных по адресу
void FLASH_W25Q32_readData (uint32_t addr, uint8_t* data, uint16_t size)
{
  uint8_t *pAddr = (uint8_t*)&addr;

  if (size > (W25Q32_TXRX_BUFFER_SIZE - 4) ) size = W25Q32_TXRX_BUFFER_SIZE - 4;  // ASSERT

  // чтение
  flashW25.txBuffer[0] = 0x03;
  flashW25.txBuffer[1] = pAddr[2] & 0x3F;
  flashW25.txBuffer[2] = pAddr[1];
  flashW25.txBuffer[3] = pAddr[0];
  FLASH_W25Q32_transmitReceive (size + 4);

  for (int i = 0; i < size; i++){
    data[i] = flashW25.rxBuffer[i + 4];
  }

}

//============================================================================
// проверка свободности памяти по адресу
char FLASH_W25Q32_checkEmpty(uint32_t addr, uint16_t size)
{
  uint8_t *pAddr = (uint8_t*)&addr;

  if (size > (W25Q32_TXRX_BUFFER_SIZE - 4) ) size = W25Q32_TXRX_BUFFER_SIZE - 4;  // ASSERT

  // чтение
  flashW25.txBuffer[0] = 0x03;
  flashW25.txBuffer[1] = pAddr[2] & 0x3F;
  flashW25.txBuffer[2] = pAddr[1];
  flashW25.txBuffer[3] = pAddr[0];
  FLASH_W25Q32_transmitReceive (size + 4);

  char ret = 1;
  // проверяем считанные данные
  for (int i = 0; i < size; i++){
    // если хоть один байт не равен 0xFF значит область занята
    if (flashW25.rxBuffer[i + 4] != (uint8_t)0xFF)  ret = 0;
  }

  return ret;
}



//============================================================================
// запись данных по адресу, область памяти,
// куда записываются данные болжна быть чистой, то есть все байты в области должны быть равны 0xFF
void FLASH_W25Q32_writeData (uint32_t addr, uint8_t* data, uint16_t size)
{


  uint8_t *pAddr = (uint8_t*)&addr;
  if (size > (W25Q32_TXRX_BUFFER_SIZE - 4) ) size = W25Q32_TXRX_BUFFER_SIZE - 4;  // ASSERT


  flashW25.txBuffer[0] = 0x06;          // write enable
  FLASH_W25Q32_transmitReceive (1);

  // запись
  flashW25.txBuffer[0] = 0x02;
  flashW25.txBuffer[1] = pAddr[2] & 0x3F;
  flashW25.txBuffer[2] = pAddr[1];
  flashW25.txBuffer[3] = pAddr[0];

  for (int i = 0; i < size; i++){
    flashW25.txBuffer[i + 4] = data[i];
  }

  FLASH_W25Q32_transmitReceive (size + 4);



  // ожидание готовности флэш-памяти
  TTimer  tmr;
  TIMERS_setTimer(&tmr, 1000);   // таймер таймаута 1 сек
  do {
    // чтение статусного регистра 1 после записи
    flashW25.txBuffer[0] = 0x05;
    FLASH_W25Q32_transmitReceive (2);
 //   printf (" %02X", flashW25.rxBuffer[1]);   //


    if ( TIMERS_expired(tmr) ){   // проверка таймаута, если если бит не установился в течении 1 сек то выходим
      flashW25.timeoutError = 1;
      break;
    }


  } while (flashW25.rxBuffer[1] & W25Q32_S1_BUSY);

  //printf ("flash write end, addr: %lX  size: %X \n", addr, size);  //


}


//======================================================================================
// стирание секотра (перевод всех байтов сектора в значение 0xFF)
void FLASH_W25Q32_eraseSector (uint32_t sectorAddr)
{


  if (sectorAddr > 0x003FFFFF) return;  // ASSRET
  uint8_t *pAddr = (uint8_t*)&sectorAddr;

  flashW25.txBuffer[0] = 0x06;          // отправка write enable
  FLASH_W25Q32_transmitReceive (1);

  // стирание сектора
  flashW25.txBuffer[0] = 0x20;
  flashW25.txBuffer[1] = pAddr[2] & 0x3F;
  flashW25.txBuffer[2] = pAddr[1];
  flashW25.txBuffer[3] = pAddr[0];

  FLASH_W25Q32_transmitReceive (4);

  // ожидание готовности флэш-памяти
  TTimer  tmr;
  TIMERS_setTimer(&tmr, 2000);   // таймер таймаута 2 сек
  do {
    // чтение статусного регистра 1 после записи
    flashW25.txBuffer[0] = 0x05;
    FLASH_W25Q32_transmitReceive (2);
   // printf (" %02X  %ld \n", flashW25.rxBuffer[1], timer1ms___);   //

    HAL_Delay(1);

    if ( TIMERS_expired(tmr) ){   // проверка таймаута, если если бит не установился в течении 2 сек то выходим
      flashW25.timeoutError = 1;
      break;
    }


  } while (flashW25.rxBuffer[1] & W25Q32_S1_BUSY);


  //printf ("erase end %lX   %d   %ld\n", sectorAddr, flashW25.timeoutError, timer1ms___);  // debug



}


//======================================================================================
// стирание всей памяти
// происходит очень долго (около минуты) во время этого процесса производить чтение/запись в флэш-память нельзя
// поэтому эту функцию лучше использовать перед загрузкой микроконтроллера
void FLASH_W25Q32_eraseChip (void)
{

  flashW25.txBuffer[0] = 0x06;          // отправка write enable
  FLASH_W25Q32_transmitReceive (1);

  // стирание чипа
  flashW25.txBuffer[0] = 0xC7;
  FLASH_W25Q32_transmitReceive (1);

  // ожидание готовности флэш-памяти
  TTimer  tmr;
  TIMERS_setTimer(&tmr, 80000);   // таймер таймаута 70 сек
  do {
    // чтение статусного регистра 1 после записи
    flashW25.txBuffer[0] = 0x05;
    FLASH_W25Q32_transmitReceive (2);

    HAL_Delay(1);

    if ( TIMERS_expired(tmr) ){   // проверка таймаута, если если бит не установился в течении 70 сек то выходим
      flashW25.timeoutError = 1;
      break;
    }


  } while (flashW25.rxBuffer[1] & W25Q32_S1_BUSY);


}

