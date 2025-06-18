#include "DBG.h"


//LV Init ����� B - � blinc_KEY.c => led_KEY_init()

//static LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
//static uint32_t str_port_was_clocked;

///*-------------------------------------------------------------------------*/
//void DBG_init(void)
//{
//    strob_init_LL();
//}
//
///*-------------------------------------------------------------------------*/
void DBG_init_HAL(void)
{
	strob_init_HAL();
}

///*-------------------------------------------------------------------------*/
//void DBG2_init(void)
//{
//    strob2_init_LL();
//}
//
/////*-------------------------------------------------------------------------*/
////void __str0(void)
////{
////    GPIO_SetBits(STR_PORT,STR_PIN);       //LV PB9(p96) CAN1TX DD4.5
////    GPIO_ResetBits(STR_PORT,STR_PIN);
////}
//
///*-------------------------------------------------------------------------*/
//void __str(void)
//{
//    int i;
////    uint8_t str_port_tacted;
//
//    str_port_was_clocked = LL_IOP_GRP1_IsEnabledClock(STR_RCC);
//    if(!str_port_was_clocked) { LL_IOP_GRP1_EnableClock(STR_RCC); }
//
//    GPIO_SetBits(STR_PORT,STR_PIN);       //LV PB9(p96) CAN1TX DD4.5
//    for (i = 0; i < 10; i++);     //Delay
//    GPIO_ResetBits(STR_PORT,STR_PIN);
//
//    if(!str_port_was_clocked) { LL_IOP_GRP1_DisableClock(STR_RCC); }
////    for (i = 0; i < 1; i++);     //Delay
//}
//
///*-------------------------------------------------------------------------*/
//void __meandr(void)
//{
//    static uint8_t m;
//
//    m ^= 1;
//
//    str_port_was_clocked = LL_IOP_GRP1_IsEnabledClock(STR_RCC);
//    if(!str_port_was_clocked) { LL_IOP_GRP1_EnableClock(STR_RCC); }
//
//    if(m) GPIO_SetBits(STR_PORT,STR_PIN);       //LV PB9(p96) CAN1TX DD4.5
//    else GPIO_ResetBits(STR_PORT,STR_PIN);
//
//    if(!str_port_was_clocked) { LL_IOP_GRP1_DisableClock(STR_RCC); }
//}
//
///*-------------------------------------------------------------------------*/
//void __strUP(void)
//{
//    str_port_was_clocked = LL_IOP_GRP1_IsEnabledClock(STR_RCC);
//    if(!str_port_was_clocked) { LL_IOP_GRP1_EnableClock(STR_RCC); }
//
//    LL_GPIO_SetOutputPin(STR_PORT, STR_PIN);
//
//    if(!str_port_was_clocked) { LL_IOP_GRP1_DisableClock(STR_RCC); }
//
//}
//
///*-------------------------------------------------------------------------*/
//void __strDWN(void)
//{
//    str_port_was_clocked = LL_IOP_GRP1_IsEnabledClock(STR_RCC);
//    if(!str_port_was_clocked) { LL_IOP_GRP1_EnableClock(STR_RCC); }
//
//    LL_GPIO_ResetOutputPin(STR_PORT, STR_PIN);
//
//    if(!str_port_was_clocked) { LL_IOP_GRP1_DisableClock(STR_RCC); }
//}
//
////===== STR2 ===============================================================
///*-------------------------------------------------------------------------*/
///*-------------------------------------------------------------------------*/
//void __str2UP(void)
//{
//    str_port_was_clocked = LL_IOP_GRP1_IsEnabledClock(LL_IOP_GRP1_PERIPH_GPIOC);
//    if(!str_port_was_clocked) LL_IOP_GRP1_EnableClock(STR2_RCC);
//
//    LL_GPIO_SetOutputPin(STR2_PORT, STR2_PIN);
//
//    if(!str_port_was_clocked) LL_IOP_GRP1_DisableClock(STR2_RCC);
//}
//
///*-------------------------------------------------------------------------*/
//void __str2DWN(void)
//{
//    str_port_was_clocked = LL_IOP_GRP1_IsEnabledClock(LL_IOP_GRP1_PERIPH_GPIOC);
//    if(!str_port_was_clocked) LL_IOP_GRP1_EnableClock(STR2_RCC);
//
//    LL_GPIO_ResetOutputPin(STR2_PORT, STR2_PIN);
//
//    if(!str_port_was_clocked) LL_IOP_GRP1_DisableClock(STR2_RCC);
//}
//
///*-------------------------------------------------------------------------*/
//void __str2(void)
//{
//    int i;
//
//    str_port_was_clocked = LL_IOP_GRP1_IsEnabledClock(LL_IOP_GRP1_PERIPH_GPIOC);
//    if(!str_port_was_clocked) LL_IOP_GRP1_EnableClock(STR2_RCC);
//
//    GPIO_SetBits(STR2_PORT,STR2_PIN);
//    for (i = 0; i < 1; i++);     //Delay
//    GPIO_ResetBits(STR2_PORT,STR2_PIN);
////    for (i = 0; i < 1; i++);     //Delay
//
//    if(!str_port_was_clocked) LL_IOP_GRP1_DisableClock(STR2_RCC);
//}
//
///*-------------------------------------------------------------------------*/
//void __meandr2(void)
//{
//    static uint8_t m;
//    m ^= 1;
//
//    str_port_was_clocked = LL_IOP_GRP1_IsEnabledClock(LL_IOP_GRP1_PERIPH_GPIOC);
//    if(!str_port_was_clocked) LL_IOP_GRP1_EnableClock(STR2_RCC);
//
//    if(m) GPIO_SetBits(STR2_PORT,STR2_PIN);
//    else GPIO_ResetBits(STR2_PORT,STR2_PIN);
//
//    if(!str_port_was_clocked) LL_IOP_GRP1_DisableClock(STR2_RCC);
//}
//
/////*-------------------------------------------------------------------------*/
////void __str2(void)
////{
////    int i;
////    GPIO_SetBits(STR_PORT,STR_PIN);       //LV PB9(p96) CAN1TX DD4.5
////    for (i = 0; i < 20; i++);     //Delay
////    GPIO_ResetBits(STR_PORT,STR_PIN);
////    for (i = 0; i < 5; i++);     //Delay
////    GPIO_SetBits(STR_PORT,STR_PIN);       //LV PB9(p96) CAN1TX DD4.5
////    for (i = 0; i < 20; i++);     //Delay
////    GPIO_ResetBits(STR_PORT,STR_PIN);
////    for (i = 0; i < 5; i++);     //Delay
////}
/////*-------------------------------------------------------------------------*/
////
////void __str2(void)
////{
////    int i;
////    GPIO_SetBits(STR2_PORT,STR2_PIN);       //LV PB9(p96) CAN1TX DD4.5
////    for (i = 0; i < 5; i++);     //Delay
////    GPIO_ResetBits(STR2_PORT,STR2_PIN);
////    for (i = 0; i < 5; i++);     //Delay
////}
/////*-------------------------------------------------------------------------*/
//
////void __str3(void)
////{
////    int i;
////    GPIO_SetBits(STR_PORT,STR_PIN);       //LV PB9(p96) CAN1TX DD4.5
////    for (i = 0; i < 20; i++);     //Delay
////    GPIO_ResetBits(STR_PORT,STR_PIN);
////    for (i = 0; i < 5; i++);     //Delay
////    GPIO_SetBits(STR_PORT,STR_PIN);       //LV PB9(p96) CAN1TX DD4.5
////    for (i = 0; i < 20; i++);     //Delay
////    GPIO_ResetBits(STR_PORT,STR_PIN);
////    for (i = 0; i < 5; i++);     //Delay
////    GPIO_SetBits(STR_PORT,STR_PIN);       //LV PB9(p96) CAN1TX DD4.5
////    for (i = 0; i < 20; i++);     //Delay
////    GPIO_ResetBits(STR_PORT,STR_PIN);
////    for (i = 0; i < 5; i++);     //Delay
////}
/////*-------------------------------------------------------------------------*/
////
////void __str_pin8(void)
////{
////    int i;
////    GPIO_SetBits(STR_PORT,GPIO_Pin_8);       //LV PB8(p95) CAN1RX DD4.4
////    for (i = 0; i < 10; i++);     //Delay
////    GPIO_ResetBits(STR_PORT,GPIO_Pin_8);
////    for (i = 0; i < 10; i++);     //Delay
////}
/////*-------------------------------------------------------------------------*/
////
////void strob_init(void)
////{
////    RCC_AHB1PeriphClockCmd(STR_RCC,ENABLE);    // ������������ ����� STR_PORT
////
////    GPIO_InitTypeDef    GPIO_initStr;
////
////    GPIO_StructInit(&GPIO_initStr);
////
//////    GPIO_initStr.GPIO_Pin = GPIO_Pin_8 | STR_PIN;        // LV - for DBG STROB
////    GPIO_initStr.GPIO_Pin = STR_PIN;        // LV - for DBG STROB
////    GPIO_initStr.GPIO_Mode = GPIO_Mode_OUT;
////    GPIO_initStr.GPIO_Speed = GPIO_Speed_100MHz;
////    GPIO_initStr.GPIO_OType = GPIO_OType_PP;
////    GPIO_initStr.GPIO_PuPd = GPIO_PuPd_DOWN;
////
////    GPIO_Init(STR_PORT,&GPIO_initStr);
////}
/////*-------------------------------------------------------------------------*/
////
//void strob_init_LL(void)    // PC2 at the moment
//{
//  LL_IOP_GRP1_EnableClock(STR_RCC);
//
//  GPIO_InitStruct.Pin = STR_PIN;
//  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
//  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
//  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
//  GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
//  LL_GPIO_Init(STR_PORT, &GPIO_InitStruct);
//}
//
///*-------------------------------------------------------------------------*/
void strob_init_HAL(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	  GPIO_InitStruct.Pin = STR_PIN;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  HAL_GPIO_Init(STR_PORT, &GPIO_InitStruct);
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
}

