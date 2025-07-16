/// @file    main.c
/// @author  Башмаков Р.А. bashmakovra@raitibor.ru
/// @brief   ВПО для STM32 СВР-Маршрутизатора

#include "main.h"

/// @brief   Функция настройки частотного блока
/// @details Данная функция задает источник тактирования, делители и множители частоты,
///          а также включает тактирование используемых блоков
/// @return  Возвращает 0 в случае успешной настройки частотного блока.
///          В противном случае возвращает код ошибки
int _Main_SystemClock_Config(void)

/// @brief   Инициализация встроеной "периферии".
/// @details Функция выполняет последовательную инициализацию следующих
///          периферийных модулей:
///          GPIO, I2C1, I2C4, USART1, USART2, USART3, TIM12, RTC, ADC1, SPI1,
///          SPI2, SPI3, DMA
/// @warning Данная функция должна быть вызвана для настройки аппаратных модулей
///          перед их использованием.
/// @retval  None
void _Main_Init_Peripherals(void);

/// @brief   Инициализация флэш-памяти W25Q32.
/// @details Выполняет инициализацию флэш-памяти W25Q32 с использованием
/// указанного SPI
///          интерфейса и GPIO для выбора чипа.
///          Эта инициализация должна быть выполнена до инициализации STM32 и
///          логгера, чтобы обеспечить очистку флэш-памяти.
/// @param   hspi Указатель на SPI интерфейс (SPI_HandleTypeDef).
/// @param   GPIO_Port Указатель на GPIO порт для выбора чипа.
/// @param   GPIO_Pin Пин GPIO для выбора чипа.
/// @retval  None
static void W25Q32_Flash_Init(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIO_Port,
                              uint16_t GPIO_Pin);

/// @brief   Устанавливает состояние всех пинов выбора чипа Ethernet-хабов.
/// @details Эта функция упрощает управление состоянием пинов выбора чипа (chip
/// select)
///          для всех четырёх Ethernet-хабов, объединяя повторяющиеся вызовы
///          HAL_GPIO_WritePin в одном месте.
/// @param   state Желаемое состояние пинов: GPIO_PIN_SET или GPIO_PIN_RESET.
/// @retval  None
void _Main_SetHubCSPins(GPIO_PinState state);

/// @brief   Проверка наличия 12В питания с фильтрацией помех.
/// @details Функция выполняет 30-кратное считывание состояния входа 12В
///          питания,
///          суммирует значения для фильтрации единичных импульсов и управляет
///          состоянием питания камер и Ethernet-хабов в зависимости от
///          результата. Использует статические локальные переменные для
///          сохранения состояния между вызовами. При отсутствии питания
///          (in12vValue == 0) и если питание было включено, функция отключает
///          питание камер и хабов. При наличии питания (in12vValue == 30) и
///          если питание было отключено, функция включает питание камер и
///          хабов.
/// @retval  None
void _Main_Check12VPower(void);

/// @brief   Настройка регистров каждого Ethernet-хаба.
/// @details Эта функция выполняет последовательность SPI-записей для настройки
///          внутренних
///          регистров каждого хаба, включая установку задержки, сброс и
///          конфигурацию SGMII.
/// @param   hub Массив структур T_KSZ9567S_SPI, представляющих хабы для
/// настройки.
/// @param   count Количество хабов в массиве.
/// @retval  None
void _Main_InitEthernetHubRegisters(T_KSZ9567S_SPI hub[], int count);

/// @brief   Инициализация массива Ethernet-хабов.
/// @details Эта функция инициализирует каждый элемент массива hub, устанавливая
///          SPI-интерфейс, порт GPIO и пин для выбора чипа (chip select).
/// @param   hub Массив структур T_KSZ9567S_SPI для инициализации.
/// @param   spi Указатель на SPI-интерфейс, используемый всеми хабами.
/// @param   ports Массив указателей на GPIO-порты для выбора чипа каждого хаба.
/// @param   pins Массив пинов GPIO для выбора чипа каждого хаба.
/// @param   count Количество хабов в массиве.
/// @retval  None
void _Main_InitEthernetHubs(T_KSZ9567S_SPI hub[], SPI_HandleTypeDef *spi,
                      GPIO_TypeDef *ports[], uint16_t pins[], int count);
                    
