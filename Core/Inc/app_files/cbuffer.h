
#ifndef CBUFFER_H
#define CBUFFER_H


typedef struct Cbuffer{
    char *data;   // указатель на массив данных
    int count;
    int size;
    int begin;
    int end;
  
} TCbuffer;


void CBUFF_init(TCbuffer* buff, char *ptr, int size);
void CBUFF_pushNBytes(TCbuffer* buff, const char* ptr, int n);
void CBUFF_getFirstNBytes(TCbuffer* buff, char* ptr, int n);
void CBUFF_reset(TCbuffer *buff);






#endif