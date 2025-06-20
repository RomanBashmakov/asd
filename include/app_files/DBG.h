#include "main.h"

#define __ENABLE_STROBS

#define STR_PORT    GPIOD
#define STR_PIN     GPIO_PIN_5

#define STR2_PORT   GPIOC
#define STR2_RCC    LL_IOP_GRP1_PERIPH_GPIOC
#define STR2_PIN    LL_GPIO_PIN_10

#define AUX_PORT    GPIOE
#define AUX_RCC     RCC_AHB1Periph_GPIOE
#define AUX_PIN     GPIO_Pin_2

#define GPIO_SetBits(R, P)      LL_GPIO_SetOutputPin(R, P)
#define GPIO_ResetBits(R, P)    LL_GPIO_ResetOutputPin(R, P)

#define OUT_SET     GPIO_SetBits(AUX_PORT, AUX_PIN)
#define OUT_RESET   GPIO_ResetBits(AUX_PORT, AUX_PIN)

#define __STOP    while(1);

/*-----------------------------------------*/
#ifdef __ENABLE_STROBS
#define _STR2     __str2();
#define _STR0     __str0();
#define _MN        __meandr();
#define _MN2       __meandr2();
#define _STR_UP     HAL_GPIO_WritePin(STR_PORT, STR_PIN, GPIO_PIN_SET);
#define _STR_DWN    HAL_GPIO_WritePin(STR_PORT, STR_PIN, GPIO_PIN_RESET);

#define _STR      _STR_UP for(u8_t i = 0; i < 10; i++) {}; _STR_DWN

#define _STR2_DWN   __str2DWN();

#else

#define _STR
#define _STR2
#define _STR0
#define _MN
#define _MN2
#define _STR_UP
#define _STR_DWN
#define _STR2_UP
#define _STR2_DWN
#endif
/*-----------------------------------------*/

void DBG_init(void);
void DBG_init_HAL(void);

void strob_init_HAL(void);