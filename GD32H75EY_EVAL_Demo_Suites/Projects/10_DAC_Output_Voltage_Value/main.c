/*!
    \file    main.c
    \brief   DAC output voltage value demo

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
#include <stdio.h>
#include "gd32h75ey_eval.h"

/* configure RCU peripheral */
void rcu_config(void);
/* configure GPIO peripheral */
void gpio_config(void);
/* configure DAC peripheral */
void dac_config(void);
/* led init function */
void led_init(void);
/* led flash function */
void led_flash(uint16_t times);
/* enable the CPU Chache */
static void cache_enable(void);

void mpu_config(void);

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
    mpu_config();
    /* configure systick */
    systick_config();

    /* configure the RCU, GPIO, DAC and Systick */
    rcu_config();
    gpio_config();
    dac_config();
    systick_config();

    /* led init function */
    led_init();
    /* led flash function */
    led_flash(1);

    while(1) {
    }
}

/*!
    \brief      enable the CPU cache
    \param[in]  none
    \param[out] none
    \retval     none
*/
void cache_enable(void)
{
    /* enable i-cache */
    SCB_EnableICache();
    /* enable d-cache */
    SCB_EnableDCache();
}

/*!
    \brief      configure the MPU attributes
    \param[in]  none
    \param[out] none
    \retval     none
*/
void mpu_config(void)
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
    \brief      configure RCU peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void rcu_config(void)
{
    /* enable the clock of peripherals */
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_DAC);
}

/*!
    \brief      configure GPIO peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void gpio_config(void)
{
    /* configure PA5 as DAC output */
    gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_5);
}

/*!
    \brief      configure DAC peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void dac_config(void)
{
    /* initialize DAC */
    dac_deinit(DAC0);
    /* DAC trigger disable */
    dac_trigger_disable(DAC0,DAC_OUT1);
    /* DAC wave mode config */
    dac_wave_mode_config(DAC0,DAC_OUT1, DAC_WAVE_DISABLE);

    /* DAC enable */
    dac_enable(DAC0,DAC_OUT1);
    dac_data_set(DAC0,DAC_OUT1, DAC_ALIGN_12B_L, 0x7FF0);
}

/*!
    \brief      led init function
    \param[in]  none
    \param[out] none
    \retval     none
*/
void led_init(void)
{
    gd_eval_led_init(LED3);
    gd_eval_led_init(LED4);
}

/*!
    \brief      led flash function
    \param[in]  times: times to flash the LEDs
    \param[out] none
    \retval     none
*/
void led_flash(uint16_t times)
{
    uint16_t i;

    for (i = 0;i < times;i++){
        /* delay 500 ms */
        delay_1ms(500);

        /* turn on LEDs */
        gd_eval_led_on(LED3);
        gd_eval_led_on(LED4);

        /* delay 500 ms */
        delay_1ms(500);

        /* turn off LEDs */
        gd_eval_led_off(LED3);
        gd_eval_led_off(LED4);
    }
}
