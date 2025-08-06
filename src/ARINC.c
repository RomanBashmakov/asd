/// @file       ARINC.c
/// @author     Башмаков Р.А. bashmakovra@raitibor.ru
/// @brief      См. ARINC.h

#include "ARINC.h"

#include "Tool_HI3220.h"

/// @brief Порт и пин RX (порт B, пин 12)
#define ARINC_PIN_RX    GPIOB, GPIO_PIN_12

/// @brief Порт и пин TX (порт A, пин 1)
#define ARINC_PIN_TX    GPIOA, GPIO_PIN_1

/// @brief Порт и пин MRST (порт C, пин 0)
#define ARINC_PIN_MRST  GPIOC, GPIO_PIN_0

/// @brief Порт и пин READY (порт C, пин 1)
#define ARINC_PIN_READY GPIOC, GPIO_PIN_1

/// @brief Порт и пин RUN (порт B, пин 5)
#define ARINC_PIN_RUN   GPIOB, GPIO_PIN_5

/// @brief Порт и пин INT (порт B, пин 6)
#define ARINC_PIN_INT   GPIOB, GPIO_PIN_6

/// @brief Порт и пин ACK (порт A, пин 8)
#define ARINC_PIN_ACK   GPIOA, GPIO_PIN_8

/// @brief Порт и пин CS (порт A, пин 9)
#define ARINC_PIN_CS    GPIOA, GPIO_PIN_9

/// @brief      Период передачи пакета по ARINC 429
#define ARINC_A429_L0300_SEND_PERIOD 500

/// @brief      ?
#define ARINC_CAMERA_CURRENT_THRESHOLD 0.05

/// @brief      Количество элементов в кольцевом буфере
#define ARINC_UART_OUTPUT_BUFFER_LENGTH 200

/// @brief      Перечисление выходных пинов HI3220
typedef enum ARINC_HI3220_GPO_Enum
{
    Tool_HI3220_GPO_MRST = 0,   ///< Пин сброса
    Tool_HI3220_GPO_RUN,        ///< Пин запуска
    Tool_HI3220_GPO_ACK,        ///< Пин подтверждения
    Tool_HI3220_GPO_CS,         ///< Пин выбора чипа
    ARINC_HI3220_GPO_COUNT      ///< Количество выходных пинов интерфейса HI3220
} ARINC_HI3220_GPO_Enum;

/// @brief      Перечисление входных пинов HI3220
typedef enum ARINC_HI3220_GPI_Enum
{
    Tool_HI3220_GPI_READY = 0,  ///< Пин готовности (только для чтения)
    Tool_HI3220_GPI_INT,        ///< Пин прерывания (только для чтения)
    ARINC_HI3220_GPI_COUNT      ///< Количество входных пинов интерфейса HI3220
} ARINC_HI3220_GPI_Enum;

/// @brief      Список выходных пинов
/// @warning    Порядок элементов в ARINC_HI3220_GPO_Enum и порядок инициализации структур в массиве ARINC_GPO_Pins_Map <br>
///                 должны быть строго идентичны, иначе прозойдет ошибочное управление ложным пином <br>
///                 в ARINC_HI3220_Write_Pin/ARINC_HI3220_Read_Pin <br>
///                 Программа будет работать некорректно, но никаких ошибок при компиляции при этом не будет
static const ARINC_Pin_Struct ARINC_GPO_Pins_Map[ARINC_HI3220_GPO_COUNT] = {
    {ARINC_PIN_MRST, ARINC_HI3220_PIN_INVERTED_ON},
    {ARINC_PIN_RUN,  ARINC_HI3220_PIN_INVERTED_OFF},
    {ARINC_PIN_ACK,  ARINC_HI3220_PIN_INVERTED_OFF},
    {ARINC_PIN_CS,   ARINC_HI3220_PIN_INVERTED_ON}
};

/// @brief      Список входных пинов
/// @warning    Порядок элементов в ARINC_HI3220_GPI_Enum и порядок инициализации структур в массиве ARINC_GPI_Pins_Map <br>
///                 должны быть строго идентичны, иначе прозойдет ошибочное управление ложным пином <br>
///                 в ARINC_HI3220_Write_Pin/ARINC_HI3220_Read_Pin <br>
///                 Программа будет работать некорректно, но никаких ошибок при компиляции при этом не будет
static const ARINC_Pin_Struct ARINC_GPI_Pins_Map[ARINC_HI3220_GPI_COUNT] = {
    {ARINC_PIN_READY, ARINC_HI3220_PIN_INVERTED_OFF},
    {ARINC_PIN_INT,   ARINC_HI3220_PIN_INVERTED_OFF}
};

/// @brief  Структура с описанием одного GPIO пина ARINC
typedef struct ARINC_Pin_Struct
{
    GPIO_TypeDef *Port;       ///< Аппаратный порт GPIO
    uint16_t      Pin;        ///< Номер пина GPIO
    uint8_t       IsInverted; ///< "1" если пин инвертированный
} ARINC_Pin_Struct;

/// @brief      Перечисление режимов инвертирования пинов HI3220
typedef enum ARINC_HI3220_Pin_Inverted_Enum
{
    ARINC_HI3220_PIN_INVERTED_OFF = 0,   ///< Пин не инвертирован
    ARINC_HI3220_PIN_INVERTED_ON = 0,    ///< Пин инвертирован
} ARINC_HI3220_Pin_Inverted_Enum;

Tool_Common_Circular_Buffer_Struct ARINC_UART_Output_Circular_Buffer;
uint8_t ARINC_TX_Buffer[ARINC_UART_OUTPUT_BUFFER_LENGTH];

