/// @file    arinc429.c
/// @author  Тузиков Г.А. tuzikovga@raitibor.ru, zhuchenkovao, Башмаков Р.А.
///          bashmakovra@raitibor.ru
/// @brief   ВПО для STM32 СВР-Маршрутизатора
/// @details реализация функций инициализации, обработки и передачи данных по
///          протоколу ARINC 429
///          с использованием микросхемы HI3220,
///          парсинг сообщений,
///          управление таймерами,
///          взаимодействие с аппаратными пинами.

#include "arinc429.h"

#include <memory.h>
#include <stdio.h>

#include "cbuffer.h"
#include "crc8_icode.h"
#include "powercontrol.h"
#include "timers_hw2.h"
#include "usart.h"

//#include "DBG.h" //DBG

#define UART_OUTPUT_BUFFER_LENGTH 200

#define UART3_TX_BUFFER_SIZE 200

T_ARINC429Control arinc429Control;
T_ARINC429ControlDateTime arinc429DT;

union W300_t ARINC_Word_300;

uint8_t arincRecvBuffer[1000];

TCbuffer cbARINCUartOutput;
char arincTxBuffer[UART_OUTPUT_BUFFER_LENGTH];
char tempBuffer[50];
uint8_t uart3TxBuffer[UART3_TX_BUFFER_SIZE];

TTimer tmrRecvTimeoutXAE21;
TTimer tmrRecvTimeoutCh2;

TTimer tmrPcRecvTimeoutCtrl;
TTimer tmrPcRecvTimeoutDT;

TTimer tmrSendTimeoutCh1;


uint8_t txPacketARINC429_1[4] = {0300, 0x03, 0x00,
                                 0x60};  // предустановка отправляемых пакетов
uint8_t txPacketARINC429_2[4] = {0301, 0x00, 0x00, 0x00};
uint8_t txPacketARINC429_3[4] = {0302, 0x00, 0x00, 0x00};

/* // пока не используется
unsigned char descriptors[] = {

//   Opcode,    RXLabel, PTP,   PTO, -  Label,  Byte1,  Byte2,  Byte3
//    DLY              DLY Value

    IM,         0x00,   0x00,   0x00,   0x23,   0x01,   0x30,   0x40,
    IM,         0x00,   0x04,   0x00,   0x5A,   0x01,   0x03,   0x04,

};
*/

THI3220PinStruct hiPins;
THI3220 hi;

Linux_Status_t Linux_Status;

