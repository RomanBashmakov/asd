/*
 * STM32F745VETx Linker Script
 * FLASH: 512KB (0x08000000)
 * RAM:   256KB (0x20000000)
 */

ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 512K
    RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 256K
    CCMRAM (xrw)    : ORIGIN = 0x10000000, LENGTH = 64K
}

SECTIONS
{
    .isr_vector :
    {
        KEEP(*(.isr_vector))
    } > FLASH

    .text :
    {
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        KEEP(*(.init))
        KEEP(*(.fini))
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    .data : AT (_etext)
    {
        _sidata = LOADADDR(.data);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM

    .bss :
    {
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    .heap (NOLOAD):
    {
        . = ALIGN(8);
        _end = .;
        _heap_start = .;
        . = . + 0x1000;
        _heap_end = .;
    } > RAM

    .stack (NOLOAD):
    {
        . = ALIGN(8);
        _estack = ORIGIN(RAM) + LENGTH(RAM);
        _stack_start = _estack - 0x1000;
    } > RAM
}
