/*
 * sw_mdio.c
 *
 *  Created on: Dec 19, 2020
 *      Author: zhuchenkovao
 */

#include "sw_mdio.h"

void Delay_nop_mdio(unsigned int nCount)
{
    while (nCount--)
    {
      __asm volatile("nop");
    }
}

void init_mdio_in(TMDIO *t)
{
    GPIO_InitTypeDef gpio = {0};

    gpio.Pin = t->mdioPin;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(t->gpio, &gpio);
}

void init_mdio_out(TMDIO *t)
{
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = t->mdioPin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_WritePin(t->gpio, t->mdioPin, GPIO_PIN_RESET);
    HAL_GPIO_Init(t->gpio, &gpio);
}

void init_mdc_in(TMDIO *t)
{
    GPIO_InitTypeDef gpio = {0};

    gpio.Pin = t->mdcPin;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(t->gpio, &gpio);
}

void init_mdc_out(TMDIO *t)
{
    GPIO_InitTypeDef gpio = {0};

    gpio.Pin = t->mdcPin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(t->gpio, &gpio);
}

void mdc_cycle(TMDIO *t)
{
    Delay_nop_mdio(t->delay_nops_count);
    HAL_GPIO_WritePin(t->gpio, t->mdcPin, GPIO_PIN_SET);
    Delay_nop_mdio(t->delay_nops_count * 2);
    HAL_GPIO_WritePin(t->gpio, t->mdcPin, GPIO_PIN_RESET);
    Delay_nop_mdio(t->delay_nops_count);
}

void SW_MDIO_write(TMDIO *t, uint8_t regAddr, uint16_t data)
{
    init_mdio_out(t);
    init_mdc_out(t);

    // send preambula
    HAL_GPIO_WritePin(t->gpio, t->mdioPin, GPIO_PIN_SET);
    Delay_nop_mdio(t->delay_nops_count);
    for (int i = 0; i < 32; i++)
    {
        mdc_cycle(t);
    }

    // start
    HAL_GPIO_WritePin(t->gpio, t->mdioPin, GPIO_PIN_RESET);
    mdc_cycle(t);
    HAL_GPIO_WritePin(t->gpio, t->mdioPin, GPIO_PIN_SET);
    mdc_cycle(t);

    // write
    HAL_GPIO_WritePin(t->gpio, t->mdioPin, GPIO_PIN_RESET);
    mdc_cycle(t);
    HAL_GPIO_WritePin(t->gpio, t->mdioPin, GPIO_PIN_SET);
    mdc_cycle(t);

    HAL_GPIO_WritePin(t->gpio, t->mdioPin, GPIO_PIN_RESET);

    uint8_t byte = t->phyAddr;
    byte <<= 3;
    for (int i = 0; i < 5; i++)
    {
        if (byte & 0x80)
        {
            HAL_GPIO_WritePin(t->gpio, t->mdioPin, GPIO_PIN_SET);
        }
        byte <<= 1;

        mdc_cycle(t);
        HAL_GPIO_WritePin(t->gpio, t->mdioPin, GPIO_PIN_RESET);
    }

    regAddr <<= 3;
    for (int i = 0; i < 5; i++)
    {
        if (regAddr & 0x80)
        {
            HAL_GPIO_WritePin(t->gpio, t->mdioPin, GPIO_PIN_SET);
        }
        regAddr <<= 1;

        mdc_cycle(t);
        HAL_GPIO_WritePin(t->gpio, t->mdioPin, GPIO_PIN_RESET);
    }

    // write 2
    HAL_GPIO_WritePin(t->gpio, t->mdioPin, GPIO_PIN_SET);
    mdc_cycle(t);
    HAL_GPIO_WritePin(t->gpio, t->mdioPin, GPIO_PIN_RESET);
    mdc_cycle(t);

    for (int i = 0; i < 16; i++)
    {
        if (data & 0x8000)
        {
            HAL_GPIO_WritePin(t->gpio, t->mdioPin, GPIO_PIN_SET);
        }
        data <<= 1;

        mdc_cycle(t);
        HAL_GPIO_WritePin(t->gpio, t->mdioPin, GPIO_PIN_RESET);
    }
}

uint16_t SW_MDIO_read(TMDIO *t, uint8_t regAddr)
{
    init_mdio_out(t);
    init_mdc_out(t);

    // send preambula
    HAL_GPIO_WritePin(t->gpio, t->mdioPin, GPIO_PIN_SET);
    Delay_nop_mdio(t->delay_nops_count);
    for (int i = 0; i < 32; i++)
    {
        mdc_cycle(t);
    }

    // start
    HAL_GPIO_WritePin(t->gpio, t->mdioPin, GPIO_PIN_RESET);
    mdc_cycle(t);
    HAL_GPIO_WritePin(t->gpio, t->mdioPin, GPIO_PIN_SET);
    mdc_cycle(t);

    // read
    HAL_GPIO_WritePin(t->gpio, t->mdioPin, GPIO_PIN_SET);
    mdc_cycle(t);
    HAL_GPIO_WritePin(t->gpio, t->mdioPin, GPIO_PIN_RESET);
    mdc_cycle(t);

    HAL_GPIO_WritePin(t->gpio, t->mdioPin, GPIO_PIN_RESET);

    uint8_t byte = t->phyAddr;
    byte <<= 3;
    for (int i = 0; i < 5; i++)
    {
        if (byte & 0x80)
        {
            HAL_GPIO_WritePin(t->gpio, t->mdioPin, GPIO_PIN_SET);
        }
        byte <<= 1;

        mdc_cycle(t);
        HAL_GPIO_WritePin(t->gpio, t->mdioPin, GPIO_PIN_RESET);
    }

    regAddr <<= 3;
    for (int i = 0; i < 5; i++)
    {
        if (regAddr & 0x80)
        {
            HAL_GPIO_WritePin(t->gpio, t->mdioPin, GPIO_PIN_SET);
        }
        regAddr <<= 1;

        mdc_cycle(t);
        HAL_GPIO_WritePin(t->gpio, t->mdioPin, GPIO_PIN_RESET);
    }

    init_mdio_in(t);
    mdc_cycle(t);

    uint16_t data = 0;
    for (int i = 0; i < 16; i++)
    {
        mdc_cycle(t);
        data <<= 1;
        if (HAL_GPIO_ReadPin(t->gpio, t->mdioPin) == 1) data |= 1;
    }

    return data;
}