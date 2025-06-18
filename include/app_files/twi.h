
#ifndef TWI_H
#define TWI_H

#include "stm32f7xx_hal.h"

typedef struct S_TWI {

  uint16_t                sda;
  uint16_t                scl;

  GPIO_TypeDef            *gpio;
  uint16_t                delay_nops_count;


} T_TWI;




void TWI_start(T_TWI *twi);
void TWI_stop(T_TWI *twi);
void TWI_release_scl(T_TWI *twi);

char TWI_send_byte(T_TWI *twi, uint8_t byte);
char TWI_read_data(T_TWI *twi, char ack);
char TWI_check_free_line(T_TWI *twi);







#endif