//
/////*-------------------------------------------------------------------------*/
//void strob2_init_LL(void)   // PC4 at the moment
//{
//  LL_IOP_GRP1_EnableClock(STR2_RCC);
//
//  GPIO_InitStruct.Pin = STR2_PIN;
//  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
//  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
//  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
//  GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
//  LL_GPIO_Init(STR2_PORT, &GPIO_InitStruct);
//}
//
///*---------------------------------------------------------------------------*/
//void addtnl_init_LL(void)   // PA2, PA3, PA6, PA7
//{
//  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
//
//  GPIO_InitStruct.Pin = LL_GPIO_PIN_2 | LL_GPIO_PIN_3 | LL_GPIO_PIN_6 | LL_GPIO_PIN_7;
//  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
//  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
//  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
//  GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
//  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//}
//
/////*-------------------------------------------------------------------------*/
////void strob2_init(void)
////{
////    RCC_AHB1PeriphClockCmd(STR2_RCC,ENABLE);    // ������������ ����� STR2_PORT
////
////    GPIO_InitTypeDef    GPIO_initStr;
////
////    GPIO_StructInit(&GPIO_initStr);
////
//////    GPIO_initStr.GPIO_Pin = GPIO_Pin_8 | STR_PIN;        // LV - for DBG STROB
////    GPIO_initStr.GPIO_Pin = STR2_PIN;        // LV - for DBG STROB
////    GPIO_initStr.GPIO_Mode = GPIO_Mode_OUT;
////    GPIO_initStr.GPIO_Speed = GPIO_Speed_100MHz;
////    GPIO_initStr.GPIO_OType = GPIO_OType_PP;
////    GPIO_initStr.GPIO_PuPd = GPIO_PuPd_DOWN;
////
////    GPIO_Init(STR2_PORT,&GPIO_initStr);
////}
/////*-------------------------------------------------------------------------*/
////
////void aux_init(void)
////{
////    RCC_AHB1PeriphClockCmd(AUX_RCC,ENABLE);    // ������������ ����� AUX_PORT
////
////    GPIO_InitTypeDef    GPIO_initStr;
////
////    GPIO_StructInit(&GPIO_initStr);
////
////    GPIO_initStr.GPIO_Pin = AUX_PIN;        // LV - for DBG STROB
////    GPIO_initStr.GPIO_Mode = GPIO_Mode_OUT;
////    GPIO_initStr.GPIO_Speed = GPIO_Speed_100MHz;
////    GPIO_initStr.GPIO_OType = GPIO_OType_PP;
////    GPIO_initStr.GPIO_PuPd = GPIO_PuPd_DOWN;
////
////    GPIO_Init(AUX_PORT,&GPIO_initStr);
////}
/////*-------------------------------------------------------------------------*/
////
//
///*-------------------------------------------------------------------------*/
//void __ddelayms(uint16_t dm)    // dirty delay in ms
//{
//    for(int32_t z = 510 * dm; z > 0; z--) __NOP();  // �� �������� ��� L073 ��� 4MHz ��� 10ms ����� 5100 ������
//}
//
///*-------------------------------------------------------------------------*/
//void __ddelayT(uint32_t t)    // dirty delay in ticks
//{
//    for(int32_t k = 0; k < t; k++) __NOP();  // �� �������� ��� L073 ��� 4MHz ��� 10ms ����� 5100 ������
//}
//
///*-------------------------------------------------------------------------*/
//void __show_pri(void)   // show interrupt priorities
//{
//    uint32_t    PRI1, PRI2, PRI3;
//
//    PRI1 = NVIC_GetPriority(LPTIM1_IRQn);       //LPTIM1 - LPTIM1_IRQn
//    PRI2 = NVIC_GetPriority(RTC_IRQn);          //RTC - RTC_IRQn
//    PRI3 = NVIC_GetPriority(RNG_LPUART1_IRQn);  //LPUART1 - RNG_LPUART1_IRQn
//    (void)PRI1;
//    (void)PRI2;
//    (void)PRI3;
//
////    NVIC_SetPriority(LPTIM1_IRQn, 1);           //LPTIM1 - LPTIM1_IRQn
////    NVIC_SetPriority(RTC_IRQn, 1);              //RTC - RTC_IRQn
////    NVIC_SetPriority(RNG_LPUART1_IRQn, 0);      //LPUART1 - RNG_LPUART1_IRQn
//}
