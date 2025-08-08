
#include "twi.h"



//#define SDA                     GPIO_PIN_7
//#define SCL                     GPIO_PIN_5
//#define GPIOX                   GPIOA
//#define DELAY_NOPS_COUNT        0x2F



void Delay_nop_twi(unsigned int nCount)
{
  while(nCount--)
  {
  }
}



void init_in(T_TWI *twi)
{
    GPIO_InitTypeDef gpio  = {0};
    gpio.Pin = twi->sda | twi->scl;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(twi->gpio, &gpio);

}


void init_out(T_TWI *twi)
{
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = twi->sda | twi->scl;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(twi->gpio, &gpio);
//*/
}


void init_sda_in(T_TWI *twi)
{
    GPIO_InitTypeDef gpio  = {0};

    gpio.Pin = twi->sda;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(twi->gpio, &gpio);

}

void init_sda_out(T_TWI *twi)
{
  GPIO_InitTypeDef gpio  = {0};
  gpio.Pin = twi->sda;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  gpio.Pull = GPIO_PULLUP;
  HAL_GPIO_WritePin(twi->gpio, twi->sda, GPIO_PIN_RESET);
  HAL_GPIO_Init(twi->gpio, &gpio);

  
}

void init_scl_in(T_TWI *twi)
{
  GPIO_InitTypeDef gpio  = {0};

  gpio.Pin = twi->scl;
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  gpio.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(twi->gpio, &gpio);

}

void init_scl_out(T_TWI *twi)
{
  GPIO_InitTypeDef gpio  = {0};

  gpio.Pin = twi->scl;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  gpio.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(twi->gpio, &gpio);



  
}




void TWI_start(T_TWI *twi)
{

  init_sda_out(twi);
  HAL_GPIO_WritePin(twi->gpio, twi->sda, GPIO_PIN_RESET);
  Delay_nop_twi(twi->delay_nops_count);
  init_scl_out(twi);
  HAL_GPIO_WritePin(twi->gpio, twi->scl, GPIO_PIN_RESET);
  Delay_nop_twi(twi->delay_nops_count);

/*  GPIO_SetBits(GPIOX, SDA);
  GPIO_SetBits(GPIOX, twi->scl);
  Delay(0x5FF);*/

}

void TWI_release_scl(T_TWI *twi)
{

  Delay_nop_twi(twi->delay_nops_count);
  init_scl_in(twi);
  Delay_nop_twi(twi->delay_nops_count);
}


void TWI_stop(T_TWI *twi)
{
  //printf ("sda %d \n",tmp);
//  if (TWI_check_free_line() == 0){
    Delay_nop_twi(twi->delay_nops_count);
    init_sda_out(twi);
    HAL_GPIO_WritePin(twi->gpio, twi->sda, GPIO_PIN_RESET);
    Delay_nop_twi(twi->delay_nops_count);
    TWI_release_scl(twi);
    Delay_nop_twi(twi->delay_nops_count);
    init_sda_in(twi);
    Delay_nop_twi(twi->delay_nops_count);
  


}

char TWI_check_free_line(T_TWI *twi)
{
  Delay_nop_twi(twi->delay_nops_count);
  return HAL_GPIO_ReadPin(twi->gpio, twi->sda);
}






char TWI_send_byte(T_TWI *twi, uint8_t byte)
{
  init_sda_out(twi);
  for (int i = 0; i < 8; i++){
    if (byte & 0x80){
        HAL_GPIO_WritePin(twi->gpio, twi->sda, GPIO_PIN_SET);
    }
    byte <<= 1;
    Delay_nop_twi(twi->delay_nops_count);
    HAL_GPIO_WritePin(twi->gpio, twi->scl, GPIO_PIN_SET);
    Delay_nop_twi(twi->delay_nops_count);
    HAL_GPIO_WritePin(twi->gpio, twi->scl, GPIO_PIN_RESET);
    Delay_nop_twi(twi->delay_nops_count);
    HAL_GPIO_WritePin(twi->gpio, twi->sda, GPIO_PIN_RESET);
    
  }
  init_sda_in(twi);
  Delay_nop_twi(twi->delay_nops_count);
  HAL_GPIO_WritePin(twi->gpio, twi->scl, GPIO_PIN_SET);
  char tmp = HAL_GPIO_ReadPin(twi->gpio, twi->sda);
  Delay_nop_twi(twi->delay_nops_count);
  HAL_GPIO_WritePin(twi->gpio, twi->scl, GPIO_PIN_RESET);
  Delay_nop_twi(twi->delay_nops_count);
 // */
  return tmp;
  
}



char TWI_send_byte_stop(T_TWI *twi, uint8_t byte)
{
  init_sda_out(twi);
  for (int i = 0; i < 8; i++){
    if (byte & 0x80){
        HAL_GPIO_WritePin(twi->gpio, twi->sda, GPIO_PIN_SET);
    }
    byte <<= 1;
    Delay_nop_twi(twi->delay_nops_count);
    HAL_GPIO_WritePin(twi->gpio, twi->scl, GPIO_PIN_SET);
    Delay_nop_twi(twi->delay_nops_count);
    HAL_GPIO_WritePin(twi->gpio, twi->scl, GPIO_PIN_RESET);
    Delay_nop_twi(twi->delay_nops_count);
    HAL_GPIO_WritePin(twi->gpio, twi->sda, GPIO_PIN_RESET);
    
  }
  init_sda_in(twi);
  Delay_nop_twi(twi->delay_nops_count);
  HAL_GPIO_WritePin(twi->gpio, twi->scl, GPIO_PIN_SET);
  char tmp = HAL_GPIO_ReadPin(twi->gpio, twi->sda);
  TWI_release_scl(twi);

/*  Delay(twi->delay_nops_count);
  GPIO_ResetBits(twi->gpio, twi->scl);
  Delay(twi->delay_nops_count);
 // */
  return tmp;
  
}



char TWI_read_data(T_TWI *twi, char ack)
{
  char tmp = 0;
  for (int i = 0; i < 8; i++){
    Delay_nop_twi(twi->delay_nops_count);
    HAL_GPIO_WritePin(twi->gpio, twi->scl, GPIO_PIN_SET);
    tmp <<= 1;
    if ( HAL_GPIO_ReadPin(twi->gpio, twi->sda) == 1) tmp |= 1;
    Delay_nop_twi(twi->delay_nops_count);
    HAL_GPIO_WritePin(twi->gpio, twi->scl, GPIO_PIN_RESET);
    Delay_nop_twi(twi->delay_nops_count);
    
  }
  
  if (ack){
    init_sda_out(twi);
    HAL_GPIO_WritePin(twi->gpio, twi->sda, GPIO_PIN_RESET);
  }
  Delay_nop_twi(twi->delay_nops_count);
  HAL_GPIO_WritePin(twi->gpio, twi->scl, GPIO_PIN_SET);
  Delay_nop_twi(twi->delay_nops_count);
  HAL_GPIO_WritePin(twi->gpio, twi->scl, GPIO_PIN_RESET);
  init_sda_in(twi);
  Delay_nop_twi(twi->delay_nops_count);

  
  return tmp;
}


