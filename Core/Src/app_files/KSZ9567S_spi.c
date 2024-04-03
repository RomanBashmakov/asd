/*
 * KSZ9567S_spi.c
 *
 *  Created on: Sep 11, 2021
 *      Author: zhuchenkovao
 */


#include "KSZ9567S_spi.h"




uint8_t txt[10] = {0};
uint8_t rxt[10] = {0};

//=================================================================================
char KSZ9567_SPI_RegReadByte(T_KSZ9567S_SPI *hub, uint16_t addr)
{

  uint32_t r;
  char *pR = (char*)&r;

  r = addr;
  r <<= 5;
  r |= 0x60000000;


  HAL_GPIO_WritePin(hub->GPIO_port, hub->GPIO_pin, GPIO_PIN_RESET);

  //HAL_Delay(1);

  //HAL_SPI_Transmit(&hspi3, &r, 1, 100);

  txt[0] = pR[3];
  txt[1] = pR[2];
  txt[2] = pR[1];
  txt[3] = pR[0];
  txt[4] = 0x00;

  HAL_SPI_TransmitReceive(hub->spi, txt, rxt, 5, 100);

 // HAL_Delay(1);

  HAL_GPIO_WritePin(hub->GPIO_port, hub->GPIO_pin, GPIO_PIN_SET);

  return rxt[4];

}



//=================================================================================
void KSZ9567_SPI_RegWriteByte(T_KSZ9567S_SPI *hub, uint16_t addr, uint8_t byte)
{

  uint32_t r;
  char *pR = (char*)&r;

  r = addr;
  r <<= 5;
  r |= 0x40000000;




  HAL_GPIO_WritePin(hub->GPIO_port, hub->GPIO_pin, GPIO_PIN_RESET);

  txt[0] = pR[3];
  txt[1] = pR[2];
  txt[2] = pR[1];
  txt[3] = pR[0];
  txt[4] = byte;
  HAL_SPI_TransmitReceive(hub->spi, txt, rxt, 5, 100);
  HAL_GPIO_WritePin(hub->GPIO_port, hub->GPIO_pin, GPIO_PIN_SET);

}




//=================================================================================
uint16_t KSZ9567_SPI_SgmiiRegRead(T_KSZ9567S_SPI *hub, uint32_t addr)
{

  uint16_t ret = 0;

  char *da = (char*)&addr;


  // write addr
  uint32_t r;
  char *pR = (char*)&r;

  r = 0x7200;
  r <<= 5;
  r |= 0x40000000;



  HAL_GPIO_WritePin(hub->GPIO_port, hub->GPIO_pin, GPIO_PIN_RESET);

  txt[0] = pR[3];
  txt[1] = pR[2];
  txt[2] = pR[1];
  txt[3] = pR[0];
  txt[4] = da[3];
  txt[5] = da[2];
  txt[6] = da[1];
  txt[7] = da[0];
  HAL_SPI_TransmitReceive(hub->spi, txt, rxt, 8, 100);
  HAL_GPIO_WritePin(hub->GPIO_port, hub->GPIO_pin, GPIO_PIN_SET);




  // read data
  r = 0x7206;
  r <<= 5;
  r |= 0x60000000;


  HAL_GPIO_WritePin(hub->GPIO_port, hub->GPIO_pin, GPIO_PIN_RESET);

  txt[0] = pR[3];
  txt[1] = pR[2];
  txt[2] = pR[1];
  txt[3] = pR[0];
  txt[4] = 0x00;
  txt[5] = 0x00;

  HAL_SPI_TransmitReceive(hub->spi, txt, rxt, 6, 100);


  HAL_GPIO_WritePin(hub->GPIO_port, hub->GPIO_pin, GPIO_PIN_SET);

  ret = (uint16_t)rxt[4] << 8;
  ret |= rxt[5];


  return ret;
}





//=================================================================================
void KSZ9567_SPI_SgmiiRegWrite(T_KSZ9567S_SPI *hub, uint32_t addr, uint16_t data)
{


  //uint16_t ret = 0;

  char *da = (char*)&addr;


  // write addr
  uint32_t r;
  char *pR = (char*)&r;

  r = 0x7200;
  r <<= 5;
  r |= 0x40000000;



  HAL_GPIO_WritePin(hub->GPIO_port, hub->GPIO_pin, GPIO_PIN_RESET);

  txt[0] = pR[3];
  txt[1] = pR[2];
  txt[2] = pR[1];
  txt[3] = pR[0];
  txt[4] = da[3];
  txt[5] = da[2];
  txt[6] = da[1];
  txt[7] = da[0];
  HAL_SPI_TransmitReceive(hub->spi, txt, rxt, 8, 100);
  HAL_GPIO_WritePin(hub->GPIO_port, hub->GPIO_pin, GPIO_PIN_SET);


  // write data
  r = 0x7206;
  r <<= 5;
  r |= 0x40000000;


  HAL_GPIO_WritePin(hub->GPIO_port, hub->GPIO_pin, GPIO_PIN_RESET);

  txt[0] = pR[3];
  txt[1] = pR[2];
  txt[2] = pR[1];
  txt[3] = pR[0];
  txt[4] = data >> 8;
  txt[5] = data & 0xFF;

  HAL_SPI_TransmitReceive(hub->spi, txt, rxt, 6, 100);


  HAL_GPIO_WritePin(hub->GPIO_port, hub->GPIO_pin, GPIO_PIN_SET);



}
