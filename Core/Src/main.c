/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "mcp23008.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>


#include "KSZ9567S_spi.h"



#include "flash_W25Q32.h"


#include "terminal.h"



#include "stm32_status.h"


#include "powercontrol.h"
#include "arinc429.h"

//#include "DBG.h" //DBG

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */



/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */


const char strVersionDT[] = "IL114  22.07.22  20:38";

u8_t Channel_No = 0;

TTimer  tmrDebug;           //
//u32_t EventPeriod;//DBG
//u32_t npkts;//DBG

TTerminal   termDbg;
TTerminal   termPc2MCU;

#define  UART2_TX_BUFFER_SIZE                   200
char uart2TxBuffer[UART2_TX_BUFFER_SIZE];   //  temp buffer for DMA

#define  UART1_TX_BUFFER_SIZE                   200
char uart1TxBuffer[UART1_TX_BUFFER_SIZE];   //  temp buffer for DMA


uint8_t uart1Recv;
uint8_t uart2Recv;
uint8_t uart3Recv;
uint8_t uart3State = 0;
char uart3RecvBuffer[10];
uint8_t uart3RecvBufferIndex = 0;
uint8_t uart3NewPacket = 0;


u8_t CamSwitch;
enum A300_Matrix_e 		A300_Matrix;
//enum System_Status_e 	System_Status;
//enum SrvRouter_Status_e SrvRouter_Status;
//enum Storage_Status_e 	Storage_Status;

Status_t SYSTEM_Status;


char in12vCnt;
char in12vValue;
char in12vTrigger;


