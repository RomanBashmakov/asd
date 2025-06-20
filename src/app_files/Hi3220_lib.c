/*
 * Hi3220_lib.c
 *
 *  Created on: Jun 17, 2020
 *      Author: zhuchenkovao
 */

#include "Hi3220_lib.h"

//===================================================================================
int HI3220_init(THI3220 *hi, THI3220PinStruct *ps, SPI_HandleTypeDef *hspi)
{


  hi->pGPIO = ps;
  hi->phspi = hspi;




  HAL_GPIO_WritePin(hi->pGPIO->PORT_cs, hi->pGPIO->PIN_cs, GPIO_PIN_SET);
  HAL_GPIO_WritePin(hi->pGPIO->PORT_ack, hi->pGPIO->PIN_ack, GPIO_PIN_SET);


  return 0;

}


//===================================================================================
void HI3220_softwareReset(THI3220 *hi, uint16_t mode)
{
  uint8_t txData[4];
  uint8_t rxData[4];



  txData[0] = (mode >> 8);
  txData[1] = (mode & 0x00FF);


  HAL_GPIO_WritePin(hi->pGPIO->PORT_cs, hi->pGPIO->PIN_cs, GPIO_PIN_RESET);

  HAL_SPI_TransmitReceive(hi->phspi, txData, rxData, 2, 100);

  HAL_GPIO_WritePin(hi->pGPIO->PORT_cs, hi->pGPIO->PIN_cs, GPIO_PIN_SET);

  HAL_Delay(10);

  //while ( HAL_GPIO_ReadPin(hi->pGPIO->PORT_int, hi->pGPIO->PIN_int) == GPIO_PIN_RESET );



}



//===================================================================================
uint8_t HI3220_readReg(THI3220 *hi, uint16_t addr)
{
  uint8_t txData[4];
  uint8_t rxData[4];

//  unsigned short *opcode = (unsigned short*)&txData;
  unsigned short opcode;// = (unsigned short*)&txData;
  //unsigned char value;

  opcode = (addr << 4) | RD_REGISTER;

  txData[0] = (opcode >> 8);
  txData[1] = (opcode & 0x00FF);


  HAL_GPIO_WritePin(hi->pGPIO->PORT_cs, hi->pGPIO->PIN_cs, GPIO_PIN_RESET);

  HAL_SPI_TransmitReceive(hi->phspi, txData, rxData, 3, 100);

  HAL_GPIO_WritePin(hi->pGPIO->PORT_cs, hi->pGPIO->PIN_cs, GPIO_PIN_SET);

  return rxData[2];

}


//===================================================================================
void HI3220_writeReg(THI3220 *hi, uint16_t addr, uint8_t data)
{
  uint8_t txData[4];
  uint8_t rxData[4];

//  unsigned short *opcode = (unsigned short*)&txData;
  unsigned short opcode;// = (unsigned short*)&txData;
  //unsigned char value;

  opcode = (addr << 4) | WR_REGISTER;

  txData[0] = (opcode >> 8);
  txData[1] = (opcode & 0x00FF);
  txData[2] = data;


  HAL_GPIO_WritePin(hi->pGPIO->PORT_cs, hi->pGPIO->PIN_cs, GPIO_PIN_RESET);

  HAL_SPI_TransmitReceive(hi->phspi, txData, rxData, 3, 100);

  HAL_GPIO_WritePin(hi->pGPIO->PORT_cs, hi->pGPIO->PIN_cs, GPIO_PIN_SET);


}


//===================================================================================
void HI3220_writeMCR(THI3220 *hi, uint8_t data)
{

  uint8_t txData[4];
  uint8_t rxData[4];


  txData[0] = MCR_WR_F;
  txData[1] = data;

  HAL_GPIO_WritePin(hi->pGPIO->PORT_cs, hi->pGPIO->PIN_cs, GPIO_PIN_RESET);

  HAL_SPI_TransmitReceive(hi->phspi, txData, rxData, 2, 100);

  HAL_GPIO_WritePin(hi->pGPIO->PORT_cs, hi->pGPIO->PIN_cs, GPIO_PIN_SET);


}


