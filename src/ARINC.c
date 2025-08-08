/// @file       ARINC.c
/// @author     Башмаков Р.А. bashmakovra@raitibor.ru
/// @brief      См. ARINC.h

#include "ARINC.h"

#include "Circular_Buffer.h"
#include "Tool_HI3220.h"

/// @brief      Порт и пин RX
#define ARINC_PIN_RX GPIOD, GPIO_PIN_8  // mcu2pc_Pin

/// @brief      Порт и пин TX
#define ARINC_PIN_TX GPIOD, GPIO_PIN_9  // pc2mcu_Pin

/// @brief      Порт и пин MRST
#define ARINC_PIN_MRST GPIOD, GPIO_PIN_0  // GPO_ar429_rstn_Pin

/// @brief      Порт и пин READY
#define ARINC_PIN_READY GPIOD, GPIO_PIN_4  // GPI_ar429_ready_Pin

/// @brief      Порт и пин RUN
#define ARINC_PIN_RUN GPIOD, GPIO_PIN_1  // GPO_ar429_run_Pin

/// @brief      Порт и пин INT
#define ARINC_PIN_INT GPIOD, GPIO_PIN_3  // GPI_ar429_intn_Pin

/// @brief      Порт и пин ACK
#define ARINC_PIN_ACK GPIOD, GPIO_PIN_2  // GPO_ar429_ack_Pin

/// @brief      Порт и пин CS
#define ARINC_PIN_CS GPIOB, GPIO_PIN_4  // GPO_ar429_csn_Pin

/// @brief      Количество элементов в кольцевом буфере
#define ARINC_UART_OUTPUT_BUFFER_LENGTH 200

/// @brief      Размер буфера передачи UART3
#define ARINC_UART3_TX_BUFFER_SIZE 200

/// @brief      Структура с описанием одного GPIO пина ARINC
typedef struct ARINC_Pin_Struct
{
    GPIO_TypeDef *Port;        ///< Аппаратный порт GPIO
    uint16_t      Pin;         ///< Номер пина GPIO
    uint8_t       IsInverted;  ///< "1" если пин инвертированный
} ARINC_Pin_Struct;

/// @brief Перечисление режимов инвертирования пинов HI3220
typedef enum ARINC_HI3220_Pin_Inverted_Enum
{
    ARINC_HI3220_PIN_INVERTED_OFF = 0,  ///< Пин не инвертирован
    ARINC_HI3220_PIN_INVERTED_ON  = 1,  ///< Пин инвертирован
} ARINC_HI3220_Pin_Inverted_Enum;

/// @brief Список входных пинов
static const ARINC_Pin_Struct ARINC_GPI_Pins_Map[TOOL_HI3220_GPI_COUNT] = {
    { ARINC_PIN_READY, ARINC_HI3220_PIN_INVERTED_OFF },
    { ARINC_PIN_INT, ARINC_HI3220_PIN_INVERTED_OFF }
};

/// @brief Список выходных пинов
static const ARINC_Pin_Struct ARINC_GPO_Pins_Map[TOOL_HI3220_GPO_COUNT] = {
    { ARINC_PIN_MRST, ARINC_HI3220_PIN_INVERTED_ON },
    { ARINC_PIN_RUN, ARINC_HI3220_PIN_INVERTED_OFF },
    { ARINC_PIN_ACK, ARINC_HI3220_PIN_INVERTED_OFF },
    { ARINC_PIN_CS, ARINC_HI3220_PIN_INVERTED_ON }
};

/// @brief Глобальные переменные
ARINC_Control_Struct         ARINC_Control;
ARINC_ControlDateTime_Struct ARINC_DateTime;
ARINC_Linux_Status_Struct    ARINC_Linux_Status;
ARINC_System_Status_Struct   ARINC_System_Status;
uint8_t                      ARINC_Channel_No = 0;

/// @brief Кольцевой буфер для UART вывода
Circular_Buffer_Struct ARINC_UART_Output_Circular_Buffer;
uint8_t                ARINC_TX_Buffer[ARINC_UART_OUTPUT_BUFFER_LENGTH];

/// @brief Буфер приема ARINC данных
uint8_t ARINC_Recv_Buffer[1000];

/// @brief Временный буфер для формирования пакетов
char    ARINC_Temp_Buffer[50];
uint8_t ARINC_UART3_TX_Buffer[ARINC_UART3_TX_BUFFER_SIZE];