/// @brief   Отправки данных терминала.
/// @param   term терминал (например, &termDbg).
/// @param   huart указатель на UART (&huart1 или &huart2).
/// @param   buffer TX-буфер (uart1TxBuffer или uart2TxBuffer).
/// @param   buffer_size размер буфера (UART1_TX_BUFFER_SIZE).
/// @retval  None
void _Main_SendTerminalData(TERMINAL_t* term, UART_HandleTypeDef* huart, 
                     uint8_t* buffer, size_t buffer_size);

/// @brief   Проверка и перезапуск UART (то, что было в while(1))
/// @retval  None
void UART_CheckErrors(void);

/// @brief   Функция обработки ARINC-429 по UART3
/// @details используется ВЕРОЯТНО для эмуляции ARINC-429 при отсутствии HI-3220.
/// @retval  None
void _Main_ARINC429_process_UART(void);

/// @brief   Определяет выбранный канал камеры по положению переключателя
/// @details Старшие 2 бита MCP23008 игнорируются, так как они жестко подключены к +V
/// @retval  Номер канала (0-CHANNELS_TOTAL-1)
uint8_t _Main_ReadCameraSelector(void);

/// @brief   Версия и дата сборки прошивки
const char strVersionDT[] = "IL114  22.07.22  20:38";

/// @brief   Терминал для отладочного вывода
TTerminal termDbg;

/// @brief   Терминал для связи с ПК
TTerminal termPc2MCU;

/// @brief   Переменные приёма UART
uint8_t uart1Recv;
uint8_t uart2Recv;
uint8_t uart3Recv;

/// @brief   Матрица A300
enum A300_Matrix_e A300_Matrix;

/// @brief   Структура статуса системы
Status_t SYSTEM_Status;

static UART_Ctx uart3 = {
    .state = 0,          //uart3.state = uart3State 			
    .buffer = {0},       //uart3.buffer = uart3RecvBuffer 		
    .bufferIndex = 0,    //uart3.bufferIndex = uart3RecvBufferIndex 
    .newDataFlag = 0     //uart3.newDataFlag = uart3NewPacket 		
};

/// @brief   Буфер передачи UART1 для DMA
char uart1TxBuffer[UART1_TX_BUFFER_SIZE];

/// @brief   Буфер передачи UART2 для DMA
char uart2TxBuffer[UART2_TX_BUFFER_SIZE];

/// @brief   Таймер отладки
TTimer tmrDebug;

/// @brief   Точка входа в программу
/// @retval  Возвращает 0
int main(void)
{
    HAL_Init();

    _Main_SystemClock_Config();
    _Main_Init_Peripherals();

    TERMINAL_init(&termDbg);
    TERMINAL_init(&termPc2MCU);
    
    TIMERS_setTimer(&tmrDebug, 30000);

    _Main_W25Q32_Flash_Init(&hspi1, GPO_nvm_ncs_GPIO_Port, GPO_nvm_ncs_Pin);

    STM32_init();  // инициализация стм32 после флеш и до инициализации лога !!!
                   // т.к. здесь может произойти очистка флэш-памяти

    _Main_SetHubCSPins(
        GPIO_PIN_RESET);  // Сброс всех пинов выбора чипа Ethernet-хабов

    // включение блока питания камер с задержкой
    HAL_Delay(MAIN_SYSTEM_START_DELAY_MS);
    HAL_GPIO_WritePin(GPO_28Vcam_en_GPIO_Port, GPO_28Vcam_en_Pin, GPIO_PIN_SET);

    _Main_SetHubCSPins(
        GPIO_PIN_SET);  // Установка всех пинов выбора чипа Ethernet-хабов

    HAL_Delay(200);

    // Инициализация ethernet hubs
    T_KSZ9567S_SPI hub[HUBS_CNT];

    _Main_InitEthernetHubs(
        hub, &hspi3,
        (GPIO_TypeDef *[]){GPO_hub1_cs_GPIO_Port, GPO_hub2_cs_GPIO_Port,
                           GPO_hub3_cs_GPIO_Port, GPO_hub4_cs_GPIO_Port},
        (uint16_t[]){GPO_hub1_cs_Pin, GPO_hub2_cs_Pin, GPO_hub3_cs_Pin,
                     GPO_hub4_cs_Pin},
        HUBS_CNT);

    _Main_InitEthernetHubRegisters(hub, HUBS_CNT);

    // включение вентилятора
    HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_2);

    // включение блока питания компьютера
    // после настройки SGMII
    HAL_GPIO_WritePin(GPO_12V_en_GPIO_Port, GPO_12V_en_Pin, GPIO_PIN_SET);
    HAL_Delay(500);

    POWER_init(&hi2c1, &hi2c4);
    HAL_Delay(300);

    ARINC429_init(&hspi2);

    HAL_UART_Receive_IT(&huart1, &uart1Recv, 1);
    HAL_UART_Receive_IT(&huart2, &uart2Recv, 1);
    HAL_UART_Receive_IT(&huart3, &uart3Recv, 1);

    // включение вентилятора на 100%
    htim12.Instance->CCR2 = 100;

    _Main_System_Init();

    while (1)
    {
        TERMINAL_process(&termDbg);
        TERMINAL_process(&termPc2MCU);

        _Main_SendTerminalData(&termDbg, &huart2, uart2TxBuffer, UART2_TX_BUFFER_SIZE);
        _Main_SendTerminalData(&termPc2MCU, &huart1, uart1TxBuffer, UART1_TX_BUFFER_SIZE);

        // devices:
        ADC_process();

        POWER_process();

        // usart3 -> ARINC429
        _Main_ARINC429_process_UART();
        ARINC429_process();

        UART_CheckErrors();

        // проверка наличия 12В и передергивание 28в и 3в (запаралелено)
        _Main_Check12VPower();

        // Опрос переключателя выбора камеры и перевод в номер выбранной камеры

        // Номер камеры передается как от 1-й до 6-й,
        // если камера не выбрана (напр. сломан переключатель) - будет передан 0
        // - признак неисправности
        SetChannelNo(_Main_ReadCameraSelector);

        SYSTEM_Status.cam_switch_fault =
            (GetChannelNo() ? 0
                            : 1);  // если свитч неисправен, он будет давать '0'
    }
}


