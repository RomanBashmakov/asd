
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f7xx_hal.h"

#include "adc.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "mcp23008.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"

#include <stdio.h>

#include "KSZ9567S_spi.h"
#include "arinc429.h"
#include "flash_W25Q32.h"
#include "powercontrol.h"
#include "stm32_status.h"
#include "terminal.h"

/// @brief   Задержка старта основной системы в миллисекундах.
#define MAIN_SYSTEM_START_DELAY_MS 500

/// @brief   Количество хабов
#define HUBS_CNT 4

/// @brief   Размер буфера передачи UART2 для DMA
#define UART2_TX_BUFFER_SIZE 200

/// @brief   Размер буфера передачи UART1 для DMA
#define UART1_TX_BUFFER_SIZE 200

#define u8_t  uint8_t
#define u16_t uint16_t
#define u32_t uint32_t
#define u64_t uint64_t
#define bool uint8_t
#define true 1
#define false 0

extern const char strVersionDT[];

extern u8_t Channel_No;

void Error_Handler(void);


#define GPO_12V_en_Pin GPIO_PIN_2
#define GPO_12V_en_GPIO_Port GPIOE
#define GPIO_aI2C_SDA_Pin GPIO_PIN_5
#define GPIO_aI2C_SDA_GPIO_Port GPIOE
#define GPO_aI2C_SCL_Pin GPIO_PIN_6
#define GPO_aI2C_SCL_GPIO_Port GPIOE
#define GPI_12v_in_Pin GPIO_PIN_0
#define GPI_12v_in_GPIO_Port GPIOC
#define ar429_miso_Pin GPIO_PIN_2
#define ar429_miso_GPIO_Port GPIOC
#define ar429_mosi_Pin GPIO_PIN_3
#define ar429_mosi_GPIO_Port GPIOC
#define GPO_nvm_ncs_Pin GPIO_PIN_4
#define GPO_nvm_ncs_GPIO_Port GPIOA
#define nvm_sck_Pin GPIO_PIN_5
#define nvm_sck_GPIO_Port GPIOA
#define nvm_miso_Pin GPIO_PIN_6
#define nvm_miso_GPIO_Port GPIOA
#define GPO_heater_Pin GPIO_PIN_11
#define GPO_heater_GPIO_Port GPIOE
#define ar429_sck_Pin GPIO_PIN_13
#define ar429_sck_GPIO_Port GPIOB
#define GPO_blower_Pin GPIO_PIN_15
#define GPO_blower_GPIO_Port GPIOB
#define mcu2pc_Pin GPIO_PIN_8
#define mcu2pc_GPIO_Port GPIOD
#define pc2mcu_Pin GPIO_PIN_9
#define pc2mcu_GPIO_Port GPIOD
#define bc_scl_Pin GPIO_PIN_12
#define bc_scl_GPIO_Port GPIOD
#define bc_sda_Pin GPIO_PIN_13
#define bc_sda_GPIO_Port GPIOD
#define GPO_hub3_cs_Pin GPIO_PIN_6
#define GPO_hub3_cs_GPIO_Port GPIOC
#define GPO_hub4_cs_Pin GPIO_PIN_7
#define GPO_hub4_cs_GPIO_Port GPIOC
#define GPO_hub1_cs_Pin GPIO_PIN_8
#define GPO_hub1_cs_GPIO_Port GPIOC
#define GPO_hub2_cs_Pin GPIO_PIN_9
#define GPO_hub2_cs_GPIO_Port GPIOC
#define mcu2pca_Pin GPIO_PIN_9
#define mcu2pca_GPIO_Port GPIOA
#define pca2mcu_Pin GPIO_PIN_10
#define pca2mcu_GPIO_Port GPIOA
#define hub_sck_Pin GPIO_PIN_10
#define hub_sck_GPIO_Port GPIOC
#define hub_miso_Pin GPIO_PIN_11
#define hub_miso_GPIO_Port GPIOC
#define hub_mosi_Pin GPIO_PIN_12
#define hub_mosi_GPIO_Port GPIOC
#define GPO_ar429_rstn_Pin GPIO_PIN_0
#define GPO_ar429_rstn_GPIO_Port GPIOD
#define GPO_ar429_run_Pin GPIO_PIN_1
#define GPO_ar429_run_GPIO_Port GPIOD
#define GPO_ar429_ack_Pin GPIO_PIN_2
#define GPO_ar429_ack_GPIO_Port GPIOD
#define GPI_ar429_intn_Pin GPIO_PIN_3
#define GPI_ar429_intn_GPIO_Port GPIOD
#define GPI_ar429_ready_Pin GPIO_PIN_4
#define GPI_ar429_ready_GPIO_Port GPIOD
#define dbg_uart_tx_Pin GPIO_PIN_5
#define dbg_uart_tx_GPIO_Port GPIOD
#define dbg_uart_rx_Pin GPIO_PIN_6
#define dbg_uart_rx_GPIO_Port GPIOD
#define GPO_28Vcam_en_Pin GPIO_PIN_7
#define GPO_28Vcam_en_GPIO_Port GPIOD
#define GPO_ar429_csn_Pin GPIO_PIN_4
#define GPO_ar429_csn_GPIO_Port GPIOB
#define nvm_mosi_Pin GPIO_PIN_5
#define nvm_mosi_GPIO_Port GPIOB
#define ac_scl_Pin GPIO_PIN_6
#define ac_scl_GPIO_Port GPIOB
#define ac_sda_Pin GPIO_PIN_7
#define ac_sda_GPIO_Port GPIOB

#define CHANNELS_TOTAL	6
#define BASE_FORMAT1	12

#define XAE21_TIMEOUT	2500 /* таймаут поступления пакетов от ХАЭ-21, их периодичность по ТЗ 100..200 мс, по ТЗ при непоступлении 3 пакетов (плюс 100мс на запас) считаем ХАЭ неисправным */
#define SVR_TIMEOUT		3000 /* таймаут поступления пакетов от сервера, их периодичность по ТЗ 1000 */

enum A300_Matrix_e {	// ARINC packet Matrix
	AMX_NC1					= 0x00,
	AMX_FUNCTIONAL_TEST		= 0x01,
	AMX_NC2					= 0x02,
	AMX_NORMAL_OPERATION	= 0x03,
};

enum System_Status_e {
	SYS_NC               	= 0x00,
	SYS_PARTIALLY_FAULT     = 0x01,
	SYS_NC2  				= 0x02,
	SYS_FAULT				= 0x03,
};

//enum SrvRouter_Status_e {
//	SRS_NORMAL_OPERATION	= 0x00,
//	SRS_PARTIALLY_FAULT		= 0x01,
//	SRS_NC					= 0x02,
//	SRS_FAULT				= 0x03,
//};
//
//enum Storage_Status_e {
//	STS_NORMAL_OPERATION	= 0x00,
//	STS_PARTIALLY_FAULT		= 0x01,
//	STS_NC					= 0x02,
//	STS_FAULT				= 0x03,
//};

typedef struct {
	enum System_Status_e 	system_stat;
	char 	srvrouter_stat;
//	bool 					 srvrouter_fault;
	char 	storage_stat;
//	bool 					storage_fault;
	bool 					XAE21_fault;
	bool 					cam_switch_fault;
} Status_t;


#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
