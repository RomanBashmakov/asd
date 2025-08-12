/// @file       Tool_ARINC.h
/// @author     Башмаков Р.А. bashmakovra@raitibor.ru
/// @brief      Заголовочный файл, содержащий объявления модуля для работы по протоколу ARINC
/// @details    Порядок работы с модулем: <br>
///                 1) Конфигурировать ARINC (см. ARINC_Configuration())<br>
///                 2) Периодически в основном цикле вызывать функции <br>
///                 3) <br>
/// @note       HI3220 подключен через SPI2
///             Для работы с HI3220 задействованы 9 выводов МК

#ifndef _ARINC_H_
#define _ARINC_H_

#include <_Includes.h>
#include <string.h>
#include <stdint.h>

//TODO добавить файл с функциями прослойками с рабочего компа
// #include "spi.h"
#include "stm32f7xx_hal.h"

/// @brief  Пин INT <br>
///             Используется в stm32f7xx_it.c
#define ARINC_PIN_INT_PIN GPIO_PIN_3    // GPI_ar429_intn_Pin


/// @brief   Структура Сервера Накопителя и Сервера Маршрутизатора
typedef struct Status
{
    uint8_t archive;  //!< Состояние Сервера Накопителя
    uint8_t router;   //!< Состояние Сервера Маршрутизатора
} Status;

/// @brief   Структура пакета с состоянием Сервера Накопителя и Сервера Маршрутизатора
typedef struct SetStatus
{
    uint8_t flag;    //!< Флаг пакета
    uint8_t code;    //!< Код пакета
    Status  status;  //!< Статус Сервера Накопителя и Сервера Маршрутизатора
} SetStatus;

/// @brief   Структура пакета с запросом на получение состояния Камеры
typedef struct GetStatusQuery
{
    uint8_t flag;    //!< Флаг пакета
    uint8_t code;    //!< Код пакета
    uint8_t camera;  //!< Номер камеры
} GetStatusQuery;

/// @brief   Структура пакета с ответом на запрос о получении состояния Камеры
typedef struct GetStatusResponse
{
    uint8_t flag;    //!< Флаг пакета
    uint8_t code;    //!< Код пакета
    uint8_t camera;  //!< Номер камеры
    uint8_t status;  //!< Состояние камеры
} GetStatusResponse;

#pragma pack(push, 1)

/// @brief Структура 32-битного ARINC-429 слова
typedef struct ARINC_Word_Struct
{
    uint32_t Parity : 1;   ///< Бит чётности (odd parity)
    uint32_t SSM    : 2;   ///< Sign/Status Matrix
    uint32_t Data   : 19;  ///< Основные данные
    uint32_t SDI    : 2;   ///< Source/Destination Identifier
    uint32_t Label  : 8;   ///< Label (адрес сообщения)
} ARINC_Word_Struct;

/// @brief Union для доступа к ARINC-429 слову как к raw значению или по полям
typedef union Tool_ARINC429_Word_Union
{
    uint32_t          Tool_ARINC429_Word_Raw;     ///< Целое 32-битное значение
    ARINC_Word_Struct Tool_ARINC429_Word_Fields;  ///< Структурированные поля ARINC-429
} Tool_ARINC429_Word_Union;

typedef enum ARINC_Ext_System_Status_Enum //
{
    ARINC_EXT_SYSTEM_STATUS_NC              = 0,
    ARINC_EXT_SYSTEM_STATUS_PART_FAULT      = 1,
    ARINC_EXT_SYSTEM_STATUS_FAULT           = 2
} ARINC_Ext_System_Status_Enum;

