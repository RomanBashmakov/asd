/// @file       ARINC.c
/// @author     Башмаков Р.А. bashmakovra@raitibor.ru
/// @brief      См. ARINC.h

#include "ARINC.h"

#include "Circular_Buffer.h"
#include "Tool_HI3220.h"

//TODO уточнить значения макросов, потому что в оригинале были маг числа и иногда разные в разных местах

/// @brief      Канал 0
#define ARINC_FIFO_CHANNEL_0 0

/// @brief      Канал 1
#define ARINC_FIFO_CHANNEL_1 1

/// @brief      Таймаута связи с сервером управления
#define ARINC_PC_RECEIVE_TIMEOUT_CONTROL 3000

/// @brief      Таймаута связи с сервером даты/времени
#define ARINC_PC_RECEIVE_TIMEOUT_DATETIME 3000//или 5000?

/// @brief      Таймаута связи с сервером ХАЭ-21 (авиационный хронометр)
#define ARINC_PC_RECEIVE_TIMEOUT_XAE21 2500//

/// @brief      Общее количество камер в системе
#define ARINC_CHANNELS_TOTAL 6U

/// @brief      Общее количество камер в системе
#define ARINC_INA226_MAX_POWER 2.0f

/// @brief Порт и пин RX
#define ARINC_PIN_RX GPIOD, GPIO_PIN_8  // mcu2pc_Pin

/// @brief Порт и пин TX
#define ARINC_PIN_TX GPIOD, GPIO_PIN_9  // pc2mcu_Pin

/// @brief Порт и пин MRST
#define ARINC_PIN_MRST GPIOD, GPIO_PIN_0  // GPO_ar429_rstn_Pin

/// @brief Порт и пин READY
#define ARINC_PIN_READY GPIOD, GPIO_PIN_4  // GPI_ar429_ready_Pin

/// @brief Порт и пин RUN
#define ARINC_PIN_RUN GPIOD, GPIO_PIN_1  // GPO_ar429_run_Pin

/// @brief Порт и пин INT
#define ARINC_PIN_INT GPIOD, GPIO_PIN_3  // GPI_ar429_intn_Pin

/// @brief Порт и пин ACK
#define ARINC_PIN_ACK GPIOD, GPIO_PIN_2  // GPO_ar429_ack_Pin

/// @brief Порт и пин CS
#define ARINC_PIN_CS GPIOB, GPIO_PIN_4  // GPO_ar429_csn_Pin

/// @brief      Период передачи пакета по ARINC 429
#define ARINC_A429_L0300_SEND_PERIOD 500

/// @brief      ?//TODO по-моему это пороговое значение тока потребления ниже которого камера считается выключенной надо уточнить
#define ARINC_CAMERA_CURRENT_THRESHOLD 0.05

/// @brief      Количество элементов в кольцевом буфере
#define ARINC_UART_OUTPUT_BUFFER_LENGTH 200

#define ARINC_UART3_TX_BUFFER_SIZE 200 //UART3_TX_BUFFER_SIZE

/// @brief  Структура с описанием одного GPIO пина ARINC
typedef struct ARINC_Pin_Struct
{
    GPIO_TypeDef *Port;        ///< Аппаратный порт GPIO
    uint16_t      Pin;         ///< Номер пина GPIO
    uint8_t       IsInverted;  ///< "1" если пин инвертированный
} ARINC_Pin_Struct;

/// @brief      Перечисление режимов инвертирования пинов HI3220
typedef enum ARINC_HI3220_Pin_Inverted_Enum
{
    ARINC_HI3220_PIN_INVERTED_OFF = 0,  ///< Пин не инвертирован
    ARINC_HI3220_PIN_INVERTED_ON  = 0,  ///< Пин инвертирован
} ARINC_HI3220_Pin_Inverted_Enum;

/// @brief      Список входных пинов
/// @warning    Порядок элементов в Tool_HI3220_GPI_Enum и порядок инициализации структур в массиве ARINC_GPI_Pins_Map <br>
///                 должны быть строго идентичны, иначе прозойдет ошибочное управление ложным пином <br>
///                 в ARINC_HI3220_Write_Pin/ARINC_HI3220_Read_Pin <br>
///                 Программа будет работать некорректно, но никаких ошибок при компиляции при этом не будет
static const ARINC_Pin_Struct ARINC_GPI_Pins_Map[TOOL_HI3220_GPI_COUNT] = {
    { ARINC_PIN_READY, ARINC_HI3220_PIN_INVERTED_OFF },
    { ARINC_PIN_INT, ARINC_HI3220_PIN_INVERTED_OFF }
};

/// @brief      Список выходных пинов
/// @warning    Порядок элементов в Tool_HI3220_GPO_Enum и порядок инициализации структур в массиве ARINC_GPO_Pins_Map <br>
///                 должны быть строго идентичны, иначе прозойдет ошибочное управление ложным пином <br>
///                 в ARINC_HI3220_Write_Pin/ARINC_HI3220_Read_Pin <br>
///                 Программа будет работать некорректно, но никаких ошибок при компиляции при этом не будет
static const ARINC_Pin_Struct ARINC_GPO_Pins_Map[TOOL_HI3220_GPO_COUNT] = {
    { ARINC_PIN_MRST, ARINC_HI3220_PIN_INVERTED_ON },
    { ARINC_PIN_RUN, ARINC_HI3220_PIN_INVERTED_OFF },
    { ARINC_PIN_ACK, ARINC_HI3220_PIN_INVERTED_OFF },
    { ARINC_PIN_CS, ARINC_HI3220_PIN_INVERTED_ON }
};