/// @brief Пакеты для передачи по ARINC 429
uint8_t ARINC_TxPacket429_1[4] = { 0300, 0x03, 0x00, 0x60 };
uint8_t ARINC_TxPacket429_2[4] = { 0301, 0x00, 0x00, 0x00 };
uint8_t ARINC_TxPacket429_3[4] = { 0302, 0x00, 0x00, 0x00 };

/// @brief Union для работы с ARINC словом 300
ARINC_W300_Union ARINC_Word_300;

/// @brief Простая реализация таймеров
typedef struct ARINC_Timer_Struct
{
    uint32_t start_time;
    uint32_t timeout;
    bool     expired;
} ARINC_Timer_Struct;

/// @brief Таймеры для различных событий
ARINC_Timer_Struct ARINC_Timer_RecvTimeout_XAE21;
ARINC_Timer_Struct ARINC_Timer_RecvTimeout_Ch2;
ARINC_Timer_Struct ARINC_Timer_PcRecvTimeout_Ctrl;
ARINC_Timer_Struct ARINC_Timer_PcRecvTimeout_DT;
ARINC_Timer_Struct ARINC_Timer_SendTimeout_Ch1;

/// @brief Функция записи на цифровой вывод МК
/// @param[in] Pin Вывод МК, подключенного к HI3220 (см. Tool_HI3220_GPO_Enum)
/// @param[in] State Устанавливаемое значение (см. Tool_Common_Pin_State_Enum)
void ARINC_HI3220_Write_Pin(const Tool_HI3220_GPO_Enum Pin, const Tool_Common_Pin_State_Enum State);

/// @brief Функция чтения цифрового вывода МК
/// @param[in] Pin Вывод МК, подключенный к HI3220 (см. Tool_HI3220_GPI_Enum)
/// @return Текущее значение на указанном выводе МК (см. Tool_Common_Pin_State_Enum)
Tool_Common_Pin_State_Enum ARINC_HI3220_Read_Pin(const Tool_HI3220_GPI_Enum Pin);

/// @brief Функция конфигурации ARINC HI-3220 для текущего проекта
int ARINC_HI3220_Configuration(void);

/// @brief Инициализация таймера
/// @param[in] timer Указатель на таймер
/// @param[in] timeout Таймаут в миллисекундах
void ARINC_Timer_Set(ARINC_Timer_Struct *timer, uint32_t timeout);

/// @brief Проверка истечения таймера
/// @param[in] timer Указатель на таймер
/// @return true если таймер истек
bool ARINC_Timer_Expired(ARINC_Timer_Struct *timer);

/// @brief Функция вычисления CRC8 (заглушка)
/// @param[in] data Указатель на данные
/// @param[in] length Длина данных
/// @return Значение CRC8
uint8_t ARINC_CRC8_Calc(uint8_t *data, uint8_t length);

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

int ARINC_Configuration(SPI_HandleTypeDef *SPI_Handle_Ptr, SPI_HandleTypeDef *SPI_Handle_Black_Box_Ptr)
{
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

    // Инициализация таймеров
    ARINC_Timer_Set(&ARINC_Timer_RecvTimeout_XAE21, ARINC_XAE21_TIMEOUT);
    ARINC_Timer_Set(&ARINC_Timer_RecvTimeout_Ch2, 5000);
    ARINC_Timer_Set(&ARINC_Timer_PcRecvTimeout_Ctrl, ARINC_SVR_TIMEOUT);
    ARINC_Timer_Set(&ARINC_Timer_PcRecvTimeout_DT, ARINC_SVR_TIMEOUT);
    ARINC_Timer_Set(&ARINC_Timer_SendTimeout_Ch1, ARINC_A429_L0300_SEND_PERIOD);

    return TOOLS_ERROR_CODE_ALL_OK;
}