/// @brief      Структура управления органами в кабине воздушного судна (14 байт)
/// @details    Содержит полное состояние органов управления для двух пилотов:                  <br>
///                 CAPT - командир экипажа, левое кресло (байты 0–6)                           <br>
///                 FO   - второй пилот, правое кресло (байты 7–13)                             <br>
///             Каждый пилот имеет                                                              <br>
///                 независимое управление дисплеем, курсором, камерами и кнопками, а система   <br>
///                 контролирует состояние обоих одновременно.                                  <br>
///             Передаётся через UART в составе пакета управления: [0xEB] [0x01] [14 байт данных] [CRC8] <br>
/// @warning    Неправильное изменение порядка, размеров или выравнивания приведёт
///                 к некорректной интерпретации данных при декодировании
typedef struct ARINC_Control_Struct // T_ARINC429Control
{
    // ---------------------------- CAPT (Командир, байты [0:6]) ----------------------------
    uint8_t Format_1                   : 7;  ///< Байт 0, биты [0:6]    Тип формата экрана CAPT [0:127]
    uint8_t Active_Menu_CAPT           : 1;  ///< Байт 0, бит  [7]      Конфигурационное меню активно [0:1]

    uint8_t Format_1_Position          : 3;  ///< Байт 1, биты [8:10]   Позиция формата [0:7]
    uint8_t Format_1_Size              : 4;  ///< Байт 1, биты [11:14]  Размер/масштаб формата [0:15]
    uint8_t Active_Cursor_CAPT         : 1;  ///< Байт 1, бит  [15]     Курсор CAPT активен [0:1]

    uint8_t Channel_Number;                  ///< Байт 2, биты [16:23]  Активный номер канала [0:255]

    uint8_t Cursor_X_CAPT_High         : 3;  ///< Байт 3, биты [24:26]  X курсора (старшие 3 бита) [0:7]
    uint8_t Camera_Number_CAPT         : 4;  ///< Байт 3, биты [27:30]  Номер активной камеры [0:15]
    uint8_t Button_Pressed_CAPT        : 1;  ///< Байт 3, бит  [31]     Кнопка управления нажата [0:1]

    uint8_t Cursor_X_CAPT_Low;               ///< Байт 4, биты [32:39]  X курсора (младшие 8 бит) [0:255]

    uint8_t Cursor_Y_CAPT_High         : 3;  ///< Байт 5, биты [40:42]  Y курсора (старшие 3 бита) [0:7]
    uint8_t Joystick_Down_CAPT         : 1;  ///< Байт 5, бит  [43]     Джойстик вниз [0:1]
    uint8_t Joystick_Left_CAPT         : 1;  ///< Байт 5, бит  [44]     Джойстик влево [0:1]
    uint8_t Joystick_Right_CAPT        : 1;  ///< Байт 5, бит  [45]     Джойстик вправо [0:1]
    uint8_t Joystick_Up_CAPT           : 1;  ///< Байт 5, бит  [46]     Джойстик вверх [0:1]
    uint8_t Left_Enter_CAPT            : 1;  ///< Байт 5, бит  [47]     Левый Enter нажат [0:1]

    uint8_t Left_Enter_Jammed_CAPT     : 1;  ///< Байт 6, бит  [48]     Левый Enter заклинен [0:1]
    uint8_t Right_Enter_CAPT           : 1;  ///< Байт 6, бит  [49]     Правый Enter нажат [0:1]
    uint8_t Right_Enter_Jammed_CAPT    : 1;  ///< Байт 6, бит  [50]     Правый Enter заклинен [0:1]
    uint8_t Escape_Button_CAPT         : 1;  ///< Байт 6, бит  [51]     Escape нажата [0:1]
    uint8_t Escape_Jammed_CAPT         : 1;  ///< Байт 6, бит  [52]     Escape заклинена [0:1]
    uint8_t OK_Button_CAPT             : 1;  ///< Байт 6, бит  [53]     OK нажата [0:1]
    uint8_t OK_Jammed_CAPT             : 1;  ///< Байт 6, бит  [54]     OK заклинена [0:1]
    uint8_t Lower_Area_Button_CAPT     : 1;  ///< Байт 6, бит  [55]     Нажата нижняя область экрана [0:1]

    // ---------------------------- FO (Второй пилот, байты [7:13]) ----------------------------
    uint8_t Format_2                   : 7;  ///< Байт 7, биты [56:62]  Тип формата экрана FO [0:127]
    uint8_t Active_Menu_FO             : 1;  ///< Байт 7, бит  [63]     Конфигурационное меню активно [0:1]

    uint8_t Format_2_Position          : 3;  ///< Байт 8, биты [64:66]  Позиция формата [0:7]
    uint8_t Format_2_Size              : 4;  ///< Байт 8, биты [67:70]  Размер/масштаб формата [0:15]
    uint8_t Active_Cursor_FO           : 1;  ///< Байт 8, бит  [71]     Курсор FO активен [0:1]

    uint8_t Cursor_X_FO_High;                ///< Байт 9, биты [72:79]  X курсора FO (старшие 8 бит) [0:255]

    uint8_t Cursor_X_FO_Low            : 3;  ///< Байт 10, биты [80:82]  X курсора (младшие 3 бита) [0:7]
    uint8_t Camera_Number_FO           : 4;  ///< Байт 10, биты [83:86]  Номер активной камеры [0:15]
    uint8_t Button_Pressed_FO          : 1;  ///< Байт 10, бит  [87]     Кнопка управления нажата [0:1]

    uint8_t Cursor_Y_FO_High;                ///< Байт 11, биты [88:95]  Y курсора FO (старшие 8 бит) [0:255]

    uint8_t Cursor_Y_FO_Low            : 3;  ///< Байт 12, биты [96:98]  Y курсора (младшие 3 бита) [0:7]
    uint8_t Joystick_Down_FO           : 1;  ///< Байт 12, бит  [99]     Джойстик вниз [0:1]
    uint8_t Joystick_Left_FO           : 1;  ///< Байт 12, бит  [100]    Джойстик влево [0:1]
    uint8_t Joystick_Right_FO          : 1;  ///< Байт 12, бит  [101]    Джойстик вправо [0:1]
    uint8_t Joystick_Up_FO             : 1;  ///< Байт 12, бит  [102]    Джойстик вверх [0:1]
    uint8_t Left_Enter_FO              : 1;  ///< Байт 12, бит  [103]    Левый Enter нажат [0:1]

    uint8_t Left_Enter_Jammed_FO       : 1;  ///< Байт 13, бит  [104]    Левый Enter заклинен [0:1]
    uint8_t Right_Enter_FO             : 1;  ///< Байт 13, бит  [105]    Правый Enter нажат [0:1]
    uint8_t Right_Enter_Jammed_FO      : 1;  ///< Байт 13, бит  [106]    Правый Enter заклинен [0:1]
    uint8_t Escape_Button_FO           : 1;  ///< Байт 13, бит  [107]    Escape нажата [0:1]
    uint8_t Escape_Jammed_FO           : 1;  ///< Байт 13, бит  [108]    Escape заклинена [0:1]
    uint8_t OK_Button_FO               : 1;  ///< Байт 13, бит  [109]    OK нажата [0:1]
    uint8_t OK_Jammed_FO               : 1;  ///< Байт 13, бит  [110]    OK заклинена [0:1]
    uint8_t Lower_Area_Button_FO       : 1;  ///< Байт 13, бит  [111]    Нажата нижняя область экрана [0:1]
} ARINC_Control_Struct;

