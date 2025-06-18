//#include "stm32l0xx.h"

//#include "stm32l0xx_ll_gpio.h"
//#include "stm32l0xx_ll_bus.h"

//#include "globals.h"
#include "main.h"

#define __ENABLE_STROBS

#define STR_PORT    GPIOD
//#define STR_RCC     RCC_AHB1Periph_GPIOB
//#define STR_RCC     LL_IOP_GRP1_PERIPH_GPIOD
//#define STR_PIN     LL_GPIO_PIN_5
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
//#define _STR      __str();
#define _STR2     __str2();
#define _STR0     __str0();
#define _MN        __meandr();
#define _MN2       __meandr2();
//#define _STR_UP     LL_GPIO_SetOutputPin(STR_PORT, STR_PIN);
//#define _STR_DWN    LL_GPIO_ResetOutputPin(STR_PORT, STR_PIN);
//#define _STR_UP     __strUP();
#define _STR_UP     HAL_GPIO_WritePin(STR_PORT, STR_PIN, GPIO_PIN_SET);
//#define _STR_DWN    __strDWN();
#define _STR_DWN    HAL_GPIO_WritePin(STR_PORT, STR_PIN, GPIO_PIN_RESET);

#define _STR      _STR_UP for(u8_t i = 0; i < 10; i++) {}; _STR_DWN

//#define _STR2_UP    { LL_IOP_GRP1_EnableClock(STR2_RCC); LL_GPIO_SetOutputPin(STR2_PORT, STR2_PIN); }
//#define _STR2_DWN   { LL_IOP_GRP1_EnableClock(STR2_RCC); LL_GPIO_ResetOutputPin(STR2_PORT, STR2_PIN); }
#define _STR2_UP    __str2UP();
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
//void DBG2_init(void);
//void strob_init(void);
//void strob_init_LL(void);
//void strob2_init_LL(void);
//void addtnl_init_LL(void);   // PA2, PA3, PA6, PA7 at the moment
//
void strob_init_HAL(void);
//
//void inline __strUP(void);
//void inline __strDWN(void);
//void inline __str2UP(void);
//void inline __str2DWN(void);

//void inline __str0(void);
//void inline __str(void);
//void inline __str2(void);
//void inline __str3(void);
//void __str_pin8(void);
//
//void __meandr(void);
//void __meandr2(void);
//
//void aux_init(void);
//void __ddelayms(uint16_t dm);    // dirty delay in ms
//void __ddelayT(uint32_t t); //dirty delay in ticks
//void __show_pri(void);   // show interrupt priorities

//unsigned int n_nonRCVD;
//unsigned int n_nonANSWR;
//unsigned int n_ANSWR;
