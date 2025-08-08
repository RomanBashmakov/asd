/*
 * stm32_status.c
 *
 *  Created on: Nov 16, 2019
 *      Author: zhuchenkovao
 */

// немасштабируемый модуль



#include "stm32_status.h"

#include "rtc.h"
#include "state_logger_service.h"
#include "flash_W25Q32.h"


//#include "remote_flasher.h"

#include "config.h"


#include "adc.h"

#include <stdio.h>

//#include "stm32f7xx_hal_flash.h"
//#include "stm32f7xx_hal_flash_ex.h"


#define           VREFINT_CAL_ADDR             0x1FF0F44A  // for STM32F745

#define           FLASH_TIMEOUT_VALUE       ((uint32_t)50000U)/* 50 s */

// АЦП
uint32_t            adcBuffer[10];
unsigned short      *pAdcBufferShort;
unsigned int        adcAvgValues[10];
unsigned int        adcAvgCount;

unsigned int        adcChannelCount;
//char                flagVbatCapture = 0;           // признак считывания канала Vbat



TStm32Status    stm32State;


//===============================================================================
// переделаная функция HAL_FLASH_OB_Launch из библиотеки, добавлено __DSB() (без него зависает)
/*HAL_StatusTypeDef HAL_FLASH_OB_Launch2(void)
{
  __disable_irq();

  while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET) asm("nop");

  __ISB();
  __DSB();



  FLASH->OPTCR |= FLASH_OPTCR_OPTSTRT;

  __ISB();
  __DSB();




  while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET)
  {
    asm("nop");
  }

  if(__HAL_FLASH_GET_FLAG(FLASH_FLAG_ALL_ERRORS) != RESET)
{
   // FLASH_SetErrorCode();
    return HAL_ERROR;
  }


  if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_EOP) != RESET)
  {

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);
  }


  HAL_NVIC_SystemReset();

  return HAL_OK;
}
*/


//===============================================================================
void  STM32_init ()
{




    stm32State.restart = 1;
    stm32State.hardFault = 0;
    stm32State.flashCleared = 0;
    stm32State.stm32flashUpdateStatus = 0;

    stm32State.backupDataNotValid = 0;
    for (int i = 0; i < 8; i++)  stm32State.lastOffDT[i] = 0;

    stm32State.startupDTSet = 0;
    for (int i = 0; i < 8; i++)  stm32State.startupDT[i] = 0;

    stm32State.vBatCaptured = 0;
    stm32State.restartCount = 0;
    //stm32State.vBat = 0;


    stm32State.readOutEnabled = -1;  // изначально неопределенное состояние
    stm32State.debugEnabled = CoreDebug->DHCSR & 1;
    stm32State.rcccsr = RCC->CSR;
    RCC->CSR = RCC_CSR_RMVF; // сброс регистра


    // попытка установить защиту от чтения прошивки
    if (stm32State.debugEnabled == 0){  // если отладчик не подключен

      if ((stm32State.rcccsr & 0x0E000000)  == (uint32_t)0x0E000000){ // если reset по событию BOR/POR

        if ((*(__IO uint8_t*)(OPTCR_BYTE1_ADDRESS)) == OB_RDP_LEVEL_0)
        {



          // временно выключенно
        /*  HAL_FLASH_OB_Unlock();
          while(__HAL_FLASH_GET_FLAG(FLASH_SR_BSY) != RESET)  asm("nop");

          *(__IO uint8_t*)OPTCR_BYTE1_ADDRESS = OB_RDP_LEVEL_1;

          FLASH->OPTCR |= FLASH_OPTCR_OPTSTRT;

          __DSB();
          __ISB();
          while(__HAL_FLASH_GET_FLAG(FLASH_SR_BSY) != RESET)  asm("nop");

          HAL_FLASH_OB_Lock();
  // */
        }
      }
    }

    // повторная проверка
    if ((*(__IO uint8_t*)(OPTCR_BYTE1_ADDRESS)) == OB_RDP_LEVEL_0) {
      stm32State.readOutEnabled = 0;
    } else {
      stm32State.readOutEnabled = 1;
    }

  /*    FLASH_OBProgramInitTypeDef OBInit;

      // установка защиты от чтения при первом запуске
      // (код взят из примера)

      HAL_FLASHEx_OBGetConfig(&OBInit);
      if (OBInit.RDPLevel == OB_RDP_LEVEL_0)
      {
        printf ("stm32: set readout protection\n"); /////


        HAL_FLASH_OB_Unlock();
        HAL_FLASH_Unlock();

       // OBInit.OptionType = OPTIONBYTE_RDP ;
       // OBInit.RDPLevel = OB_RDP_LEVEL_1;

        __disable_irq();
        __DSB();

       // HAL_FLASHEx_OBProgram(&OBInit);

        while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET) asm("nop");

        __DSB();

        *(__IO uint8_t*)OPTCR_BYTE1_ADDRESS = OB_RDP_LEVEL_1;

        if (HAL_FLASH_OB_Launch2() != HAL_OK)
        {
          printf ("stm32: error set readout protection\n"); /////
        }


        HAL_FLASH_OB_Lock();
        HAL_FLASH_Lock();


        printf ("stm32: continue\n"); /////

      }
*/



    pAdcBufferShort = (unsigned short*)adcBuffer;
    adcAvgCount = 0;
    adcChannelCount = hadc1.Init.NbrOfConversion;


    // чтение времени выключения из feram
   // FERAM_FM25L16B_readData (FERAM_SHUTDOWN_TIME_ADDRESS, stm32State.lastOffDT, 8);


    HAL_ADC_Start_DMA(&hadc1, adcBuffer, adcChannelCount);

    TIMERS_setTimer(&stm32State.tmrVbatCapture, 3000);

    uint32_t UID;
    char *pUid = (char *)&UID;
    UID = HAL_GetUIDw0();
    stm32State.UID[0][0] = pUid[0];
    stm32State.UID[0][1] = pUid[1];
    stm32State.UID[0][2] = pUid[2];
    stm32State.UID[0][3] = pUid[3];
    stm32State.UID[0][4] = 0;

    UID = HAL_GetUIDw1();
    stm32State.UID[1][0] = pUid[0];
    stm32State.UID[1][1] = pUid[1];
    stm32State.UID[1][2] = pUid[2];
    stm32State.UID[1][3] = pUid[3];
    stm32State.UID[1][4] = 0;

    UID = HAL_GetUIDw2();
    stm32State.UID[2][0] = pUid[0];
    stm32State.UID[2][1] = pUid[1];
    stm32State.UID[2][2] = pUid[2];
    stm32State.UID[2][3] = pUid[3];
    stm32State.UID[2][4] = 0;




    STM32_readBackupData();





}

