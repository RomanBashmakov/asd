/*
 * state_logger_service.h
 *
 *  Created on: Nov 27, 2019
 *      Author: zhuchenkovao
 */

#ifndef INC_STATE_LOGGER_SERVICE_H_
#define INC_STATE_LOGGER_SERVICE_H_

// сервис статуса и логирования
// не масштабируемый модуль

//#include "devices.h"

#include "timers_hw2.h"


#define  LOG_RECORD_SIZE                  16    // размер записи лога 16 байт, для кратности размера блока флэш памяти 4096 байт



// устройства:
// stm32 (LMT87, battery)
// RTC DS1340ZZ
// flash W25Q32
// Ethernet Interface
// tablo1 (service)
// tablo2 (service)
// rfid1 (service)
// rfid2 (service)
// UPS
// Logic Inputs
// CAS (весы) (service)
// traffic light (service)
// user (telnet)


// нумерация устройств сделана через enum


enum DevNums {
  LDEV_STM32 = 0,
  LDEV_VBAT,
  LDEV_LMT87,
  LDEV_RTC_DS1340,
  LDEV_W25Q32,
  LDEV_IFACE,
  LDEV_USER,
  LDEV_FM25L16B,
  LDEV_CONFIG,
  LDEV_REMOTE_FLASHER,
  LDEV_TABLO1,
  LDEV_TABLO2,
  LDEV_RFID1,
  LDEV_RFID2,
  LDEV_RFID3,
  LDEV_RFID4,
  LDEV_CAS,
  LDEV_LOGICINPUT,
  LDEV_TRAFFICLIGHT1,
  LDEV_TRAFFICLIGHT2,
  LDEV_STREETLAMP1,
  LDEV_STREETLAMP2,
  LDEV_UPS,
  LDEV_GSM_MODEM,
  LDEV_RESERV8,     // резервные номера устройств, в случае добавления новых устройств, использовать их
  LDEV_RESERV9,
  LDEV_RESERV10,
  LDEV_RESERV11,
  LDEV_RESERV12,
  LDEV_RESERV13,
  LDEV_RESERV14,
  LDEV_RESERV15,

  LDEV_COUNT


};



#define  CHECK_DEVICES_COUNT                32        //  = LDEV_COUNT

#define  STCFG_CONTROL_ENABLED              0x0001             // включен контроль состояния
#define  STCFG_TIMEGRP_ENABLED              0x0002             // включена группировка по времени


#define  STATE_EXTENDED_INFO                0x0400             // дополнительная информация по предыдущему сообщению

#define  STATE_CONTROL_DISABLED             0x0500             // контроль состояний и логирование для данного устройства выключен


#define  STATE_STM32_OK                     0x0000             //
#define  STATE_STM32_STARTUP                0x0001             //
#define  STATE_STM32_SHUTDOWN               0x0102             //
#define  STATE_STM32_BACKUPDATA_INVALID     0x0203             //
#define  STATE_STM32_LOG_FLASH_CLEARED      0x0104             //
#define  STATE_STM32_SHUTDOWN_TIME_INVALID  0x0205             //
#define  STATE_STM32_HARDFAULT_OCCURED      0x0206             //
#define  STATE_STM32_RDP_DISABLED           0x0107             //


#define  STATE_IFACE_LINKDOWN               0x0200             //
#define  STATE_IFACE_10M_HALFDUPLEX         0x0101             //
#define  STATE_IFACE_10M_FULLDUPLEX         0x0102             //
#define  STATE_IFACE_100M_HALFDUPLEX        0x0003             //
#define  STATE_IFACE_100M_FULLDUPLEX        0x0004             //

#define  STATE_RTC_VALID                    0x0000             //
#define  STATE_RTC_INVALID                  0x0201             //
#define  STATE_RTC_DISCONNECTED             0x0202             //


#define  STATE_FLASH_OK                     0x0000             //
#define  STATE_FLASH_IDINVALID              0x0201             //
#define  STATE_FLASH_TIMEOUT_ERROR          0x0202             //
#define  STATE_FLASH_DISCONNECTED           0x0203             //

#define  STATE_FERAM_OK                     0x0000             //
#define  STATE_FERAM_DISCONNECTED           0x0201             //


#define  STATE_LMT87_UNKNOWN                0x0200             // неизвестное состояние (больше +200 или меньне -100)
#define  STATE_LMT87_P80_P200               0x0201             // от +80 до +200
#define  STATE_LMT87_M20_P80                0x0002             // от -20 до +80
#define  STATE_LMT87_M40_M20                0x0103             // от -40 до -20
#define  STATE_LMT87_M100_M40               0x0204             // от -100 до -40


#define  STATE_VBAT_UNKNOWN                 0x0200             // неизвестное состояние (больше +3.6 или меньне 0)
#define  STATE_VBAT_P28_P36                 0x0001             // от +2.8 до +3.6
#define  STATE_VBAT_P20_P28                 0x0102             // от +2.0 до +2.8
#define  STATE_VBAT_P15_P20                 0x0203             // от +1.5 до +2.0
#define  STATE_VBAT_0_P15                   0x0204             // от 0 до +1.5


#define  STATE_TABLO_OK                     0x0000             // табло ок
#define  STATE_TABLO_DISCONNECTED           0x0201             // нет связи с табло


