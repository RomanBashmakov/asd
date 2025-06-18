/*
 * utils.h
 *
 *  Created on: 18 дек. 2019 г.
 *      Author: zhuchenkovao
 */

#ifndef INC_UTILS_H_
#define INC_UTILS_H_


#include <stdint.h>


// utils:
uint8_t  strToBCD (char *str, uint8_t max);
uint8_t  itoBCD (uint8_t value);

char getTimeFromStr (char *str, uint8_t *h, uint8_t *m, uint8_t *s);
char getDateFromStr (char *str, uint8_t *y, uint8_t *m, uint8_t *d);

int atoi_base10(char* s, char *ok);
uint32_t atoi_base16(char* s, char *ok);
int ip4addr_aton_custom(const char *cp, uint32_t *addr);

void __ddelayT(uint32_t t);    // "dirty" delay in ticks

#endif /* INC_UTILS_H_ */
