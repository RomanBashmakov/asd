/// @file       ARINC.h
/// @author     Башмаков Р.А. bashmakovra@raitibor.ru
/// @brief      Заголовочный файл, содержащий объявления модуля для работы по протоколу ARINC
/// @details    Порядок работы с модулем:
///                 1) Конфигурировать ARINC (см. ARINC_Configuration())        <br>
///                 2) Периодически в основном цикле вызывать ARINC_Process()   <br>
///                 3) Обрабатывать прерывания через HAL_GPIO_EXTI_Callback()   <br>
/// @note       HI3220 подключен через SPI2. Для работы с HI3220 задействованы 9 выводов МК

#ifndef _ARINC_H_
#define _ARINC_H_

#include <_Includes.h>
#include <stdbool.h>
#include <stdint.h>

// TODO добавить файл с функциями прослойками с рабочего компа
//  #include "spi.h"
#include "stm32f7xx_hal.h"

/// @brief  Пин INT
/// Используется в stm32f7xx_it.c
#define ARINC_PIN_INT_PIN GPIO_PIN_3  // GPI_ar429_intn_Pin

/// @brief Период передачи пакета по ARINC 429 с лейблом 0300
#define ARINC_A429_L0300_SEND_PERIOD 500

/// @brief Пороговое значение тока для определения состояния камеры
#define ARINC_CAMERA_CURRENT_THRESHOLD 0.05

/// @brief Таймаут для ХАЭ21
#define ARINC_XAE21_TIMEOUT 5000

/// @brief Таймаут для сервера
#define ARINC_SVR_TIMEOUT 5000

/// @brief Общее количество каналов камер
#define ARINC_CHANNELS_TOTAL 6

#pragma pack(push, 1)

/// @brief Структура Сервера Накопителя и Сервера Маршрутизатора
typedef struct ARINC_Status
{
    uint8_t Archive;  ///< Состояние Сервера Накопителя
    uint8_t Router;   ///< Состояние Сервера Маршрутизатора
} ARINC_Status;

/// @brief Структура пакета с состоянием Сервера Накопителя и Сервера Маршрутизатора
typedef struct ARINC_SetStatus
{
    uint8_t      Flag;    ///< Флаг пакета
    uint8_t      Code;    ///< Код пакета
    ARINC_Status Status;  ///< Статус Сервера Накопителя и Сервера Маршрутизатора
} ARINC_SetStatus;

/// @brief Структура пакета с запросом на получение состояния Камеры
typedef struct ARINC_GetStatusQuery
{
    uint8_t Flag;    ///< Флаг пакета
    uint8_t Code;    ///< Код пакета
    uint8_t Camera;  ///< Номер камеры
} ARINC_GetStatusQuery;

/// @brief Структура пакета с ответом на запрос о получении состояния Камеры
typedef struct ARINC_GetStatusResponse
{
    uint8_t flag;    ///< Флаг пакета
    uint8_t code;    ///< Код пакета
    uint8_t camera;  ///< Номер камеры
    uint8_t status;  ///< Состояние камеры
} ARINC_GetStatusResponse;

/// @brief Структура 32-битного ARINC-429 слова
typedef struct ARINC_Word_Struct
{
    uint32_t ARINC_Word_Parity : 1;   ///< Бит чётности (odd parity)
    uint32_t ARINC_Word_SSM    : 2;   ///< Sign/Status Matrix
    uint32_t ARINC_Word_Data   : 19;  ///< Основные данные
    uint32_t ARINC_Word_SDI    : 2;   ///< Source/Destination Identifier
    uint32_t ARINC_Word_Label  : 8;   ///< Label (адрес сообщения)
} ARINC_Word_Struct;

/// @brief Union для доступа к ARINC-429 слову как к raw значению или по полям
typedef union Tool_ARINC429_Word_Union
{
    uint32_t          Tool_ARINC429_Word_Raw;     ///< Целое 32-битное значение
    ARINC_Word_Struct Tool_ARINC429_Word_Fields;  ///< Структурированные поля ARINC-429
} Tool_ARINC429_Word_Union;

