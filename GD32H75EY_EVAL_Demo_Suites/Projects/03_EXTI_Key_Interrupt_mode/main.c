/*!
    \file    main.c
    \brief   EXTI key interrupt demo

    \version 2026-02-26, V1.1.0, demo for GD32H75E
*/

/*
    Copyright (c) 2026, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

#include "gd32h75e.h"
#include "systick.h"

/* enable the CPU cache */
static void cache_enable(void);
/* configure the MPU attributes */
static void mpu_config(void);
/* flash the LED for test */
static void led_flash(uint8_t times);

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    /* enable the CPU cache */
    cache_enable();
    /* configure the MPU */
    mpu_config();
    /* systick configuration */
    systick_config();

    /* enable the LED3 GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOC);
    /* configure LED3 GPIO port */
    gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_3);
    gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_3);
    /* reset LED3 GPIO pin */
    gpio_bit_reset(GPIOC, GPIO_PIN_3);

    /* flash the LED for test */
    led_flash(1);

    /* enable the Tamper key GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_SYSCFG);

    /* configure key pin as input */
    gpio_mode_set(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_13);

    /* enable and set key EXTI interrupt priority */
    nvic_irq_enable(EXTI10_15_IRQn, 2U, 0U);
    /* connect key EXTI line to key GPIO pin */
    syscfg_exti_line_config(EXTI_SOURCE_GPIOC, EXTI_SOURCE_PIN13);
    /* configure key EXTI line */
    exti_init(EXTI_13, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(EXTI_13);

    while(1) {
    }
}

/*!
    \brief      enable the CPU cache
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void cache_enable(void)
{
    /* enable I-cache */
    SCB_EnableICache();

    /* enable D-cache */
    SCB_EnableDCache();
}

/*!
    \brief      configure the MPU attributes
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void mpu_config(void)
{
    mpu_region_init_struct mpu_init_struct;
    mpu_region_struct_para_init(&mpu_init_struct);

    /* disable the MPU */
    ARM_MPU_Disable();
    ARM_MPU_SetRegion(0, 0);

    /* configure the MPU attributes for the entire 4GB area, Reserved, no access */
    /* This configuration is highly recommended to prevent Speculative Prefetching of external memory, 
       which may cause CPU read locks and even system errors */
    mpu_init_struct.region_base_address  = 0x0;
    mpu_init_struct.region_size          = MPU_REGION_SIZE_4GB;
    mpu_init_struct.access_permission    = MPU_AP_NO_ACCESS;
    mpu_init_struct.access_bufferable    = MPU_ACCESS_NON_BUFFERABLE;
    mpu_init_struct.access_cacheable     = MPU_ACCESS_NON_CACHEABLE;
    mpu_init_struct.access_shareable     = MPU_ACCESS_SHAREABLE;
    mpu_init_struct.region_number        = MPU_REGION_NUMBER0;
    mpu_init_struct.subregion_disable    = 0x87;
    mpu_init_struct.instruction_exec     = MPU_INSTRUCTION_EXEC_NOT_PERMIT;
    mpu_init_struct.tex_type             = MPU_TEX_TYPE0;
    mpu_region_config(&mpu_init_struct);
    mpu_region_enable();

    /* enable the MPU */
    ARM_MPU_Enable(MPU_MODE_PRIV_DEFAULT);
}

/*!
    \brief      flash the LED for test
    \param[in]  times: times to flash the LEDs
    \param[out] none
    \retval     none
*/
static void led_flash(uint8_t times)
{
    uint8_t i;

    for(i = 0; i < times; i++) {
        /* delay 500 ms */
        delay_1ms(500);
        /* turn on the LED */
        gpio_bit_set(GPIOC, GPIO_PIN_3);

        /* delay 500 ms */
        delay_1ms(500);
        /* turn off the LED */
        gpio_bit_reset(GPIOC, GPIO_PIN_3);
    }
}