/// @brief      Функция записи на цифровой вывод МК
/// @param[in]  Pin    Вывод МК, подключенного к HI3220 (см. ARINC_HI3220_GPO_Enum)
/// @param[in]  State  Устанавливаемое значение (см. Tool_Common_Pin_State_Enum)
/// @warning    Последовательность пинов из ARINC_HI3220_GPO_Enum должна точно соответствовать ARINC_GPO_Pins_Map
void ARINC_HI3220_Write_Pin(const ARINC_HI3220_GPO_Enum Pin, const Tool_Common_Pin_State_Enum State);
                          
/// @brief      Функция чтения цифрового вывода МК
/// @param[in]  Pin Вывод МК, подключенный к HI3220 (см. ARINC_HI3220_GPI_Enum)
/// @warning    Последовательность пинов из ARINC_HI3220_GPI_Enum должна точно соответствовать ARINC_GPI_Pins_Map
/// @return     Текущее значение на указанном выводе МК (см. Tool_Common_Pin_State_Enum)
Tool_Common_Pin_State_Enum ARINC_HI3220_Read_Pin(const ARINC_HI3220_GPI_Enum Pin);

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

int ARINC_Configuration(SPI_HandleTypeDef *SPI_Handle_Ptr, SPI_HandleTypeDef *SPI_Handle_Black_Box_Ptr)
{
    Tool_HI3220_Hardware_Reset();

    Tool_HI3220_Configuration(ARINC_HI3220_SPI_TX,
                              ARINC_HI3220_SPI_RX,
                              ARINC_HI3220_SPI_TX_RX,
                              ARINC_HI3220_Write_Pin,
                              HAL_Delay,
                              ARINC_HI3220_Read_Pin);

    Tool_HI3220_Self_Test();

    Tool_Circular_Buffer_Init(&ARINC_UART_Output_Circular_Buffer[0], ARINC_TX_Buffer, ARINC_UART_OUTPUT_BUFFER_LENGTH);
}

void ARINC_HI3220_Write_Pin(const ARINC_HI3220_GPO_Enum Pin, const Tool_Common_Pin_State_Enum State)
{
    GPIO_PinState HAL_State = 0;
    Tool_Common_Pin_State_Enum Final_State = 0;

    if (Pin >= ARINC_HI3220_GPO_COUNT)
    {
        return;
    }
    
    if (ARINC_GPO_Pins_Map[Pin].IsInverted == ARINC_HI3220_PIN_INVERTED_ON)
    {
        Final_State = (State == Tool_Common_Pin_State_High) ? Tool_Common_Pin_State_Low : Tool_Common_Pin_State_High;
    }
    
    HAL_State = (Final_State == Tool_Common_Pin_State_High) ? GPIO_PIN_SET : GPIO_PIN_RESET;

    HAL_GPIO_WritePin(ARINC_GPO_Pins_Map[Pin].Port, ARINC_GPO_Pins_Map[Pin].Pin, HAL_State);
}

Tool_Common_Pin_State_Enum ARINC_HI3220_Read_Pin(const ARINC_HI3220_GPI_Enum Pin)
{
    GPIO_PinState Physical_State = 0;

    if (Pin >= ARINC_HI3220_GPI_COUNT)
    {
        return Tool_Common_Pin_State_Low;
    }

    Physical_State = HAL_GPIO_ReadPin(ARINC_GPI_Pins_Map[Pin].Port, ARINC_GPI_Pins_Map[Pin].Pin);

    return (Physical_State == GPIO_PIN_SET) ? Tool_Common_Pin_State_High : Tool_Common_Pin_State_Low;
}

int ARINC_HI3220_Configuration(void)
{
    uint8_t Value = 0;
    uint8_t Count = 0;

    Tool_HI3220_MCR_Union   MCR_Union   = {0};
    Tool_HI3220_PIER_Union  PIER_Union  = {0};
    Tool_HI3220_ATXC_Union  ATXC_Union  = {0};
    Tool_HI3220_ARXC_Union  ARXC_Union  = {0};
    Tool_HI3220_RIERL_Union RIERL_Union = {0};

    TOOLS_ERROR_CFG_ASSERT(Tool_HI3220_Is_Configured);

    // Конфигурация главного управляющего регистра
    if (Tool_HI3220_Get_Register(TOOL_HI3220_MCR_WR_F, &MCR_Union.Raw) != TOOLS_ERROR_CODE_ALL_OK)
    {
        return TOOLS_ERROR_CODE_OTHER;
    }
    Tool_HI3220_Print(&MCR_Union.Raw, 1);

    MCR_Union.bits.A429RX = 1;
    MCR_Union.bits.A429TX = 1;

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
    Tool_HI3220_Print(&Value, 1);

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
    Tool_HI3220_Print(&Value, 1);

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
    Tool_HI3220_Print(&Value, 1);

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
    Tool_HI3220_Print(&RIERL_Union.Raw, 1);

    // Инициализация фильтра (стр. 13–14). Включение приема сообщений со всеми метками на всех каналах
    Value = 0xFF;
    for (int i = TOOL_HI3220_RXEN_MAP; i < TOOL_HI3220_RXINT_MAP; i++)
    {
        Tool_HI3220_Address_Write(i, &Value, 1, NULL);
    }

    // Инициализация таблицы прерываний по меткам. Включение прерываний по приходу любых сообщений
    for (int i = TOOL_HI3220_RXINT_MAP; i < TOOL_HI3220_RXINT_ENDMAP; i++)
    {
        Tool_HI3220_Address_Write(i, &Value, 1, NULL);
    }
}