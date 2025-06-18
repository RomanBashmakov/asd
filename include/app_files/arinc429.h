/*
 * arinc429.h
 *
 *  Created on: 15 сент. 2021 г.
 *      Author: zhuchenkovao
 */

#ifndef INC_APP_FILES_ARINC429_H_
#define INC_APP_FILES_ARINC429_H_

#define A429_L0300_SEND_PERIOD	500

#include "stm32f7xx_hal.h"

#include "Hi3220_lib.h"
#include "ina226.h"

#define CAMERA_CURRENT_THRESHOLD 0.05

#pragma pack (push, 1)

/// @brief   Структура Сервера Накопителя и Сервера Маршрутизатора
typedef struct Status
{
    unsigned char archive; //!< Состояние Сервера Накопителя
    unsigned char router;  //!< Состояние Сервера Маршрутизатора
}Status;

/// @brief   Структура пакета с состоянием Сервера Накопителя и Сервера Маршрутизатора
typedef struct SetStatus
{
    unsigned char flag;   //!< Флаг пакета
    unsigned char code;   //!< Код пакета
    Status        status; //!< Статус Сервера Накопителя и Сервера Маршрутизатора
}SetStatus;

/// @brief   Структура пакета с запросом на получение состояния Камеры
typedef struct GetStatusQuery
{
    unsigned char flag;   //!< Флаг пакета
    unsigned char code;   //!< Код пакета
    unsigned char camera; //!< Номер камеры
}GetStatusQuery;

/// @brief   Структура пакета с ответом на запрос о получении состояния Камеры
typedef struct GetStatusResponse
{
    unsigned char flag;   //!< Флаг пакета
    unsigned char code;   //!< Код пакета
    unsigned char camera; //!< Номер камеры
    unsigned char status; //!< Состояние камеры
}GetStatusResponse;

#pragma pack (pop)

extern T_INA226 powerSensA[];
extern T_INA226 powerSensB[];

typedef struct {
  // arinc protocol data
  // 1
  char FORMAT1; //(7 бит)
  char ACTIVE_CONF_MENU_CAPT;  // (1 бит)

  // 2
  char FORMAT1_POZITION; // (3 бита)
  char FORMAT1_SIZE; // (4 бита)
  char ACTIVE_CURSOR_CAPT; // (1 бит)

  // 3
  uint16_t CURSOR_X_CAPT;  // (старшие 8 бит)

  // 4
  // CURSOR_X_CAPT; // (младшие 3 бита)
  char CAMS_NUMBER_CAPT; // (4 бита)
  char BTN_PRESSED_CAPT; // (1 бит)

  // 5
  uint16_t CURSOR_Y_CAPT; // (старшие 8 бит)

  // 6
  // CURSOR_Y_CAPT; // (младшие 3 бита)
  char JOYSTICK_DOWN_CAPT; // (1 бит)
  char JOYSTICK_LEFT_CAPT; // (1 бит)
  char JOYSTICK_RIGHT_CAPT; // (1 бит)
  char JOYSTICK_UP_CAPT; // (1 бит)
  char LEFT_ENTER_CAPT; // (1 бит)

  // 7
  char LENT_JAMMED_CAPT; // (1 бит)
  char RIGHT_ENTER_CAPT; // (1 бит)
  char RENT_JAMMED_CAPT; // (1 бит)
  char ESC_BTN_CAPT; //  (1 бит)
  char ESC_JAMMED_CAPT; // (1 бит)
  char OK_BTN_CAPT; // (1 бит)
  char OK_JAMMED_CAPT; // (1 бит)
  char LWR_AREA_BTN_CAPT; //  (1 бит)

  // 8
  char FORMAT2; // (7 бит)
  char ACTIVE_CONF_MENU_FO; // (1 бит)

  // 9
  char FORMAT2_POZITION; // (3 бита)
  char FORMAT2_SIZE; // (4 бита)
  char ACTIVE_CURSOR_FO; // (1 бит)

  // 10
  uint16_t CURSOR_X_FO; // (старшие 8 бит)

  // 11
  //char CURSOR_X_FO; // (младшие 3 бита)
  char CAMS_NUMBER_FO; // (4 бит)
  char BTN_PRESSED_FO; // (1 бит)

  // 12
  uint16_t CURSOR_Y_FO; //  (старшие 8 бит)

  // 13
  //char CURSOR_Y_FO; //  (младшие 3 бита)
  char JOYSTICK_DOWN_FO; // (1 бит)
  char JOYSTICK_LEFT_FO; // (1 бит)
  char JOYSTICK_RIGHT_FO; // (1 бит)
  char JOYSTICK_UP_FO; //  (1 бит)
  char LEFT_ENTER_FO; // (1 бит)

  // 14
  char LENT_JAMMED_FO; // (1 бит)
  char RIGHT_ENTER_FO; // (1 бит)
  char RENT_JAMMED_FO; // (1 бит)
  char ESC_BTN_FO; // (1 бит)
  char ESC_JAMMED_FO; // (1 бит)
  char OK_BTN_FO; // (1 бит)
  char OK_JAMMED_FO; // (1 бит)
  char LWR_AREA_BTN_FO; // (1 бит)
} T_ARINC429Control;