//=================================================================================
uint8_t _Main_ReadCameraSelector(void) {
    const uint8_t REGISTER = 9;
    const uint8_t MASK = 0x3F;
    
    // Читаем текущее положение переключателя
    uint8_t switch_position = MCP23008_Read_Reg(&ic_mcp23008, REGISTER) & MASK;
    
    // Определяем первый установленный бит (выбранный канал)
    for(uint8_t channel = 0; channel < CHANNELS_TOTAL; channel++) {
        if(switch_position & 0x01) {
            return channel;
        }
        switch_position >>= 1;
    }
    
    return 0; // Значение по умолчанию, если ни один канал не выбран
}

void _Main_ARINC429_process_UART(void) {
    if (uart3.newDataFlag) {
        char buffer[127] = {0};
        uint8_t size = sizeof(buffer);  // 127

        // Сбрасываем флаг перед обработкой
        uart3.newDataFlag = 0;

        // Формируем пакет
        ARINC429_Proto_InputPacket(uart3.buffer, uart3.bufferIndex, buffer, &size);

        // Отправляем, если есть данные

        if (size > 0) {
            HAL_UART_Transmit(&huart3, (uint8_t*)buffer, size, 100);
        }
    }
}

void UART_CheckErrors(void) {
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
}

void _Main_SendTerminalData(TERMINAL_t* term, UART_HandleTypeDef* huart, 
                     uint8_t* buffer, size_t buffer_size) {
    if (term->cbOutput.count && huart->gState == HAL_UART_STATE_READY) {
        int count = term->cbOutput.count;
        if (count > buffer_size) count = buffer_size;
        CBUFF_getFirstNBytes(&term->cbOutput, (char*)buffer, count);
        HAL_UART_Transmit(huart, buffer, count, 100);
    }
}

void _Main_System_Init(void)
{
    // Внешнее объявление структуры MCP23008 для работы с устройством
    extern MCP23008_t ic_mcp23008;

    // Инициализация MCP23008
    MCP23008_init();

    // Установка матрицы A300 в нормальный режим работы
    A300_Matrix = AMX_NORMAL_OPERATION;

    // Установка системного статуса в состояние ошибки (SYS_FAULT)
    SYSTEM_Status.system_stat = SYS_FAULT;

    // Установка статуса маршрутизатора сервиса в 1 (возможно, индикатор ошибки)
    SYSTEM_Status.srvrouter_stat = 1;

    // Установка флага ошибки XAE21
    SYSTEM_Status.XAE21_fault = true;

    // Установка флага ошибки переключателя камеры
    SYSTEM_Status.cam_switch_fault = true;

    // Установка формата ARINC429 в базовый формат
    arinc429Control.FORMAT1 = BASE_FORMAT1;
}

