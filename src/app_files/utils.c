/*
 * utils.c
 *
 *  Created on: 18 дек. 2019 г.
 *      Author: zhuchenkovao
 */
#include <ctype.h>




#include "utils.h"


// BCD convert functions:
// BCD to int
uint8_t  strToBCD (char *str, uint8_t max)
{
  uint8_t ret = 0;

  if(str[0] >= '0' && str[0] <= '9')  // 0-9
  {
   ret = ( str[0] - '0')  << 4 ;
  }
  else
  {
   ret = 0xFF;
  }

  if(str[1] >= '0' && str[1] <= '9'){
    ret |= ( str[1] - '0');

  } else {
    ret = 0xFF;
  }

  if (ret > max) ret = 0xFF;

  return ret;

}


//=================================================================================
// int to BCD
uint8_t  itoBCD (uint8_t value)
{
  uint8_t ret = 0;

  if (value < 100){
    ret = ( (value/10*16) + (value%10) );
  } ret = 0;

  return ret;
}



//=================================================================================
char getTimeFromStr (char *str, uint8_t *h, uint8_t *m, uint8_t *s)
{

  char ret = 0;
  if (str[2] == ':'){
    *h = strToBCD(str, 0x24);
    if (*h == 0xFF ){
      ret = -1;
    }

  } else ret = -1;

  if (str[5] == ':'){
    *m = strToBCD(&str[3], 0x59);
    if (*m == 0xFF ){
      ret = -1;
    }

    *s = strToBCD(&str[6], 0x59);
    if (*s == 0xFF ){
      ret = -1;
    }

  } else ret = -1;



  return ret;


}

//-------------------------------------------------------------------------------
char getDateFromStr (char *str, uint8_t *y, uint8_t *m, uint8_t *d)
{

  char ret = 0;
  if (str[2] == '.'){
    *d = strToBCD(str, 0x31);
    if (*d == 0xFF ){
      ret = -1;
    }

  } else ret = -1;

  if (str[5] == '.'){
    *m = strToBCD(&str[3], 0x12);
    if (*m == 0xFF ){
      ret = -1;
    }

    *y = strToBCD(&str[6], 0x99);
    if (*y == 0xFF ){
      ret = -1;
    }

  } else ret = -1;



  return ret;


}

//-------------------------------------------------------------------------------------
int atoi_base10(char* s, char *ok)
{
    int n = 0, sign;
    if (*s == '-') {
      sign = -1;
      s++;
    }
    else sign = 1;

    while( *s >= '0' && *s <= '9' ) {
        n *= 10;
        n += *s++;
        n -= '0';
    }

    if (*s == 0) *ok = 1;
    else *ok = 0;


    return (n * sign);

}

//-------------------------------------------------------------------------------------
uint32_t atoi_base16(char* s, char *ok)
{
    int n = 0, correct = 0;
    /*if (*s == '-') {
      sign = -1;
      s++;
    }
    else sign = 1;*/

    do {
      if (*s >= '0' && *s <= '9'){
        correct = 1;
        n *= 16;
        n += *s++;
        n -= '0';
      } else if (*s >= 'a' && *s <= 'f') {
        correct = 1;
        n *= 16;
        n += *s++;
        n += 10;
        n -= 'a';

      } else if (*s >= 'A' && *s <= 'F') {
        correct = 1;
        n *= 16;
        n += *s++;
        n += 10;
        n -= 'A';

      } else {
        correct = 0;
      }

    } while (correct);

    if (*s == 0) *ok = 1;
    else *ok = 0;


    return n;

}

//--------------------------------------------------------
// модифицированная функция конвертации ip адреса из строки в число
//int ip4addr_aton_custom(const char *cp, uint32_t *addr)
//{
//  uint32_t val;
//  uint8_t base;
//  char c;
//  uint32_t parts[4];
//  uint32_t *pp = parts;
//
//  c = *cp;
//  for (;;) {
//    /*
//     * Collect number up to ``.''.
//     * Values are specified as for C:
//     * 0x=hex, 0=octal, 1-9=decimal.
//     */
//    if (!isdigit(c)) {
//      return 0;
//    }
//    val = 0;
//    base = 10;
//    if (c == '0') {
//      c = *++cp;
//      if (c == 'x' || c == 'X') {
//        base = 16;
//        c = *++cp;
//      } else {
//        base = 8;
//      }
//    }
//    for (;;) {
//      if (isdigit(c)) {
//        val = (val * base) + (uint32_t)(c - '0');
//        c = *++cp;
//      } else if (base == 16 && isxdigit(c)) {
//        val = (val << 4) | (uint32_t)(c + 10 - (islower(c) ? 'a' : 'A'));
//        c = *++cp;
//      } else {
//        break;
//      }
//    }
//    if (c == '.') {
//      /*
//       * Internet format:
//       *  a.b.c.d
//       *  a.b.c   (with c treated as 16 bits)
//       *  a.b (with b treated as 24 bits)
//       */
//      if (pp >= parts + 3) {
//        return 0;
//      }
//      *pp++ = val;
//      c = *++cp;
//    } else {
//      break;
//    }
//  }
//  /*
//   * Check for trailing characters.
//   */
//  if (c != '\0' && !isspace(c)) {
//    return 0;
//  }
//  /*
//   * Concoct the address according to
//   * the number of parts specified.
//   */
//  switch (pp - parts + 1) {
//
//  case 0:
//    return 0;       /* initial nondigit */
//
//  case 1:             /* a -- 32 bits */
//    return 0;       //
//
//  case 2:             /* a.b -- 8.24 bits */
//    return 0;       //
//
//  case 3:             /* a.b.c -- 8.8.16 bits */
//    return 0;       //
//
//  case 4:             /* a.b.c.d -- 8.8.8.8 bits */
//    if (val > 0xff) {
//      return 0;
//    }
//    if ((parts[0] > 0xff) || (parts[1] > 0xff) || (parts[2] > 0xff)) {
//      return 0;
//    }
//    val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
//    break;
//  default:
//    LWIP_ASSERT("unhandled", 0);
//    break;
//  }
//  if (addr) {
//    *addr = lwip_htonl(val);
//  }
//  return 1;
//}

//--------------------------------------------------------
void __ddelayT(uint32_t t)    // "dirty" delay in ticks
{
    for(int32_t k = 0; k < t; k++) asm("nop");//__NOP();
}
//--------------------------------------------------------