/// @brief      Структура даты и времени
/// @details    Содержит компоненты времени и даты в виде отдельных полей.
///                 Используется для передачи временной метки в протоколах ARINC
/// @warning    Диапазоны значений для каждого поля соответствуют
///                 стандартным календарным ограничениям.
typedef struct ARINC_Date_Time_Struct // T_ARINC429ControlDateTime
{
    uint8_t Hour;    ///< Часы
    uint8_t Minute;  ///< Минуты
    uint8_t Second;  ///< Секунды
    uint8_t Year;    ///< Год
    uint8_t Month;   ///< Месяц
    uint8_t Day;     ///< День месяца
} ARINC_Date_Time_Struct;

/// @brief      "125". Время в BCD. Структура слова ARINC-429
/// @details    Форматирует часы и минуты в BCD представлении.
///                 В скобках - номера бит относительно всего 32-битного слова.
typedef struct ARINC_Word_125_Struct // W125_t
{
// Бит №:  [31]      [30:29]   [28:26]    [25:22]     [21:18]      [17:14]      [13:10]        [9:8]   [7:0]
// Поле :  Parity    Matrix    Hour_Tens  Hour_Unit   Minute_Tens  Minute_Unit  Minute_Second  SDI     Label
// Данные: [0:1]     [0:3]     [0:2]      [0:9]       [0:9]        [0:5]        [0:9]          [0:3]   [0:255]
    uint32_t Label             : 8;  ///< Биты [0:7]    LABEL [0:255]
    uint32_t SDI               : 2;  ///< Биты [8:9]    Source/Destination Identifier [0:3]

    uint32_t Minute_Second     : 4;  ///< Биты [10:13]  Минуты, единицы [0:9] (BCD)
    uint32_t Minute_Unit       : 4;  ///< Биты [14:17]  Минуты, десятки [0:5] (BCD)
    uint32_t Minute_Tens       : 4;  ///< Биты [18:21]  Минуты, сотни? (специфично) [0:9] (BCD)

    uint32_t Hour_Unit         : 4;  ///< Биты [22:25]  Часы, единицы [0:9] (BCD)
    uint32_t Hour_Tens         : 3;  ///< Биты [26:28]  Часы, десятки [0:2] (BCD)

    uint32_t Matrix            : 2;  ///< Биты [29:30]  Sign/Status Matrix [0:3]
    uint32_t Parity            : 1;  ///< Бит  [31]     Parity (odd) [0:1]
} ARINC_Word_125_Struct;

