/// @file       main.c
/// @author     Башмаков Р.А. bashmakovra@raitibor.ru
/// @brief      ВПО для STM32 СВР-Маршрутизатора
#include "_Main.h"

/// @brief      Конфигурация HI3220
/// @param      pin_id Идентификатор пина (из enum Tool_HI3220_PinID)
/// @return     Возвращает TOOLS_ERROR_CODE_ALL_OK в случае успешной настройки HI3220.
///                 В противном случае, возвращает код ошибки
int _Main_HI3220_Configuration(void);

/// @brief      Функция настройки цифровых входов
/// @details    Данная функция выполняет включение тактирования всех портов
/// @return     Возвращает 0 в случае успешного включения тактирования всех портов.
///                 В противном случае возвращает код ошибки
int _Main_GPIO_Configuration(void);

/// @brief      Интерфейс UART, используемый для связи 
UART_HandleTypeDef huart1;

/// @brief      Интерфейс UART, используемый для связи 
UART_HandleTypeDef huart2;

/// @brief      Интерфейс UART, используемый для связи с ПК(?) //TODO выяснить как называется конкретно
UART_HandleTypeDef _Main_Handle_UART3_PC;

DMA_HandleTypeDef hdma_usart3_tx;

SPI_HandleTypeDef hspi2;

int main(void)
{
    int Result = TOOLS_ERROR_CODE_ALL_OK;

    Result += _Main_GPIO_Configuration();

    Result += _Main_Clock_Configuration();
    // Result += _Main_SPI_1_Configuration();
    // Result += _Main_SPI_2_Configuration();
    // Result += _Main_SPI_3_Configuration();
    // Result += _Main_I2C_1_Configuration();
    // Result += _Main_I2C_4_Configuration();
    // Result += _Main_ADC1_Configuration();
    // Result += _Main_RTC_Configuration();
    Result += _Main_USART_UART_1_Configuration(&huart1);
    Result += _Main_USART_UART_2_Configuration(&huart2);
    Result += _Main_USART_UART_3_Configuration(&_Main_Handle_UART3_PC);
    // Result += _Main_TIM_12_Configuration();// + TIMERS_setTimer

    HAL_Delay(1000);

    // Result += _Main_Parameters_Read();
    // Result += _Main_TERMINAL_init();
    // Result += _Main_W25Q32_Flash_Init();

    Result += _Main_Critical_Error_Handler(Result);

    while (1)
    {
        // ARINC_Handler();

        // if (Status_Control == Control_State_Error)
        // {
        //     ARINC_Set_Error_Maint_SW();
        // }

        // ARINC_Process();
    }

    return 0;
}

void _Main_Clock_Configuration(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
    RCC_OscInitStruct.LSIState       = RCC_LSI_ON;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM       = 25;
    RCC_OscInitStruct.PLL.PLLN       = 432;
    RCC_OscInitStruct.PLL.PLLP       = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ       = 2;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        return -1;
    }

    if (HAL_PWREx_EnableOverDrive() != HAL_OK)
    {
        return -2;
    }

    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
    {
        return -3;
    }

    return TOOLS_ERROR_CODE_ALL_OK;
}

int _Main_GPIO_Configuration(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

    return TOOLS_ERROR_CODE_ALL_OK;
}

int _Main_USART_UART_1_Configuration(UART_HandleTypeDef *huart1)
{
    huart1->Instance                    = USART1;
    huart1->Init.BaudRate               = 115200;
    huart1->Init.WordLength             = UART_WORDLENGTH_8B;
    huart1->Init.StopBits               = UART_STOPBITS_1;
    huart1->Init.Parity                 = UART_PARITY_NONE;
    huart1->Init.Mode                   = UART_MODE_TX_RX;
    huart1->Init.HwFlowCtl              = UART_HWCONTROL_NONE;
    huart1->Init.OverSampling           = UART_OVERSAMPLING_16;
    huart1->Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXOVERRUNDISABLE_INIT;
    huart1->AdvancedInit.OverrunDisable = UART_ADVFEATURE_OVERRUN_DISABLE;

    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        return -1;
    }

    return TOOLS_ERROR_CODE_ALL_OK;
}

int _Main_USART_UART_2_Configuration(UART_HandleTypeDef *huart2)
{
    huart2->Instance                    = USART2;
    huart2->Init.BaudRate               = 115200;
    huart2->Init.WordLength             = UART_WORDLENGTH_8B;
    huart2->Init.StopBits               = UART_STOPBITS_1;
    huart2->Init.Parity                 = UART_PARITY_NONE;
    huart2->Init.Mode                   = UART_MODE_TX_RX;
    huart2->Init.HwFlowCtl              = UART_HWCONTROL_NONE;
    huart2->Init.OverSampling           = UART_OVERSAMPLING_16;
    huart2->Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
    huart2->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXOVERRUNDISABLE_INIT;
    huart2->AdvancedInit.OverrunDisable = UART_ADVFEATURE_OVERRUN_DISABLE;

    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        return -1;
    }

    return TOOLS_ERROR_CODE_ALL_OK;
}

int _Main_USART_UART_3_Configuration(UART_HandleTypeDef *huart3)
{
    huart3->Instance                    = USART3;
    huart3->Init.BaudRate               = 115200;
    huart3->Init.WordLength             = UART_WORDLENGTH_8B;
    huart3->Init.StopBits               = UART_STOPBITS_1;
    huart3->Init.Parity                 = UART_PARITY_NONE;
    huart3->Init.Mode                   = UART_MODE_TX_RX;
    huart3->Init.HwFlowCtl              = UART_HWCONTROL_NONE;
    huart3->Init.OverSampling           = UART_OVERSAMPLING_16;
    huart3->Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
    huart3->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXOVERRUNDISABLE_INIT;
    huart3->AdvancedInit.OverrunDisable = UART_ADVFEATURE_OVERRUN_DISABLE;

    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        return -1;
    }

    return TOOLS_ERROR_CODE_ALL_OK;
}

void _Main_Critical_Error_Handler(int Value)
{
    if (Value == TOOLS_ERROR_CODE_ALL_OK || HAL_OK)
    {
        return;
    }

    // Alive_To_Dead(); //TODO разобраться
    // Black_Box_Program(); //TODO разобраться
    __disable_irq();
    while (1)
        ;
}