//==================================================================
void ARINC429_init(SPI_HandleTypeDef *hspi)
{
    // init HI3220
    hiPins.PORT_mrst = GPO_ar429_rstn_GPIO_Port;
    hiPins.PIN_mrst = GPO_ar429_rstn_Pin;

    hiPins.PORT_ready = GPI_ar429_ready_GPIO_Port;
    hiPins.PIN_ready = GPI_ar429_ready_Pin;

    hiPins.PORT_run = GPO_ar429_run_GPIO_Port;
    hiPins.PIN_run = GPO_ar429_run_Pin;

    hiPins.PORT_int = GPI_ar429_intn_GPIO_Port;
    hiPins.PIN_int = GPI_ar429_intn_Pin;

    hiPins.PORT_ack = GPO_ar429_ack_GPIO_Port;
    hiPins.PIN_ack = GPO_ar429_ack_Pin;

    hiPins.PORT_cs = GPO_ar429_csn_GPIO_Port;
    hiPins.PIN_cs = GPO_ar429_csn_Pin;

    // инвертировано для оптопары
    HAL_GPIO_WritePin(GPO_ar429_rstn_GPIO_Port, GPO_ar429_rstn_Pin,
                      GPIO_PIN_SET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(GPO_ar429_rstn_GPIO_Port, GPO_ar429_rstn_Pin,
                      GPIO_PIN_RESET);
    HI3220_init(&hi, &hiPins, hspi);

    // начало теста внутренней пямяти:
    // чтение и вывод текущего режима
    uint8_t retValue = HI3220_readByteAtAddr(&hi, BISTFH);
    printf("BISTFH: %02Xh ------\n", retValue);

    // программная перезагрузка в режиме 6
    HI3220_softwareReset(&hi, SW_RESET_MODE6);

    // чтение и вывод текущего режима
    retValue = HI3220_readByteAtAddr(&hi, BISTFH);
    printf("BISTFH: %02Xh ------\n", retValue);

    // чтение и вывод регистра BISTS
    retValue = HI3220_readByteAtAddr(&hi, BISTS);
    printf("BISTS: %02Xh ------\n", retValue);

    // тест внутренней пямяти:
    // выключение run
    HAL_GPIO_WritePin(hi.pGPIO->PORT_run, hi.pGPIO->PIN_run, GPIO_PIN_RESET);

    // выбор режима теста 1 и запуск теста
    HI3220_writeReg(&hi, BISTSREG, 0x18);

    // ожидание завершения теста
    uint32_t tickstart = HAL_GetTick();
    // while ( HAL_GPIO_ReadPin(hi.pGPIO->PORT_ready, hi.pGPIO->PIN_ready) ==
    // GPIO_PIN_RESET ) { // инвертировано для оптопары
    while (HAL_GPIO_ReadPin(hi.pGPIO->PORT_ready, hi.pGPIO->PIN_ready) ==
           GPIO_PIN_SET)
    {
        printf("test\n");  //
        if ((HAL_GetTick() - tickstart) >= 20)
        {
            printf("error HI-3220\n");
        }
    }

    // чтение и вывод регистра BISTS
    retValue = HI3220_readByteAtAddr(&hi, BISTS);
    printf("BISTS: %02Xh ------\n", retValue);

    // программная перезагрузка в режиме 0
    HI3220_softwareReset(&hi, SW_RESET_MODE0);

    // чтение и вывод текущего режима
    retValue = HI3220_readByteAtAddr(&hi, BISTFH);
    printf("BISTFH: %02Xh ------\n", retValue);

    // конец теста внутренней пямяти

    retValue = HI3220_readReg(&hi, _MCRW);
    printf("MCR: %02Xh ------\n", retValue);

    // конфигурация главного управляющего регистра
    HI3220_writeMCR(&hi, retValue | A429RX | A429TX);

    retValue = HI3220_readReg(&hi, _MCRW);  //
    printf("MCR: %02Xh ------\n", retValue);

    // конфигурация передачи:
    // конфикурация управляющего регистра канала 0
    HI3220_writeReg(&hi, _ATXC0, SP_LO | PARITYEN);

    // конфикурация управляющего регистра канала 1
    HI3220_writeReg(&hi, _ATXC1, SP_LO | PARITYEN);

    // конфигурация приема:
    // конфикурация управляющего регистра канала 0
    HI3220_writeReg(&hi, _ARXC0, ENABLE | SP_LO | PARITYEN);

    retValue = HI3220_readReg(&hi, ARXC0_R);  // отладочная проверка
    printf("RCR0: %02Xh ------\n", retValue);

    // конфикурация управляющего регистра канала 1
    HI3220_writeReg(&hi, _ARXC1, ENABLE | SP_LO | PARITYEN);

    retValue = HI3220_readReg(&hi, ARXC1_R);  // отладочная проверка
    printf("RCR1: %02Xh ------\n", retValue);

    // включение выдачи сигнала INT на пин, при приеме нового сообщения в память
    // МС
    HI3220_writeByteAtAddr(&hi, PIER, 0xF8 | RXRAMNEW);

    retValue = HI3220_readByteAtAddr(&hi, PIER);  // отладочная проверка
    printf("PIER: %02Xh ------\n", retValue);  //

    // включение обработки прерываний на приемнике 0,1
    HI3220_writeByteAtAddr(&hi, RIERL, 3);

    retValue = HI3220_readByteAtAddr(&hi, RIERL);  // отладочная проверка
    printf("RIERL: %02Xh ------\n", retValue);  //

    // инициализация фильтра
    // включение приема сообщений со всеми метками на всех каналах
    for (int i = RXEN_MAP; i < RXINT_MAP; i++)
    {
        HI3220_writeByteAtAddr(&hi, i, 0xFF);
    }

    // инициализация таблицы прерываний по меткам
    // включение прерываний по приходу сообщений со всеми метками на всех
    // каналах
    for (int i = RXINT_MAP; i < RXINT_ENDMAP; i++)
    {
        HI3220_writeByteAtAddr(&hi, i, 0xFF);
    }

    // отправка через планировщик:
    // инициализация не нулевым значением регистра повторения
    // HI3220_writeByteAtAddr(&hi, RRR0, 25);                 // rate is passed

    // загрузка таблицы дескрипторов (сообщения и их параметры для планировщика)
    // ARINC_LoadDescriptors(&hi, ATXC0TABLE, 0, descriptors, 2);
    // включение бита RUN в регистре управлением отправкой
    // HI3220_writeByteAtAddr(&hi, ATXC0, RUN | SP_HI);

    CBUFF_init(&cbARINCUartOutput, arincTxBuffer, UART_OUTPUT_BUFFER_LENGTH);

    TIMERS_setTimer(&tmrRecvTimeoutXAE21, XAE21_TIMEOUT);
    TIMERS_setTimer(&tmrRecvTimeoutCh2, 5000);

    TIMERS_setTimer(&tmrPcRecvTimeoutCtrl, SVR_TIMEOUT);
    TIMERS_setTimer(&tmrPcRecvTimeoutDT, SVR_TIMEOUT);

    TIMERS_setTimer(&tmrSendTimeoutCh1,
                    A429_L0300_SEND_PERIOD);  // отправка по первому каналу
                                              // ARINC в интервале 1с
}

//==================================================================
void ARINC429_process()
{
    extern Status_t SYSTEM_Status;

    extern enum A300_Matrix_e A300_Matrix;
    //	extern u32_t npkts;//DBG

    bool Critical_Fault = false;
    bool Non_Critical_Fault = false;
    u8_t cams_N_faults = 0;

    //	u8_t hi_MCR;
    //	u8_t hi_MSR;
    //	u8_t hi_RCV0;
    //	u8_t hi_PIR4;
    //	u8_t hi_PIRA;

    if (TIMERS_expired(tmrRecvTimeoutXAE21))
    {
        // memset( &arinc429Control, 0, sizeof(T_ARINC429Control));
        //    memset( &arinc429DT, 0, sizeof(T_ARINC429ControlDateTime));  //
        //    для ИЛ-114
        SYSTEM_Status.XAE21_fault = true;
    }

    if (TIMERS_expired(tmrRecvTimeoutCh2))
    {
        //
    }

    if (TIMERS_expired(tmrPcRecvTimeoutCtrl))
    {
    }

    if (TIMERS_expired(tmrPcRecvTimeoutDT))
    {
    }

    uint8_t t = HAL_GPIO_ReadPin(
        hi.pGPIO->PORT_int,
        hi.pGPIO->PIN_int);  // опрос вывода прерывания HI-3220 (by ZHuchenkov)

    if (t == 0)
    {
        // Send ACK (right here, immediate after INT - LV)
        HAL_GPIO_WritePin(hi.pGPIO->PORT_ack, hi.pGPIO->PIN_ack,
                          GPIO_PIN_RESET);
        __ddelayT(6);
        HAL_GPIO_WritePin(hi.pGPIO->PORT_ack, hi.pGPIO->PIN_ack, GPIO_PIN_SET);
        // Send ACK (right here, immediate after INT - LV)

        uint8_t reg = HI3220_readReg(&hi, _RPIRL);  // fast read RPIRL reg
        // printf ("ar1 %02X\n", reg ); //

        if ((reg & 0x01) == 0x01)
        {
            // read count fifo ch0
            HI3220_writeMAP(
                &hi,
                FCV0);  // по адресу FCV0 (0x8068) лежит RX FIFO COUNT 0, для
                        // универсальности тут лучше сделать (FCV0 + ch)
            uint8_t count = HI3220_readAtMAP(&hi);
            // printf ("ar1 %d\n", count);

            if (count)
            {
                HI3220_readFIFO(&hi, 0 /* FIFO chan */, arincRecvBuffer, count);

                char *p = (char *)arincRecvBuffer;
                for (int i = 0; i < count; i++)
                {
                    ARINC429_parseMessageCh1(p);

                    p += 4;
                }
            }
        }

        if ((reg & 0x02) == 0x02)
        {
            // read count fifo ch1

            HI3220_writeMAP(&hi, FCV1);
            uint8_t count = HI3220_readAtMAP(&hi);
            // printf ("ar2  %d\n", count);

            if (count)
            {
                HI3220_readFIFO(&hi, 1, arincRecvBuffer, count);

                char *p = (char *)arincRecvBuffer;
                for (int i = 0; i < count; i++)
                {
                    ARINC429_parseMessageCh2(p);

                    p += 4;
                }
            }
        }
    }

    // Формирование и отправка ARINC с лейблом 0300
    if (TIMERS_expired(tmrSendTimeoutCh1))
    {
        TIMERS_setTimer(&tmrSendTimeoutCh1,
                        A429_L0300_SEND_PERIOD);  // отправка по первому каналу
                                                  // ARINC в интервале 1с

        // передача сообщений ARINC-429
        //    HI3220_transmitDirect (&hi, 0, txPacketARINC429_1, 1);
        ARINC_Word_300.str.label = 0300;
        ARINC_Word_300.str.sdi = 0x03;
        ARINC_Word_300.str.nc = 0x00;
        // проверка потребляемой мощности по камерам и формирование состояний
        // пакета ARINC-429 если потребляемая мощность меньше 2 ватт, то
        // устанавливаем 1
        ARINC_Word_300.str.cam1_fault =
            (T_INA226_getPower(&powerSensA[0]) < 2) ? true : false;
        if (ARINC_Word_300.str.cam1_fault)
        {
            Non_Critical_Fault = true;
            cams_N_faults++;
        }
        ARINC_Word_300.str.cam2_fault =
            (T_INA226_getPower(&powerSensA[1]) < 2) ? true : false;
        if (ARINC_Word_300.str.cam2_fault)
        {
            Non_Critical_Fault = true;
            cams_N_faults++;
        }
        ARINC_Word_300.str.cam3_fault =
            (T_INA226_getPower(&powerSensA[2]) < 2) ? true : false;
        if (ARINC_Word_300.str.cam3_fault)
        {
            Non_Critical_Fault = true;
            cams_N_faults++;
        }
        ARINC_Word_300.str.cam4_fault =
            (T_INA226_getPower(&powerSensA[3]) < 2) ? true : false;
        if (ARINC_Word_300.str.cam4_fault)
        {
            Non_Critical_Fault = true;
            cams_N_faults++;
        }
        ARINC_Word_300.str.cam5_fault =
            (T_INA226_getPower(&powerSensA[4]) < 2) ? true : false;
        if (ARINC_Word_300.str.cam5_fault)
        {
            Non_Critical_Fault = true;
            cams_N_faults++;
        }
        ARINC_Word_300.str.cam6_fault =
            (T_INA226_getPower(&powerSensA[5]) < 2) ? true : false;
        if (ARINC_Word_300.str.cam6_fault)
        {
            Non_Critical_Fault = true;
            cams_N_faults++;
        }

        // Проверка статусов оборудования
        ARINC_Word_300.str.SrvRouter_Status = SYSTEM_Status.srvrouter_stat;
        ARINC_Word_300.str.Storage_Status = SYSTEM_Status.storage_stat;

        if (SYSTEM_Status.srvrouter_stat == 1 ||
            ARINC_Word_300.str.Storage_Status == 1 ||
            cams_N_faults >= CHANNELS_TOTAL)
        {
            Critical_Fault = true;
        }

        ARINC_Word_300.str.XAE21_fault = SYSTEM_Status.XAE21_fault;
        if (ARINC_Word_300.str.XAE21_fault)
        {
            Non_Critical_Fault = true;
        }
        ARINC_Word_300.str.cam_switch_fault = SYSTEM_Status.cam_switch_fault;
        if (ARINC_Word_300.str.cam_switch_fault)
        {
            Non_Critical_Fault = true;
        }

        // определение состояния ВСЕЙ СИСТЕМЫ
        if (Critical_Fault)
            ARINC_Word_300.str.System_Status = SYS_FAULT;
        else if (Non_Critical_Fault)
            ARINC_Word_300.str.System_Status = SYS_PARTIALLY_FAULT;
        else
            ARINC_Word_300.str.System_Status = SYS_NC;

        ARINC_Word_300.str.matrix = A300_Matrix;
        ARINC_Word_300.str.nc = 0x00;

        HI3220_transmitDirect(&hi, 0, ARINC_Word_300.arinc_array, 1);
    }

    // process send to uart3
    if (huart3.gState == HAL_UART_STATE_READY)
    {
        int count = cbARINCUartOutput.count;
        if (count > UART3_TX_BUFFER_SIZE) count = UART3_TX_BUFFER_SIZE;
        CBUFF_getFirstNBytes(&cbARINCUartOutput, (char *)uart3TxBuffer, count);
        // transmit uart 3 DMA
        HAL_UART_Transmit_DMA(&huart3, uart3TxBuffer, count);
    }
}

//==================================================================
void ARINC429_parseMessageCh1(char *data)
{
    //	extern u32_t EventPeriod;
    extern TTimer tmrDebug;
    extern Status_t SYSTEM_Status;

    static union W150_t W150;
    static union W125_t W125;
    static union W260_t W260;

    u32_t A_Word = 0;

    // Восстанавливаем 32-битное ARINC-слово из четырехбайтного массива
    for (int j = 0; j < 4; j++)
    {
        //    printf (" %02X", data[j]);
        A_Word <<= 8;
        A_Word |= data[3 - j];  //
    }
    //    printf ("\n");

    switch (data[0])
    {  // Label

        case 0150:  // (ХАЭ в ИЛ-114 время)
                    // время в ДК (двоичный код)

            //  	  EventPeriod = TIMERS_ShowTime(&tmrDebug);
            TIMERS_setTimer(&tmrDebug, 30000);

            W150.word429 = A_Word;

            arinc429DT.sec = W150.str.second;
            arinc429DT.minute = W150.str.minute;
            arinc429DT.hour = W150.str.hour;

            TIMERS_setTimer(&tmrRecvTimeoutXAE21, XAE21_TIMEOUT);
            SYSTEM_Status.XAE21_fault = false;

            break;

        case 0125:  // (ХАЭ в ИЛ-114 дата)
            // дата в ДДК (двоично-десятичный код)

            W125.word429 = A_Word;
            TIMERS_setTimer(&tmrRecvTimeoutXAE21, XAE21_TIMEOUT);
            SYSTEM_Status.XAE21_fault = false;

            break;

        case 0260:  // (ХАЭ в ИЛ-114 дата)
            // дата в ДДК (двоично-десятичный код)

            W260.word429 = A_Word;

            arinc429DT.year = W260.str.yearT * 10 + W260.str.yearU;
            arinc429DT.month = W260.str.monthT * 10 + W260.str.monthU;
            arinc429DT.day = W260.str.dayT * 10 + W260.str.dayU;

            TIMERS_setTimer(&tmrRecvTimeoutXAE21, XAE21_TIMEOUT);
            SYSTEM_Status.XAE21_fault = false;

            break;

        default:
            break;
    }
}

////==================================================================
// void ARINC429_parseMessageCh1_ZH(char *data)
//{
//
///*  for (int j = 0; j < 4; j++){
//    printf (" %02X", data[j]);
//  }
//  printf ("\n");*/
//
//
//  unsigned char t10;  // переменная для конвертации из ДДК в ДК
//
//  switch (data[0]){
//    case 0150:   // (ХАЭ в ИЛ-114 время)
//      // время в ДК (двоичный код)
//      arinc429DT.sec = (data[1] >> 3) & 0x1F;
//      arinc429DT.sec |= (data[2] & 1) << 5;
//      arinc429DT.minute = (data[2] >> 1) & 0x3F;
//      arinc429DT.hour = (data[2] >> 7) & 0x01;
//      arinc429DT.hour |= (data[3] & 0xF) << 1;
//
//      TIMERS_setTimer(&tmrRecvTimeoutXAE21, 5000);
//
//      break;
//
//    case 0260:  // (ХАЭ в ИЛ-114 дата)
//      // дата в ДДК (двоично-десятичный код)
//      arinc429DT.year = (data[1] >> 2) & 0xF;
//      t10 = (data[1] >> 6) & 2;
//      t10 += (data[2] & 3) << 2;
//      arinc429DT.year += t10 * 10;
//
//      arinc429DT.month = (data[2] >> 2) & 0x1F;
//      t10 = (data[2] >> 6) & 1;
//      arinc429DT.month += t10 * 10;
//
//      arinc429DT.day = (data[2] >> 7) & 1;
//      arinc429DT.day |= (data[3] & 7) << 1;
//      t10 = (data[3] >> 3) & 3;
//      arinc429DT.day += t10 * 10;
//
//
//      TIMERS_setTimer(&tmrRecvTimeoutXAE21, 5000);
//
//      break;
//
//  }
//}
//

//==================================================================
void ARINC429_parseMessageCh2(char *data)
{
    /*for (int j = 0; j < 4; j++){
      printf (" %02X", data[j]);
    }
    printf ("\n");*/

    // по второму каналу пока что данные не обрабатываются
}

//==================================================================
void ARINC429_Proto_InputPacket(char *packet, char size, char *out,
                                char *outSize)
{
    extern Status_t SYSTEM_Status;
    //	static u32_t cnt = 0;//DBG

    *outSize = 0;

    if (size < 1)
    {
        return;
    }

    // Обработка пакетов от Сервера Маршрутизатора
    if (packet[0] == 0x55)
    {
        if (size == sizeof(SetStatus) && packet[1] == 0x33)
        {
            SetStatus setStatusPacket = {0};
            memcpy(&setStatusPacket, packet, size);
            SYSTEM_Status.srvrouter_stat = !setStatusPacket.status.router;
            SYSTEM_Status.storage_stat = !setStatusPacket.status.archive;

            return;
        }
        else if (size == sizeof(GetStatusQuery) && packet[1] == 0x11)
        {
            GetStatusQuery getStatusQuery = {0};
            GetStatusResponse getStatusResponse = {0};
            memcpy(&getStatusQuery, packet, size);

            getStatusResponse.flag = 0x55;
            getStatusResponse.code = 0x22;
            getStatusResponse.camera = getStatusQuery.camera;

            if (getStatusQuery.camera >= 6)
            {
                getStatusResponse.status = 0;
            }
            else
            {
                getStatusResponse.status =
                    (T_INA226_getCurrent(&powerSensA[getStatusQuery.camera]) <
                             CAMERA_CURRENT_THRESHOLD
                         ? 0
                         : 1);
            }

            memcpy(out, (unsigned char *)&getStatusResponse,
                   sizeof(GetStatusResponse));
            *outSize = sizeof(GetStatusResponse);

            return;
        }
    }

    if (size == 3)
    {
        //	if (crc8CalcICode((uint8_t*)packet, 2) == packet[2]){
        if (!crc8CalcICode((uint8_t *)packet, 3))
        {
            if (packet[0] == 0xCE)
            {
                Linux_Status = *(Linux_Status_t *)(&packet[1]);
                SYSTEM_Status.srvrouter_stat = Linux_Status.srv_stat;
                SYSTEM_Status.storage_stat = Linux_Status.storage_stat;
                //			cnt++;//DBG
            }
            else
            {
                if (packet[1] == 1)
                {
                    tempBuffer[0] = 0xEB;
                    tempBuffer[1] = 1;
                    ARINC429_Proto_buildUartPacketControl(&tempBuffer[2]);
                    tempBuffer[16] = crc8CalcICode((uint8_t *)tempBuffer, 16);

                    //        pcCtrlConnected = 1;
                    //			SYSTEM_Status.srvrouter_stat = SYS_NORMAL_OPERATION;
                    ///* когда сервер будет присылать свой реальный статус,
                    //поставить именно его и там, где он будет приниматься
                    //(здесь временно - только по факту приема пакета от него)
                    //*/
                    // статус сервера приходит еще и через Linux_Status, решить
                    // что важнее по логике
                    TIMERS_setTimer(&tmrPcRecvTimeoutCtrl, SVR_TIMEOUT);

                    CBUFF_pushNBytes(&cbARINCUartOutput, tempBuffer, 17);
                }
                else if (packet[1] == 2)
                {
                    tempBuffer[0] = 0xEB;
                    tempBuffer[1] = 2;
                    ARINC429_Proto_buildUartPacketDT(&tempBuffer[2]);
                    tempBuffer[8] = crc8CalcICode((uint8_t *)tempBuffer, 8);

                    //        pcDTConnected = 1;
                    //			SYSTEM_Status.srvrouter_stat = SYS_NORMAL_OPERATION;
                    ///* когда сервер будет присылать свой реальный статус,
                    //поставить именно его и там, где он будет приниматься
                    //(здесь временно - только по факту приема пакета от него)
                    //*/
                    // статус сервера приходит еще и через Linux_Status, решить
                    // что важнее по логике
                    TIMERS_setTimer(&tmrPcRecvTimeoutDT, 5000);

                    CBUFF_pushNBytes(&cbARINCUartOutput, tempBuffer, 9);
                }
            }
        }
    }
    else
    {
    }
}

//==================================================================
void ARINC429_Proto_buildUartPacketControl(char *packet)
{
    packet[0] = (arinc429Control.FORMAT1 & 0x7F) << 1;
    packet[0] |= arinc429Control.ACTIVE_CONF_MENU_CAPT & 1;

    packet[1] = (arinc429Control.FORMAT1_POZITION & 0x07) << 5;
    packet[1] |= (arinc429Control.FORMAT1_SIZE & 0x0F) << 1;
    packet[1] |= arinc429Control.ACTIVE_CURSOR_CAPT & 1;

    //  packet[2] = arinc429Control.CURSOR_X_CAPT >> 3;
    packet[2] = GetChannelNo();

    packet[3] = (arinc429Control.CURSOR_X_CAPT & 0x07) << 5;
    packet[3] |= (arinc429Control.CAMS_NUMBER_CAPT & 0x0F) << 1;
    packet[3] |= arinc429Control.BTN_PRESSED_CAPT & 1;

    packet[4] = arinc429Control.CURSOR_Y_CAPT >> 3;

    packet[5] = (arinc429Control.CURSOR_Y_CAPT & 0x07) << 5;
    packet[5] |= (arinc429Control.JOYSTICK_DOWN_CAPT & 1) << 4;
    packet[5] |= (arinc429Control.JOYSTICK_LEFT_CAPT & 1) << 3;
    packet[5] |= (arinc429Control.JOYSTICK_RIGHT_CAPT & 1) << 2;
    packet[5] |= (arinc429Control.JOYSTICK_UP_CAPT & 1) << 1;
    packet[5] |= (arinc429Control.LEFT_ENTER_CAPT & 1);

    packet[6] = (arinc429Control.LENT_JAMMED_CAPT & 1) << 7;
    packet[6] |= (arinc429Control.RIGHT_ENTER_CAPT & 1) << 6;
    packet[6] |= (arinc429Control.RENT_JAMMED_CAPT & 1) << 5;
    packet[6] |= (arinc429Control.ESC_BTN_CAPT & 1) << 4;
    packet[6] |= (arinc429Control.ESC_JAMMED_CAPT & 1) << 3;
    packet[6] |= (arinc429Control.OK_BTN_CAPT & 1) << 2;
    packet[6] |= (arinc429Control.OK_JAMMED_CAPT & 1) << 1;
    packet[6] |= (arinc429Control.LWR_AREA_BTN_CAPT & 1);

    packet[7] = (arinc429Control.FORMAT2 & 0x7F) << 1;
    packet[7] |= (arinc429Control.ACTIVE_CONF_MENU_FO & 1);

    packet[8] = (arinc429Control.FORMAT2_POZITION & 0x07) << 5;
    packet[8] |= (arinc429Control.FORMAT2_SIZE & 0x0F) << 1;
    packet[8] |= (arinc429Control.ACTIVE_CURSOR_FO & 1);

    packet[9] = arinc429Control.CURSOR_X_FO >> 3;

    packet[10] = (arinc429Control.CURSOR_X_FO & 0x07) << 5;
    packet[10] |= (arinc429Control.CAMS_NUMBER_FO & 0x0F) << 1;
    packet[10] |= arinc429Control.BTN_PRESSED_FO & 1;

    packet[11] = arinc429Control.CURSOR_Y_FO >> 3;

    packet[12] = (arinc429Control.CURSOR_Y_FO & 0x07) << 5;
    packet[12] |= (arinc429Control.JOYSTICK_DOWN_FO & 1) << 4;
    packet[12] |= (arinc429Control.JOYSTICK_LEFT_FO & 1) << 3;
    packet[12] |= (arinc429Control.JOYSTICK_RIGHT_FO & 1) << 2;
    packet[12] |= (arinc429Control.JOYSTICK_UP_FO & 1) << 1;
    packet[12] |= (arinc429Control.LEFT_ENTER_FO & 1);

    packet[13] = (arinc429Control.LENT_JAMMED_FO & 1) << 7;
    packet[13] |= (arinc429Control.RIGHT_ENTER_FO & 1) << 6;
    packet[13] |= (arinc429Control.RENT_JAMMED_FO & 1) << 5;
    packet[13] |= (arinc429Control.ESC_BTN_FO & 1) << 4;
    packet[13] |= (arinc429Control.ESC_JAMMED_FO & 1) << 3;
    packet[13] |= (arinc429Control.OK_BTN_FO & 1) << 2;
    packet[13] |= (arinc429Control.OK_JAMMED_FO & 1) << 1;
    packet[13] |= (arinc429Control.LWR_AREA_BTN_FO & 1);
}

//==================================================================
void ARINC429_Proto_buildUartPacketDT(char *packet)
{
    packet[0] = arinc429DT.hour & 0x1F;
    packet[1] = arinc429DT.minute & 0x3F;
    packet[2] = arinc429DT.sec & 0x3F;
    packet[3] = arinc429DT.year;
    packet[4] = arinc429DT.month & 0x1F;
    packet[5] = arinc429DT.day & 0x1F;
}

//==================================================================
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_3)
    {
        if (!HAL_GPIO_ReadPin(GPI_ar429_intn_GPIO_Port, GPI_ar429_intn_Pin))
        {
            // Send ACK (right here, immediate after INT - LV)
            HAL_GPIO_WritePin(hi.pGPIO->PORT_ack, hi.pGPIO->PIN_ack,
                              GPIO_PIN_RESET);
            __ddelayT(6);
            HAL_GPIO_WritePin(hi.pGPIO->PORT_ack, hi.pGPIO->PIN_ack,
                              GPIO_PIN_SET);
            // Send ACK (right here, immediate after INT - LV)
        }
    }
}
//==================================================================
