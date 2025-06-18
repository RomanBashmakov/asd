/*
 * terminal.c
 *
 *  Created on: Sep 16, 2021
 *      Author: zhuchenkovao
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>


#include "utils.h"

#include "gpio.h"
#include "tim.h"

#include "powercontrol.h"
#include "arinc429.h"
#include "stm32_status.h"

#include "terminal.h"



#define  TNSERVER_STR_BUFFER_MAXSIZE        350






extern const char strVersionDT[];

static const char strVer[] = "\r\nVersion: %s\r\n>";
static const char strRN[] = ">";
static const char strAvailableCmd[] = "\r\nAvailable commands:\r\nhelp \r\nver \r\nstatus\r\nfan [on|off]\r\n>";
static const char strCmdNotFound[] = "\r\nCommand not found, type 'help' to view available commands\r\n>";
static const char strSuccessRN[] = "success\r\n>";
static const char strWrongParamFormat[] = "\r\nwrong parameter format\r\n>";
static const char strRepeatCancelled[] = "\r\nrepeat cancelled\r\n>";



char    strBuffer[TNSERVER_STR_BUFFER_MAXSIZE];




//-------------------------------------------------------------------------------------
/*int atoi_base10(char* s, char *ok)
{
    int n = 0, sign;
    if (*s == '-') {
      sign = -1;
      s++;
    }
    else sign = 1;

    while( *s >= '0' && *s <= '9' ) {
        n *= 10;
        n += *s++;
        n -= '0';
    }

    if (*s == 0) *ok = 1;
    else *ok = 0;


    return (n * sign);

}*/



//==============================================================================
void  TERMINAL_init (TTerminal * tns)
{

  // инициализация




  if (tns != NULL){


    tns->newCmd = 0;
    tns->cmdIndex = 0;
    tns->newSpecialCmd = 0;
    tns->specialCmd = 0;

    TIMERS_disableTimer(&tns->tmrCmdRepeat);
    tns->cmdRepeat = -1;
    for (int i = 0; i < 20; i++) tns->cmdStatusModule[i] = 0;

   // tns->devices = devs;
 //   tns->devsValid = DEVICES_checkValid(devs);   // проверка валидности

    tns->lockTerm = 1;  // по умолчанию изменение параметров заблокированно

    tns->cbTerminalCmdIndex = 0;
    CBUFF_init(&tns->cbTerminalCmd[0], tns->cbTerminalCmdData, 20);
    CBUFF_init(&tns->cbTerminalCmd[1], tns->cbTerminalCmdData2, 20);
    CBUFF_init(&tns->cbTerminalCmd[2], tns->cbTerminalCmdData3, 20);


    CBUFF_init(&tns->cbOutput, tns->outputBuffer, TERM_OUTPUT_BUFFER_LENGTH);





  }



}





