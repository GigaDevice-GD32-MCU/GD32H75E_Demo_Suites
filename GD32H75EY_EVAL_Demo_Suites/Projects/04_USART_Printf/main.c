/*!
    \file    main.c
    \brief   USART printf demo

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
#include "gd32h75ey_eval.h"
#include "systick.h"
#include <stdio.h>

static void test_status_led_init(void);
static void led_flash(uint8_t times);
static void usart_config(void);
void cache_enable(void);
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
    /* initialize the LEDs */
    test_status_led_init();

    /* configure systick */
    systick_config();

    /* flash the LEDs for 2 time */
    led_flash(2);

    /* configure USART2 */
    usart_config();

    /* configure TAMPER key */
    gd_eval_key_init(KEY_TAMPER, KEY_MODE_GPIO);

    /* output a message on hyperterminal using printf function */
    printf("\r\n USART printf example: please press the Tamper key \r\n");

    /* wait for completion of USART transmission */
    while(RESET == usart_flag_get(EVAL_COM, USART_FLAG_TC)) {
    }

    while(1) {
        /* check if the tamper key is pressed */
        if(RESET == gd_eval_key_state_get(KEY_TAMPER)) {
            delay_1ms(50);

            if(RESET == gd_eval_key_state_get(KEY_TAMPER)) {
                delay_1ms(50);

                if(RESET == gd_eval_key_state_get(KEY_TAMPER)) {
                    /* turn on LED3 */
                    gd_eval_led_on(LED3);
                    /* output a message on hyperterminal using printf function */
                    printf("\r\n USART printf example \r\n");

                    /* wait for completion of USART transmission */
                    while(RESET == usart_flag_get(EVAL_COM, USART_FLAG_TC)) {
                    }
                } else {
                    /* turn off LED3 */
                    gd_eval_led_off(LED3);
                }
            } else {
                /* turn off LED3 */
                gd_eval_led_off(LED3);
            }
        } else {
            /* turn off LED3 */
            gd_eval_led_off(LED3);
        }
    }
}
/*!
    \brief      test status led initialize
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void test_status_led_init(void)
{
    gd_eval_led_init(LED3);
    gd_eval_led_init(LED4);
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
        /* delay 400 ms */
        delay_1ms(400);

        /* turn on LEDs */
        gd_eval_led_on(LED3);
        gd_eval_led_on(LED4);

        /* delay 400 ms */
        delay_1ms(400);
        /* turn off LEDs */
        gd_eval_led_off(LED3);
        gd_eval_led_off(LED4);
    }
}

/*!
    \brief      usart configure
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void usart_config(void)
{
    /* enable GPIO clock */
    rcu_periph_clock_enable(EVAL_COM_GPIO_CLK);

    /* enable USART clock */
    rcu_periph_clock_enable(EVAL_COM_CLK);

    /* connect port to USART2 TX */
    gpio_af_set(EVAL_COM_GPIO_PORT, EVAL_COM_AF, EVAL_COM_TX_PIN);

    /* connect port to USART2 RX */
    gpio_af_set(EVAL_COM_GPIO_PORT, EVAL_COM_AF, EVAL_COM_RX_PIN);

    /* configure USART TX as alternate function push-pull */
    gpio_mode_set(EVAL_COM_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, EVAL_COM_TX_PIN);
    gpio_output_options_set(EVAL_COM_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, EVAL_COM_TX_PIN);

    /* configure USART RX as alternate function push-pull */
    gpio_mode_set(EVAL_COM_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, EVAL_COM_RX_PIN);
    gpio_output_options_set(EVAL_COM_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, EVAL_COM_RX_PIN);

    /* USART configure */
    usart_deinit(EVAL_COM);
    usart_word_length_set(EVAL_COM, USART_WL_8BIT);
    usart_stop_bit_set(EVAL_COM, USART_STB_1BIT);
    usart_parity_config(EVAL_COM, USART_PM_NONE);
    usart_baudrate_set(EVAL_COM, 115200U);
    usart_receive_config(EVAL_COM, USART_RECEIVE_ENABLE);
    usart_transmit_config(EVAL_COM, USART_TRANSMIT_ENABLE);
    usart_enable(EVAL_COM);
}

/*!
    \brief      toggle the led every 500ms
    \param[in]  none
    \param[out] none
    \retval     none
*/
void led_spark(void)
{
    static __IO uint32_t timingdelaylocal = 0U;

    if(timingdelaylocal) {

        if(timingdelaylocal < 500U) {
            gd_eval_led_on(LED3);
            gd_eval_led_on(LED4);
        } else {
            gd_eval_led_off(LED3);
            gd_eval_led_off(LED4);
        }

        timingdelaylocal--;
    } else {
        timingdelaylocal = 1000U;
    }
}

/*!
    \brief      enable the CPU chache
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
