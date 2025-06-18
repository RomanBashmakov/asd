#ifndef TIMERS_HW_H__
#define TIMERS_HW_H__

#include <stdint.h>

typedef struct STimer {

  uint32_t        tmr;
  uint32_t        old;

} TTimer;

extern volatile uint32_t timer1ms___;

//void HAL_SYSTICK_Callback(void);

void TIMERS_setTimer(TTimer *tmr, uint32_t time_ms);
char TIMERS_expired(TTimer tmr);
char TIMERS_notExpired(TTimer tmr);
void TIMERS_disableTimer(TTimer *tmr);
uint32_t TIMERS_ShowTime(TTimer *tmr);

void Delay_nop(uint32_t nCount);

#endif