//===================================================================================
void HI3220_writeMAP(THI3220 *hi, uint16_t addr)
{
  uint8_t txData[4];
  uint8_t rxData[4];


  txData[0] = WR_MAP;
  txData[1] = (addr >> 8);
  txData[2] = (addr & 0x00FF);

  HAL_GPIO_WritePin(hi->pGPIO->PORT_cs, hi->pGPIO->PIN_cs, GPIO_PIN_RESET);

  HAL_SPI_TransmitReceive(hi->phspi, txData, rxData, 3, 100);

  HAL_GPIO_WritePin(hi->pGPIO->PORT_cs, hi->pGPIO->PIN_cs, GPIO_PIN_SET);




}


//===================================================================================
void HI3220_writeByteAtAddr (THI3220 *hi, uint16_t addr, uint8_t byte)
{

  HI3220_writeMAP (hi, addr);
  HI3220_writeAtMAP (hi, byte);

}

//===================================================================================
uint8_t HI3220_readByteAtAddr (THI3220 *hi, uint16_t addr)
{
  HI3220_writeMAP (hi, addr);
  return HI3220_readAtMAP (hi);

}



//===================================================================================
uint8_t HI3220_readAtMAP (THI3220 *hi)
{
  uint8_t txData[4];
  uint8_t rxData[4];


  txData[0] = RD_MEMORY_MAP;


  HAL_GPIO_WritePin(hi->pGPIO->PORT_cs, hi->pGPIO->PIN_cs, GPIO_PIN_RESET);

  HAL_SPI_TransmitReceive(hi->phspi, txData, rxData, 2, 100);

  HAL_GPIO_WritePin(hi->pGPIO->PORT_cs, hi->pGPIO->PIN_cs, GPIO_PIN_SET);


  return rxData[1];

}

//===================================================================================
void HI3220_writeAtMAP (THI3220 *hi, uint8_t data)
{
  uint8_t txData[4];
  uint8_t rxData[4];


  txData[0] = WR_MEMORY_MAP;
  txData[1] = data;

  HAL_GPIO_WritePin(hi->pGPIO->PORT_cs, hi->pGPIO->PIN_cs, GPIO_PIN_RESET);

  HAL_SPI_TransmitReceive(hi->phspi, txData, rxData, 2, 100);

  HAL_GPIO_WritePin(hi->pGPIO->PORT_cs, hi->pGPIO->PIN_cs, GPIO_PIN_SET);



}



//===================================================================================
void HI3220_readFIFO (THI3220 *hi, unsigned short FIFOChan, unsigned char *bufferRX, unsigned short count)
{
  uint8_t txData[4];
  uint8_t rxData[4];


  txData[0] = FIFO_RD;
  txData[1] = FIFOChan << 4;

  HAL_GPIO_WritePin(hi->pGPIO->PORT_cs, hi->pGPIO->PIN_cs, GPIO_PIN_RESET);

  HAL_SPI_TransmitReceive(hi->phspi, txData, rxData, 2, 100);



  HAL_SPI_Receive(hi->phspi, bufferRX, count << 2, 100);   // count = count * 4


  HAL_GPIO_WritePin(hi->pGPIO->PORT_cs, hi->pGPIO->PIN_cs, GPIO_PIN_SET);


}


//===================================================================================
void HI3220_transmitDirect(THI3220 *hi, unsigned short chan, u8_t *pAW_buf, unsigned short count)
{
  u8_t cmd = TX0_TTT + chan;

  HI_SPI_CS_DWN;

  HAL_SPI_Transmit(hi->phspi, &cmd, 1, 100);

  for(u8_t i = 0; i < 4 /* ARINC word = 4 bytes*/ ; i++) {
	  HAL_SPI_Transmit(hi->phspi, &pAW_buf[i], 1, 100);
  }

  HI_SPI_CS_UP;

  HI_RUN_UP;
//  __ddelayT(5);    // "dirty" delay in ticks
////  HAL_Delay(1);
////  HI_RUN_DWN;
}