//==============================================================================
void TERMINAL_process (TTerminal * tns)
{



  if (tns->newSpecialCmd){
    tns->newSpecialCmd = 0;
    switch (tns->specialCmd){
    case 236:  // Ctrl-D


      break;
    case 248:  // Ctrl-C
      tns->cmdRepeat = -1;
      TIMERS_disableTimer(&tns->tmrCmdRepeat);
      TERMINAL_putMsg(tns, strRepeatCancelled);
      break;
    }


  }

  if ( TIMERS_expired(tns->tmrCmdRepeat) ){
    if (tns->cmdRepeat == -1) TIMERS_disableTimer(&tns->tmrCmdRepeat);
    else TIMERS_setTimer(&tns->tmrCmdRepeat, tns->cmdRepeat);

    TERMINAL_printStatus(tns);

  }






  if (tns->newCmd){
    tns->newCmd = 0;

    int strLen = 0;



    // printf ("cmd: %s\n  %s\n     %s\n", tns->cmd[0], tns->cmd[1], tns->cmd[2]); //

    if (strncmp(tns->cmd[0], "help", 4) == 0)
    {
      TERMINAL_putMsg(tns, strAvailableCmd);

    }else if (strncmp(tns->cmd[0], "ver", 3) == 0)
    {
      strLen = snprintf (strBuffer,
                         TNSERVER_STR_BUFFER_MAXSIZE,
                         strVer,
                         strVersionDT
                         );
      TERMINAL_putMsgLen(tns, strBuffer, strLen);

    }else if (strncmp(tns->cmd[0], "status", tns->cmdLength[0]) == 0)
    {
      if (tns->cmdIndex > 0){

        for (int i = 0; i < 20; i++) tns->cmdStatusModule[i] = tns->cmd[1][i];

        if (tns->cmdIndex > 1){

          char  ok;
          int v;
          v = atoi_base10(tns->cmd[2], &ok);

          if (ok){
            if (v < 1) v = 1;            // слишком маленький таймер "повесит" IP стэк  !!!
            tns->cmdRepeat = v * 1000;
            TIMERS_setTimer(&tns->tmrCmdRepeat, tns->cmdRepeat);
            TERMINAL_printStatus(tns);

          }else{
            TERMINAL_putMsg(tns, strWrongParamFormat);
          }

        } else {
          TERMINAL_printStatus(tns);
        }
      }else{
        tns->cmdStatusModule[0] = 0;
        TERMINAL_printStatus(tns);
      }




    }else if (strncmp(tns->cmd[0], "fan", 3) == 0)
    {
      if (tns->cmdIndex > 0){
        if (strncmp(tns->cmd[1], "on", tns->cmdLength[1]) == 0){
          HAL_GPIO_WritePin(GPO_blower_GPIO_Port, GPO_blower_Pin, GPIO_PIN_SET);

          TERMINAL_putMsg(tns, strSuccessRN);
        }else if (strncmp(tns->cmd[1], "off", tns->cmdLength[1]) == 0){
          HAL_GPIO_WritePin(GPO_blower_GPIO_Port, GPO_blower_Pin, GPIO_PIN_RESET);
          TERMINAL_putMsg(tns, strSuccessRN);
        }
      }

    }else if (strncmp(tns->cmd[0], "tim", 3) == 0)
    {

      if (tns->cmdIndex > 0){

        char  ok;
        int v;
        v = atoi_base10(tns->cmd[1], &ok);

        htim12.Instance->CCR2 = v;
      }


    }else{
      TERMINAL_putMsg(tns, strCmdNotFound);
    }

    // очистка комманд
    for (int i = 0; i < 20; i++) tns->cmd[0][i] = 0;
    for (int i = 0; i < 20; i++) tns->cmd[1][i] = 0;
    for (int i = 0; i < 20; i++) tns->cmd[2][i] = 0;


  }



}




//==============================================================================
void TERMINAL_recvByte(TTerminal * tns, char tmp)
{

  if (tns == 0) return;






  // поиск строки оконченной символом \n и запись в tns->cmd


   //printf ("%02X  %c\n", tmp, tmp);  //

    if (tmp == (unsigned char)0x0D || tmp == (unsigned char)0x0A){
      if (tns->cbTerminalCmd[tns->cbTerminalCmdIndex].count  || tns->cbTerminalCmdIndex > 0){

        tns->cmdLength[0] = tns->cbTerminalCmd[0].count;
        CBUFF_getFirstNBytes(&tns->cbTerminalCmd[0], tns->cmd[0], tns->cbTerminalCmd[0].count);

        if (tns->cbTerminalCmdIndex > 0) {
          tns->cmdLength[1] = tns->cbTerminalCmd[1].count;
          CBUFF_getFirstNBytes(&tns->cbTerminalCmd[1], tns->cmd[1], tns->cbTerminalCmd[1].count);
        }
        if (tns->cbTerminalCmdIndex > 1) {
          tns->cmdLength[2] = tns->cbTerminalCmd[2].count;
          CBUFF_getFirstNBytes(&tns->cbTerminalCmd[2], tns->cmd[2], tns->cbTerminalCmd[2].count);
        }
        tns->cmdIndex = tns->cbTerminalCmdIndex;
        tns->cbTerminalCmdIndex = 0;

        tns->newCmd = 1;
        //tns->cmd[count - 1] = 0;
        return;
      }else{
        TERMINAL_putMsg(tns, strRN);

      }
    }else{

      if (tmp > (unsigned char)0x20 && tmp <= (unsigned char)0x7E) { // если символ читаемый (в русской кодировке не принимаются)
        // сохранить в буфер команд
        CBUFF_pushNBytes(&tns->cbTerminalCmd[tns->cbTerminalCmdIndex], (char*)&tmp, 1);



      } else if ( (tmp == (unsigned char)0x20) ) { // пробел

        //printf ("%d  %d\n", tns->cbTerminalCmd[tns->cbTerminalCmdIndex].count, tns->cbTerminalCmdIndex);  //


        // следующий буфер команд
        tns->cbTerminalCmdIndex++;
        if (tns->cbTerminalCmdIndex > 2) tns->cbTerminalCmdIndex = 2;



      } else if ( (tmp == (unsigned char)0x03) ) {
        tns->newSpecialCmd = 1;
        tns->specialCmd = 248;

        tns->cbTerminalCmdIndex = 0;
        tns->cmdIndex = 0;


        CBUFF_reset(&tns->cbTerminalCmd[0]);
        CBUFF_reset(&tns->cbTerminalCmd[1]);
        CBUFF_reset(&tns->cbTerminalCmd[2]);

      } else if ( (tmp == (unsigned char)0xFF) ) {
        tns->specialCmd = 1;

      } else if ( (tmp >= (unsigned char)0xC0) ) {
        if (tns->specialCmd){
          tns->newSpecialCmd = 1;
          tns->specialCmd = tmp;

          tns->cbTerminalCmdIndex = 0;
          tns->cmdIndex = 0;


          CBUFF_reset(&tns->cbTerminalCmd[0]);
          CBUFF_reset(&tns->cbTerminalCmd[1]);
          CBUFF_reset(&tns->cbTerminalCmd[2]);




        }

      } else {
         //char que = '?';

      }


    }









}