/// @brief      Структура кольцевого буфера для асинхронной передачи по UART к ПК
Circular_Buffer_Struct ARINC_UART_Output_Circular_Buffer; //TCbuffer cbARINCUartOutput //TODO пока для ориентации пусть побудет, чтобы проще искать по старому и новому коду

/// @brief      Буфер внутри кольцевого буфера ARINC_UART_Output_Circular_Buffer
uint8_t         ARINC_TX_Buffer[ARINC_UART_OUTPUT_BUFFER_LENGTH]; //arincTxBuffer //TODO пока для ориентации пусть побудет, чтобы проще искать по старому и новому коду

/// @brief      Функция записи на цифровой вывод МК
/// @param[in]  Pin    Вывод МК, подключенного к HI3220 (см. Tool_HI3220_GPO_Enum)
/// @param[in]  State  Устанавливаемое значение (см. Tool_Common_Pin_State_Enum)
/// @warning    Последовательность пинов из Tool_HI3220_GPO_Enum должна точно соответствовать ARINC_GPO_Pins_Map
void ARINC_HI3220_Write_Pin(const Tool_HI3220_GPO_Enum Pin, const Tool_Common_Pin_State_Enum State);

/// @brief      Функция чтения цифрового вывода МК
/// @param[in]  Pin Вывод МК, подключенный к HI3220 (см. Tool_HI3220_GPI_Enum)
/// @warning    Последовательность пинов из Tool_HI3220_GPI_Enum должна точно соответствовать ARINC_GPI_Pins_Map
/// @return     Текущее значение на указанном выводе МК (см. Tool_Common_Pin_State_Enum)
Tool_Common_Pin_State_Enum ARINC_HI3220_Read_Pin(const Tool_HI3220_GPI_Enum Pin);

/// @brief      Функция конфигурации ARINC HI-3220 для текущего проекта
/// @details    Поэтапная настройка работы с HI-3220:                                                                       <br>
///                 - Проверка корректности внутренней конфигурации модуля                                                  <br>
///                 - Чтение и вывод регистра MCR (главного управляющего регистра)                                          <br>
///                 - Включение модулей приёма и передачи ARINC 429 (MCR A429RX/AX29TX=1)                                   <br>
///                 - Конфигурирование передающих каналов (ATXC0, ATXC1): формат, скорость 12.5кбит/с, включение паритета   <br>
///                 - Конфигурирование приёмных каналов (ARXC0, ARXC1): включение, скорость 12.5кбит/с, паритет             <br>
///                 - Отладочное считывание и вывод настроек каналов ARXC0 и ARXC1                                          <br>
///                 - Настройка Processor Interrupt Enable Register (PIER): разрешение INT по новому слову в RXRAM2         <br>
///                 - Отладочное считывание и вывод значения PIER                                                           <br>
///                 - Разрешение приёмных прерываний на RX0 и RX1 (RIERL), считывание и вывод                               <br>
///                 - Инициализация карт разрешения приёма меток (Label Enable Maps, RXEN_MAP): разрешить все метки         <br>
///                 - Инициализация карт разрешения прерываний по меткам (RXINT_MAP): разрешить все метки                   <br>
///             Все значения и маски формируются через специализированные структуры/union.                                  <br>
/// @see        Адресация, структура регистров и области памяти в соответствии с стр. 13–17, 23–24
/// @return     Возвращает TOOLS_ERROR_CODE_ALL_OK в случае успешного выполнения функции.
///                 В противном случае, возвращает код ошибки
int ARINC_HI3220_Configuration(void);

/// @brief      Глобальная структура управления кабиной
/// @details    Содержит текущее состояние органов управления CAPT и FO.
///                 Обновляется при приёме данных от внешних систем.
ARINC_Control_Struct ARINC_Control;

/// @brief      Глобальная структура даты и времени
/// @details    Содержит текущие значения времени и даты, получаемые от ХАЭ-21
///                 через ARINC слова 150 (время в binary) и 260 (дата в BCD).
ARINC_Date_Time_Struct ARINC_DateTime;

/// @brief      Номер активного канала камеры [1:6]
/// @details    Определяется по положению переключателя камер.
///                 Значение 0 означает неисправность переключателя
uint8_t ARINC_Channel_Number;

/// @brief      Центральная структура системного статуса
/// @details    Глобальная переменная для централизованного мониторинга состояния
///                 всех подсистем. Обновляется различными модулями и используется
///                 для формирования сводного статуса в ARINC слове 0300
ARINC_System_Status_Struct ARINC_System_Status;

/// @brief      Интерфейс SPI для связи с HI3220
SPI_HandleTypeDef *ARINC_SPI_Handle;

/// @brief      Интерфейс UART для связи с ПК
UART_HandleTypeDef *ARINC_UART_Handle;