typedef union ARINC_Word_125_Union
{
    ARINC_Word_125_Struct Struct;
    uint32_t              Raw;
    uint8_t               ARINC_Array[4];
} ARINC_Word_125_Union;

/// @brief      "150". Время в двоичном виде. Структура слова ARINC-429.
typedef struct ARINC_Word_150_Struct // W150_t
{
// Бит №:  [31]     [30:29]   [28]       [27:23]   [22:17]   [16:11]    [10:8]       [7:0]
// Поле :  Parity   Matrix    Not_Used   Hour      Minute    Second     Second_Sub   Label
// Данные: [0:1]    [0:3]     [0:1]      [0:23]    [0:59]    [0:59]     [0:7]        [0:255]
    uint32_t Label             : 8;  ///< Биты [0:7]    LABEL [0:255]
    uint32_t Second_Sub        : 3;  ///< Биты [8:10]   Доли секунды [0:7]
    uint32_t Second            : 6;  ///< Биты [11:16]  Секунды [0:59]
    uint32_t Minute            : 6;  ///< Биты [17:22]  Минуты [0:59]
    uint32_t Hour              : 5;  ///< Биты [23:27]  Часы [0:23]
    uint32_t Not_Used          : 1;  ///< Бит  [28]     Не используется [0:1]
    uint32_t Matrix            : 2;  ///< Биты [29:30]  Sign/Status Matrix [0:3]
    uint32_t Parity            : 1;  ///< Бит  [31]     Parity (odd) [0:1]
} ARINC_Word_150_Struct;

typedef union ARINC_Word_150_Union
{
    ARINC_Word_150_Struct Struct;
    uint32_t              Raw;
    uint8_t               ARINC_Array[4];
} ARINC_Word_150_Union;

/// @brief      "260". Дата в BCD. Структура слова ARINC-429.
typedef struct ARINC_Word_260_Struct // W260_t
{
// Бит №:  [31]     [30:29]     [28:27]    [26:23]   [22]         [21:18]      [17:14]    [13:10]     [9:8]   [7:0]
// Поле :  Parity   Matrix      Day_Tens   Day_Units Month_Tens   Month_Units  Year_Tens  Year_Units  SDI     Label
// Данные: [0:1]    [0:3]       [0:3]      [0:9]     [0:1]        [0:9]        [0:9]      [0:9]       [0:3]   [0:255]
    uint32_t Label             : 8;  ///< Биты [0:7]    LABEL [0:255]
    uint32_t SDI               : 2;  ///< Биты [8:9]    Source/Destination Identifier [0:3]

    uint32_t Year_Units        : 4;  ///< Биты [10:13]  Год, единицы [0:9] (BCD)
    uint32_t Year_Tens         : 4;  ///< Биты [14:17]  Год, десятки [0:9] (BCD)

    uint32_t Month_Units       : 4;  ///< Биты [18:21]  Месяц, единицы [0:9] (BCD)
    uint32_t Month_Tens        : 1;  ///< Бит  [22]     Месяц, десятки [0:1] (BCD)

    uint32_t Day_Units         : 4;  ///< Биты [23:26]  День, единицы [0:9] (BCD)
    uint32_t Day_Tens          : 2;  ///< Биты [27:28]  День, десятки [0:3] (BCD)

    uint32_t Matrix            : 2;  ///< Биты [29:30]  Sign/Status Matrix [0:3]
    uint32_t Parity            : 1;  ///< Бит  [31]     Parity (odd) [0:1]
} ARINC_Word_260_Struct;

typedef union ARINC_Word_260_Union
{
    ARINC_Word_260_Struct Struct;
    uint32_t              Raw;
    uint8_t               ARINC_Array[4];
} ARINC_Word_260_Union;