//===============================================================================
// чтение бэкап регистров
void  STM32_readBackupData ()
{



    uint32_t tmp = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0);      //
    if (tmp != (uint32_t)0x5A32A567) {
      stm32State.backupDataNotValid = 1;

      HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, 0x5A32A567);   // DR0 - хранение признака валидности backup регистров
      HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0);            // DR1 - кол рестартов контроллера

      //HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, 0);            // DR2, DR3 - хранение времени выключения контроллера
      //HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3, 0);

      HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR4, 0);            // DR4 - хранение переменной-признака запланированной очистки флеш-памяти
      HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR5, 0);            // DR5 - хранение последнего номера записи при запланированной очистке флэш-памяти

      HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR6, 0);            // DR6 - хранение переменной-признака запланированной прошивки контроллера
      HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR7, 0);            // DR7 - хранение размера прошивки

      HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR8, 0);            // DR8 - хранение признака HardFault (контроллер был перезагружен в результате серьёзной ошибки)


    }
    else {
      stm32State.backupDataNotValid = 0;
      tmp = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1);
      stm32State.restartCount = tmp;
      tmp++;
      HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, tmp);

      //uint32_t *pTimeOff = (uint32_t *)stm32State.lastOffDT;
      //uint32_t *pDateOff = (uint32_t *)&stm32State.lastOffDT[4];
      //*pTimeOff = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR2);
      //*pDateOff = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR3);



      tmp = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR4);
      if (tmp == (uint32_t)0xFECA1213){
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR4, 0);  // сбрасываем чтобы в сл раз еще раз не очищать флэш
        // если запланировано очищение флеш-памяти, то очищаем и ждем пока не закончится очищение
        // очищение должно занять времени около минуты
        printf ("flash erase start...\n");
        FLASH_W25Q32_eraseChip ();
        stm32State.flashCleared = 1;
        printf ("flash erase completed...\n");


      }

      tmp = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR6);
      if (tmp == (uint32_t)0xCAFE6778){
        // выключение ETHERNET !!!
        //HAL_GPIO_WritePin(GPO_eth_rstn_GPIO_Port, GPO_eth_rstn_Pin, GPIO_PIN_RESET);

        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR6, 0);  // сбрасываем чтобы в сл раз еще раз не прошивать флэш стм
        //
        uint32_t sz = STM32_getStm32UpdateFlashSize();
        printf ("stm32 flash update start, size: %ld...\n", sz);

        //STMRF_copyFlashToFalsh(sz);
      }

      tmp = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR8);
      if (tmp == (uint32_t)0x5A5AA5A5){
        stm32State.hardFault = 1;
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR8, 0);  // сбрасываем чтобы в сл раз не обнаружить hardfault

      }



    }






}


