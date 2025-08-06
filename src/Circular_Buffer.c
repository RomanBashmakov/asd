/// @file       Circular_Buffer.c
/// @author     Башмаков Р.А. bashmakovra@raitibor.ru
/// @brief      См. Circular_Buffer.h

#include "Circular_Buffer.h"

void Circular_Buffer_Init(Circular_Buffer_Struct *Buffer_Struct_Ptr,
                               uint8_t                            *Data_Ptr,
                               uint16_t                            Buffer_Size)
{
    Buffer_Struct_Ptr->Buffer_Size   = Buffer_Size;
    Buffer_Struct_Ptr->Data_Ptr      = Data_Ptr;
    Buffer_Struct_Ptr->Begin_Index   = 0;
    Buffer_Struct_Ptr->End_Index     = 0;
    Buffer_Struct_Ptr->Element_Count = 0;
}

void Circular_Buffer_Push(Circular_Buffer_Struct *Buffer_Struct_Ptr,
                               const uint8_t                      *Data_Ptr,
                               uint16_t                            Data_Size)
{
    for (uint16_t i = 0; i < Data_Size; i++)
    {
        Buffer_Struct_Ptr->Data_Ptr[Buffer_Struct_Ptr->End_Index] = Data_Ptr[i];
        Buffer_Struct_Ptr->End_Index++;
        if (Buffer_Struct_Ptr->End_Index >= Buffer_Struct_Ptr->Buffer_Size)
            Buffer_Struct_Ptr->End_Index = 0;

        if (Buffer_Struct_Ptr->Element_Count < Buffer_Struct_Ptr->Buffer_Size)
            Buffer_Struct_Ptr->Element_Count++;
        else
        {
            Buffer_Struct_Ptr->Begin_Index++;
            if (Buffer_Struct_Ptr->Begin_Index >= Buffer_Struct_Ptr->Buffer_Size)
                Buffer_Struct_Ptr->Begin_Index = 0;
        }
    }
}

void Circular_Buffer_Pop(Circular_Buffer_Struct *Buffer_Struct_Ptr,
                              uint8_t                            *Data_Ptr,
                              uint16_t                            Data_Size)
{
    uint16_t Index = 0;

    if (Data_Size > Buffer_Struct_Ptr->Element_Count)
    {
        Data_Size = Buffer_Struct_Ptr->Element_Count;
    }

    while (Index < Data_Size)
    {
        if (Data_Ptr != 0)
        {
            Data_Ptr[Index] = Buffer_Struct_Ptr->Data_Ptr[Buffer_Struct_Ptr->Begin_Index];
        }

        Index++;

        Buffer_Struct_Ptr->Begin_Index++;

        Buffer_Struct_Ptr->Element_Count--;

        if (Buffer_Struct_Ptr->Begin_Index >= Buffer_Struct_Ptr->Buffer_Size)
        {
            Buffer_Struct_Ptr->Begin_Index = 0;
        }
    }
}

void Circular_Buffer_Reset(Circular_Buffer_Struct *Buffer_Struct_Ptr)
{
    Buffer_Struct_Ptr->Begin_Index   = 0;
    Buffer_Struct_Ptr->End_Index     = 0;
    Buffer_Struct_Ptr->Element_Count = 0;
}