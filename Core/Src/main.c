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
#include "gpio.h"
#include "i2c.h"
#include "mcp23008.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>

#include "KSZ9567S_spi.h"
#include "arinc429.h"
#include "flash_W25Q32.h"
#include "powercontrol.h"
#include "stm32_status.h"
#include "terminal.h"

//#include "DBG.h" //DBG

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/**
 * @def MAIN_SYSTEM_START_DELAY_MS
 * @brief Задержка старта основной системы в миллисекундах.
 */
#define MAIN_SYSTEM_START_DELAY_MS 0
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/**
 * @brief Версия и дата сборки прошивки.
 */
const char strVersionDT[] = "IL114  22.07.22  20:38";

/**
 * @brief Номер выбранного канала (камеры).
 */
u8_t Channel_No = 0;

/**
 * @brief Таймер отладки.
 */
TTimer tmrDebug;

/**
 * @brief Терминал для отладочного вывода.
 */
TTerminal termDbg;

/**
 * @brief Терминал для связи с ПК.
 */
TTerminal termPc2MCU;

/**
 * @def UART2_TX_BUFFER_SIZE
 * @brief Размер буфера передачи UART2 для DMA.
 */
#define UART2_TX_BUFFER_SIZE 200

/**
 * @brief Буфер передачи UART2 для DMA.
 */
char uart2TxBuffer[UART2_TX_BUFFER_SIZE];

/**
 * @def UART1_TX_BUFFER_SIZE
 * @brief Размер буфера передачи UART1 для DMA.
 */
#define UART1_TX_BUFFER_SIZE 200

/**
 * @brief Буфер передачи UART1 для DMA.
 */
char uart1TxBuffer[UART1_TX_BUFFER_SIZE];

/**
 * @brief Переменные приёма UART.
 */
uint8_t uart1Recv;
uint8_t uart2Recv;
uint8_t uart3Recv;

/**
 * @brief Состояние приёма UART3.
 */
uint8_t uart3State = 0;

/**
 * @brief Буфер приёма UART3.
 */
char uart3RecvBuffer[10];

/**
 * @brief Индекс буфера приёма UART3.
 */
uint8_t uart3RecvBufferIndex = 0;

/**
 * @brief Флаг нового пакета UART3.
 */
uint8_t uart3NewPacket = 0;

/**
 * @brief Переключатель камеры.
 */
u8_t CamSwitch;

/**
 * @brief Матрица A300.
 */
enum A300_Matrix_e A300_Matrix;

/**
 * @brief Структура статуса системы.
 */
Status_t SYSTEM_Status;

/**
 * @brief Счётчики и значения для 12В питания.
 */
char in12vCnt;
char in12vValue;
char in12vTrigger;

/**
 * @brief Внешнее объединение ARINC_Word_300.
 */
extern union W300_t ARINC_Word_300;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

