/*!
    \file    main.c
    \brief   ADC temperature vrefint demo

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

#define ADC_TEMP_CALIBRATION_VALUE_25                REG16(0x1FF0F7C0)
#define ADC_TEMP_CALIBRATION_VALUE_MINUS40           REG16(0x1FF0F7C2)

static float temperature;
static float vref_value;
static float avg_slope;
int32_t temperature_value_25;
int32_t temperature_value_minus40;

/* configure the different system clocks */
void rcu_config(void);
/* configure the ADC peripheral */
void adc_config(void);
/* configure the MPU attributes */
void mpu_config(void);

/*!
    \brief      enable the CPU Chache
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void cache_enable(void)
{
    /* Enable I-Cache */
    SCB_EnableICache();

    /* Enable D-Cache */
    SCB_EnableDCache();
}

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
    /* peripheral clocks configuration */
    rcu_config();
    /* configure systick */
    systick_config();
    /* ADC configuration */
    adc_config();
    /* USART configuration */
    gd_eval_com_init(EVAL_COM);
    
    temperature_value_25 = ADC_TEMP_CALIBRATION_VALUE_25 & 0x0FFF;
    temperature_value_minus40 =ADC_TEMP_CALIBRATION_VALUE_MINUS40 & 0x0FFF;
    avg_slope = -(temperature_value_25 - temperature_value_minus40) / (25.0f + 40.0f);

    while(1) {
        /* ADC software trigger enable */
        adc_software_trigger_enable(ADC2, ADC_INSERTED_CHANNEL);
        /* delay a time in milliseconds */
        delay_1ms(1000);

        /* value convert */
        temperature = (temperature_value_25 - (int32_t)ADC_IDATA0(ADC2))*3.3f/4096/avg_slope*1000 + 25 ;
        vref_value = (ADC_IDATA1(ADC2) * 3.3f / 4096);

        /* value print */
        printf(" the temperature data is %2.0f degrees Celsius\r\n", temperature);
        printf(" the reference voltage data is %5.3fV \r\n", vref_value);
        printf(" \r\n");
    }
}

/*!
    \brief      configure the different system clocks
    \param[in]  none
    \param[out] none
    \retval     none
*/
void rcu_config(void)
{
    /* enable ADC clock */
    rcu_periph_clock_enable(RCU_ADC2);
}

/*!
    \brief      configure the ADC peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void adc_config(void)
{
    /* reset ADC */
    adc_deinit(ADC2);
    /* ADC clock config */
    adc_clock_config(ADC2, ADC_CLK_SYNC_HCLK_DIV6);
    /* ADC contineous function enable */
    adc_special_function_config(ADC2, ADC_CONTINUOUS_MODE, DISABLE);
    /* ADC scan mode enable */
    adc_special_function_config(ADC2, ADC_SCAN_MODE, ENABLE);
    /* ADC resolution config */
    adc_resolution_config(ADC2, ADC_RESOLUTION_12B);
    /* ADC data alignment config */
    adc_data_alignment_config(ADC2, ADC_DATAALIGN_RIGHT);

    /* ADC channel length config */
    adc_channel_length_config(ADC2, ADC_INSERTED_CHANNEL, 2);

    /* ADC temperature sensor channel config */
    adc_inserted_channel_config(ADC2, 0, ADC_CHANNEL_18, 480);
    /* ADC internal reference voltage channel config */
    adc_inserted_channel_config(ADC2, 1, ADC_CHANNEL_19, 480);

    /* enable ADC temperature channel */
    adc_internal_channel_config(ADC_CHANNEL_INTERNAL_TEMPSENSOR, ENABLE);
    /* enable internal reference voltage channel */
    adc_internal_channel_config(ADC_CHANNEL_INTERNAL_VREFINT, ENABLE);

    /* ADC trigger config */
    adc_external_trigger_config(ADC2, ADC_INSERTED_CHANNEL, EXTERNAL_TRIGGER_DISABLE);

    /* enable ADC interface */
    adc_enable(ADC2);
    /* wait for ADC stability */
    delay_1ms(1);
    /* ADC calibration mode config */
    adc_calibration_mode_config(ADC2, ADC_CALIBRATION_OFFSET);
    /* ADC calibration number config */
    adc_calibration_number(ADC2, ADC_CALIBRATION_NUM1);
    /* ADC calibration and reset calibration */
    adc_calibration_enable(ADC2);

    /* ADC software trigger enable */
    adc_software_trigger_enable(ADC2, ADC_INSERTED_CHANNEL);
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