extern union W300_t ARINC_Word_300;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//=============================================================================
// функция отладочного вывода в SWD  Stm32
int _write(int file, char *ptr, int len)
{
  /* Implement your write code here, this is used by puts and printf for example */
  int i=0;
  for(i=0 ; i<len ; i++)
    ITM_SendChar((*ptr++));
  return len;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

	u8_t sw_pos__;

	/* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */


  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_SPI3_Init();
  MX_I2C1_Init();
  MX_SPI2_Init();
  MX_ADC1_Init();
  MX_RTC_Init();
  MX_I2C4_Init();
  MX_USART3_UART_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM12_Init();
  /* USER CODE BEGIN 2 */

  TERMINAL_init(&termDbg);
  TERMINAL_init(&termPc2MCU);

  TIMERS_setTimer(&tmrDebug, 30000);

  printf ("init \n");

  //int t1 = 0;

  printf ("W25Q32 init...\n");
  FLASH_W25Q32_init (&hspi1, GPO_nvm_ncs_GPIO_Port, GPO_nvm_ncs_Pin); // инициализация флэш-памяти до инициализации stm32 и лога !!! чтобы произошла очистка флэш-памяти

  printf ("STM32 status init...\n");
  STM32_init ();  // инициализация стм32 после флеш и до инициализации лога !!! т.к. здесь может произойти очистка флэш-памяти


  HAL_GPIO_WritePin(GPO_hub1_cs_GPIO_Port, GPO_hub1_cs_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPO_hub2_cs_GPIO_Port, GPO_hub2_cs_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPO_hub3_cs_GPIO_Port, GPO_hub3_cs_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPO_hub4_cs_GPIO_Port, GPO_hub4_cs_Pin, GPIO_PIN_RESET);

  HAL_Delay(500);

  // включение блока питания камер
  HAL_GPIO_WritePin(GPO_28Vcam_en_GPIO_Port, GPO_28Vcam_en_Pin, GPIO_PIN_SET);

  HAL_GPIO_WritePin(GPO_hub1_cs_GPIO_Port, GPO_hub1_cs_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPO_hub2_cs_GPIO_Port, GPO_hub2_cs_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPO_hub3_cs_GPIO_Port, GPO_hub3_cs_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPO_hub4_cs_GPIO_Port, GPO_hub4_cs_Pin, GPIO_PIN_SET);

  HAL_Delay(200);

  // init ethernet hubs
  T_KSZ9567S_SPI hub[4];

  hub[0].spi = &hspi3;
  hub[0].GPIO_port = GPO_hub1_cs_GPIO_Port;
  hub[0].GPIO_pin = GPO_hub1_cs_Pin;

  hub[1].spi = &hspi3;
  hub[1].GPIO_port = GPO_hub2_cs_GPIO_Port;
  hub[1].GPIO_pin = GPO_hub2_cs_Pin;

  hub[2].spi = &hspi3;
  hub[2].GPIO_port = GPO_hub3_cs_GPIO_Port;
  hub[2].GPIO_pin = GPO_hub3_cs_Pin;

  hub[3].spi = &hspi3;
  hub[3].GPIO_port = GPO_hub4_cs_GPIO_Port;
  hub[3].GPIO_pin = GPO_hub4_cs_Pin;


  KSZ9567_SPI_RegWriteByte(&hub[0], 0x6301, 0x18);  // set ingress internal delay to 1.5ns
  KSZ9567_SPI_SgmiiRegWrite(&hub[0], 0x1F0000, 0x8140);    // reset
  KSZ9567_SPI_SgmiiRegWrite(&hub[0], 0x1F8001, 0x14);    // (SGMII) set to MAC device, set SGMII Link Status to 1
  KSZ9567_SPI_SgmiiRegWrite(&hub[0], 0x1F0004, 0x20);

  KSZ9567_SPI_RegWriteByte(&hub[1], 0x6301, 0x18);  // set ingress internal delay to 1.5ns
  KSZ9567_SPI_SgmiiRegWrite(&hub[1], 0x1F0000, 0x8140);    // reset
  KSZ9567_SPI_SgmiiRegWrite(&hub[1], 0x1F8001, 0x14);    // (SGMII) set to MAC device, set SGMII Link Status to 1
  KSZ9567_SPI_SgmiiRegWrite(&hub[1], 0x1F0004, 0x20);

  KSZ9567_SPI_RegWriteByte(&hub[2], 0x6301, 0x18);  // set ingress internal delay to 1.5ns
  KSZ9567_SPI_SgmiiRegWrite(&hub[2], 0x1F0000, 0x8140);    // reset
  KSZ9567_SPI_SgmiiRegWrite(&hub[2], 0x1F8001, 0x14);    // (SGMII) set to MAC device, set SGMII Link Status to 1
  KSZ9567_SPI_SgmiiRegWrite(&hub[2], 0x1F0004, 0x20);

  KSZ9567_SPI_RegWriteByte(&hub[3], 0x6301, 0x18);  // set ingress internal delay to 1.5ns
  KSZ9567_SPI_SgmiiRegWrite(&hub[3], 0x1F0000, 0x8140);    // reset
  KSZ9567_SPI_SgmiiRegWrite(&hub[3], 0x1F8001, 0x14);    // (SGMII) set to MAC device, set SGMII Link Status to 1
  KSZ9567_SPI_SgmiiRegWrite(&hub[3], 0x1F0004, 0x20);


  // включение вентилятора
  HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_2);
  //HAL_GPIO_WritePin(GPO_blower_GPIO_Port, GPO_blower_Pin, GPIO_PIN_SET);

  // включение блока питания компьютера
  // после настройки SGMII
  HAL_GPIO_WritePin(GPO_12V_en_GPIO_Port, GPO_12V_en_Pin, GPIO_PIN_SET);
  HAL_Delay(500);


  POWER_init(&hi2c1, &hi2c4);
  HAL_Delay(300);

  ARINC429_init(&hspi2);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  HAL_UART_Receive_IT(&huart1, &uart1Recv, 1);
  HAL_UART_Receive_IT(&huart2, &uart2Recv, 1);
  HAL_UART_Receive_IT(&huart3, &uart3Recv, 1);


  in12vCnt = 0;
  in12vValue = 0;
  in12vTrigger = 0;


  htim12.Instance->CCR2 = 100;    // включение вентилятора на 100%, шим был в качестве эксперимента, оказалось не удачно

//==================================================================================//
  extern MCP23008_t  ic_mcp23008;

  MCP23008_init();

//  u8_t __regs[11];
//
//  for(u8_t i = 0; i < 11; i++) {
//    __regs[i] = MCP23008_Read_Reg(&ic_mcp23008, i);
//  }


  // ===== STATUSES: Set INITIAL STATE =====
  A300_Matrix = 		AMX_NORMAL_OPERATION;
//  System_Status = 		SYS_NORMAL_OPERATION;
//  SrvRouter_Status = 	SRS_NORMAL_OPERATION;
//  Storage_Status = 		STS_NORMAL_OPERATION;

