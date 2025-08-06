/// @file       Circular_Buffer.c
/// @author     Башмаков Р.А. bashmakovra@raitibor.ru
/// @brief      Заголовочный файл с объявлениями функций работы с циклическим буфером

#ifndef _CIRCULAR_BUFFER_H_
#define _CIRCULAR_BUFFER_H_

#include <stdint.h>

/// @brief      Структура циклического буфера
typedef struct Circular_Buffer_Struct
{
    uint8_t *Data_Ptr;       ///< Указатель на область данных буфера
    uint16_t Element_Count;  ///< Количество элементов в буфере
    uint16_t Buffer_Size;    ///< Размер буфера (в байтах)
    uint16_t Begin_Index;    ///< Индекс первого элемента
    uint16_t End_Index;      ///< Индекс позиции для следующей записи
} Circular_Buffer_Struct;

/// @brief      Инициализация циклического буфера
/// @details    Данная функция выполняет начальную инициализацию структуры буфера
/// @param[out] Buffer_Struct_Ptr   Указатель на структуру буфера, которая будет инициализирована
/// @param[in]  Data_Ptr            Указатель на область памяти для хранения данных буфера
/// @param[in]  Buffer_Size         Размер буфера (количество байт)
void Circular_Buffer_Init(Circular_Buffer_Struct *Buffer_Struct_Ptr,
                               uint8_t                            *Data_Ptr,
                               uint16_t                            Buffer_Size);

/// @brief      Запись нескольких байт в циклический буфер
/// @details    Данная функция записывает массив байт в буфер
/// @param[in,out] Buffer_Struct_Ptr   Указатель на структуру буфера (значения будут изменены)
/// @param[in]     Data_Ptr            Указатель на массив исходных данных
/// @param[in]     Data_Size           Количество записываемых байт
void Circular_Buffer_Push(Circular_Buffer_Struct *Buffer_Struct_Ptr,
                               const uint8_t                      *Data_Ptr,
                               uint16_t                            Data_Size);

/// @brief      Чтение нескольких байт из начала циклического буфера
/// @details    Данная функция читает n байт из начала буфера
/// @param[in,out] Buffer_Struct_Ptr   Указатель на структуру буфера (значения будут изменены)
/// @param[out]    Data_Ptr            Указатель на буфер для хранения прочитанных данных
/// @param[in]     Data_Size           Количество байт для чтения
void Circular_Buffer_Pop(Circular_Buffer_Struct *Buffer_Struct_Ptr,
                              uint8_t                            *Data_Ptr,
                              uint16_t                            Data_Size);

/// @brief      Сброс состояния циклического буфера
/// @details    Данная функция сбрасывает состояние буфера,
///                 очищая его от всех данных
/// @param[out] Buffer_Struct_Ptr   Указатель на структуру буфера, состояние которой будет сброшено
void Circular_Buffer_Reset(Circular_Buffer_Struct *Buffer_Struct_Ptr);

#endif