//TODO в оригинале описания не было, уточнить суть структур
/// @brief      Структура режима работы канала
/// @details    Двухбитовое поле Matrix в ARINC-429 определяет статус/режим работы передатчика
///             или системы в целом. Конкретные значения и трактовка зависят от спецификации ARINC.
///             Данное перечисление использовано в ARINC_Word_300_Struct.
/// @warning    Назначения некоторых кодов (NC) требуют уточнения в официальной документации.
typedef enum ARINC_A300_Matrix_Enum // enum A300_Matrix_e
{
    ARINC_A300_MATRIX_NC1               = 0x00, ///< Не используется / зарезервировано #1
    ARINC_A300_MATRIX_FUNCTIONAL_TEST   = 0x01, ///< Режим функционального теста
    ARINC_A300_MATRIX_NC2               = 0x02, ///< Не используется / зарезервировано #2
    ARINC_A300_MATRIX_NORMAL_OPERATION  = 0x03  ///< Нормальная работа
} ARINC_A300_Matrix_Enum;

/// @brief      Структура статуса системы в ARINC-429 сообщении
/// @details    Определяет общий статус системы или устройства (2 бита).
///             Используется в ARINC_Word_300_Struct для передачи диагностической информации.
/// @warning    Коды NC — зарезервированы или не используются; интерпретация зависит от контекста протокола.
typedef enum ARINC_System_Status_Enum // enum System_Status_e
{
    ARINC_SYSTEM_STATUS_NC1              = 0x00, ///< Не используется / зарезервировано #1
    ARINC_SYSTEM_STATUS_PARTIALLY_FAULT  = 0x01, ///< Частичная неисправность
    ARINC_SYSTEM_STATUS_NC2              = 0x02, ///< Не используется / зарезервировано #2
    ARINC_SYSTEM_STATUS_FAULT            = 0x03  ///< Полная неисправность
} ARINC_System_Status_Enum;

/// @brief      "300". Состояние оборудования. Структура слова ARINC-429.
typedef struct ARINC_Word_300_Struct // W300_t
{
// Бит №:  [31]     [30:29]     [28:22]        [21]           [20]         [19]            [18]                    [17:16]         [15]            [14]            [13]            [12]            [11]            [10]            [9:8]   [7:0]
// Поле :  Parity   Matrix      Not_Used       Camera_Switch  XAE21_Fault  Storage_Status  Service_Router_Status   System_Status   Camera_6_Fault  Camera_5_Fault  Camera_4_Fault  Camera_3_Fault  Camera_2_Fault  Camera_1_Fault  SDI     Label
// Данные: [0:1]    [0:3]       [0:127]        [0:1]          [0:1]        [0:1]           [0:1]                   [0:3]           [0:1]           [0:1]           [0:1]           [0:1]           [0:1]           [0:1]           [0:3]   [0:255]    
    uint32_t Label             : 8;  ///< Биты [0:7]    LABEL [0:255]
    uint32_t SDI               : 2;  ///< Биты [8:9]    Source/Destination Identifier [0:3]

    uint32_t Camera_1_Fault    : 1;  ///< Бит  [10]     Неисправность камеры 1 [0:1]
    uint32_t Camera_2_Fault    : 1;  ///< Бит  [11]     Неисправность камеры 2 [0:1]
    uint32_t Camera_3_Fault    : 1;  ///< Бит  [12]     Неисправность камеры 3 [0:1]
    uint32_t Camera_4_Fault    : 1;  ///< Бит  [13]     Неисправность камеры 4 [0:1]
    uint32_t Camera_5_Fault    : 1;  ///< Бит  [14]     Неисправность камеры 5 [0:1]
    uint32_t Camera_6_Fault    : 1;  ///< Бит  [15]     Неисправность камеры 6 [0:1]

    ARINC_System_Status_Enum System_Status : 2;  ///< Биты [16:17]  Код состояния системы
    uint32_t Service_Router_Status         : 1;  ///< Бит  [18]     Состояние сервисного маршрутизатора [0:1]
    uint32_t Storage_Status                : 1;  ///< Бит  [19]     Состояние накопителя [0:1]
    uint32_t XAE21_Fault                   : 1;  ///< Бит  [20]     Неисправность XAE21 [0:1]
    uint32_t Camera_Switch_Fault           : 1;  ///< Бит  [21]     Неисправность переключателя камер [0:1]

    uint32_t Not_Used              : 7;  ///< Биты [22:28]  Не используются [0:127]
    ARINC_A300_Matrix_Enum Matrix  : 2;  ///< Биты [29:30]  Sign/Status Matrix [0:3]
    uint32_t Parity                : 1;  ///< Бит  [31]     Parity (odd) [0:1]
} ARINC_Word_300_Struct;