int _Main_SystemClock_Config(void)
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
        return -1;
    }
    /** Activate the Over-Drive mode
     */
    if (HAL_PWREx_EnableOverDrive() != HAL_OK)
    {
        return -2;
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
        return -3;
    }

    return 0;
}

/// @brief    Переопределенный HAL-овский weak-колбэк на приём по UART
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    //Терминал ПК
    if (huart == &huart1)
    {
        TERMINAL_recvByte(&termPc2MCU, uart1Recv);
        HAL_UART_Receive_IT(&huart1, &uart1Recv, 1);
    }

    //Терминал Debug
    else if (huart == &huart2)
    {
        TERMINAL_recvByte(&termDbg, uart2Recv);
        HAL_UART_Receive_IT(&huart2, &uart2Recv, 1);
    }

    //ARINC
    else if (huart == &huart3)
    {
        if (uart3Recv == (uint8_t)0xBE || uart3Recv == (uint8_t)0xCE)
        {
            uart3.state = 1;
            uart3.bufferIndex = 0;
            uart3.newDataFlag = 0;
        }

        if (uart3.state)
        {
            uart3.buffer[uart3.bufferIndex] = uart3Recv;
            uart3.bufferIndex++;
            if (uart3.bufferIndex > 2)
            {
                uart3.state = 0;
                uart3.newDataFlag = 1;
            }
        }

        HAL_UART_Receive_IT(&huart3, &uart3Recv, 1);
    }
}

void _Main_Check12VPower(void)
{
    static char in12vCnt = 0;
    static char in12vValue = 0;
    static char in12vTrigger = 0;

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
                HAL_GPIO_WritePin(GPO_28Vcam_en_GPIO_Port, GPO_28Vcam_en_Pin,
                                  GPIO_PIN_RESET);
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
                HAL_GPIO_WritePin(GPO_28Vcam_en_GPIO_Port, GPO_28Vcam_en_Pin,
                                  GPIO_PIN_SET);
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
}

void _Main_SetHubCSPins(GPIO_PinState state)
{
    HAL_GPIO_WritePin(GPO_hub1_cs_GPIO_Port, GPO_hub1_cs_Pin, state);
    HAL_GPIO_WritePin(GPO_hub2_cs_GPIO_Port, GPO_hub2_cs_Pin, state);
    HAL_GPIO_WritePin(GPO_hub3_cs_GPIO_Port, GPO_hub3_cs_Pin, state);
    HAL_GPIO_WritePin(GPO_hub4_cs_GPIO_Port, GPO_hub4_cs_Pin, state);
}

void _Main_Init_Peripherals(void)
{
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_SPI1_Init();
    MX_SPI2_Init();
    MX_SPI3_Init();
    MX_I2C1_Init();
    MX_ADC1_Init();
    MX_RTC_Init();
    MX_I2C4_Init();
    MX_USART3_UART_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();
    MX_TIM12_Init();
}

static void _Main_W25Q32_Flash_Init(SPI_HandleTypeDef *hspi, GPIO_TypeDef *GPIO_Port,
                              uint16_t GPIO_Pin)
{
    printf("W25Q32 init...\n");
    FLASH_W25Q32_init(hspi, GPIO_Port, GPIO_Pin);
}

void _Main_InitEthernetHubs(T_KSZ9567S_SPI hub[], SPI_HandleTypeDef *spi,
                      GPIO_TypeDef *ports[], uint16_t pins[], int count)
{
    for (int i = 0; i < count; i++)
    {
        hub[i].spi = spi;
        hub[i].GPIO_port = ports[i];
        hub[i].GPIO_pin = pins[i];
    }
}

void _Main_InitEthernetHubRegisters(T_KSZ9567S_SPI hub[], int count)
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

/// @brief   Обработка ошибок
/// @retval  None
void Error_Handler(void)
{
    static u32_t Errs_Qty = 0;
    Errs_Qty++;
}

/// @brief   Функция отладочного вывода через SWD (Serial Wire Debug) интерфейс
///          STM32.
/// @details Используется для перенаправления вывода функций printf и puts
/// @param   file Не используется, параметр для совместимости с системным
/// вызовом write
/// @param   ptr Указатель на буфер с данными для вывода
/// @param   len Количество байт для вывода из буфера
/// @retval  Количество успешно выведенных байт.
int _write(int file, char *ptr, int len)
{
    int i = 0;
    for (i = 0; i < len; i++) ITM_SendChar((*ptr++));
    return len;
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
