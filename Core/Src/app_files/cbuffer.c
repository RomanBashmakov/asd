

#include "cbuffer.h"


// кольцевой буффер (портировано из c++)

// конструктор
void CBUFF_init(TCbuffer* buff, char *ptr, int size) { 
    buff->size = size;
    buff->data = ptr;
    buff->begin = 0;
    buff->end = 0;
    buff->count = 0;
}


void CBUFF_pushNBytes(TCbuffer* buff, const char* ptr, int n)
{
    for(int i = 0; i < n ; i++){
        buff->data[buff->end] = ptr[i];
        buff->end++;
        if (buff->end >= buff->size){
            buff->end = 0;
        }
        if (buff->count < buff->size){
            buff->count++;
        }else{
            buff->begin++;
            if (buff->begin >= buff->size){
                buff->begin = 0;
            }
        }
    }
}


void CBUFF_getFirstNBytes(TCbuffer* buff, char* ptr, int n)
{
    if (n > buff->count) n = buff->count;
    int k = 0;
    while (k < n){
        if (ptr != 0) ptr[k] = buff->data[buff->begin];
        k++;
        buff->begin++;
        buff->count--;
        if(buff->begin >= buff->size) buff->begin = 0;
    }

}

void CBUFF_reset(TCbuffer *buff)
{
    buff->begin = 0;
    buff->end = 0;
    buff->count = 0;
}

