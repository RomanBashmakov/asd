/*
 * config.h
 *
 *  Created on: Jan 22, 2020
 *      Author: zhuchenkovao
 */

#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_


//#include "feram_FM25L16B.h"

#include "state_logger_service.h"



#define   CONFIG_STRUCT_SIZE                  508    // версия структуры конфигурации должен быть не больше (512 - 4) иначе надо менять размер буфера чтения-записи FERAM
#define   CONFIG1_RESERV_SIZE                 397    // размер структуры должен быть 508, размер резерва должен быть 508 - (размер полей структуры)

#define   CONFIG_VERSION                      1      // версия структуры конфигурации


#define   FERAM_CHECK_ADDRESS                 0      // адрес дя проверки работоспособности FERAM
#define   FERAM_SHUTDOWN_TIME_ADDRESS         4      // для сохранения времени выключения
#define   FERAM_CONFIG_ADDRESS_1              12     // для сохранения сектора конфигурации 1



#define   CONFIG_STATE_OK                     0
#define   CONFIG_STATE_READ_ERROR_CRC         1
#define   CONFIG_STATE_READ_ERROR_VERSION     2


#define   CONFIG_NTP_STRING_SIZE              64



#if CHECK_DEVICES_COUNT!=32
  #error CHECK_DEVICES_COUNT != 32
#endif



#pragma pack (push, 1)

typedef struct SConfig {

  // версия структуры конфигурации (должна изменяться в случае не совместимости с предыдущей структурой)
  uint16_t    configVersion;

  // параметры сети
  uint32_t    ip;
  uint32_t    netmask;
  uint32_t    gateway;

  // строка адреса сервера времени в интеренете
  char        ntpServerAddress[CONFIG_NTP_STRING_SIZE];

  // параметры логера и контроля сотояний устройств
  uint8_t     stateControlCfg[CHECK_DEVICES_COUNT];

  char        reserv[CONFIG1_RESERV_SIZE];

  char        crc8;

} TConfig;

#pragma pack (pop)



extern TConfig   config1;
extern uint8_t  configState;


char CONFIG_init(void);
void CONFIG_save(void);
void CONFIG_loadDefault(void);



#endif /* INC_CONFIG_H_ */