void Init_Peripherals(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void Init_Peripherals(void)
{
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
}

/**
 * @brief Функция отладочного вывода через SWD (Serial Wire Debug) интерфейс
 * STM32. Используется для перенаправления вывода функций printf и puts.
 * @param file Не используется, параметр для совместимости с системным вызовом
 * write.
 * @param ptr Указатель на буфер с данными для вывода.
 * @param len Количество байт для вывода из буфера.
 * @retval Количество успешно выведенных байт.
 *
 * Данная функция посимвольно отправляет данные через ITM_SendChar, что
 * позволяет выводить отладочную информацию в отладчик, поддерживающий SWD.
 */
int _write(int file, char *ptr, int len)
{
    /* Implement your write code here, this is used by puts and printf for
     * example
     */
    int i = 0;
    for (i = 0; i < len; i++) ITM_SendChar((*ptr++));
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

    /* MCU
     * Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the
     * Systick.
     */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    Init_Peripherals();
    /* USER CODE BEGIN 2 */

    TERMINAL_init(&termDbg);
    TERMINAL_init(&termPc2MCU);

    TIMERS_setTimer(&tmrDebug, 30000);

    printf("init \n");

    W25Q32_Flash_Init(&hspi1, GPO_nvm_ncs_GPIO_Port, GPO_nvm_ncs_Pin);

    printf("STM32 status init...\n");
    STM32_init();  // инициализация стм32 после флеш и до инициализации лога !!!
                   // т.к. здесь может произойти очистка флэш-памяти

    HAL_GPIO_WritePin(GPO_hub1_cs_GPIO_Port, GPO_hub1_cs_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPO_hub2_cs_GPIO_Port, GPO_hub2_cs_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPO_hub3_cs_GPIO_Port, GPO_hub3_cs_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPO_hub4_cs_GPIO_Port, GPO_hub4_cs_Pin, GPIO_PIN_RESET);

    HAL_Delay(500);

    // включение блока питания камер
    HAL_Delay(MAIN_SYSTEM_START_DELAY_MS);
    HAL_GPIO_WritePin(GPO_28Vcam_en_GPIO_Port, GPO_28Vcam_en_Pin, GPIO_PIN_SET);

    HAL_GPIO_WritePin(GPO_hub1_cs_GPIO_Port, GPO_hub1_cs_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPO_hub2_cs_GPIO_Port, GPO_hub2_cs_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPO_hub3_cs_GPIO_Port, GPO_hub3_cs_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPO_hub4_cs_GPIO_Port, GPO_hub4_cs_Pin, GPIO_PIN_SET);

    HAL_Delay(200);

    // init ethernet hubs
    T_KSZ9567S_SPI hub[4];

    InitEthernetHubs(
        hub, &hspi3,
        (GPIO_TypeDef *[]){GPO_hub1_cs_GPIO_Port, GPO_hub2_cs_GPIO_Port,
                           GPO_hub3_cs_GPIO_Port, GPO_hub4_cs_GPIO_Port},
        (uint16_t[]){GPO_hub1_cs_Pin, GPO_hub2_cs_Pin, GPO_hub3_cs_Pin,
                     GPO_hub4_cs_Pin},
        4);

    InitEthernetHubRegisters(hub, 4);

    // включение вентилятора
    HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_2);
    // HAL_GPIO_WritePin(GPO_blower_GPIO_Port, GPO_blower_Pin, GPIO_PIN_SET);

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

    htim12.Instance->CCR2 = 100;  // включение вентилятора на 100%, шим был в
                                  // качестве эксперимента, оказалось не удачно

    //==================================================================================//
    extern MCP23008_t ic_mcp23008;

    MCP23008_init();

    // ===== STATUSES: Set INITIAL STATE =====
    A300_Matrix = AMX_NORMAL_OPERATION;
    //  System_Status = 		SYS_NORMAL_OPERATION;
    //  SrvRouter_Status = 	SRS_NORMAL_OPERATION;
    //  Storage_Status = 		STS_NORMAL_OPERATION;

    //  extern Status_t Status;

    SYSTEM_Status.system_stat = SYS_FAULT;
    //  SYSTEM_Status.srvrouter = SRS_NORMAL_OPERATION;
    //  SYSTEM_Status.storage = 	STS_NORMAL_OPERATION;
    SYSTEM_Status.srvrouter_stat = 1;
    //  SYSTEM_Status.storage_stat 	= SYS_NORMAL_OPERATION /* когда
    //  сервер будет присылать его реальный статус поставить здесь SYS_FAULT*/;
    SYSTEM_Status.storage_stat = 1;
    SYSTEM_Status.XAE21_fault = true;
    SYSTEM_Status.cam_switch_fault = true;

    arinc429Control.FORMAT1 = BASE_FORMAT1;

    //  npkts = 0;//DBG
    //  DBG_init_HAL();//DBG

    //==================================================================================//

    while (1)
    {
        
        /*
            Макрос _STR служит для генерации короткого импульса на определённом выводе микроконтроллера.
            Для измерения времени выполнения кода с помощью осциллографа (наверно)
        */
        _STR

        // services:
        // terminal 1
        TERMINAL_process(&termDbg);
        if (termDbg.cbOutput.count)
        {
            if (huart2.gState == HAL_UART_STATE_READY)
            {
                int count = termDbg.cbOutput.count;
                if (count > UART2_TX_BUFFER_SIZE) count = UART2_TX_BUFFER_SIZE;
                CBUFF_getFirstNBytes(&termDbg.cbOutput, (char *)uart2TxBuffer,
                                     count);
                // transmit uart 2
                HAL_UART_Transmit(&huart2, (unsigned char *)uart2TxBuffer,
                                  count, 100);
            }
        }

        // terminal 2
        TERMINAL_process(&termPc2MCU);
        if (termPc2MCU.cbOutput.count)
        {
            if (huart1.gState == HAL_UART_STATE_READY)
            {
                int count = termPc2MCU.cbOutput.count;
                if (count > UART1_TX_BUFFER_SIZE) count = UART1_TX_BUFFER_SIZE;
                CBUFF_getFirstNBytes(&termPc2MCU.cbOutput,
                                     (char *)uart1TxBuffer, count);
                // transmit uart 1
                HAL_UART_Transmit(&huart1, (unsigned char *)uart1TxBuffer,
                                  count, 100);
            }
        }

        // usart3 arinc-429 translate
        if (uart3NewPacket)
        {
            char buffer[127] = {0};
            char size = 127;
            uart3NewPacket = 0;

            ARINC429_Proto_InputPacket(uart3RecvBuffer, uart3RecvBufferIndex,
                                       &buffer[0], &size);
            if (size > 0)
            {
                HAL_UART_Transmit(&huart3, &buffer[0], size, 100);
            }
        }

        // devices:
        STM32_process();

        POWER_process();

        ARINC429_process();

        // check and reset uart1
        if (huart1.RxState == HAL_UART_STATE_READY)
        {
            __HAL_UART_CLEAR_OREFLAG(&huart1);
            if (HAL_UART_Receive_IT(&huart1, &uart1Recv, 1) != HAL_OK)
                Error_Handler();
        }  // */
        // check and reset uart2
        if (huart2.RxState == HAL_UART_STATE_READY)
        {
            __HAL_UART_CLEAR_OREFLAG(&huart2);
            if (HAL_UART_Receive_IT(&huart2, &uart2Recv, 1) != HAL_OK)
                Error_Handler();
        }  // */
        // check and reset uart3
        if (huart3.RxState == HAL_UART_STATE_READY)
        {
            __HAL_UART_CLEAR_OREFLAG(&huart3);
            if (HAL_UART_Receive_IT(&huart3, &uart3Recv, 1) != HAL_OK)
                Error_Handler();
        }  // */

        // проверка наличия 12В и передергивание 28в и 3в (запаралелено)
        char t = HAL_GPIO_ReadPin(GPI_12v_in_GPIO_Port, GPI_12v_in_Pin);

        in12vValue += t;
        in12vCnt++;

        if (in12vCnt >= 30)
        {  // 30-ти кратная проверка, для фильтрации единичных импульсов
            if (in12vValue == 0)
            {
                if (in12vTrigger == 0)
                {
                    in12vTrigger = 1;
                    HAL_GPIO_WritePin(GPO_28Vcam_en_GPIO_Port,
                                      GPO_28Vcam_en_Pin, GPIO_PIN_RESET);
                    HAL_GPIO_WritePin(GPO_hub1_cs_GPIO_Port, GPO_hub1_cs_Pin,
                                      GPIO_PIN_RESET);
                    HAL_GPIO_WritePin(GPO_hub2_cs_GPIO_Port, GPO_hub2_cs_Pin,
                                      GPIO_PIN_RESET);
                    HAL_GPIO_WritePin(GPO_hub3_cs_GPIO_Port, GPO_hub3_cs_Pin,
                                      GPIO_PIN_RESET);
                    HAL_GPIO_WritePin(GPO_hub4_cs_GPIO_Port, GPO_hub4_cs_Pin,
                                      GPIO_PIN_RESET);
                }
            }
            else if (in12vValue == 30)
            {
                if (in12vTrigger == 1)
                {
                    in12vTrigger = 0;
                    HAL_GPIO_WritePin(GPO_28Vcam_en_GPIO_Port,
                                      GPO_28Vcam_en_Pin, GPIO_PIN_SET);
                    HAL_GPIO_WritePin(GPO_hub1_cs_GPIO_Port, GPO_hub1_cs_Pin,
                                      GPIO_PIN_SET);
                    HAL_GPIO_WritePin(GPO_hub2_cs_GPIO_Port, GPO_hub2_cs_Pin,
                                      GPIO_PIN_SET);
                    HAL_GPIO_WritePin(GPO_hub3_cs_GPIO_Port, GPO_hub3_cs_Pin,
                                      GPIO_PIN_SET);
                    HAL_GPIO_WritePin(GPO_hub4_cs_GPIO_Port, GPO_hub4_cs_Pin,
                                      GPIO_PIN_SET);
                }
            }

            in12vCnt = 0;
            in12vValue = 0;
        }

        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */

        // Опрос переключателя выбора камеры и перевод в номер выбранной камеры
        sw_pos__ = MCP23008_Read_Reg(&ic_mcp23008, 9) &
                   0x3F;  // старшие два бита MCP23008 жестко сидят на +V

        u8_t ch_no;
        for (ch_no = 0; ch_no < CHANNELS_TOTAL; ch_no++)
        {
            if (sw_pos__ & 0x01) break;
            sw_pos__ >>= 1;
        }

        // Номер камеры передается как от 1-й до 6-й,
        // если камера не выбрана (напр. сломан переключатель) - будет передан 0
        // - признак неисправности
        Channel_No = (ch_no < CHANNELS_TOTAL ? ch_no + 1 : 0);

        SYSTEM_Status.cam_switch_fault =
            (Channel_No ? 0 : 1);  // если свитч неисправен, он будет давать '0'
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
    RCC_OscInitStruct.OscillatorType =
        RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_HSE;
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
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
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
    if (huart == &huart1)
    {
        // printf ("%02X  %c", uart1Recv, uart1Recv);
        TERMINAL_recvByte(&termPc2MCU, uart1Recv);
        HAL_UART_Receive_IT(&huart1, &uart1Recv, 1);
    }
    else if (huart == &huart2)
    {
        // printf ("%02X  %c", uart2Recv, uart2Recv);
        TERMINAL_recvByte(&termDbg, uart2Recv);
        HAL_UART_Receive_IT(&huart2, &uart2Recv, 1);
    }
    else if (huart == &huart3)
    {
        if (uart3Recv == (uint8_t)0xBE || uart3Recv == (uint8_t)0xCE)
        {
            uart3State = 1;
            uart3RecvBufferIndex = 0;
            uart3NewPacket = 0;
        }

        if (uart3State)
        {
            uart3RecvBuffer[uart3RecvBufferIndex] = uart3Recv;
            uart3RecvBufferIndex++;
            if (uart3RecvBufferIndex > 2)
            {
                uart3State = 0;
                uart3NewPacket = 1;
            }
        }

        // printf ("%02X  %c", uart3Recv, uart3Recv);
        HAL_UART_Receive_IT(&huart3, &uart3Recv, 1);
    }
}

/* USER CODE END 4 */

/**
 * @brief Инициализация флэш-памяти W25Q32.
 *
 * Выполняет инициализацию флэш-памяти W25Q32 с использованием указанного SPI
 * интерфейса и GPIO для выбора чипа (chip select).
 * Эта инициализация должна быть выполнена до инициализации STM32 и логгера,
 * чтобы обеспечить очистку флэш-памяти.
 *
 * @param hspi Указатель на SPI интерфейс (SPI_HandleTypeDef).
 * @param GPIO_Port Указатель на GPIO порт для выбора чипа.
 * @param GPIO_Pin Пин GPIO для выбора чипа.
 */
static void W25Q32_Flash_Init(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIO_Port, uint16_t GPIO_Pin)
{
    printf("W25Q32 init...\n");
    FLASH_W25Q32_init(hspi, GPIO_Port, GPIO_Pin);
}

/**
 * @brief Инициализация массива Ethernet-хабов.
 *
 * Эта функция инициализирует каждый элемент массива hub, устанавливая
 * SPI-интерфейс, порт GPIO и пин для выбора чипа (chip select).
 *
 * @param hub Массив структур T_KSZ9567S_SPI для инициализации.
 * @param spi Указатель на SPI-интерфейс, используемый всеми хабами.
 * @param ports Массив указателей на GPIO-порты для выбора чипа каждого хаба.
 * @param pins Массив пинов GPIO для выбора чипа каждого хаба.
 * @param count Количество хабов в массиве.
 */
void InitEthernetHubs(T_KSZ9567S_SPI hub[], SPI_HandleTypeDef *spi,
                      GPIO_TypeDef *ports[], uint16_t pins[], int count)
{
    for (int i = 0; i < count; i++)
    {
        hub[i].spi = spi;
        hub[i].GPIO_port = ports[i];
        hub[i].GPIO_pin = pins[i];
    }
}

/**
 * @brief Настройка регистров каждого Ethernet-хаба.
 *
 * Эта функция выполняет последовательность SPI-записей для настройки внутренних
 * регистров каждого хаба, включая установку задержки, сброс и конфигурацию
 * SGMII.
 *
 * @param hub Массив структур T_KSZ9567S_SPI, представляющих хабы для настройки.
 * @param count Количество хабов в массиве.
 */
void InitEthernetHubRegisters(T_KSZ9567S_SPI hub[], int count)
{
    for (int i = 0; i < count; i++)
    {
        KSZ9567_SPI_RegWriteByte(&hub[i], 0x6301,
                                 0x18);  // set ingress internal delay to 1.5ns
        KSZ9567_SPI_SgmiiRegWrite(&hub[i], 0x1F0000, 0x8140);  // reset
        KSZ9567_SPI_SgmiiRegWrite(
            &hub[i], 0x1F8001,
            0x14);  // (SGMII) set to MAC device, set SGMII Link Status to 1
        KSZ9567_SPI_SgmiiRegWrite(&hub[i], 0x1F0004, 0x20);
    }
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state
     */

    static u32_t Errs_Qty = 0;

    Errs_Qty++;
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
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
    /* User can add his own implementation to report the file name and line
       number, tex: printf("Wrong parameters value: file %s on line %d\r\n",
       file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
