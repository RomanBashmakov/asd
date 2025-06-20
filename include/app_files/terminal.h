/*
 * terminal.h
 *
 *  Created on: Sep 16, 2021
 *      Author: zhuchenkovao
 */

#ifndef INC_APP_FILES_TERMINAL_H_
#define INC_APP_FILES_TERMINAL_H_

#include "timers_hw2.h"
#include "cbuffer.h"

#define  TERM_OUTPUT_BUFFER_LENGTH           1000

typedef struct STerminal {
  int32_t           cmdRepeat;             // значение повторения команды с введённой параметром повторения
  TTimer            tmrCmdRepeat;          // таймер повторения команды с введённой параметром повторения
  char              cmdStatusModule[20];   // сохранение названия модуля для вывода статуса именно этого модуля

  TCbuffer          cbOutput;
  char              outputBuffer[TERM_OUTPUT_BUFFER_LENGTH];

  TCbuffer          cbTerminalCmd[3];      // переменные кольцевого буфера
  uint8_t           cbTerminalCmdIndex;
  char              cbTerminalCmdData[20];
  char              cbTerminalCmdData2[20];
  char              cbTerminalCmdData3[20];

  char              newCmd;                // переменные введённых команд
  char              cmdIndex;
  char              cmd[3][20];
  uint8_t           cmdLength[3];

  char              newSpecialCmd;
  char              specialCmd;
  char              lockTerm;              // замок на изменение параметров

} TTerminal;

void  TERMINAL_init (TTerminal * tns);
void  TERMINAL_process( TTerminal * tns);

void  TERMINAL_recvByte( TTerminal * tns, char tmp);

void TERMINAL_putMsg(TTerminal * tns, const char *str);
void TERMINAL_putMsgLen(TTerminal * tns, const char *str, uint16_t len);

void TERMINAL_printStatus (TTerminal * tns);

#endif /* INC_APP_FILES_TERMINAL_H_ */