#define  STATE_TRAFLIGHT_OK                 0x0000             //
#define  STATE_TRAFLIGHT_ERROR_RED          0x0201             // красный канал не исправен
#define  STATE_TRAFLIGHT_ERROR_GREEN        0x0202             // зеленый канал не исправен
#define  STATE_TRAFLIGHT_ERROR_RG           0x0203             // оба канала не исправны



#define  STATE_STREETLAMP_OK                0x0000             //
#define  STATE_STREETLAMP_ERROR             0x0201             // нет тока



#define  STATE_CONFIG_OK                    0x0000             // конфигурация загружена успешно
#define  STATE_CONFIG_ERROR_CRC             0x0201             //
#define  STATE_CONFIG_ERROR_VERSION         0x0202             //



#define  TIME_EVENT_GROUP_CACHE_SIZE          32
#define  TIME_EVENT_GROUP_TIMEOUT             3600000          // 1 час


#define  LOG_TEMP_DATA_SIZE                   128
#define  LOG_READ_FLASH_BLOCK_SIZE            (LOG_TEMP_DATA_SIZE / 2)






#define   LOG_STRINGS_DEV_COUNT              24           // количество устройств (кроме "резервных")
#define   LOG_STRINGS_EVENT_COUNT            20           // количество событий для каждого устройства
#define   LOG_STRINGS_EVENT_TYPE_COUNT       3            // количество типов событий




enum   commands{
  SLSCMD_NO_COMMAND = 0,
  SLSCMD_GET_FLASH,
  SLSCMD_GET_LOG,
  SLSCMD_GET_STATE


};





// описание логирования

#pragma pack  (push, 1)

// описание события
typedef struct SEvent {

  uint8_t           deviceNum;      // номер устройства, к которому относится событие
  uint16_t          eventTypeNum;   // тип события  (инфо, предупреждение, ошибка) и номер события
  uint8_t           repeatCount;    // количество повторений этого события за последний час (не более 255)

} TEvent;


// группировка события по времени (1 час)
/*typedef struct SEventGroup {
  TEvent            ev;
} TEventGroup;
*/

// описание записи лога во флэш-памяти
typedef struct SLogRecord {

  uint32_t          recordNum;      // номер записи (с каждой записью увеличивается на 1)
  uint8_t           recordDT[6];    // дата и время записи
  TEvent            ev;             // описание события
  uint8_t           reserv;         // резервный байт, чтобы длинна записи была равна 16 байт, может быть равен нулю
  uint8_t           recordCRC;      // контрольная сумма записи (CRC8 bit)

} TLogRecord;



#pragma  pack  (pop)






// контроль состояний
// описание состояний
/*typedef struct SDeviceDescription{


} TDeviceDescription;
// */

typedef struct SEventTimeGrp{

  TTimer                  tmrGrpEventTimeout;

  TEvent                  event;

} TEventTimeGrp;




typedef struct SLoggerService {

  char                  lastValidRecordFound;      // используется в поиске
  uint32_t              lastValidRecordNum;        // сначала используется в поиске, потом будет хранить номер текущей записи
  uint32_t              lastValidRecordAddr;       // используется в поиске


  uint32_t              currentRecordAddr;         // текущий адрес для новой записи, указывает на свободное место, после последней записаной записи


  uint8_t               command;              // комманда и параметры (комманды принимаются по TCP через commonTCPServer)
  uint32_t              param1;
  uint32_t              param2;
  uint8_t               connectionNum;        // номер соединения (для отправки данных в commonTCPServer)
  char                  tempDataUpdated;      //
  char                  sheduleSocketClose;        // признак завершения отправки и закрытия сокета (используется в LOG_process)


  uint16_t              stateConfig[CHECK_DEVICES_COUNT];
  uint16_t              currentState[CHECK_DEVICES_COUNT];
  uint16_t              newState[CHECK_DEVICES_COUNT];


  TEventTimeGrp         eventTimeGrpCache[TIME_EVENT_GROUP_CACHE_SIZE];

} TLoggerService;


extern const char*  logStringsDevs[LOG_STRINGS_DEV_COUNT];

extern TLoggerService   logService;



void LOG_init (void);
void LOG_process (void);
char LOG_checkFlashEmpty (uint32_t startAddrToFind, uint32_t endAddrToFind);
void LOG_incRecordAddr (uint32_t *addr);   // для правильного увеличения номера записи с учетом перехода через границы памяти
void LOG_decRecordAddr (uint32_t *addr);   // для правильного уменьшения номера записи с учетом перехода через границы памяти
void LOG_writeRecordToFlash(TEvent  *event);
void LOG_writeRecord(TEvent  *event);
char LOG_readRecord(uint32_t addr,  TLogRecord *pLogRecord);
char LOG_checkEmptyRecord(TLogRecord *pLogRecord);
int LOG_printRecord(TLogRecord *pLogRecord, char *data, int dataSize, TLogRecord *pAdditionalInfo);
int LOG_printTimeGrpItem(int num, char *data, int dataSize);
void LOG_fillStrings1to4 (TEvent *ev);

void LOG_STATE_resetState(uint16_t state[LDEV_RESERV10 + 5]);
void LOG_STATE_checkChangeStateAndUpdate(void);
void LOG_STATE_generateNewState(void);


#endif /* INC_STATE_LOGGER_SERVICE_H_ */