void ARINC_Process(void)
{
    bool    Critical_Fault     = false;
    bool    Non_Critical_Fault = false;
    uint8_t cams_N_faults      = 0;

    // Проверка таймаутов
    if (ARINC_Timer_Expired(&ARINC_Timer_RecvTimeout_XAE21))
    {
        ARINC_System_Status.XAE21_fault = true;
    }

    if (ARINC_Timer_Expired(&ARINC_Timer_PcRecvTimeout_Ctrl))
    {
        // Таймаут связи с контроллером
    }

    if (ARINC_Timer_Expired(&ARINC_Timer_PcRecvTimeout_DT))
    {
        // Таймаут связи даты/времени
    }

    // Опрос вывода прерывания HI-3220
    uint8_t int_pin_state = HAL_GPIO_ReadPin(ARINC_GPI_Pins_Map[TOOL_HI3220_GPI_INT].Port,
                                             ARINC_GPI_Pins_Map[TOOL_HI3220_GPI_INT].Pin);

    if (int_pin_state == 0)
    {
        // Отправка ACK
        ARINC_HI3220_Write_Pin(TOOL_HI3220_GPO_ACK, TOOL_COMMON_PIN_STATE_LOW);
        HAL_Delay(6);
        ARINC_HI3220_Write_Pin(TOOL_HI3220_GPO_ACK, TOOL_COMMON_PIN_STATE_HIGH);

        // Чтение регистра прерываний
        uint8_t reg = 0;  // TODO: HI3220_readReg(&hi, _RPIRL);

        if ((reg & 0x01) == 0x01)
        {
            // Обработка первого канала
            // TODO: реализовать чтение FIFO и обработку данных
            uint8_t count = 0;  // TODO: получить количество сообщений в FIFO

            if (count)
            {
                // TODO: HI3220_readFIFO(&hi, 0, ARINC_Recv_Buffer, count);

                char *p = (char *)ARINC_Recv_Buffer;
                for (int i = 0; i < count; i++)
                {
                    ARINC_ParseMessageCh1(p);
                    p += 4;
                }
            }
        }

        if ((reg & 0x02) == 0x02)
        {
            // Обработка второго канала
            uint8_t count = 0;  // TODO: получить количество сообщений в FIFO

            if (count)
            {
                char *p = (char *)ARINC_Recv_Buffer;
                for (int i = 0; i < count; i++)
                {
                    ARINC_ParseMessageCh2(p);
                    p += 4;
                }
            }
        }
    }

    // Формирование и отправка ARINC с лейблом 0300
    if (ARINC_Timer_Expired(&ARINC_Timer_SendTimeout_Ch1))
    {
        ARINC_Timer_Set(&ARINC_Timer_SendTimeout_Ch1, ARINC_A429_L0300_SEND_PERIOD);

        ARINC_Word_300.str.label = 0300;
        ARINC_Word_300.str.sdi   = 0x03;
        ARINC_Word_300.str.nc    = 0x00;

        // Проверка состояния камер
        ARINC_Word_300.str.cam1_fault = (ARINC_Get_Camera_Power(0) < 2) ? true : false;
        if (ARINC_Word_300.str.cam1_fault)
        {
            Non_Critical_Fault = true;
            cams_N_faults++;
        }

        ARINC_Word_300.str.cam2_fault = (ARINC_Get_Camera_Power(1) < 2) ? true : false;
        if (ARINC_Word_300.str.cam2_fault)
        {
            Non_Critical_Fault = true;
            cams_N_faults++;
        }

        ARINC_Word_300.str.cam3_fault = (ARINC_Get_Camera_Power(2) < 2) ? true : false;
        if (ARINC_Word_300.str.cam3_fault)
        {
            Non_Critical_Fault = true;
            cams_N_faults++;
        }

        ARINC_Word_300.str.cam4_fault = (ARINC_Get_Camera_Power(3) < 2) ? true : false;
        if (ARINC_Word_300.str.cam4_fault)
        {
            Non_Critical_Fault = true;
            cams_N_faults++;
        }

        ARINC_Word_300.str.cam5_fault = (ARINC_Get_Camera_Power(4) < 2) ? true : false;
        if (ARINC_Word_300.str.cam5_fault)
        {
            Non_Critical_Fault = true;
            cams_N_faults++;
        }

        ARINC_Word_300.str.cam6_fault = (ARINC_Get_Camera_Power(5) < 2) ? true : false;
        if (ARINC_Word_300.str.cam6_fault)
        {
            Non_Critical_Fault = true;
            cams_N_faults++;
        }

        // Статусы системы
        ARINC_Word_300.str.SrvRouter_Status = ARINC_System_Status.srvrouter_stat;
        ARINC_Word_300.str.Storage_Status   = ARINC_System_Status.storage_stat;

        if (ARINC_System_Status.srvrouter_stat == 1 || ARINC_Word_300.str.Storage_Status == 1 ||
            cams_N_faults >= ARINC_CHANNELS_TOTAL)
        {
            Critical_Fault = true;
        }

        ARINC_Word_300.str.XAE21_fault = ARINC_System_Status.XAE21_fault;
        if (ARINC_Word_300.str.XAE21_fault)
        {
            Non_Critical_Fault = true;
        }

        ARINC_Word_300.str.cam_switch_fault = ARINC_System_Status.cam_switch_fault;
        if (ARINC_Word_300.str.cam_switch_fault)
        {
            Non_Critical_Fault = true;
        }

        // Определение общего состояния системы
        if (Critical_Fault)
            ARINC_Word_300.str.System_Status = ARINC_SYS_FAULT;
        else if (Non_Critical_Fault)
            ARINC_Word_300.str.System_Status = ARINC_SYS_PARTIALLY_FAULT;
        else
            ARINC_Word_300.str.System_Status = ARINC_SYS_NC;

        ARINC_Word_300.str.matrix = ARINC_A300_MATRIX_NORMAL;  // TODO: получить реальное значение
        ARINC_Word_300.str.nc     = 0x00;

        // TODO: HI3220_transmitDirect(&hi, 0, ARINC_Word_300.arinc_array, 1);
    }

    // Обработка передачи UART3
    // TODO: реализовать передачу через UART если нужно
}