//  extern Status_t Status;

  SYSTEM_Status.system_stat =	SYS_FAULT;
//  SYSTEM_Status.srvrouter = SRS_NORMAL_OPERATION;
//  SYSTEM_Status.storage = 	STS_NORMAL_OPERATION;
  SYSTEM_Status.srvrouter_stat = 1;
//  SYSTEM_Status.storage_stat 	= SYS_NORMAL_OPERATION /* когда сервер будет присылать его реальный статус поставить здесь SYS_FAULT*/;
  SYSTEM_Status.storage_stat 	= 1;
  SYSTEM_Status.XAE21_fault = true;
  SYSTEM_Status.cam_switch_fault = true;

  arinc429Control.FORMAT1 = BASE_FORMAT1;

//  npkts = 0;//DBG
//  DBG_init_HAL();//DBG


//==================================================================================//

  while (1)
  {

//	  _STR

    // однократная прямая передача сообщений (может быть использованна вместе с сообщениями передаваемими планировщиком)
    //HI3220_transmitDirect (&hi, 0, txData, 1);
    //HAL_Delay(100);

    //uint8_t sss[5] = { 0xA5, 2, 3, 4, 5};

    if (TIMERS_expired(tmrDebug)){
      //TIMERS_setTimer(&tmrDebug, 100);
      //TIMERS_disableTimer(&tmrDebug);

     // HAL_GPIO_WritePin(GPO_28Vcam_en_GPIO_Port, GPO_28Vcam_en_Pin, GPIO_PIN_RESET);
    //  HAL_GPIO_WritePin(GPO_12V_en_GPIO_Port, GPO_12V_en_Pin, GPIO_PIN_RESET);

    //  HAL_GPIO_WritePin(GPO_hub1_cs_GPIO_Port, GPO_hub1_cs_Pin, GPIO_PIN_RESET);
    //  HAL_GPIO_WritePin(GPO_hub2_cs_GPIO_Port, GPO_hub2_cs_Pin, GPIO_PIN_RESET);
    //  HAL_GPIO_WritePin(GPO_hub3_cs_GPIO_Port, GPO_hub3_cs_Pin, GPIO_PIN_RESET);
    //  HAL_GPIO_WritePin(GPO_hub4_cs_GPIO_Port, GPO_hub4_cs_Pin, GPIO_PIN_RESET);




      //HAL_UART_Transmit(&huart1, uartTestTX1, 4, 100);
      //HAL_UART_Transmit(&huart2, uartTestTX2, 4, 100);
      //HAL_UART_Transmit(&huart3, uartTestTX2, 4, 100);


      //printf("blower toggle\n");
     // HAL_GPIO_TogglePin(GPO_blower_GPIO_Port, GPO_blower_Pin);
     // HAL_GPIO_WritePin(GPO_28Vcam_en_GPIO_Port, GPO_28Vcam_en_Pin, GPIO_PIN_RESET);







      // debug ksz9567
    /*  printf ("-----------------------\n");
      printf ("sg1 0: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[0], 0x1F0000));
      printf ("sg1 1: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[0], 0x1F0001));
      printf ("sg1 4: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[0], 0x1F0004));
      printf ("sg1 5: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[0], 0x1F0005));
      printf ("sg1 6: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[0], 0x1F0006));
      printf ("sg1 8000: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[0], 0x1F8000));
      printf ("sg1 8001: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[0], 0x1F8001));
      printf ("sg1 8002: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[0], 0x1F8002));
      //

      printf ("-----------------------\n");
      printf ("sg2 0: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[1], 0x1F0000));
      printf ("sg2 1: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[1], 0x1F0001));
      printf ("sg2 4: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[1], 0x1F0004));
      printf ("sg2 5: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[1], 0x1F0005));
      printf ("sg2 6: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[1], 0x1F0006));
      printf ("sg2 8000: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[1], 0x1F8000));
      printf ("sg2 8001: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[1], 0x1F8001));
      printf ("sg2 8002: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[1], 0x1F8002));



      printf ("-----------------------\n");
      printf ("sg3 0: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[2], 0x1F0000));
      printf ("sg3 1: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[2], 0x1F0001));
      printf ("sg3 4: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[2], 0x1F0004));
      printf ("sg3 5: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[2], 0x1F0005));
      printf ("sg3 6: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[2], 0x1F0006));
      printf ("sg3 8000: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[2], 0x1F8000));
      printf ("sg3 8001: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[2], 0x1F8001));
      printf ("sg3 8002: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[2], 0x1F8002));
      //

      printf ("-----------------------\n");
      printf ("sg4 0: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[3], 0x1F0000));
      printf ("sg4 1: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[3], 0x1F0001));
      printf ("sg4 4: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[3], 0x1F0004));
      printf ("sg4 5: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[3], 0x1F0005));
      printf ("sg4 6: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[3], 0x1F0006));
      printf ("sg4 8000: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[3], 0x1F8000));
      printf ("sg4 8001: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[3], 0x1F8001));
      printf ("sg4 8002: %X\n", KSZ9567_SPI_SgmiiRegRead(&hub[3], 0x1F8002));
*/

/*
      uint8_t speed;
      uint8_t duplex;
      uint16_t rd;
*/
      // correct control register after speed change:
  /*    uint8_t speed = (KSZ9567_SPI_SgmiiRegRead(&hub[0], 0x1F8002) >> 2) & 3;
      uint8_t duplex = (KSZ9567_SPI_SgmiiRegRead(&hub[0], 0x1F8002) >> 1) & 1;
      printf ("speed1: %d, duplex: %d\n", speed, duplex);
      uint16_t rd = KSZ9567_SPI_SgmiiRegRead(&hub[0], 0x1F0000);
      rd &= 0xDEBF;
      rd |= (speed & 1) << 13;
      rd |= (speed & 2) << 5;
      rd |= (duplex & 1) << 8;
      printf ("sg1 rd: %X\n", rd);
      KSZ9567_SPI_SgmiiRegWrite(&hub[0], 0x1F0000, rd);


      // correct control register after speed change:
      speed = (KSZ9567_SPI_SgmiiRegRead(&hub[1], 0x1F8002) >> 2) & 3;
      duplex = (KSZ9567_SPI_SgmiiRegRead(&hub[1], 0x1F8002) >> 1) & 1;
      printf ("speed2: %d, duplex: %d\n", speed, duplex);
      rd = KSZ9567_SPI_SgmiiRegRead(&hub[1], 0x1F0000);
      rd &= 0xDEBF;
      rd |= (speed & 1) << 13;
      rd |= (speed & 2) << 5;
      rd |= (duplex & 1) << 8;
      printf ("sg2 rd: %X\n", rd);
      KSZ9567_SPI_SgmiiRegWrite(&hub[1], 0x1F0000, rd);

*/
/*
      // correct control register after speed change:
      speed = (KSZ9567_SPI_SgmiiRegRead(&hub[2], 0x1F8002) >> 2) & 3;
      duplex = (KSZ9567_SPI_SgmiiRegRead(&hub[2], 0x1F8002) >> 1) & 1;
      printf ("speed3: %d, duplex: %d\n", speed, duplex);
      rd = KSZ9567_SPI_SgmiiRegRead(&hub[2], 0x1F0000);
      rd &= 0xDEBF;
      rd |= (speed & 1) << 13;
      rd |= (speed & 2) << 5;
      rd |= (duplex & 1) << 8;
      printf ("sg3 rd: %X\n", rd);
   //   KSZ9567_SPI_SgmiiRegWrite(&hub[2], 0x1F0000, rd);



      // correct control register after speed change:
      speed = (KSZ9567_SPI_SgmiiRegRead(&hub[3], 0x1F8002) >> 2) & 3;
      duplex = (KSZ9567_SPI_SgmiiRegRead(&hub[3], 0x1F8002) >> 1) & 1;
      printf ("speed4: %d, duplex: %d\n", speed, duplex);
      rd = KSZ9567_SPI_SgmiiRegRead(&hub[3], 0x1F0000);
      rd &= 0xDEBF;
      rd |= (speed & 1) << 13;
      rd |= (speed & 2) << 5;
      rd |= (duplex & 1) << 8;
      printf ("sg4 rd: %X\n", rd);
     // KSZ9567_SPI_SgmiiRegWrite(&hub[3], 0x1F0000, rd);



     //  */







    }



    // services:
    // terminal 1
    TERMINAL_process (&termDbg);
    if (termDbg.cbOutput.count){
      if (huart2.gState == HAL_UART_STATE_READY){
        int count = termDbg.cbOutput.count;
        if (count > UART2_TX_BUFFER_SIZE) count = UART2_TX_BUFFER_SIZE;
        CBUFF_getFirstNBytes(&termDbg.cbOutput, (char*)uart2TxBuffer, count);
        // transmit uart 2
        HAL_UART_Transmit(&huart2, (unsigned char*)uart2TxBuffer, count, 100);
      }
    }

    // terminal 2
    TERMINAL_process (&termPc2MCU);
    if (termPc2MCU.cbOutput.count){
      if (huart1.gState == HAL_UART_STATE_READY){
        int count = termPc2MCU.cbOutput.count;
        if (count > UART1_TX_BUFFER_SIZE) count = UART1_TX_BUFFER_SIZE;
        CBUFF_getFirstNBytes(&termPc2MCU.cbOutput, (char*)uart1TxBuffer, count);
        // transmit uart 1
        HAL_UART_Transmit(&huart1, (unsigned char*)uart1TxBuffer, count, 100);
      }
    }

    // usart3 arinc-429 translate
    if (uart3NewPacket){
char buffer[127] = {0};
char size = 127;
        uart3NewPacket = 0;

      ARINC429_Proto_InputPacket (uart3RecvBuffer, uart3RecvBufferIndex, &buffer[0], &size);
      if (size > 0)
      {
        HAL_UART_Transmit(&huart3, &buffer[0], size, 100);
      }

    }


    // devices:
    STM32_process();

    POWER_process();

//	  _STR_UP
    ARINC429_process();
//	  _STR_DWN


    // check and reset uart1
    if(huart1.RxState == HAL_UART_STATE_READY)
    {
      __HAL_UART_CLEAR_OREFLAG(&huart1);
      if(HAL_UART_Receive_IT(&huart1, &uart1Recv, 1) != HAL_OK) Error_Handler();
    }// */
    // check and reset uart2
    if(huart2.RxState == HAL_UART_STATE_READY)
    {
      __HAL_UART_CLEAR_OREFLAG(&huart2);
      if(HAL_UART_Receive_IT(&huart2, &uart2Recv, 1) != HAL_OK) Error_Handler();
    }// */
    // check and reset uart3
    if(huart3.RxState == HAL_UART_STATE_READY)
    {
      __HAL_UART_CLEAR_OREFLAG(&huart3);
      if(HAL_UART_Receive_IT(&huart3, &uart3Recv, 1) != HAL_OK) Error_Handler();
    }// */


    // проверка наличия 12В и передергивание 28в и 3в (запаралелено)
    char t =  HAL_GPIO_ReadPin(GPI_12v_in_GPIO_Port, GPI_12v_in_Pin);
    //printf ("xxx: %d\n", HAL_GPIO_ReadPin(GPI_12v_in_GPIO_Port, GPI_12v_in_Pin));

    in12vValue += t;
    in12vCnt++;

    if (in12vCnt >= 30) {    // 30-ти кратная проверка, для фильтрации единичных импульсов
      if (in12vValue == 0){
        if (in12vTrigger == 0){
          in12vTrigger = 1;
          HAL_GPIO_WritePin(GPO_28Vcam_en_GPIO_Port, GPO_28Vcam_en_Pin, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(GPO_hub1_cs_GPIO_Port, GPO_hub1_cs_Pin, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(GPO_hub2_cs_GPIO_Port, GPO_hub2_cs_Pin, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(GPO_hub3_cs_GPIO_Port, GPO_hub3_cs_Pin, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(GPO_hub4_cs_GPIO_Port, GPO_hub4_cs_Pin, GPIO_PIN_RESET);

        }
      }else if (in12vValue == 30){
        if (in12vTrigger == 1){
          in12vTrigger = 0;
          HAL_GPIO_WritePin(GPO_28Vcam_en_GPIO_Port, GPO_28Vcam_en_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPO_hub1_cs_GPIO_Port, GPO_hub1_cs_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPO_hub2_cs_GPIO_Port, GPO_hub2_cs_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPO_hub3_cs_GPIO_Port, GPO_hub3_cs_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(GPO_hub4_cs_GPIO_Port, GPO_hub4_cs_Pin, GPIO_PIN_SET);
        }
      }

      in12vCnt = 0;
      in12vValue = 0;

    }

    // проверка потребляемой мощности по камерам и формирование состояний пакета ARINC-429
    // если потребляемая мощность меньше 2 ватт, то устанавливаем 1
//    if (T_INA226_getPower(&powerSensA[0]) < 2) txPacketARINC429_1[1] |= 0x04;
//    else txPacketARINC429_1[1] &= ~0x04;
//    if (T_INA226_getPower(&powerSensA[1]) < 2) txPacketARINC429_1[1] |= 0x08;
//    else txPacketARINC429_1[1] &= ~0x08;
//    if (T_INA226_getPower(&powerSensA[2]) < 2) txPacketARINC429_1[1] |= 0x10;
//    else txPacketARINC429_1[1] &= ~0x10;

//    ARINC_Word_300.str.label = 0300;
//    ARINC_Word_300.str.sdi = 0x03;
//    ARINC_Word_300.str.cam1_fault = (T_INA226_getPower(&powerSensA[0]) < 2)? 1 : 0;
//    ARINC_Word_300.str.cam2_fault = (T_INA226_getPower(&powerSensA[1]) < 2)? 1 : 0;
//    ARINC_Word_300.str.cam3_fault = (T_INA226_getPower(&powerSensA[2]) < 2)? 1 : 0;
//    ARINC_Word_300.str.cam4_fault = (T_INA226_getPower(&powerSensA[3]) < 2)? 1 : 0;
//    ARINC_Word_300.str.cam5_fault = (T_INA226_getPower(&powerSensA[4]) < 2)? 1 : 0;
//    ARINC_Word_300.str.cam6_fault = (T_INA226_getPower(&powerSensA[5]) < 2)? 1 : 0;
//	ARINC_Word_300.str.System_Status = System_Status;
//	ARINC_Word_300.str.SrvRouter_Status = SrvRouter_Status;
//	ARINC_Word_300.str.Storage_Status = Storage_Status;
//	ARINC_Word_300.str.matrix = A300_Matrix;
//

/*    if (t1 > 100) {

      t1 = 0;
    }*/

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  // Опрос переключателя выбора камеры и перевод в номер выбранной камеры
	  sw_pos__ = MCP23008_Read_Reg(&ic_mcp23008, 9) & 0x3F;	// старшие два бита MCP23008 жестко сидят на +V
	  // ==============================================
	  u8_t ch_no;
	  for(ch_no = 0; ch_no < CHANNELS_TOTAL; ch_no++) {
		  if(sw_pos__ & 0x01) break;
		  sw_pos__ >>= 1;
	  }
	  Channel_No = (ch_no < CHANNELS_TOTAL? ch_no + 1: 0);
	  // Номер камеры передается как от 1-й до 6-й,
	  // если камера не выбрана (напр. сломан переключатель) - будет передан 0 - признак неисправности
	  // ==============================================
	  SYSTEM_Status.cam_switch_fault = (Channel_No? 0: 1);	// если свитч неисправен, он будет давать '0'

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

//====================================================================
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

  if (huart == &huart1){

    //printf ("%02X  %c", uart1Recv, uart1Recv);
    TERMINAL_recvByte(&termPc2MCU, uart1Recv);
    HAL_UART_Receive_IT(&huart1, &uart1Recv, 1);

  } else if (huart == &huart2) {

    //printf ("%02X  %c", uart2Recv, uart2Recv);
    TERMINAL_recvByte(&termDbg, uart2Recv);
    HAL_UART_Receive_IT(&huart2, &uart2Recv, 1);

  } else if (huart == &huart3) {

    if (uart3Recv == (uint8_t) 0xBE || uart3Recv == (uint8_t) 0xCE){
      uart3State = 1;
      uart3RecvBufferIndex = 0;
      uart3NewPacket = 0;
    }

    if (uart3State){
      uart3RecvBuffer[uart3RecvBufferIndex] = uart3Recv;
      uart3RecvBufferIndex++;
      if (uart3RecvBufferIndex > 2){
        uart3State = 0;
        uart3NewPacket = 1;
      }

    }


    //printf ("%02X  %c", uart3Recv, uart3Recv);
    HAL_UART_Receive_IT(&huart3, &uart3Recv, 1);
  }


}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

	static u32_t Errs_Qty = 0;

	Errs_Qty++;
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

