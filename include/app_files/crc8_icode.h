/*
 * crc8_icode.h
 *
 *  Created on: 3 дек. 2019 г.
 *      Author: zhuchenkovao
 */

#ifndef INC_CRC8_ICODE_H_
#define INC_CRC8_ICODE_H_


#include <stdint.h>

// рассчет crc 8 бит, алгоритм I-CODE параметры алгоритма:
// (poly = 0x1D, init = 0xFD, refIn = false, refOut = false, xorOut = 0x00)



uint8_t crc8CalcICode(uint8_t *data, int size);



#endif /* INC_CRC8_ICODE_H_ */
