/*!
    \file    esc_timer.c
    \brief   esc timer configuration

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
#include "esc_timer.h"

/*!\brief      configure ESC timer with 1ms interval
    \param[in]  none
    \param[out] none
    \retval     none
*/
void ESC_TIMER_Configuration(void)
{
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER2);

    timer_deinit(TIMER2);

    /* timer configuration */
    timer_struct_para_init(&timer_initpara);
    timer_initpara.prescaler            = 299;
    timer_initpara.alignedmode          = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection     = TIMER_COUNTER_UP;
    timer_initpara.period               = 999;
    timer_initpara.clockdivision        = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter    = 0;
    timer_init(TIMER2, &timer_initpara);

    timer_interrupt_flag_clear(TIMER2, TIMER_INT_FLAG_UP);

    timer_auto_reload_shadow_enable(TIMER2);

    /* timer2 interrupt enable */
    timer_interrupt_enable(TIMER2, TIMER_INT_UP);

    /* timer2 enable counter */
    timer_enable(TIMER2);

#if ECAT_TIMER_INT
    nvic_irq_enable(TIMER2_IRQn, 2U, 2U);
#endif
}