//==============================================================================
void TERMINAL_putMsg(TTerminal * tns, const char *str)
{

  CBUFF_pushNBytes(&tns->cbOutput, str, strlen(str));


}




//==============================================================================
void TERMINAL_putMsgLen(TTerminal * tns, const char *str, uint16_t len)
{

  CBUFF_pushNBytes(&tns->cbOutput, str, len);

}




//-------
void TERMINAL_printStatus (TTerminal * tns)
{
  int strLen;
  extern Status_t SYSTEM_Status;


  if (tns){

    if (tns->cmdStatusModule[0] == 0 || strncmp(tns->cmdStatusModule, "stm32", 5) == 0){
      strLen = snprintf (strBuffer,
                         TNSERVER_STR_BUFFER_MAXSIZE,
                         "stm32:\r\n t1:%.1f  t2:%.1f  vdda:%.3f  debug:%lu  readProtect:%d  csr:%08lX\r\n uid1: %02X %02X %02X %02X,    uid2: %02X %02X %02X %02X (%s),    uid3: %02X %02X %02X %02X (%s)\r\n",
                         stm32State.tempLM94022_1,
                         stm32State.tempLM94022_2,
                         stm32State.vdda / 1000.0,
                         stm32State.debugEnabled,
                         stm32State.readOutEnabled,
                         stm32State.rcccsr,
                         stm32State.UID[0][0],
                         stm32State.UID[0][1],
                         stm32State.UID[0][2],
                         stm32State.UID[0][3],

                         stm32State.UID[1][0],
                         stm32State.UID[1][1],
                         stm32State.UID[1][2],
                         stm32State.UID[1][3],
                         stm32State.UID[1],

                         stm32State.UID[2][0],
                         stm32State.UID[2][1],
                         stm32State.UID[2][2],
                         stm32State.UID[2][3],
                         stm32State.UID[2]
                         );
      TERMINAL_putMsgLen(tns, strBuffer, strLen);



    }

    if (tns->cmdStatusModule[0] == 0 || strncmp(tns->cmdStatusModule, "pwr", 3) == 0){

      char t = 1;

      for (int i = 0; i < 10; i++){
        strLen = snprintf (strBuffer,
                           TNSERVER_STR_BUFFER_MAXSIZE,
                           "%d: %.1fV %.3fA %.3fW\r\n",
                           t,
                           T_INA226_getVoltage( &powerSensA[i] ),
                           T_INA226_getCurrent( &powerSensA[i] ),
                           T_INA226_getPower( &powerSensA[i] )
                           );
        TERMINAL_putMsgLen(tns, strBuffer, strLen);
        t++;

      }

      for (int i = 0; i < 10; i++){
        strLen = snprintf (strBuffer,
                           TNSERVER_STR_BUFFER_MAXSIZE,
                           "%d: %.1fV %.3fA %.3fW\r\n",
                           t,
                           T_INA226_getVoltage( &powerSensB[i] ),
                           T_INA226_getCurrent( &powerSensB[i] ),
                           T_INA226_getPower( &powerSensB[i] )
                           );
        TERMINAL_putMsgLen(tns, strBuffer, strLen);
        t++;

      }

      //tcpSocketSend(tns->tcpSocket,  strBuffer, strLen);



    }

    if (tns->cmdStatusModule[0] == 0 || strncmp(tns->cmdStatusModule, "hub", 3) == 0){



      strLen = snprintf (strBuffer,
                         TNSERVER_STR_BUFFER_MAXSIZE,
                         "hubs:\r\n"

                                                     );
      TERMINAL_putMsgLen(tns, strBuffer, strLen);

    }

    if (tns->cmdStatusModule[0] == 0 || strncmp(tns->cmdStatusModule, "arinc", 5) == 0){

      strLen = snprintf (strBuffer,
                         TNSERVER_STR_BUFFER_MAXSIZE,
                         "arinc429:\r\nformat1:%d %d %d, CAPT: coursor x:%d y:%d, cams:%d joystick:%d%d%d%d\r\nactive: menu:%d cursor:%d btn:%d%d%d%d%d\r\n",
                         arinc429Control.FORMAT1,
                         arinc429Control.FORMAT1_POZITION,
                         arinc429Control.FORMAT1_SIZE,
                         arinc429Control.CURSOR_X_CAPT,
                         arinc429Control.CURSOR_Y_CAPT,
                         arinc429Control.CAMS_NUMBER_CAPT,
                         arinc429Control.JOYSTICK_UP_CAPT,
                         arinc429Control.JOYSTICK_DOWN_CAPT,
                         arinc429Control.JOYSTICK_LEFT_CAPT,
                         arinc429Control.JOYSTICK_RIGHT_CAPT,
                         arinc429Control.ACTIVE_CONF_MENU_CAPT,
                         arinc429Control.ACTIVE_CURSOR_CAPT,
                         arinc429Control.LEFT_ENTER_CAPT,
                         arinc429Control.OK_BTN_CAPT,
                         arinc429Control.ESC_BTN_CAPT,
                         arinc429Control.LWR_AREA_BTN_CAPT,
                         arinc429Control.RIGHT_ENTER_CAPT
                                                     );
      TERMINAL_putMsgLen(tns, strBuffer, strLen);

      strLen = snprintf (strBuffer,
                         TNSERVER_STR_BUFFER_MAXSIZE,
                         "format2:%d %d %d, FO: coursor x:%d y:%d, cams:%d joystick:%d%d%d%d\r\nactive: menu:%d cursor:%d btn:%d%d%d%d%d\r\n",
                         arinc429Control.FORMAT2,
                         arinc429Control.FORMAT2_POZITION,
                         arinc429Control.FORMAT2_SIZE,
                         arinc429Control.CURSOR_X_FO,
                         arinc429Control.CURSOR_Y_FO,
                         arinc429Control.CAMS_NUMBER_FO,
                         arinc429Control.JOYSTICK_UP_FO,
                         arinc429Control.JOYSTICK_DOWN_FO,
                         arinc429Control.JOYSTICK_LEFT_FO,
                         arinc429Control.JOYSTICK_RIGHT_FO,
                         arinc429Control.ACTIVE_CONF_MENU_FO,
                         arinc429Control.ACTIVE_CURSOR_FO,
                         arinc429Control.LEFT_ENTER_FO,
                         arinc429Control.OK_BTN_FO,
                         arinc429Control.ESC_BTN_FO,
                         arinc429Control.LWR_AREA_BTN_FO,
                         arinc429Control.RIGHT_ENTER_FO
                                                     );
      TERMINAL_putMsgLen(tns, strBuffer, strLen);

      strLen = snprintf (strBuffer,
                         TNSERVER_STR_BUFFER_MAXSIZE,
//                         "DT: %02d.%02d.%d  %02d:%02d:%02d   pc: ctrl %d, dt %d",
                         "DT: %02d.%02d.%d  %02d:%02d:%02d   pc: srvrouter_stat %d",
                         arinc429DT.day,
                         arinc429DT.month,
                         arinc429DT.year + 2000,
                         arinc429DT.hour,
                         arinc429DT.minute,
                         arinc429DT.sec,
						 SYSTEM_Status.srvrouter_stat
//                         pcCtrlConnected,
//                         pcDTConnected
                         );
      TERMINAL_putMsgLen(tns, strBuffer, strLen);

    }

    TERMINAL_putMsg(tns, "\r\n>");
  }

}
