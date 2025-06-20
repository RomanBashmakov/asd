#include "DBG.h"

void DBG_init_HAL(void)
{
	strob_init_HAL();
}

void strob_init_HAL(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	  GPIO_InitStruct.Pin = STR_PIN;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  HAL_GPIO_Init(STR_PORT, &GPIO_InitStruct);
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
}