void ARINC_ParseMessageCh1(char *data)
{
    static ARINC_W150_Union W150;
    static ARINC_W125_Union W125;
    static ARINC_W260_Union W260;

    uint32_t A_Word = 0;

    // Восстанавливаем 32-битное ARINC-слово из четырехбайтного массива
    for (int j = 0; j < 4; j++)
    {
        A_Word <<= 8;
        A_Word |= data[3 - j];
    }

    switch (data[0])
    {  // Label

        case 0150:  // время в ДК (двоичный код)
            W150.word429 = A_Word;

            ARINC_DateTime.sec    = W150.str.second;
            ARINC_DateTime.minute = W150.str.minute;
            ARINC_DateTime.hour   = W150.str.hour;

            ARINC_Timer_Set(&ARINC_Timer_RecvTimeout_XAE21, ARINC_XAE21_TIMEOUT);
            ARINC_System_Status.XAE21_fault = false;
            break;

        case 0125:  // время в ДДК (двоично-десятичный код)
            W125.word429 = A_Word;
            ARINC_Timer_Set(&ARINC_Timer_RecvTimeout_XAE21, ARINC_XAE21_TIMEOUT);
            ARINC_System_Status.XAE21_fault = false;
            break;

        case 0260:  // дата в ДДК (двоично-десятичный код)
            W260.word429 = A_Word;

            ARINC_DateTime.year  = W260.str.yearT * 10 + W260.str.yearU;
            ARINC_DateTime.month = W260.str.monthT * 10 + W260.str.monthU;
            ARINC_DateTime.day   = W260.str.dayT * 10 + W260.str.dayU;

            ARINC_Timer_Set(&ARINC_Timer_RecvTimeout_XAE21, ARINC_XAE21_TIMEOUT);
            ARINC_System_Status.XAE21_fault = false;
            break;

        default:
            break;
    }
}

void ARINC_ParseMessageCh2(char *data)
{
    // По второму каналу пока данные не обрабатываются
    // TODO: реализовать при необходимости
}

