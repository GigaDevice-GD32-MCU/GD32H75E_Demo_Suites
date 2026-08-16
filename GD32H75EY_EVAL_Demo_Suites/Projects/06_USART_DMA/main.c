/*!
    \file    main.c
    \brief   transmit/receive data using DMA interrupt

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
#include "gd32h75ey_eval.h"
#include <stdio.h>

#define USART2_RDATA_ADDRESS      (&USART_RDATA(USART2))
#define USART2_TDATA_ADDRESS      (&USART_TDATA(USART2))
#define ARRAYNUM(arr_nanme)       (uint32_t)(sizeof(arr_nanme) / sizeof(*(arr_nanme)))
#define BUFFER_SIZE               96

__IO FlagStatus g_transfer_complete = RESET;
__attribute__ ((aligned(32))) uint8_t rxbuffer[32];
__attribute__ ((aligned(32))) uint8_t txbuffer[] = "\n\rUSART DMA interrupt receive and transmit example, please input 32 characters:\n\r";

void com_usart_init(void);
void nvic_config(void);
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

    dma_single_data_parameter_struct dma_init_struct;
    /* enable DMA clock */
    rcu_periph_clock_enable(RCU_DMA0);
    /* enable DMAMUX clock */
    rcu_periph_clock_enable(RCU_DMAMUX);
    /* initialize the com */
    com_usart_init();
    /*configure DMA interrupt*/
    nvic_config();

    /* clean d-cache by address before DMA transmission */
    SCB_CleanDCache_by_Addr((uint32_t*)txbuffer, BUFFER_SIZE);

    /* initialize DMA channel 0 */
    dma_deinit(DMA0, DMA_CH0);
    dma_single_data_para_struct_init(&dma_init_struct);
    dma_init_struct.request      = DMA_REQUEST_USART2_TX;
    dma_init_struct.direction    = DMA_MEMORY_TO_PERIPH;
    dma_init_struct.memory0_addr  = (uint32_t)txbuffer;
    dma_init_struct.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.number       = ARRAYNUM(txbuffer);
    dma_init_struct.periph_addr  = (uint32_t)USART2_TDATA_ADDRESS;
    dma_init_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.priority     = DMA_PRIORITY_ULTRA_HIGH;
    dma_single_data_mode_init(DMA0, DMA_CH0, &dma_init_struct);

    /* configure DMA mode */
    dma_circulation_disable(DMA0, DMA_CH0);
    /* USART DMA enable for transmission and reception */
    usart_dma_transmit_config(USART2, USART_TRANSMIT_DMA_ENABLE);
    /* enable DMA channel 0 transfer complete interrupt */
    dma_interrupt_enable(DMA0, DMA_CH0, DMA_CHXCTL_FTFIE);
    /* enable DMA channel 0 */
    dma_channel_enable(DMA0, DMA_CH0);

    /* initialize DMA channel 1 */
    dma_deinit(DMA0, DMA_CH1);
    dma_single_data_para_struct_init(&dma_init_struct);
    dma_init_struct.request      = DMA_REQUEST_USART2_RX;
    dma_init_struct.direction    = DMA_PERIPH_TO_MEMORY;
    dma_init_struct.memory0_addr  = (uint32_t)rxbuffer;
    dma_init_struct.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.number       = 32;
    dma_init_struct.periph_addr  = (uint32_t)USART2_RDATA_ADDRESS;
    dma_init_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.priority     = DMA_PRIORITY_ULTRA_HIGH;
    dma_single_data_mode_init(DMA0, DMA_CH1, &dma_init_struct);

    /* configure DMA mode */
    dma_circulation_disable(DMA0, DMA_CH1);
    /* USART DMA enable for reception */
    usart_dma_receive_config(USART2, USART_RECEIVE_DMA_ENABLE);
    /* enable DMA channel 1 transfer complete interrupt */
    dma_interrupt_enable(DMA0, DMA_CH1, DMA_CHXCTL_FTFIE);
    /* enable DMA channel 1 */
    dma_channel_enable(DMA0, DMA_CH1);

    /* waiting for the transfer to complete*/
    while(RESET == g_transfer_complete) {
    }

    g_transfer_complete = RESET;
    
    /* waiting for the transfer to complete*/
    while(RESET == g_transfer_complete) {
    }

    /* invalidate d-cache by address after DMA reception*/
    SCB_InvalidateDCache_by_Addr((uint32_t*)rxbuffer, BUFFER_SIZE);
    printf("\n\r%s\n\r", rxbuffer);

    while(1) {
    }
}

/*!
    \brief      configure USART
    \param[in]  none
    \param[out] none
    \retval     none
*/
void com_usart_init(void)
{
    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART2);

    /* enable COM GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOB);

    /* connect port to USART TX */
    gpio_af_set(GPIOB, GPIO_AF_7, GPIO_PIN_10);

    /* connect port to USART RX */
    gpio_af_set(GPIOB, GPIO_AF_7, GPIO_PIN_11);

    /* configure USART TX as alternate function push-pull */
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_10);

    /* configure USART RX as alternate function push-pull */
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_11);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_100_220MHZ, GPIO_PIN_11);

    /* USART configure */
    usart_deinit(USART2);
    usart_baudrate_set(USART2, 115200U);
    usart_receive_config(USART2, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART2, USART_TRANSMIT_ENABLE);

    usart_enable(USART2);
}

/*!
    \brief      configure DMA interrupt
    \param[in]  none
    \param[out] none
    \retval     none
*/
void nvic_config(void)
{
    nvic_irq_enable(DMA0_Channel0_IRQn, 2, 1);
    nvic_irq_enable(DMA0_Channel1_IRQn, 2, 0);
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