// TODO заглушка
int ARINC_HI3220_SPI_TX(const uint8_t *const Data_Ptr, const uint32_t Data_Size)
{
    return 0;
}
// TODO заглушка
int ARINC_HI3220_SPI_RX(uint8_t *const Data_Ptr, const uint32_t Data_Size)
{
    return 0;
}
// TODO заглушка
int ARINC_HI3220_SPI_TX_RX(const uint8_t *const Tx_Data_Ptr, uint8_t *const Rx_Data_Ptr, const uint32_t Data_Size)
{
    return 0;
}
// TODO заглушка
void ARINC_Print(uint8_t *const Data_Ptr, const uint32_t Data_Size)
{
    return;
}

/// @brief      Вывод кода ошибки
void ARINC_Print_Err(uint8_t Error_Id)
{
    uint8_t const Data_Ptr[] = {Error_Id};
    ARINC_Print(&Data_Ptr[0], 1);
}

int ARINC_Configuration(SPI_HandleTypeDef *SPI_Handle_Ptr, SPI_HandleTypeDef *SPI_Handle_Black_Box_Ptr, UART_HandleTypeDef *UART_Handle_Ptr)
{
    if(!SPI_Handle_Ptr) 
    {
        return TOOLS_ERROR_CODE_ARG;
    }

    if(!UART_Handle_Ptr) 
    {
        return TOOLS_ERROR_CODE_ARG;
    }

    ARINC_SPI_Handle  = SPI_Handle_Ptr;
    ARINC_UART_Handle = UART_Handle_Ptr;

    Tool_HI3220_Hardware_Reset();

    Tool_HI3220_Configuration(ARINC_HI3220_SPI_TX,
                              ARINC_HI3220_SPI_RX,
                              ARINC_HI3220_SPI_TX_RX,
                              ARINC_HI3220_Write_Pin,
                              HAL_Delay,
                              ARINC_Print,
                              HAL_GetTick,
                              ARINC_HI3220_Read_Pin);

    Tool_HI3220_Self_Test();

    Circular_Buffer_Init(&ARINC_UART_Output_Circular_Buffer, ARINC_TX_Buffer, ARINC_UART_OUTPUT_BUFFER_LENGTH);

    return TOOLS_ERROR_CODE_ALL_OK;
}

void ARINC_HI3220_Write_Pin(const Tool_HI3220_GPO_Enum Pin, const Tool_Common_Pin_State_Enum State)
{
    GPIO_PinState              HAL_State   = 0;
    Tool_Common_Pin_State_Enum Final_State = 0;

    if (Pin >= TOOL_HI3220_GPO_COUNT)
    {
        return;
    }

    if (ARINC_GPO_Pins_Map[Pin].IsInverted == ARINC_HI3220_PIN_INVERTED_ON)
    {
        Final_State = (State == TOOL_COMMON_PIN_STATE_HIGH) ? TOOL_COMMON_PIN_STATE_LOW : TOOL_COMMON_PIN_STATE_HIGH;
    }

    HAL_State = (Final_State == TOOL_COMMON_PIN_STATE_HIGH) ? GPIO_PIN_SET : GPIO_PIN_RESET;

    HAL_GPIO_WritePin(ARINC_GPO_Pins_Map[Pin].Port, ARINC_GPO_Pins_Map[Pin].Pin, HAL_State);
}

Tool_Common_Pin_State_Enum ARINC_HI3220_Read_Pin(const Tool_HI3220_GPI_Enum Pin)
{
    GPIO_PinState Physical_State = 0;

    if (Pin >= TOOL_HI3220_GPI_COUNT)
    {
        return TOOL_COMMON_PIN_STATE_LOW;
    }

    Physical_State = HAL_GPIO_ReadPin(ARINC_GPI_Pins_Map[Pin].Port, ARINC_GPI_Pins_Map[Pin].Pin);

    return (Physical_State == GPIO_PIN_SET) ? TOOL_COMMON_PIN_STATE_HIGH : TOOL_COMMON_PIN_STATE_LOW;
}

