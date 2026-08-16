/*!
    \file    esc_exti.c
    \brief   esc exti configuration

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
#include "esc_exti.h"

/*!\brief      configure EXTI interrupt 
    \param[in]  none
    \param[out] none
    \retval     none
*/
void EXTI_IRQ_Configuration(void)
{

    /* enable the GPIOJ Clock */
    rcu_periph_clock_enable(RCU_GPIOJ);
    rcu_periph_clock_enable(RCU_SYSCFG);

    /* configure pin as input floating */
    gpio_mode_set(GPIOJ, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_10);

    /* enable and set key EXTI interrupt priority */
    nvic_irq_enable(EXTI10_15_IRQn, 2U, 0U);

    /* connect  EXTI line to button GPIO pin */
    syscfg_exti_line_config(EXTI_SOURCE_GPIOJ, EXTI_SOURCE_PIN10);

    /* configure EXTI Line10 */
    exti_init(EXTI_10, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(EXTI_10);

}
/*!\brief      configure EXTI SYNC0 
    \param[in]  none
    \param[out] none
    \retval     none
*/
void EXTI_SYNC0_Configuration(void)
{

    /* enable the GPIOC Clock */
    rcu_periph_clock_enable(RCU_GPIOE);
    rcu_periph_clock_enable(RCU_SYSCFG);

    /* configure pin as input floating */
    gpio_mode_set(GPIOE, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_4);

    /* enable and set key EXTI interrupt priority */
    nvic_irq_enable(EXTI4_IRQn, 2U, 0U);

    /* connect  EXTI line to GPIO pin */
    syscfg_exti_line_config(EXTI_SOURCE_GPIOE, EXTI_SOURCE_PIN4);

    /* configure EXTI Line */
    exti_init(EXTI_4, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(EXTI_4);

}

/*!\brief      configure EXTI SYNC1 
    \param[in]  none
    \param[out] none
    \retval     none
*/
void EXTI_SYNC1_Configuration(void)
{
    /* enable the GPIOD Clock */
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_SYSCFG);

    /* configure pin as input floating */
    gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_3);


    /* enable and set key EXTI interrupt priority */
    nvic_irq_enable(EXTI3_IRQn, 2U, 0U);

    /* connect  EXTI line to button GPIO pin */
    syscfg_exti_line_config(EXTI_SOURCE_GPIOB, EXTI_SOURCE_PIN3);

    /* configure EXTI Line0 */
    exti_init(EXTI_3, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(EXTI_3);
}

/*!
    \brief      disable global int
    \param[in]  none
    \param[out] none
    \retval     none
*/
void disable_global_int(void)
{
    NVIC_DisableIRQ(EXTI10_15_IRQn);
    
    NVIC_DisableIRQ(EXTI4_IRQn);

    NVIC_DisableIRQ(EXTI3_IRQn);

    NVIC_DisableIRQ(TIMER2_IRQn);
}

/*!
    \brief      enable global int
    \param[in]  none
    \param[out] none
    \retval     none
*/
void enable_global_int(void)
{
    NVIC_EnableIRQ(EXTI10_15_IRQn);

    NVIC_EnableIRQ(EXTI4_IRQn);

    NVIC_EnableIRQ(EXTI3_IRQn);

    NVIC_EnableIRQ(TIMER2_IRQn);
}