void ARINC_Proto_InputPacket(char *packet, char size, char *out, char *outSize)
{
    *outSize = 0;

    if (size < 1)
    {
        return;
    }

    // Обработка пакетов от Сервера Маршрутизатора
    if (packet[0] == 0x55)
    {
        if (size == sizeof(ARINC_SetStatus) && packet[1] == 0x33)
        {
            ARINC_SetStatus setStatusPacket = { 0 };
            memcpy(&setStatusPacket, packet, size);
            ARINC_System_Status.srvrouter_stat = !setStatusPacket.Status.Router;
            ARINC_System_Status.storage_stat   = !setStatusPacket.Status.Archive;
            return;
        }
        else if (size == sizeof(ARINC_GetStatusQuery) && packet[1] == 0x11)
        {
            ARINC_GetStatusQuery    getStatusQuery    = { 0 };
            ARINC_GetStatusResponse getStatusResponse = { 0 };
            memcpy(&getStatusQuery, packet, size);

            getStatusResponse.flag   = 0x55;
            getStatusResponse.code   = 0x22;
            getStatusResponse.camera = getStatusQuery.Camera;

            if (getStatusQuery.Camera >= 6)
            {
                getStatusResponse.status = 0;
            }
            else
            {
                getStatusResponse.status = (ARINC_Get_Camera_Current(getStatusQuery.Camera) < ARINC_CAMERA_CURRENT_THRESHOLD ? 0 : 1);
            }

            memcpy(out, (unsigned char *)&getStatusResponse, sizeof(ARINC_GetStatusResponse));
            *outSize = sizeof(ARINC_GetStatusResponse);
            return;
        }
    }

    if (size == 3)
    {
        if (!ARINC_CRC8_Calc((uint8_t *)packet, 3))
        {
            if (packet[0] == 0xCE)
            {
                ARINC_Linux_Status                 = *(ARINC_Linux_Status_Struct *)(&packet[1]);
                ARINC_System_Status.srvrouter_stat = ARINC_Linux_Status.srv_stat;
                ARINC_System_Status.storage_stat   = ARINC_Linux_Status.storage_stat;
            }
            else
            {
                if (packet[1] == 1)
                {
                    ARINC_Temp_Buffer[0] = 0xEB;
                    ARINC_Temp_Buffer[1] = 1;
                    ARINC_Proto_BuildUartPacketControl(&ARINC_Temp_Buffer[2]);
                    ARINC_Temp_Buffer[16] = ARINC_CRC8_Calc((uint8_t *)ARINC_Temp_Buffer, 16);

                    ARINC_Timer_Set(&ARINC_Timer_PcRecvTimeout_Ctrl, ARINC_SVR_TIMEOUT);
                    // TODO: добавить в кольцевой буфер для передачи
                }
                else if (packet[1] == 2)
                {
                    ARINC_Temp_Buffer[0] = 0xEB;
                    ARINC_Temp_Buffer[1] = 2;
                    ARINC_Proto_BuildUartPacketDT(&ARINC_Temp_Buffer[2]);
                    ARINC_Temp_Buffer[8] = ARINC_CRC8_Calc((uint8_t *)ARINC_Temp_Buffer, 8);

                    ARINC_Timer_Set(&ARINC_Timer_PcRecvTimeout_DT, 5000);
                    // TODO: добавить в кольцевой буфер для передачи
                }
            }
        }
    }
}

