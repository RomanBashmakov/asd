/// @file    main.c
/// @author  Башмаков Р.А. bashmakovra@raitibor.ru
/// @brief   ВПО для STM32 СВР-Маршрутизатора
#include "_Main.h"

/// @brief   Конфигурация HI3220
/// @param   pin_id Идентификатор пина (из enum Tool_HI3220_PinID)
/// @return  Возвращает TOOLS_ERROR_CODE_ALL_OK в случае успешной настройки HI3220.
///          В противном случае, возвращает код ошибки
int _Main_HI3220_Configuration(void);

/// @brief   Функция настройки цифровых входов
/// @details Данная функция выполняет включение тактирования всех портов
/// @return  Возвращает 0 в случае успешного включения тактирования всех портов.
///          В противном случае возвращает код ошибки
int _Main_GPIO_Configuration(void);

int main(void)
{
    int Result = TOOLS_ERROR_CODE_ALL_OK;

    Result += _Main_GPIO_Configuration();
    
    // _Main_Critical_Error_Handler(HAL_Init());

    // Result += _Main_Clock_Configuration();
    // Result += _Main_SPI_1_Configuration();
    // Result += _Main_SPI_2_Configuration();
    // Result += _Main_SPI_3_Configuration();
    // Result += _Main_I2C_1_Configuration();
    // Result += _Main_I2C_4_Configuration();
    // Result += _Main_ADC1_Configuration();
    // Result += _Main_RTC_Configuration();
    // Result += _Main_USART_UART_1_Configuration();
    // Result += _Main_USART_UART_2_Configuration();
    // Result += _Main_USART_UART_3_Configuration();
    // Result += _Main_TIM_12_Configuration();// + TIMERS_setTimer

    // HAL_Delay(1000);

    // Result += _Main_Parameters_Read();
    // Result += _Main_TERMINAL_init();
    // Result += _Main_W25Q32_Flash_Init();

    // Result += HI3220_Configuration_MW();

    // Result += _Main_Critical_Error_Handler(Result);

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

int _Main_GPIO_Configuration(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

    return 0;
}