/// @brief Структура управления ARINC429
typedef struct ARINC_Control_Struct
{
    // arinc protocol data
    // 1
    char FORMAT1;                //(7 бит)
    char ACTIVE_CONF_MENU_CAPT;  // (1 бит)

    // 2
    char FORMAT1_POZITION;    // (3 бита)
    char FORMAT1_SIZE;        // (4 бита)
    char ACTIVE_CURSOR_CAPT;  // (1 бит)

    // 3
    uint16_t CURSOR_X_CAPT;  // (старшие 8 бит)

    // 4
    char CAMS_NUMBER_CAPT;  // (4 бита)
    char BTN_PRESSED_CAPT;  // (1 бит)

    // 5
    uint16_t CURSOR_Y_CAPT;  // (старшие 8 бит)

    // 6
    char JOYSTICK_DOWN_CAPT;   // (1 бит)
    char JOYSTICK_LEFT_CAPT;   // (1 бит)
    char JOYSTICK_RIGHT_CAPT;  // (1 бит)
    char JOYSTICK_UP_CAPT;     // (1 бит)
    char LEFT_ENTER_CAPT;      // (1 бит)

    // 7
    char LENT_JAMMED_CAPT;   // (1 бит)
    char RIGHT_ENTER_CAPT;   // (1 бит)
    char RENT_JAMMED_CAPT;   // (1 бит)
    char ESC_BTN_CAPT;       // (1 бит)
    char ESC_JAMMED_CAPT;    // (1 бит)
    char OK_BTN_CAPT;        // (1 бит)
    char OK_JAMMED_CAPT;     // (1 бит)
    char LWR_AREA_BTN_CAPT;  // (1 бит)

    // 8
    char FORMAT2;              // (7 бит)
    char ACTIVE_CONF_MENU_FO;  // (1 бит)

    // 9
    char FORMAT2_POZITION;  // (3 бита)
    char FORMAT2_SIZE;      // (4 бита)
    char ACTIVE_CURSOR_FO;  // (1 бит)

    // 10
    uint16_t CURSOR_X_FO;  // (старшие 8 бит)

    // 11
    char CAMS_NUMBER_FO;  // (4 бит)
    char BTN_PRESSED_FO;  // (1 бит)

    // 12
    uint16_t CURSOR_Y_FO;  // (старшие 8 бит)

    // 13
    char JOYSTICK_DOWN_FO;   // (1 бит)
    char JOYSTICK_LEFT_FO;   // (1 бит)
    char JOYSTICK_RIGHT_FO;  // (1 бит)
    char JOYSTICK_UP_FO;     // (1 бит)
    char LEFT_ENTER_FO;      // (1 бит)

    // 14
    char LENT_JAMMED_FO;   // (1 бит)
    char RIGHT_ENTER_FO;   // (1 бит)
    char RENT_JAMMED_FO;   // (1 бит)
    char ESC_BTN_FO;       // (1 бит)
    char ESC_JAMMED_FO;    // (1 бит)
    char OK_BTN_FO;        // (1 бит)
    char OK_JAMMED_FO;     // (1 бит)
    char LWR_AREA_BTN_FO;  // (1 бит)

} ARINC_Control_Struct;

/// @brief Структура даты и времени ARINC429
typedef struct ARINC_ControlDateTime_Struct
{
    char hour;
    char minute;
    char sec;

    char year;
    char month;
    char day;
} ARINC_ControlDateTime_Struct;

/// @brief Статус Linux сервера
typedef struct ARINC_Linux_Status_Struct
{
    unsigned char srv_stat     : 1;
    unsigned char storage_stat : 1;
} ARINC_Linux_Status_Struct;

/// @brief Состояния системы
typedef enum ARINC_System_Status_Enum
{
    ARINC_SYS_NC              = 0,  ///< Нормальная работа
    ARINC_SYS_PARTIALLY_FAULT = 1,  ///< Частичная неисправность
    ARINC_SYS_FAULT           = 2   ///< Неисправность
} ARINC_System_Status_Enum;

/// @brief Матрица A300
typedef enum ARINC_A300_Matrix_Enum
{
    ARINC_A300_MATRIX_NORMAL          = 0,
    ARINC_A300_MATRIX_TEST            = 1,
    ARINC_A300_MATRIX_FUNCTIONAL_TEST = 2,
    ARINC_A300_MATRIX_INVALID         = 3
} ARINC_A300_Matrix_Enum;

/*--- Структуры ARINC-429 слов ---*/

/*--- 125 -----------------------*/
typedef struct ARINC_Word125_Struct
{  // Time in BCD
    uint32_t label    : 8;
    uint32_t sdi      : 2;
    uint32_t minutesS : 4;
    uint32_t minutesU : 4;
    uint32_t minutesT : 4;
    uint32_t hoursU   : 4;
    uint32_t hoursT   : 3;
    uint32_t matrix   : 2;
    uint32_t parity   : 1;
} ARINC_Word125_Struct;

typedef union ARINC_W125_Union
{
    ARINC_Word125_Struct str;
    uint32_t             word429;
} ARINC_W125_Union;

/*--- 150 -----------------------*/
typedef struct ARINC_Word150_Struct
{  // Time in binary
    uint32_t label   : 8;
    uint32_t secondS : 3;
    uint32_t second  : 6;
    uint32_t minute  : 6;
    uint32_t hour    : 5;
    uint32_t nc      : 1;
    uint32_t matrix  : 2;
    uint32_t parity  : 1;
} ARINC_Word150_Struct;

typedef union ARINC_W150_Union
{
    ARINC_Word150_Struct str;
    uint32_t             word429;
} ARINC_W150_Union;