void ARINC_Proto_BuildUartPacketControl(char *packet)
{
    packet[0] = (ARINC_Control.FORMAT1 & 0x7F) << 1;
    packet[0] |= ARINC_Control.ACTIVE_CONF_MENU_CAPT & 1;

    packet[1] = (ARINC_Control.FORMAT1_POZITION & 0x07) << 5;
    packet[1] |= (ARINC_Control.FORMAT1_SIZE & 0x0F) << 1;
    packet[1] |= ARINC_Control.ACTIVE_CURSOR_CAPT & 1;

    packet[2] = ARINC_Channel_No;

    packet[3] = (ARINC_Control.CURSOR_X_CAPT & 0x07) << 5;
    packet[3] |= (ARINC_Control.CAMS_NUMBER_CAPT & 0x0F) << 1;
    packet[3] |= ARINC_Control.BTN_PRESSED_CAPT & 1;

    packet[4] = ARINC_Control.CURSOR_Y_CAPT >> 3;

    packet[5] = (ARINC_Control.CURSOR_Y_CAPT & 0x07) << 5;
    packet[5] |= (ARINC_Control.JOYSTICK_DOWN_CAPT & 1) << 4;
    packet[5] |= (ARINC_Control.JOYSTICK_LEFT_CAPT & 1) << 3;
    packet[5] |= (ARINC_Control.JOYSTICK_RIGHT_CAPT & 1) << 2;
    packet[5] |= (ARINC_Control.JOYSTICK_UP_CAPT & 1) << 1;
    packet[5] |= (ARINC_Control.LEFT_ENTER_CAPT & 1);

    packet[6] = (ARINC_Control.LENT_JAMMED_CAPT & 1) << 7;
    packet[6] |= (ARINC_Control.RIGHT_ENTER_CAPT & 1) << 6;
    packet[6] |= (ARINC_Control.RENT_JAMMED_CAPT & 1) << 5;
    packet[6] |= (ARINC_Control.ESC_BTN_CAPT & 1) << 4;
    packet[6] |= (ARINC_Control.ESC_JAMMED_CAPT & 1) << 3;
    packet[6] |= (ARINC_Control.OK_BTN_CAPT & 1) << 2;
    packet[6] |= (ARINC_Control.OK_JAMMED_CAPT & 1) << 1;
    packet[6] |= (ARINC_Control.LWR_AREA_BTN_CAPT & 1);

    packet[7] = (ARINC_Control.FORMAT2 & 0x7F) << 1;
    packet[7] |= (ARINC_Control.ACTIVE_CONF_MENU_FO & 1);

    packet[8] = (ARINC_Control.FORMAT2_POZITION & 0x07) << 5;
    packet[8] |= (ARINC_Control.FORMAT2_SIZE & 0x0F) << 1;
    packet[8] |= (ARINC_Control.ACTIVE_CURSOR_FO & 1);

    packet[9] = ARINC_Control.CURSOR_X_FO >> 3;

    packet[10] = (ARINC_Control.CURSOR_X_FO & 0x07) << 5;
    packet[10] |= (ARINC_Control.CAMS_NUMBER_FO & 0x0F) << 1;
    packet[10] |= ARINC_Control.BTN_PRESSED_FO & 1;

    packet[11] = ARINC_Control.CURSOR_Y_FO >> 3;

    packet[12] = (ARINC_Control.CURSOR_Y_FO & 0x07) << 5;
    packet[12] |= (ARINC_Control.JOYSTICK_DOWN_FO & 1) << 4;
    packet[12] |= (ARINC_Control.JOYSTICK_LEFT_FO & 1) << 3;
    packet[12] |= (ARINC_Control.JOYSTICK_RIGHT_FO & 1) << 2;
    packet[12] |= (ARINC_Control.JOYSTICK_UP_FO & 1) << 1;
    packet[12] |= (ARINC_Control.LEFT_ENTER_FO & 1);

    packet[13] = (ARINC_Control.LENT_JAMMED_FO & 1) << 7;
    packet[13] |= (ARINC_Control.RIGHT_ENTER_FO & 1) << 6;
    packet[13] |= (ARINC_Control.RENT_JAMMED_FO & 1) << 5;
    packet[13] |= (ARINC_Control.ESC_BTN_FO & 1) << 4;
    packet[13] |= (ARINC_Control.ESC_JAMMED_FO & 1) << 3;
    packet[13] |= (ARINC_Control.OK_BTN_FO & 1) << 2;
    packet[13] |= (ARINC_Control.OK_JAMMED_FO & 1) << 1;
    packet[13] |= (ARINC_Control.LWR_AREA_BTN_FO & 1);
}

void ARINC_Proto_BuildUartPacketDT(char *packet)
{
    packet[0] = ARINC_DateTime.hour & 0x1F;
    packet[1] = ARINC_DateTime.minute & 0x3F;
    packet[2] = ARINC_DateTime.sec & 0x3F;
    packet[3] = ARINC_DateTime.year;
    packet[4] = ARINC_DateTime.month & 0x1F;
    packet[5] = ARINC_DateTime.day & 0x1F;
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
    else
    {
        Final_State = State;
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

    // Инициализация фильтра (стр. 13–14). Включение приема сообщений со всеми метками на всех каналах
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

    return TOOLS_ERROR_CODE_ALL_OK;
}

void ARINC_Timer_Set(ARINC_Timer_Struct *timer, uint32_t timeout)
{
    timer->start_time = HAL_GetTick();
    timer->timeout    = timeout;
    timer->expired    = false;
}

bool ARINC_Timer_Expired(ARINC_Timer_Struct *timer)
{
    if (!timer->expired)
    {
        uint32_t current_time = HAL_GetTick();
        if ((current_time - timer->start_time) >= timer->timeout)
        {
            timer->expired = true;
        }
    }
    return timer->expired;
}

uint8_t ARINC_CRC8_Calc(uint8_t *data, uint8_t length)
{
    // TODO: реализовать CRC8 расчет
    return 0;
}

float ARINC_Get_Camera_Current(uint8_t camera_id)
{
    // TODO: заглушка - заменить на реальную функцию получения тока
    return (camera_id < ARINC_CHANNELS_TOTAL) ? 0.1f : 0.0f;
}

float ARINC_Get_Camera_Power(uint8_t camera_id)
{
    // TODO: заглушка - заменить на реальную функцию получения мощности
    return (camera_id < ARINC_CHANNELS_TOTAL) ? 3.0f : 0.0f;
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