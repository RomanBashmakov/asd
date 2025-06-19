/*
 * timers_hw.c
 *
 *  Created on: Nov 6, 2019
 *      Author: zhuchenkovao
 */

#include <timers_hw2.h>

volatile unsigned long timer1ms___ = 0;

void TIMERS_setTimer(TTimer *tmr, uint32_t time_ms)
{
  if (tmr){
    tmr->old = timer1ms___;
    tmr->tmr = time_ms;
  }
}

char TIMERS_expired(TTimer tmr)
{
  if (timer1ms___ - tmr.old > tmr.tmr) return 1;
  return 0;
}

char TIMERS_notExpired(TTimer tmr)
{
  if (timer1ms___ - tmr.old > tmr.tmr) return 0;
  return 1;
}


void TIMERS_disableTimer(TTimer *tmr)
{
  if (tmr){
    tmr->old = 0;
    tmr->tmr = -1;
  }
}


uint32_t TIMERS_ShowTime(TTimer *tmr)
{
  if (tmr){
	  return (timer1ms___ - tmr->old);
  }
  else return 0;
}