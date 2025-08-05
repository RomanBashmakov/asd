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

#include "spi.h"
#include "stm32f7xx_hal.h"

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
int ARINC_Configuration(SPI_HandleTypeDef *SPI_Handle_Ptr, SPI_HandleTypeDef *SPI_Handle_Black_Box_Ptr)

#endif
