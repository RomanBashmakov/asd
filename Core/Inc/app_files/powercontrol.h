/*
 * powercontrol.h
 *
 *  Created on: Sep 14, 2021
 *      Author: zhuchenkovao
 */

#ifndef INC_APP_FILES_POWERCONTROL_H_
#define INC_APP_FILES_POWERCONTROL_H_


#include "ina226.h"


extern T_INA226 powerSensA[];
extern T_INA226 powerSensB[];




void POWER_init(I2C_HandleTypeDef  *hi2cA, I2C_HandleTypeDef  *hi2cB);
void POWER_process();








#endif /* INC_APP_FILES_POWERCONTROL_H_ */