/*--- 260 -----------------------*/
typedef struct ARINC_Word260_Struct
{  // Date in BCD
    uint32_t label  : 8;
    uint32_t sdi    : 2;
    uint32_t yearU  : 4;
    uint32_t yearT  : 4;
    uint32_t monthU : 4;
    uint32_t monthT : 1;
    uint32_t dayU   : 4;
    uint32_t dayT   : 2;
    uint32_t matrix : 2;
    uint32_t parity : 1;
} ARINC_Word260_Struct;

typedef union ARINC_W260_Union
{
    ARINC_Word260_Struct str;
    uint32_t             word429;
} ARINC_W260_Union;

/*--- 300 -----------------------*/
typedef struct ARINC_Word300_Struct
{  // Из протокола ИВ СВР-МТО_14.04
    uint32_t                 label            : 8;
    uint32_t                 sdi              : 2;
    uint32_t                 cam1_fault       : 1;
    uint32_t                 cam2_fault       : 1;
    uint32_t                 cam3_fault       : 1;
    uint32_t                 cam4_fault       : 1;
    uint32_t                 cam5_fault       : 1;
    uint32_t                 cam6_fault       : 1;
    ARINC_System_Status_Enum System_Status    : 2;
    uint32_t                 SrvRouter_Status : 1;
    uint32_t                 Storage_Status   : 1;
    uint32_t                 XAE21_fault      : 1;
    uint32_t                 cam_switch_fault : 1;
    uint32_t                 nc               : 7;
    ARINC_A300_Matrix_Enum   matrix           : 2;
    uint32_t                 parity           : 1;
} ARINC_Word300_Struct;

typedef union ARINC_W300_Union
{
    ARINC_Word300_Struct str;
    uint32_t             word429;
    uint8_t              arinc_array[4];
} ARINC_W300_Union;

#pragma pack(pop)

/// @brief Структура системного статуса
typedef struct ARINC_System_Status_Struct
{
    bool    XAE21_fault;
    bool    cam_switch_fault;
    uint8_t srvrouter_stat;
    uint8_t storage_stat;
    uint8_t system;
} ARINC_System_Status_Struct;

/// @brief Глобальные переменные
extern ARINC_Control_Struct         ARINC_Control;
extern ARINC_ControlDateTime_Struct ARINC_DateTime;
extern ARINC_Linux_Status_Struct    ARINC_Linux_Status;
extern ARINC_System_Status_Struct   ARINC_System_Status;
extern uint8_t                      ARINC_Channel_No;

/// @brief      Функция инициализации модуля
/// @details    Конфигурирует HI-3220, включает его и настраивает порты
/// @param[in]  SPI_Handle_Ptr Указатель на дескриптор SPI для HI3220
/// @param[in]  SPI_Handle_Black_Box_Ptr Указатель на дескриптор SPI для черного ящика
/// @return     Возвращает TOOLS_ERROR_CODE_ALL_OK в случае успешного выполнения функции <br>
///                 В противном случае, возвращает код ошибки
int ARINC_Configuration(SPI_HandleTypeDef *SPI_Handle_Ptr, SPI_HandleTypeDef *SPI_Handle_Black_Box_Ptr);

/// @brief      Основная функция обработки ARINC
/// @details    Должна вызываться периодически в основном цикле программы
void ARINC_Process(void);

/// @brief      Функция разбора сообщений первого канала ARINC 429
/// @param[in]  data Указатель на данные сообщения (4 байта)
void ARINC_ParseMessageCh1(char *data);

/// @brief      Функция разбора сообщений второго канала ARINC 429
/// @param[in]  data Указатель на данные сообщения (4 байта)
void ARINC_ParseMessageCh2(char *data);

/// @brief      Обработка входящих пакетов протокола
/// @param[in]  packet Указатель на пакет данных
/// @param[in]  size Размер пакета
/// @param[out] out Выходной буфер для ответа
/// @param[out] outSize Размер выходного пакета
void ARINC_Proto_InputPacket(char *packet, char size, char *out, char *outSize);

/// @brief      Формирование пакета управления для UART
/// @param[out] packet Указатель на выходной буфер пакета
void ARINC_Proto_BuildUartPacketControl(char *packet);

/// @brief      Формирование пакета даты/времени для UART
/// @param[out] packet Указатель на выходной буфер пакета
void ARINC_Proto_BuildUartPacketDT(char *packet);

/// @brief      Функция получения тока потребления камеры (заглушка)
/// @param[in]  camera_id ID камеры
/// @return     Значение тока в амперах
float ARINC_Get_Camera_Current(uint8_t camera_id);

/// @brief      Функция получения мощности потребления камеры (заглушка)
/// @param[in]  camera_id ID камеры
/// @return     Значение мощности в ваттах
float ARINC_Get_Camera_Power(uint8_t camera_id);

#endif