int ARINC_HI3220_Configuration(void)
{
    uint8_t  Value = 0;
    uint16_t Count = 0;

    Tool_HI3220_MCR_Union   MCR_Union   = { 0 };
    Tool_HI3220_PIER_Union  PIER_Union  = { 0 };
    Tool_HI3220_ATXC_Union  ATXC_Union  = { 0 };
    Tool_HI3220_ARXC_Union  ARXC_Union  = { 0 };
    Tool_HI3220_RIERL_Union RIERL_Union = { 0 };

    TOOLS_ERROR_CFG_ASSERT(TOOLS_ERROR_STATUS_IS_CONFIGURED_NO);

    // Конфигурация главного управляющего регистра
    if (Tool_HI3220_Get_Register(TOOL_HI3220_MCR_WR_F, &MCR_Union.Raw) != TOOLS_ERROR_CODE_ALL_OK)
    {
        return TOOLS_ERROR_CODE_OTHER;
    }
    ARINC_Print(&MCR_Union.Raw, 1);

    MCR_Union.Bits.A429RX = 1;
    MCR_Union.Bits.A429TX = 1;

    if (Tool_HI3220_Set_MCR(MCR_Union.Raw) != TOOLS_ERROR_CODE_ALL_OK)
    {
        return TOOLS_ERROR_CODE_OTHER;
    }

    // Конфигурация передачи:

    ATXC_Union.Bits.RATE     = 1;  // Скорость передачи — низкая (12.5кбит/с)
    ATXC_Union.Bits.PARITYEN = 1;  // Контроль четности включен

    // Конфигурация управляющего регистра канала 0
    if (Tool_HI3220_Set_Register(TOOL_HI3220_ATXC0_SHORT, ATXC_Union.Raw) != TOOLS_ERROR_CODE_ALL_OK)
    {
        return TOOLS_ERROR_CODE_OTHER;
    }

    // Конфигурация управляющего регистра канала 1
    if (Tool_HI3220_Set_Register(TOOL_HI3220_ATXC1_SHORT, ATXC_Union.Raw) != TOOLS_ERROR_CODE_ALL_OK)
    {
        return TOOLS_ERROR_CODE_OTHER;
    }

    // Конфигурация приема:
    ARXC_Union.Bits.ENABLE   = 1;
    ARXC_Union.Bits.RATE     = 1;
    ARXC_Union.Bits.PARITYEN = 1;

    // Конфигурация управляющего регистра канала 0
    if (Tool_HI3220_Set_Register(TOOL_HI3220__ARXC0, ARXC_Union.Raw) != TOOLS_ERROR_CODE_ALL_OK)
    {
        return TOOLS_ERROR_CODE_OTHER;
    }

    // Отладочная проверка
    if (Tool_HI3220_Get_Register(TOOL_HI3220_ARXC0_R, &Value) != TOOLS_ERROR_CODE_ALL_OK)
    {
        return TOOLS_ERROR_CODE_OTHER;
    }
    ARINC_Print(&Value, 1);

    // Конфигурация управляющего регистра канала 1
    if (Tool_HI3220_Set_Register(TOOL_HI3220__ARXC1, ARXC_Union.Raw) != TOOLS_ERROR_CODE_ALL_OK)
    {
        return TOOLS_ERROR_CODE_OTHER;
    }

    // Отладочная проверка
    if (Tool_HI3220_Get_Register(TOOL_HI3220_ARXC1_R, &Value) != TOOLS_ERROR_CODE_ALL_OK)
    {
        return TOOLS_ERROR_CODE_OTHER;
    }
    ARINC_Print(&Value, 1);

    // Включить разрешение выдачи INT по событию "новое сообщение в RX RAM2"
    PIER_Union.Bits.RSV    = TOOL_HI3220_PIER_MASK;
    PIER_Union.Bits.RXRAM2 = 1;

    // Готовим байт для записи
    Tool_HI3220_Address_Write(TOOL_HI3220_PIER, &PIER_Union.Raw, 1, NULL);

    // Отладочная проверка
    if (Tool_HI3220_Address_Read(TOOL_HI3220_PIER, &Value, 1, &Count) != TOOLS_ERROR_CODE_ALL_OK)
    {
        return TOOLS_ERROR_CODE_OTHER;
    }
    ARINC_Print(&Value, 1);

    // Включаем обработку прерываний на приемнике 0 и 1:
    RIERL_Union.Bits.RX0 = 1;
    RIERL_Union.Bits.RX1 = 1;

    // Запись значения в регистр RIERL
    Tool_HI3220_Address_Write(TOOL_HI3220_RIERL, &RIERL_Union.Raw, 1, NULL);

    // Отладочная проверка — чтение значения регистра RIERL и его вывод
    if (Tool_HI3220_Address_Read(TOOL_HI3220_RIERL, &RIERL_Union.Raw, 1, &Count) != TOOLS_ERROR_CODE_ALL_OK)
    {
        return TOOLS_ERROR_CODE_OTHER;
    }
    ARINC_Print(&RIERL_Union.Raw, 1);

    // Инициализация фильтра (стр. 13–14). Включение приема сообщений со всеми метками на всех каналах
    Value = 0xFF;
    for (uint16_t i = TOOL_HI3220_RXEN_MAP; i < TOOL_HI3220_RXINT_MAP; i++)
    {
        Tool_HI3220_Address_Write(i, &Value, 1, NULL);
    }

    // Инициализация таблицы прерываний по меткам. Включение прерываний по приходу любых сообщений
    for (uint16_t i = TOOL_HI3220_RXINT_MAP; i < TOOL_HI3220_RXINT_ENDMAP; i++)
    {
        Tool_HI3220_Address_Write(i, &Value, 1, NULL);
    }
}

