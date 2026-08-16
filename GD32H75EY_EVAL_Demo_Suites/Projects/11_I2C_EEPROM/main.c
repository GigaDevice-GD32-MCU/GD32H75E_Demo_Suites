/*!
    \file    main.c
    \brief   use the I2C bus to write and read EEPROM

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

#include <stdio.h>
#include "gd32h75ey_eval.h"
#include "systick.h"
#include "i2c.h"
#include "at24cxx.h"

uint8_t count = 0;

void cache_enable(void);
void mpu_config(void);
void led_turn_on(uint8_t led_number);
void led_config(void);

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    /* enable I-Cache and D-Cache */
    cache_enable();
    mpu_config();
    /* configure systick */
    systick_config();

    /* configure LEDs */
    led_config();

    /* configure USART */
    gd_eval_com_init(EVAL_COM);

    printf("I2C-24C02 configured....\n\r");

    /* configure GPIO */
    gpio_config();

    /* configure I2C */
    i2c_config();

    /* initialize EEPROM */
    i2c_eeprom_init();

    printf("\r\nThe I2C is hardware interface ");
    printf("\r\nThe speed is %d", I2C_SPEED);

    if(I2C_OK == i2c_24c02_test()) {
        while(1) {
            /* turn off all LEDs */
            gd_eval_led_off(LED3);
            gd_eval_led_off(LED4);
            /* turn on a LED */
            led_turn_on(count % 2);
            count++;
            if(count >= 2) {
                count = 0;
            }
            delay_1ms(500);
        }
    }
    /* turn on all LEDs */
    gd_eval_led_on(LED3);
    gd_eval_led_on(LED4);

    while(1) {
    }
}

/*!
    \brief      configure the LEDs
    \param[in]  none
    \param[out] none
    \retval     none
*/
void led_config(void)
{
    /* initialize off LED3, LED4 */
    gd_eval_led_init(LED3);
    gd_eval_led_init(LED4);

    /* turn off LED3, LED4 */
    gd_eval_led_off(LED3);
    gd_eval_led_off(LED4);
}

/*!
    \brief      trun on a LED
    \param[in]  led_number
    \param[out] none
    \retval     none
*/
void led_turn_on(uint8_t led_number)
{
    switch(led_number) {
        case 0:
            gd_eval_led_on(LED3);
            break;
        case 1:
            gd_eval_led_on(LED4);
            break;
        default:
            /* turn on all LEDs */
            gd_eval_led_on(LED3);
            gd_eval_led_on(LED4);
            break;
    }
}

/*!
    \brief      enable the CPU Cache
    \param[in]  none
    \param[out] none
    \retval     none
*/
void cache_enable(void)
{
    /* enable I-Cache */
    SCB_EnableICache();

    /* enable D-Cache */
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