typedef union ARINC_Word_300_Union
{
    ARINC_Word_300_Struct Struct;
    uint32_t              Raw;
    uint8_t               ARINC_Array[4];
} ARINC_Word_300_Union;

/// @brief      Статус Linux-серверов системы
/// @details    Состояния двух Linux-серверов, упакованное в 2 бита.
///             Используется как интерфейс между Linux-частью системы и ARINC-подсистемой.
///             Передаётся по UART пакетом вида:
///                 Байт 0: 0xCE (код команды "статус Linux")                                           <br>
///                 Байт 1: [Service Router Status][Storage Server Status][резерв:6]                    <br>
///                 Байт 2: CRC8                                                                        <br>
///             После приёма статусы попадают в слово ARINC-429 №300 для широковещательной передачи.    <br>
/// @note       Логика полей:
///                 0 = Сервер работает нормально
///                 1 = Сервер неисправен или недоступен
typedef struct ARINC_Linux_Status_Struct // Linux_Status_t
{
    uint8_t Service_Router_Status : 1;  ///< Бит [0] — Статус сервера маршрутизации [0:1]
    uint8_t Storage_Server_Status : 1;  ///< Бит [1] — Статус сервера накопителя  [0:1]
} ARINC_Linux_Status_Struct;

/// @brief      Основной цикл обработки подсистемы ARINC‑429
/// @details    Центральная функция диспетчеризации событий ARINC‑429. <br>
///             Вызывается периодически в основном цикле `main()` и выполняет: <br>
///                 1. Контроль таймаутов связи: <br>
///                     XAE‑21 — источник времени/даты. <br>
///                     Сервер маршрутизации (Service Router). <br>
///                     Сервер даты/времени (Date/Time Server). <br>
///                 2. Обработку прерываний от приёмопередатчика HI‑3220: <br>
///                     Подтверждение ACK на активное прерывание. <br>
///                     Чтение регистра прерываний и количества слов в FIFO. <br>
///                     Побайтовое чтение FIFO для каналов 0 и 1. <br>
///                     Парсинг сообщений функциями `ARINC429_Parse_Message_Ch1()`/`Ch2()`. <br>
///                 3. Формирование слова статуса №0300 (раз в 500 мс): <br>
///                     LABEL = 0300, SDI = 3. <br>
///                     Оценка состояния камер по потребляемому току (INA‑226). <br>
///                     Заполнение статусов Linux‑серверов (см. ARINC_Linux_Status_Struct). <br>
///                     Запись признаков отказа модулей (XAE‑21, переключатель камер и др.). <br>
///                     Определение общего `System_Status` (FAULT / PARTIALLY_FAULT / NC). <br>
///                     Передача слова 0300 через HI‑3220 в канал 0. <br>
///                 4. Обслуживание очереди передачи UART3: <br>
///                     Если DMA готово — выборка данных из кольцевого буфера <br>
///                       `cbARINCUartOutput` и передача `HAL_UART_Transmit_DMA()`. <br>
/// @note       Без вызова этой функции подсистема ARINC‑429 не будет: <br>
///                 получать и обрабатывать сообщения; <br>
///                 транслировать системный статус в авионику; <br>
///                 поддерживать актуальные данные о состоянии оборудования. <br>
///             Зависимости: <br>
///                 ├─ HAL_GetTick <br>
///                 ├─ Tool_HI3220_Address_Write/Tool_HI3220_Address_Read <br>
///                 ├─ ARINC_HI3220_Write_Pin/ARINC_HI3220_Read_Pin <br>
///                 ├─ ARINC429_Parse_Message_Ch1() <br> //TODO поставить новую функцию здесь когда она будет
///                 ├─ T_INA226_GetPower() <br> //TODO поставить новую функцию здесь когда она будет
///                 └─ HAL_UART_Transmit_DMA() <br> //TODO еще раз внимательно разобраться как здесь использовался DMA
/// @warning    Нарушение порядка вызовов или исключение этой функции из главного цикла
///             приведёт к полной остановке обмена ARINC‑429 и рассинхронизации системы.
void ARINC_process(void);

