#include "channel_utils.h"

/// @brief   Номер выбранного канала (камеры).
static uint8_t Channel_No = 0;

uint8_t GetChannelNo(void)
{
    return Channel_No;
}

void SetChannelNo(uint8_t ch_no)
{
    Channel_No = (ch_no < CHANNELS_TOTAL ? ch_no + 1 : 0);
}