void ARINC_Process(void)
{
    // ════════════════════════════════════════════════════════════════════════════════════════
    //                           ЛОКАЛЬНЫЕ ПЕРЕМЕННЫЕ И ФЛАГИ
    // ════════════════════════════════════════════════════════════════════════════════════════

    /// @brief      Флаг критической неисправности (требует немедленного внимания)
    /// @details    Устанавливается при: отказе серверов, потере всех камер
    uint8_t Critical_Fault = TOOL_COMMON_BOOLEAN_LEVEL_FALSE;

    /// @brief      Флаг некритической неисправности (система частично работоспособна)
    /// @details    Устанавливается при: неисправности отдельных камер, ХАЭ-21, переключателя
    uint8_t Non_Critical_Fault = TOOL_COMMON_BOOLEAN_LEVEL_FALSE;

    /// @brief      Счётчик неисправных камер [0:6]
    uint8_t Cameras_Fault_Count = 0;

    /// @brief      Содержимое регистра прерываний
    uint8_t Interrupt_Register = 0;

    /// @brief      Количество слов в FIFO
    uint8_t Words_Count = 0;

    /// @brief      Увеличивается при каждом успешном прохождении очередного этапа в данной функции
    uint8_t Error_Result = 0;

    /// @brief      Флаг критической неисправности (требует немедленного внимания)
    /// @details    Устанавливается при: отказе серверов, потере всех камер
    uint8_t Critical_Fault = TOOL_COMMON_BOOLEAN_LEVEL_FALSE;

    // ════════════════════════════════════════════════════════════════════════════════════════
    //                              КОНТРОЛЬ ТАЙМАУТОВ СВЯЗИ
    // ════════════════════════════════════════════════════════════════════════════════════════

    // Проверка таймаута связи с ХАЭ-21 (авиационный хронометр)
    if (ARINC_Timer_Expired(&Timer_Receive_Timeout_XAE21)) //XAE21_TIMEOUT = 2500
    {
        // При потере связи с ХАЭ-21 помечаем его как неисправный, т.к. время/дата могут быть неактуальными
        ARINC_System_Status.XAE21_Fault = TOOL_COMMON_BOOLEAN_LEVEL_TRUE;

        // обнулить структуры времени, если нужно (было в исходном коде, но закомичено)
        // memset(&ARINC_Control, 0, sizeof(ARINC_Control_Struct));
        // memset(&ARINC_DateTime, 0, sizeof(ARINC_Date_Time_Struct));
    }

    // Проверка таймаута связи с сервером управления
    if (ARINC_Timer_Expired(&Timer_PC_Receive_Timeout_Control))// SVR_TIMEOUT = 3000
    {
        // Потеря связи с сервером управления
        // В оригинале здесь был закоммиченный флаг pcCtrlConnected = 0 
        // Можно установить соответствующий статус, если нужно
    }

    // Проверка таймаута связи с сервером даты/времени
    if (ARINC_Timer_Expired(&Timer_PC_Receive_Timeout_DateTime))// SVR_TIMEOUT = 3000 (в другом месте вообще 5000)
    {
        // Потеря связи с сервером даты/времени
        // В оригинале здесь был закоммиченный флаг pcDTConnected = 0
    }

    // ════════════════════════════════════════════════════════════════════════════════════════
    //                           ОБРАБОТКА ПРЕРЫВАНИЙ ОТ HI-3220
    // ════════════════════════════════════════════════════════════════════════════════════════

    /// @brief      Чтение состояния пина прерывания HI-3220
    /// @details    INT активен по низкому уровню (0 = есть прерывание, 1 = нет прерывания)
    uint8_t Interrupt_Pin_State = ARINC_HI3220_Read_Pin(TOOL_HI3220_GPI_INT);
    
    if (Interrupt_Pin_State == TOOL_COMMON_PIN_STATE_LOW)
    {
        // ┌─────────────────────────────────────────────────────────────────────────┐
        // │                        НЕМЕДЛЕННОЕ ПОДТВЕРЖДЕНИЕ ACK                    │
        // └─────────────────────────────────────────────────────────────────────────┘
        // Критически важно: ACK должен быть отправлен как можно быстрее
        // после получения прерывания, иначе HI-3220 может "зависнуть"

        ARINC_HI3220_Write_Pin(TOOL_HI3220_GPO_ACK, TOOL_COMMON_PIN_STATE_LOW);   // ACK активен
        HAL_Delay(6);                                                             // Задержка 6 микросекунд(или милисекунд, в исходном коде было так) (требование HI-3220)
        ARINC_HI3220_Write_Pin(TOOL_HI3220_GPO_ACK, TOOL_COMMON_PIN_STATE_HIGH);  // ACK неактивен

        // ┌─────────────────────────────────────────────────────────────────────────┐
        // │                     ЧТЕНИЕ РЕГИСТРА ПРЕРЫВАНИЙ                          │
        // └─────────────────────────────────────────────────────────────────────────┘
        /// @brief      Чтение регистра RPIRL (Receiver Processor Interrupt Register Low)
        /// @details    Содержит битовые флаги прерываний от приёмных каналов: <br>
        ///                 Бит 0: прерывание от RX канала 0 <br>
        ///                 Бит 1: прерывание от RX канала 1 <br>
        if (Tool_HI3220_Get_Register(TOOL_HI3220_RPIRL_FAST, &Interrupt_Register) != TOOLS_ERROR_CODE_ALL_OK) 
        {
            ARINC_Print_Err(Error_Result);
            return;
        }
        Error_Result++;

        // ┌─────────────────────────────────────────────────────────────────────────┐
        // │                     ОБРАБОТКА КАНАЛА 0 (ОСНОВНОЙ)                       │
        // └─────────────────────────────────────────────────────────────────────────┘
        if ((Interrupt_Register & 0x01) == 0x01) // Есть данные в FIFO канала 0
        {
            Words_Count = 0;

            // Установка указателя на счётчик FIFO канала 0
            // Чтение количества слов в FIFO (каждое слово = 4 байта)
            if (Tool_HI3220_Address_Read(TOOL_HI3220_FCV0, &Words_Count, 1, NULL) != TOOLS_ERROR_CODE_ALL_OK) 
            {
                ARINC_Print_Err(Error_Result);
                return;
            }
            Error_Result++;

            if (Words_Count > 0)
            {
                // Чтение всех слов из FIFO в приёмный буфер

                uint32_t ARINC_Receive_Buffer[Words_Count]; // автоматический буфер в стеке //TODO проверить как будет работать, может лучше перенести в начало и выделить памяти с запасом

                if (Tool_HI3220_ARINC_RX(ARINC_FIFO_CHANNEL_0, &ARINC_Receive_Buffer[0], Words_Count) != TOOLS_ERROR_CODE_ALL_OK) 
                {
                    ARINC_Print_Err(Error_Result);
                    return;
                }
                Error_Result++;

                // Обработка каждого принятого ARINC слова
                uint8_t *Buffer_Pointer = (uint8_t *)ARINC_Receive_Buffer;
                for (uint8_t i = 0; i < Words_Count; i++)
                {
                    // Парсинг слова (определение типа по Label и извлечение данных)
                    ARINC_Parse_Message_Channel_1(Buffer_Pointer);

                    // Переход к следующему слову (каждое слово = 4 байта)
                    Buffer_Pointer += 4;
                }
            }
        }

        // ┌─────────────────────────────────────────────────────────────────────────┐
        // │                  ОБРАБОТКА КАНАЛА 1 (РЕЗЕРВНЫЙ/ВСПОМОГАТЕЛЬНЫЙ)         │
        // └─────────────────────────────────────────────────────────────────────────┘
        if ((Interrupt_Register & 0x02) == 0x02)
        {  // Есть данные в FIFO канала 1 (обработчик аналогично каналу 0)

            Words_Count = 0;

            if (Tool_HI3220_Address_Read(TOOL_HI3220_FCV1, &Words_Count, 1, NULL) != TOOLS_ERROR_CODE_ALL_OK) 
            {
                ARINC_Print_Err(Error_Result);
                return;
            }
            Error_Result++;

            if (Words_Count > 0)
            {
                // Чтение всех слов из FIFO в приёмный буфер

                uint32_t ARINC_Receive_Buffer[Words_Count]; // автоматический буфер в стеке //TODO проверить как будет работать, может лучше перенести в начало и выделить памяти с запасом

                if (Tool_HI3220_ARINC_RX(ARINC_FIFO_CHANNEL_1, &ARINC_Receive_Buffer[0], Words_Count) != TOOLS_ERROR_CODE_ALL_OK) 
                {
                    ARINC_Print_Err(Error_Result);
                    return;
                }
                Error_Result++;

                uint8_t *Buffer_Pointer = (uint8_t *)ARINC_Receive_Buffer;
                for (uint8_t i = 0; i < Words_Count; i++)
                {
                    // В текущей реализации канал 1 не используется активно
                    ARINC_Parse_Message_Channel_2(Buffer_Pointer);
                    Buffer_Pointer += 4;
                }
            }
        }
    }
    
    //TODO еще почему-то в прошлой версии везде был Ch2 (третий, если по сквозной нумерации) но он был на заглушках/пустой поэтому пока убрал его совсем из всех упоминаний/переменных/макросов

    // ════════════════════════════════════════════════════════════════════════════════════════
    //                       ФОРМИРОВАНИЕ И ОТПРАВКА СТАТУСНОГО СЛОВА 0300
    // ════════════════════════════════════════════════════════════════════════════════════════

    // Проверка таймера периодической отправки (каждые 500мс)
    if (ARINC_Timer_Expired(&Timer_Send_Timeout_Channel1)) //A429_L0300_SEND_PERIOD = 500
    {
        // Перезапуск таймера на следующий период
        ARINC_Timer_Set(&Timer_Send_Timeout_Channel1, ARINC_A429_L0300_SEND_PERIOD);

        /// @brief      Создание union для работы со словом 0300
        ARINC_Word_300_Union Status_Word_300;

        // ┌─────────────────────────────────────────────────────────────────────────┐
        // │                        БАЗОВЫЕ ПОЛЯ СЛОВА 0300                          │
        // └─────────────────────────────────────────────────────────────────────────┘
        Status_Word_300.Struct.Label    = 0300; 
        Status_Word_300.Struct.SDI      = 0x03; //TODO в исходном коде не было, разобраться потом что к чему
        Status_Word_300.Struct.Not_Used = 0x00;

        // ┌─────────────────────────────────────────────────────────────────────────┐
        // │                      ДИАГНОСТИКА КАМЕР ПО ТОКУ                          │
        // └─────────────────────────────────────────────────────────────────────────┘
        /// @details    Каждая камера мониторится по потребляемому току через INA226.
        ///                Если ток меньше 2 Ватт, камера считается неисправной.
        ///                Неисправности камер не критичны для всей системы.

        // Камера 1
        Status_Word_300.Struct.Camera_1_Fault = (T_INA226_Get_Power(&Power_Sensor_A[0]) < ARINC_INA226_MAX_POWER);
        if (Status_Word_300.Struct.Camera_1_Fault)
        {
            Non_Critical_Fault = TOOL_COMMON_BOOLEAN_LEVEL_TRUE;
            Cameras_Fault_Count++;
        }

        // Камера 2
        Status_Word_300.Struct.Camera_2_Fault = (T_INA226_Get_Power(&Power_Sensor_A[1]) < ARINC_INA226_MAX_POWER);
        if (Status_Word_300.Struct.Camera_2_Fault)
        {
            Non_Critical_Fault = TOOL_COMMON_BOOLEAN_LEVEL_TRUE;
            Cameras_Fault_Count++;
        }

        // Камера 3
        Status_Word_300.Struct.Camera_3_Fault = (T_INA226_Get_Power(&Power_Sensor_A[2]) < ARINC_INA226_MAX_POWER);
        if (Status_Word_300.Struct.Camera_3_Fault)
        {
            Non_Critical_Fault = TOOL_COMMON_BOOLEAN_LEVEL_TRUE;
            Cameras_Fault_Count++;
        }

        // Камера 4
        Status_Word_300.Struct.Camera_4_Fault = (T_INA226_Get_Power(&Power_Sensor_A[3]) < ARINC_INA226_MAX_POWER);
        if (Status_Word_300.Struct.Camera_4_Fault)
        {
            Non_Critical_Fault = TOOL_COMMON_BOOLEAN_LEVEL_TRUE;
            Cameras_Fault_Count++;
        }

        // Камера 5
        Status_Word_300.Struct.Camera_5_Fault = (T_INA226_Get_Power(&Power_Sensor_A[4]) < ARINC_INA226_MAX_POWER);
        if (Status_Word_300.Struct.Camera_5_Fault)
        {
            Non_Critical_Fault = TOOL_COMMON_BOOLEAN_LEVEL_TRUE;
            Cameras_Fault_Count++;
        }

        // Камера 6
        Status_Word_300.Struct.Camera_6_Fault = (T_INA226_Get_Power(&Power_Sensor_A[5]) < ARINC_INA226_MAX_POWER);
        if (Status_Word_300.Struct.Camera_6_Fault)
        {
            Non_Critical_Fault = TOOL_COMMON_BOOLEAN_LEVEL_TRUE;
            Cameras_Fault_Count++;
        }

        // ┌─────────────────────────────────────────────────────────────────────────┐
        // │                    СТАТУСЫ СЕРВЕРОВ И ОБОРУДОВАНИЯ                      │
        // └─────────────────────────────────────────────────────────────────────────┘
        /// @details    Копирование статусов из центральной структуры системного состояния
        Status_Word_300.Struct.Service_Router_Status = ARINC_System_Status.Service_Router_Status;
        Status_Word_300.Struct.Storage_Status        = ARINC_System_Status.Storage_Server_Status;
        Status_Word_300.Struct.XAE21_Fault           = ARINC_System_Status.XAE21_Fault;
        Status_Word_300.Struct.Camera_Switch_Fault   = ARINC_System_Status.Camera_Switch_Fault;

        // ┌─────────────────────────────────────────────────────────────────────────┐
        // │                      ОПРЕДЕЛЕНИЕ КРИТИЧНОСТИ ОТКАЗОВ                    │
        // └─────────────────────────────────────────────────────────────────────────┘
        /// @brief      Логика определения критических отказов
        /// @details    Критический отказ возникает при:
        ///                 Отказе сервера маршрутизации (srvrouter_stat == 1)
        ///                 Отказе сервера накопления (storage_stat == 1)
        ///                 Потере ВСЕХ камер (cams_N_faults >= CHANNELS_TOTAL)
        if (ARINC_System_Status.Service_Router_Status == 1 ||
            ARINC_System_Status.Storage_Server_Status == 1 ||
            Cameras_Fault_Count >= ARINC_CHANNELS_TOTAL)
        {
            Critical_Fault = TOOL_COMMON_BOOLEAN_LEVEL_TRUE;
        }

        // ┌─────────────────────────────────────────────────────────────────────────┐
        // │                   УЧЁТ НЕКРИТИЧЕСКИХ НЕИСПРАВНОСТЕЙ                     │
        // └─────────────────────────────────────────────────────────────────────────┘
        /// @details    Некритические неисправности не останавливают работу системы,
        ///                 но должны быть переданы для информирования экипажа
        if (Status_Word_300.Struct.XAE21_Fault)
        {
            Non_Critical_Fault = TOOL_COMMON_BOOLEAN_LEVEL_TRUE;
        }

        if (Status_Word_300.Struct.Camera_Switch_Fault)
        {
            Non_Critical_Fault = TOOL_COMMON_BOOLEAN_LEVEL_TRUE;
        }

        // ┌─────────────────────────────────────────────────────────────────────────┐
        // │                    ИТОГОВОЕ ОПРЕДЕЛЕНИЕ СТАТУСА СИСТЕМЫ                 │
        // └─────────────────────────────────────────────────────────────────────────┘
        /// @brief Трёхуровневая система статусов
        if (Critical_Fault)
        {
            /// Критическая неисправность: система неработоспособна
            Status_Word_300.Struct.System_Status = ARINC_SYSTEM_STATUS_FAULT;
        }
        else if (Non_Critical_Fault)
        {
            /// Частичная неисправность: система работает с ограничениями
            Status_Word_300.Struct.System_Status = ARINC_SYSTEM_STATUS_PARTIALLY_FAULT;
        }
        else
        {
            /// Нормальная работа: все системы функционируют корректно
            Status_Word_300.Struct.System_Status = ARINC_SYSTEM_STATUS_NC1;  // Normal Configuration
        }

        // ┌─────────────────────────────────────────────────────────────────────────┐
        // │                      ФИНАЛЬНЫЕ ПОЛЯ И ОТПРАВКА                          │
        // └─────────────────────────────────────────────────────────────────────────┘
        /// @brief      Установка режима работы передатчика
        Status_Word_300.Struct.Matrix = ARINC_A300_MATRIX_NORMAL_OPERATION;

        /// @brief      Отправка сформированного слова через HI-3220
        /// @details    Слово отправляется через канал 0 в формате массива байтов
        if (Tool_HI3220_ARINC_TX(ARINC_FIFO_CHANNEL_0, &Status_Word_300.Raw, 1) != TOOLS_ERROR_CODE_ALL_OK) 
        {
            ARINC_Print_Err(Error_Result);
            return;
        }
        Error_Result++;
    }

    // ════════════════════════════════════════════════════════════════════════════════════════
    //                              ОБСЛУЖИВАНИЕ UART ПЕРЕДАЧИ
    // ════════════════════════════════════════════════════════════════════════════════════════

    /// @brief      Проверка готовности UART3 для передачи данных
    /// @details    Если UART свободен и есть данные в буфере - отправляем через DMA
    if (ARINC_UART_Handle->gState == HAL_UART_STATE_READY)
    {
        // Определение количества байтов для передачи
        uint16_t Bytes_To_Send = ARINC_UART_Output_Circular_Buffer.Element_Count;
        if (Bytes_To_Send > ARINC_UART3_TX_BUFFER_SIZE)
        {
            Bytes_To_Send = ARINC_UART3_TX_BUFFER_SIZE;  // Ограничение размера буфера DMA
        }

        // Извлечение данных из кольцевого буфера
        Circular_Buffer_Pop(&ARINC_UART_Output_Circular_Buffer,
                            (uint8_t *)ARINC_TX_Buffer,
                            Bytes_To_Send);

        // Запуск DMA передачи
        HAL_UART_Transmit_DMA(ARINC_UART_Handle, ARINC_TX_Buffer, Bytes_To_Send);
    }
}