typedef struct {

  char  hour;
  char  minute;
  char  sec;

  char  year;
  char  month;
  char  day;


} T_ARINC429ControlDateTime;


typedef struct {
    unsigned char srv_stat		:1;
    unsigned char storage_stat	:1;
} Linux_Status_t;

/*--------------------------------------------------------------------------*/

/*--- 125 ----------------------*/
typedef struct {	// Time in BCD
    u32_t label      :8;
    u32_t sdi        :2;
    u32_t minutesS   :4;
    u32_t minutesU   :4;
    u32_t minutesT   :4;
    u32_t hoursU     :4;
    u32_t hoursT     :3;
    u32_t matrix     :2;
    u32_t parity     :1;
} A429_word125_t;

union W125_t {
    A429_word125_t  str;
    u32_t           word429;
};

/*--- 150 ----------------------*/
typedef struct {	// Time in binary
    u32_t label     :8;
    u32_t secondS   :3;
    u32_t second    :6;
    u32_t minute    :6;
    u32_t hour      :5;
    u32_t nc		:1;
    u32_t matrix    :2;
    u32_t parity    :1;
} A429_word150_t;

union W150_t {
    A429_word150_t  str;
    u32_t           word429;
};

/*--- 260 ----------------------*/
typedef struct {	// Date in BCD
    u32_t label     :8;
    u32_t sdi       :2;
    u32_t yearU     :4;
    u32_t yearT     :4;
    u32_t monthU    :4;
    u32_t monthT    :1;
    u32_t dayU      :4;
    u32_t dayT      :2;
    u32_t matrix    :2;
    u32_t parity    :1;
} A429_word260_t;

union W260_t {
    A429_word260_t  str;
    u32_t           word429;
};

/*------------------------------*/

/*--- 300 ----------------------*/
typedef struct {	// Из протокола ИВ СВР-МТО_14.04
    u32_t label      		:8;
    u32_t sdi        		:2;
    u32_t cam1_fault 		:1;
    u32_t cam2_fault 		:1;
    u32_t cam3_fault 		:1;
    u32_t cam4_fault 		:1;
    u32_t cam5_fault 		:1;
    u32_t cam6_fault 		:1;
    enum System_Status_e System_Status		:2;
    u32_t SrvRouter_Status	:1;
    u32_t Storage_Status	:1;
    u32_t XAE21_fault		:1;
    u32_t cam_switch_fault	:1;
    u32_t nc				:7;
    enum A300_Matrix_e matrix     		:2;
    u32_t parity     		:1;
} A429_word300_t;

typedef union W300_t {
    A429_word300_t  str;
    u32_t           word429;
    u8_t           	arinc_array[4];
} AW300_t;

//typedef union W300_t AW300_t;

/*--- 301 ----------------------*/
/*--- заготовка ----------------*/
typedef struct {
    u32_t label     :8;
    u32_t sdi       :2;
    u32_t nc		:19;
    u32_t matrix    :2;
    u32_t parity    :1;
} A429_word301_t;

union W301_t {
    A429_word301_t  str;
    u32_t           word429;
};

/*--- 302 ----------------------*/
typedef struct {
    u32_t label     :8;
    u32_t sdi       :2;
    u32_t nc		:19;
    u32_t matrix    :2;
    u32_t parity    :1;
} A429_word302_t;

union W302_t {
    A429_word302_t  str;
    u32_t           word429;
};
/*------------------------------*/
/*--------------------------------------------------------------------------*/

extern T_ARINC429Control           arinc429Control;
extern T_ARINC429ControlDateTime   arinc429DT;
//extern char  pcCtrlConnected;
//extern char  pcDTConnected;

extern uint8_t txPacketARINC429_1[];
extern uint8_t txPacketARINC429_2[];
extern uint8_t txPacketARINC429_3[];

void ARINC429_init(SPI_HandleTypeDef *hspi);
void ARINC429_process();

void ARINC429_parseMessageCh1(char *data);
void ARINC429_parseMessageCh2(char *data);

void ARINC429_Proto_InputPacket(char *packet, char size, char *out, char *outSize);
void ARINC429_Proto_buildUartPacketControl(char *packet);
void ARINC429_Proto_buildUartPacketDT(char *packet);

#endif /* INC_APP_FILES_ARINC429_H_ */