/// @brief      Парсинг ARINC-429 сообщений канала 1 (от ХАЭ-21)
/// @details    Обрабатывает входящие ARINC слова от авиационного хронометра ХАЭ-21: <br>
///                 **Слово 150:** Время в двоичном формате (hour, minute, second) <br>
///                 **Слово 125:** Время в BCD формате (резерв, не используется активно)  <br> 
///                 **Слово 260:** Дата в BCD формате (year, month, day) <br>
///             При успешном приёме любого слова: <br>
///                 Обновляются соответствующие поля ARINC_DateTime <br>
///                 Сбрасывается таймаут связи с ХАЭ-21 <br>
///                 Снимается флаг неисправности XAE21_Fault
/// @param[in]  Data_Ptr Указатель на 4-байтный массив с принятым ARINC словом
///                 (порядок байтов: [Label][Byte1][Byte2][Byte3])
/// @warning    Указатель Data_Pointer должен указывать на валидный массив из 4 байт.
///                 Некорректные данные могут привести к неопределённому поведению
/// @note       Функция автоматически определяет тип слова по полю Label (Data_Pointer[0])
///                 и выполняет соответствующую обработку
void ARINC_Parse_Message_Channel_1(uint8_t *Data_Ptr);

/// @brief      Обработка входящих UART пакетов от Linux-серверов 
/// @details    Универсальный обработчик пакетов различных типов: <br>
///                 **Пакеты 0x55 (от сервера маршрутизации):** <br>
///                     0x55 0x33: SetStatus — установка статусов серверов <br>
///                     0x55 0x11: GetStatusQuery — запрос статуса камеры   <br>
///                 **Пакеты 0xCE (статус Linux):** <br>
///                     Содержат битовые флаги состояния srv_stat/storage_stat <br>
///                     Обновляют ARINC_System_Status для передачи в слово 0300 <br>
///                 **Пакеты 0xBE (запросы данных):** <br>
///                     0xBE 0x01: запрос пакета управления (Control) <br>
///                     0xBE 0x02: запрос пакета даты/времени (DateTime)
/// @param[in]  Packet_Ptr    Указатель на входящий пакет
/// @param[in]  Packet_Size   Размер пакета в байтах
/// @param[out] Output_Ptr    Указатель на буфер для ответного пакета
/// @param[out] Output_Size   Указатель на переменную размера ответа
/// @warning    Все указатели должны быть валидными. Output_Pointer должен иметь
///                 достаточный размер для размещения ответного пакета (до 20 байт)
/// @note       Функция автоматически вычисляет и добавляет CRC8 к ответным пакетам
void ARINC_Protocol_Input_Packet(uint8_t *Packet_Ptr, uint8_t Packet_Size, 
                                 uint8_t *Output_Ptr, uint8_t *Output_Size);

#pragma pack(pop)

/// @brief      Функция инициализации модуля
/// @details    Конфигурирует HI-3220, включает его и настраивает порты
/// @param[in]  TX_Ptr    Указатель на функцию передачи данных  (См. Tool_HI3220_SPI_TX())
/// @param[in]  RX_Ptr    Указатель на функцию получения данных (См. Tool_HI3220_SPI_RX())
/// @param[in]  TRX_Ptr   Указатель на функцию одновременной приемо-передачи данных (См. Tool_HI3220_TRX())
/// @param[in]  Set_Pin   Указатель на функцию записи значения на цифровой вывод (См. Tool_HI3220_Set_Pin())
/// @param[in]  Read_Pin  Указатель на функцию чтения пина (См. Tool_HI3220_Read_Pin())
/// @param[in]  Delay_Ptr Указатель на функцию приостановки работы ПО (См. Tool_HI3220_Delay())
/// @return     Возвращает TOOLS_ERROR_CODE_ALL_OK в случае успешного выполнения функции.
///                 В противном случае, возвращает код ошибки
int ARINC_Configuration(SPI_HandleTypeDef *SPI_Handle_Ptr, SPI_HandleTypeDef *SPI_Handle_Black_Box_Ptr);

#endif
