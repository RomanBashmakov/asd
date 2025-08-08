/*
 * powercontrol.c
 *
 *  Created on: Sep 14, 2021
 *      Author: zhuchenkovao
 */



#include <stdio.h>

#include "i2c.h"

#include "powercontrol.h"


#include "timers_hw2.h"

/// @brief      Задержка перед инциализацией питания камер (мс)
#define POWER_CONTROL_INIT_DELAY 0

TTimer  tmrAReadTimeout;
TTimer  tmrBReadTimeout;

TTimer  tmrSlowOnIntervalA;
TTimer  tmrSlowOnIntervalB;
TTimer  tmrVoltagePresentCalcDelay;


T_INA226 powerSensA[10];
T_INA226 powerSensB[10];
uint8_t currentPsNumA = 0;
uint8_t currentPsNumB = 0;


char slowOnA = 0;
char slowOnB = 0;
//char voltagePresentCalc = 0;
uint8_t slowOnPsNumA = 0;
uint8_t slowOnPsNumB = 0;


//====================================================================
void POWER_init(I2C_HandleTypeDef  *hi2cA, I2C_HandleTypeDef  *hi2cB)
{

  printf("init INA226\n");
  HAL_Delay(POWER_CONTROL_INIT_DELAY);
  for (int i = 0; i < 10; i++){

    powerSensA[i].hi2c = hi2cA;
    powerSensA[i].address = i;

    T_INA226_init(&powerSensA[i]);
 //   HAL_Delay(500);

  }

  for (int i = 0; i < 10; i++){

    powerSensB[i].hi2c = hi2cB;
    powerSensB[i].address = i;

    T_INA226_init(&powerSensB[i]);
//    HAL_Delay(500);

  }

  //slowOn = 0;
  //TIMERS_setTimer(&tmrVoltagePresentCalcDelay, 4000);
  //TIMERS_setTimer(&tmrSlowOnInterval, 500);


  TIMERS_disableTimer(&tmrAReadTimeout);
  TIMERS_disableTimer(&tmrBReadTimeout);


}




//====================================================================
void POWER_process()
{


  // в ИЛ-114 используется только первая группа датчиков и всего первые 6 из группы


  if (slowOnA){
    // режим плавного включения в группе А
    if (TIMERS_expired(tmrSlowOnIntervalA)){
      TIMERS_setTimer(&tmrSlowOnIntervalA, 500);
      if (slowOnPsNumA < 10){
        if (powerSensA[slowOnPsNumA].connected) T_INA226_AlertOn(&powerSensA[slowOnPsNumA]);
        slowOnPsNumA++;
      } else  {
        slowOnA = 0;
        TIMERS_setTimer(&tmrAReadTimeout, 2000);

        currentPsNumA = 0;

        for (int i = 0; i < 10; i++){
           powerSensA[i].state = ST_INA226_IDLE;
        }


      }

    }
  }else {
    // обычный режим работы в группе А

    if (TIMERS_expired(tmrAReadTimeout)){
      TIMERS_setTimer(&tmrAReadTimeout, 2000);
      printf ("tmrAReadTimeout\n");


      // в случае отвала чтения по i2c передергиваем DMA и сбрасываем все состояния


      hi2c1.ErrorCode = HAL_I2C_ERROR_NONE;
      hi2c1.State = HAL_I2C_STATE_READY;
      hi2c1.PreviousState = HAL_I2C_MODE_NONE;
      hi2c1.Mode = HAL_I2C_MODE_NONE;

      //HAL_DMA_Abort(hi2c1.hdmarx);
      //HAL_DMA_Abort(hi2c1.hdmatx);


      for (int i = 0; i < 10; i++){
        powerSensA[i].state = ST_INA226_IDLE;

      }



    }


    // обработка чтения датчиков мощности поочереди
    T_INA226_process(&powerSensA[currentPsNumA]);
    if (powerSensA[currentPsNumA].state == ST_INA226_READ_COMPLETE){
      powerSensA[currentPsNumA].state = ST_INA226_IDLE;
      TIMERS_setTimer(&tmrAReadTimeout, 2000);  // устанавливаем таймер проверки отвала чтения по I2C
      /*printf ("A%d--: %f V   %f A  %f W\n", currentPsNumA,
              T_INA226_getVoltage(&powerSensA[currentPsNumA]),
              T_INA226_getCurrent(&powerSensA[currentPsNumA]),
              T_INA226_getPower(&powerSensA[currentPsNumA])
                   );// */
      currentPsNumA++;
      if (currentPsNumA > 9) {   // в ИЛ-114 обработка только первых 5-ти датчиков в группе А
        currentPsNumA = 0;

        // calc voltage present
        char voltagePresent = 0;
        for (int i = 0; i < 10; i++){
          if (powerSensA[i].V > 0x5FF){
            voltagePresent = 1;
          }
        }

        if (voltagePresent == 0){
          // power off all channels
          //voltagePresentCalc = 0;

          slowOnPsNumA = 0;
          slowOnA = 1;
          TIMERS_setTimer(&tmrSlowOnIntervalA, 500);


          TIMERS_disableTimer(&tmrAReadTimeout);


          for (int i = 0; i < 10; i++){
            if (powerSensA[i].connected)  T_INA226_AlertOff(&powerSensA[i]);

          }
        }




      }
    }







  }










  // debug output
/*  printf("ps num: %d-------------\n", currentPsNum);
  int16_t v = T_INA226_getReg(&powerSens[currentPsNum], 2);
  printf("v: %fV\n", (float)v * 0.00125);
  v = T_INA226_getReg(&powerSens[currentPsNum], 4);
  printf("c: %fA\n", (float)v * 0.0001);
  v = T_INA226_getReg(&powerSens[currentPsNum], 3);
  printf("p: %fW\n", (float)v * 0.0025);
  currentPsNum++;
  if (currentPsNum >= 19) currentPsNum = 0;*/


}
