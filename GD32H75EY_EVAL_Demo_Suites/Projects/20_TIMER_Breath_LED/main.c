/*!
    \file    main.c
    \brief   TIMER Breath LED demo

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

/* enable the CPU Chache */
void cache_enable(void);
/* configure the GPIO ports */
void gpio_config(void);
/* configure the TIMER peripheral */
void timer_config(void);
/* configure the MPU attributes */
void mpu_config(void);

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    int16_t i = 0;
    FlagStatus breathe_flag = SET;

    /* enable the CPU Chache */
    cache_enable();
    mpu_config();
    /* configure systick */
    systick_config();
    /* configure the GPIO ports */
    gpio_config();
    /* configure the TIMER peripheral */
    timer_config();

    while(1) {
        /* delay a time in milliseconds */
        delay_1ms(40);
        if(SET == breathe_flag) {
            i = i + 10;
        } else {
            i = i - 10;
        }
        if(500 < i) {
            breathe_flag = RESET;
        }
        if(0 >= i) {
            breathe_flag = SET;
        }
        /* configure TIMER channel output pulse value */
        timer_channel_output_pulse_value_config(TIMER23, TIMER_CH_0, i);
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
    \brief      configure the GPIO ports
    \param[in]  none
    \param[out] none
    \retval     none
  */
void gpio_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOF);

    /*configure PF11(TIMER23 CH0) as alternate function*/
    gpio_mode_set(GPIOF, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11);
    gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_11);
    gpio_af_set(GPIOF, GPIO_AF_14, GPIO_PIN_11);
}

/**
    \brief      configure the TIMER peripheral
    \param[in]  none
    \param[out] none
    \retval     none
  */
void timer_config(void)
{
    /* TIMER23 configuration: generate PWM signals with different duty cycles:
       TIMER0CLK = 300MHz / (299+1) = 1MHz */

    timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER23);
    timer_deinit(TIMER23);

    /* TIMER23 configuration */
    timer_initpara.prescaler         = 299;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 500;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER23, &timer_initpara);

    /* CH0 configuration in PWM mode */
    timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;
    timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
    timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
    timer_channel_output_config(TIMER23, TIMER_CH_0, &timer_ocintpara);

    timer_channel_output_pulse_value_config(TIMER23, TIMER_CH_0, 250);
    timer_channel_output_mode_config(TIMER23, TIMER_CH_0, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER23, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);

    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER23);
    timer_enable(TIMER23);
}