//===============================================================================
void  STM32_setStartupDT ( uint8_t *DTData)
{

    for (int i = 0; i < 7; i++)  stm32State.startupDT[i] = DTData[i];
    stm32State.startupDTSet = 1;


}

//===============================================================================
// сохранение времени выключения перенесено в FERAM
/*void  STM32_writeOffTime (uint8_t *DTData)
{
  uint32_t tmp, *pUint32 = (uint32_t*)DTData;
  tmp = *pUint32;
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, tmp);
  pUint32 = (uint32_t*)&DTData[4];
  tmp = *pUint32;
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3, tmp);

}*/


//===============================================================================
// запланировать стирание флэш-памяти при следующей загрузке контроллера
void  STM32_setSheduleEraseW25Q32 (void)
{

  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR4, 0xFECA1213);
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR5, logService.lastValidRecordNum);

}


//===============================================================================
// запланировать перепрошивку флэш-памяти контроллера при следующей загрузке контроллера
void  STM32_setSheduleUpdateFlashStm32 (uint32_t size)
{

  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR6, 0xCAFE6778);
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR7, size);

}



//===============================================================================
// получить последний номер записи лога после очистки флеш
uint32_t  STM32_getLastLogNum (void)
{
  return HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR5);

}

//===============================================================================
// получить размер обновляемой прошивки
uint32_t  STM32_getStm32UpdateFlashSize (void)
{
  return HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR7);
}

//===============================================================================
// вычисление параметров по АЦП:
void  STM32_process ()
{




   /* if (TIMERS_expired(stm32State.tmrVbatCapture) ){
      TIMERS_setTimer(&stm32State.tmrVbatCapture, 3000);   // считывание напряжение батарейки - раз в 3 секунды, чтобы через делитель не разрядить её

      flagVbatCapture = 1;
    }*/




    // process adc data sensors
    if (adcAvgCount >= 2048){
      adcAvgCount = 0;
      stm32State.adcValues[0] = adcAvgValues[0]  >> 11;   // LM
      stm32State.adcValues[1] = adcAvgValues[1]  >> 11;   // LM
      stm32State.adcValues[2] = adcAvgValues[2]  >> 11;   // temp int
      stm32State.adcValues[3] = adcAvgValues[3]  >> 11;   // vrefint
      stm32State.adcValues[4] = adcAvgValues[4]  >> 11;   //




      HAL_ADC_Start_DMA(&hadc1, adcBuffer, adcChannelCount);




      unsigned short vrefcal = *((unsigned short*)VREFINT_CAL_ADDR);

      float vdda = (float) (3300.0f * vrefcal) / stm32State.adcValues[3];


      stm32State.voltages[0]  = vdda / 4095 * stm32State.adcValues[0];
      stm32State.voltages[1]  = vdda / 4095 * stm32State.adcValues[1];
      stm32State.voltages[2]  = vdda / 4095 * stm32State.adcValues[2];
      stm32State.voltages[3]  = vdda / 4095 * stm32State.adcValues[3];
      stm32State.voltages[4]  = vdda / 4095 * stm32State.adcValues[4];


      stm32State.vdda = vdda;
      stm32State.vBat = stm32State.voltages[0];  // на 04.09.20 пока не подключено




      float  tSens;
      tSens = (float)(stm32State.voltages[0] - 2619.66666f) / -13.5083333f;
      stm32State.tempLM94022_1 = tSens;

      tSens = (float)(stm32State.voltages[1] - 2619.66666f) / -13.5083333f;
      stm32State.tempLM94022_2 = tSens;


      stm32State.tempStm = (( (float)stm32State.voltages[3] - 760)/  2.5 ) + 25;
      if (stm32State.vBatCaptured == 1) stm32State.vBatCaptured = 2;


    }





}









//===============================================================================
// private functions:

//===============================================================================
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle)
{


  if (adcAvgCount == 0){
    adcAvgValues[0] = 0;
    adcAvgValues[1] = 0;
    adcAvgValues[2] = 0;
    adcAvgValues[3] = 0;
    adcAvgValues[4] = 0;

  }


  adcAvgValues[0] += pAdcBufferShort[0];
  adcAvgValues[1] += pAdcBufferShort[1];
  adcAvgValues[2] += pAdcBufferShort[2];
  adcAvgValues[3] += pAdcBufferShort[3];
  adcAvgValues[4] += pAdcBufferShort[4];
  adcAvgCount++;



  if (adcAvgCount < 2048){

    HAL_ADC_Start_DMA(&hadc1, adcBuffer, adcChannelCount);


  }

}