//TODO Если приходящие байты находятся в другом порядке (например, big-endian, а МК — little-endian), такой каст не подойдёт напрямую: byte order будет съеден
int ARINC_Parse_Message_Channel_1(uint8_t *Data_Ptr)
{
    // Приводим к универсальному union, чтобы прочитать Label
    const Tool_ARINC429_Word_Union *word_union = (const Tool_ARINC429_Word_Union *)Data_Ptr;
    uint8_t Label = word_union->Tool_ARINC429_Word_Fields.Label;

    // Обновляем таймер и сбрасываем ошибку ХАЭ-21
    TIMERS_setTimer(&Timer_Recv_Timeout_XAE21, ARINC_PC_RECEIVE_TIMEOUT_XAE21);
    ARINC_System_Status.XAE21_Fault = TOOL_COMMON_BOOLEAN_LEVEL_FALSE;

    switch (Label)
    {
        case 0x96: // 0150 - время в двоичном формате
        {
            const ARINC_Word_150_Union *w150 = (const ARINC_Word_150_Union *)Data_Ptr;

            ARINC_DateTime.Hour   = w150->Struct.Hour;
            ARINC_DateTime.Minute = w150->Struct.Minute;
            ARINC_DateTime.Second = w150->Struct.Second;

            break;
        }

        case 0x4C: // 0125 - время в BCD формате //TODO(резерв, не понял как используется и используется ли вообще)
        {
            const ARINC_Word_125_Union *w125 = (const ARINC_Word_125_Union *)Data_Ptr;

            //TODO если например всё-таки надо будет использовать:
            // uint8_t minutes = w125->Struct.Minute_Tens * 10 + w125->Struct.Minute_Unit;
            // uint8_t hours = w125->Struct.Hour_Tens * 10 + w125->Struct.Hour_Unit;

            break;
        }

        case 0x9A: // 0260 - дата в BCD формате
        {
            const ARINC_Word_260_Union *w260 = (const ARINC_Word_260_Union *)Data_Ptr;

            ARINC_DateTime.Year  = w260->Struct.Year_Tens * 10 + w260->Struct.Year_Units;
            ARINC_DateTime.Month = w260->Struct.Month_Tens * 10 + w260->Struct.Month_Units;
            ARINC_DateTime.Day   = w260->Struct.Day_Tens * 10 + w260->Struct.Day_Units;

            // Снова сброс таймаута и ошибки - на всякий случай
            TIMERS_setTimer(&Timer_Recv_Timeout_XAE21, ARINC_PC_RECEIVE_TIMEOUT_XAE21);
            ARINC_System_Status.XAE21_Fault = TOOL_COMMON_BOOLEAN_LEVEL_FALSE;

            break;
        }

        default:
            return TOOLS_ERROR_CODE_ARG;
            break;
    }

    return TOOLS_ERROR_CODE_ALL_OK;
}

int ARINC429_parseMessageCh2(char *data)
{
    //TODO в исходном коде по второму каналу пока что данные не обрабатываются
    return TOOLS_ERROR_CODE_ALL_OK;
}



void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_3)
    {
        if (!ARINC_HI3220_Read_Pin(TOOL_HI3220_GPI_INT))
        {
            // Send ACK (right here, immediate after INT - LV)
            ARINC_HI3220_Write_Pin(TOOL_HI3220_GPO_ACK, TOOL_COMMON_PIN_STATE_LOW);
            HAL_Delay(6);
            ARINC_HI3220_Write_Pin(TOOL_HI3220_GPO_ACK, TOOL_COMMON_PIN_STATE_HIGH);
        }
    }
}