////===================================================================================
//void HI3220_transmitDirect_ZH_LV (THI3220 *hi, unsigned short chan, unsigned char *bufferTX, unsigned short msgCount)
//{
//  u8_t cmd = TX0_TTT + chan;
//
//  HI_SPI_CS_DWN;
//
//  HAL_SPI_Transmit(hi->phspi, &cmd, 1, 100);
//
//  for(u8_t i = 0; i < 4; i++) {
//	  HAL_SPI_Transmit(hi->phspi, &bufferTX[i], 1, 100);
//  }
//
//  //  ===== Чтение адреса 0x6C00 (LV) ======
//  //  cmd = RD_MAP;//DBG
//  //  HAL_SPI_Transmit(hi->phspi, &cmd, 1, 100);//DBG
//  //  cmd = 0x00;//DBG
//  //  HAL_SPI_Transmit(hi->phspi, &cmd, 1, 100);//DBG
//  //  cmd = 0x6C;//DBG
//  //  HAL_SPI_Transmit(hi->phspi, &cmd, 1, 100);//DBG
//  //  HAL_SPI_Receive(hi->phspi, &C, 4, 100);//DBG
//
//  //  HAL_SPI_Transmit(hi->phspi, bufferTX, msgCount << 2, 100);   // bytesCount = msgCount * 4
//
//  HI_SPI_CS_UP;
//
//  HI_RUN_UP;
//  HAL_Delay(10);
//  HI_RUN_DWN;
//  HI_RUN_DWN;
//}
//

//===================================================================================
void HI3220_transmitDirect_ZH (THI3220 *hi, unsigned short chan, unsigned char *bufferTX, unsigned short msgCount)
{
  uint8_t txData[4];
  uint8_t rxData[4];


  txData[0] = TX0_TTT + chan;

  HAL_GPIO_WritePin(hi->pGPIO->PORT_cs, hi->pGPIO->PIN_cs, GPIO_PIN_RESET);

  HAL_SPI_TransmitReceive(hi->phspi, txData, rxData, 1, 100);



  HAL_SPI_Transmit(hi->phspi, bufferTX, msgCount << 2, 100);   // bytesCount = msgCount * 4


  HAL_GPIO_WritePin(hi->pGPIO->PORT_cs, hi->pGPIO->PIN_cs, GPIO_PIN_SET);


}


//===================================================================================
void HI3220_transmitDirectDMA (THI3220 *hi, unsigned char *bufferTX, unsigned short bytesCount)
{



  HAL_GPIO_WritePin(hi->pGPIO->PORT_cs, hi->pGPIO->PIN_cs, GPIO_PIN_RESET);


  HAL_SPI_Transmit_DMA(hi->phspi, bufferTX, bytesCount);




}




// отправка с использованием планировщика
//===================================================================================
// ARINC_LoadDescriptors
// This loads the descriptor table with scheduler entries from an 8 byte array.
// One or more descriptors are allowed.
void ARINC_LoadDescriptors(THI3220 *hi,
    unsigned short tx_chan,      // Transmit channel 0-7 (for block pointer)
                       unsigned short seq,              // Sequence start location 0-255) into descriptor table
                       unsigned char *descriptorBuffer, // pointer descriptor buffer
                       unsigned char mesgCnt)           // number of descriptor(messages) to load
{

  unsigned short map, i, descriptors, idx;


  //modeDIPs_g = getModeDIPs();

  map = tx_chan + (seq * 8);     // set map address
  HI3220_writeMAP (hi, map);           // write the map address
   // new method using single array Opcode+data
  for( descriptors = 0, idx = 0; descriptors < mesgCnt; descriptors++)
  {
    //if(modeDIPs_g&2)
    //  {
    //    printf(" map: %.4X ",Read_MAP(0));
    //  }
    for (i = 0; i < 8; i++)
    {
        // if(modeDIPs_g&2)
         //   {
         //     printf(" %.2X ", descriptorBuffer[idx]);
         //   }
      HI3220_writeAtMAP (hi, descriptorBuffer[idx++]);

    }
      //if(modeDIPs_g&2)
      //{
      //  printf("\n\r");
      //}
  }

  HI3220_writeAtMAP (hi, EOS);   // End of Seq, writes 0 at end

}
