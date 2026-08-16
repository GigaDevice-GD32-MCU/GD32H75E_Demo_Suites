/*!
    \file    main.c
    \brief   CAN network demo

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
#include "string.h"

#define  BUFFER_SIZE    8U

/* global variable declarations */
can_mailbox_descriptor_struct transmit_message[2];
can_mailbox_descriptor_struct mailbox_receive_message;
can_rx_fifo_struct fifo_receive_message;
can_rx_fifo_id_filter_struct fifo_element[8];
FlagStatus can0_mailbox_receive_flag;
FlagStatus can0_fifo_receive_flag;

/* function declarations */
void can_gpio_config(void);
void bsp_board_config(void);
void can_config(void);
void communication_check(void);
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
    uint8_t i = 0U;
    FlagStatus can1_tx_state = RESET;
    FlagStatus can2_tx_state = RESET;

    mpu_config();
    can_struct_para_init(CAN_MDSC_STRUCT, &transmit_message[0]);
    can_struct_para_init(CAN_MDSC_STRUCT, &transmit_message[1]);
    can_struct_para_init(CAN_MDSC_STRUCT, &mailbox_receive_message);
    /* enable the CPU Cache */
    cache_enable();
    /* configure systick */
    systick_config();
    /* configure board */
    bsp_board_config();
    /* configure GPIO */
    can_gpio_config();
    /* initialize CAN and filter */
    can_config();

    printf("\r\n Communication test CAN0, CAN1 and CAN2, please press WAKEUP or TAMPER key to start! \r\n");

    /* fill the data for CAN1 transmit */
    for(i = 0; i < BUFFER_SIZE; i++){
        transmit_message[0].data[i] = 0xA0 + i;
    }
    /* initialize transmit message */
    transmit_message[0].rtr = 0U;
    transmit_message[0].ide = 0U;
    transmit_message[0].code = CAN_MB_TX_STATUS_DATA;
    transmit_message[0].brs = 0U;
    transmit_message[0].fdf = 0U;
    transmit_message[0].prio = 0U;
    transmit_message[0].data_bytes = BUFFER_SIZE;
    transmit_message[0].id = 0x55U;

    /* fill the data for CAN2 transmit */
    for(i = 0; i < BUFFER_SIZE; i++){
        transmit_message[1].data[i] = 0xB0 + i;
    }
    /* initialize transmit message */
    transmit_message[1].rtr = 0U;
    transmit_message[1].ide = 1U;
    transmit_message[1].code = CAN_MB_TX_STATUS_DATA;
    transmit_message[1].brs = 0U;
    transmit_message[1].fdf = 0U;
    transmit_message[1].prio = 0U;
    transmit_message[1].data_bytes = BUFFER_SIZE;
    transmit_message[1].id = 0xAAU;
    
    /* CAN0 maibox 8 receive the data from CAN1 */
    mailbox_receive_message.rtr = 0U;
    mailbox_receive_message.ide = 0U;
    mailbox_receive_message.code = CAN_MB_RX_STATUS_EMPTY;
    /* rx mailbox */
    mailbox_receive_message.id = 0x55U;
    can_mailbox_config(CAN0, 8U, &mailbox_receive_message);

    /* CAN0 FIFO receive the data from CAN2 */
    for(i=0; i<8; i++){
        fifo_element[i].extended_frame = CAN_EXTENDED_FRAME_ACCEPTED;
        fifo_element[i].remote_frame = CAN_DATA_FRAME_ACCEPTED;
        fifo_element[i].id = 0xAAU + i;
    }
    can_rx_fifo_filter_table_config(CAN0, fifo_element);
    can_rx_fifo_clear(CAN0);

    while(1) {
        /* test whether the WAKEUP key is pressed */
        if(1U == gd_eval_key_state_get(KEY_WAKEUP)) {
            delay_1ms(100U);
            if(1U == gd_eval_key_state_get(KEY_WAKEUP)) {
                if((RESET == can2_tx_state) || (SET == can_flag_get(CAN2, CAN_FLAG_MB1))) {
                    can2_tx_state = SET;
                    /* transmit message */
                    can_mailbox_config(CAN2, 1U, &transmit_message[1]);

                    printf("\r\nCAN2 transmit data: \r\n");
                    for(i = 0U; i < transmit_message[1].data_bytes; i++) {
                        printf("%02x ", transmit_message[1].data[i]);
                    }
                }
                /* waiting for the WAKEUP key up */
                while(1U == gd_eval_key_state_get(KEY_WAKEUP));
            }
        }
        /* test whether the TAMPER key is pressed */
        if(0U == gd_eval_key_state_get(KEY_TAMPER)) {
            delay_1ms(100U);
            if(0U == gd_eval_key_state_get(KEY_TAMPER)) {
                if((RESET == can1_tx_state) || (SET == can_flag_get(CAN1, CAN_FLAG_MB0))) {
                    can1_tx_state = SET;
                    /* transmit message */
                    can_mailbox_config(CAN1, 0U, &transmit_message[0]);

                    printf("\r\nCAN1 transmit data: \r\n");
                    for(i = 0U; i < transmit_message[0].data_bytes; i++) {
                        printf("%02x ", transmit_message[0].data[i]);
                    }
                }
                /* waiting for the WAKEUP key up */
                while(0U == gd_eval_key_state_get(KEY_TAMPER));
            }
        }
        communication_check();
    }
}

/*!
    \brief      configure GPIO
    \param[in]  none
    \param[out] none
    \retval     none
*/
void can_gpio_config(void)
{
    /* configure CAN1 and CAN2 clock source */
    rcu_can_clock_config(IDX_CAN0, RCU_CANSRC_APB2);
    rcu_can_clock_config(IDX_CAN1, RCU_CANSRC_APB2);
    rcu_can_clock_config(IDX_CAN2, RCU_CANSRC_APB2);
    /* enable CAN clock */
    rcu_periph_clock_enable(RCU_CAN0);
    rcu_periph_clock_enable(RCU_CAN1);
    rcu_periph_clock_enable(RCU_CAN2);
    /* enable CAN port clock */
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOF);

    /* configure CAN0_RX GPIO */
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_8);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);
    gpio_af_set(GPIOB, GPIO_AF_9, GPIO_PIN_8);
    /* configure CAN0_TX GPIO */
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_9);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9);
    gpio_af_set(GPIOB, GPIO_AF_9, GPIO_PIN_9);

    /* configure CAN1_RX GPIO */
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_5);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5);
    gpio_af_set(GPIOB, GPIO_AF_9, GPIO_PIN_5);
    /* configure CAN1_TX GPIO */
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_13);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_13);
    gpio_af_set(GPIOB, GPIO_AF_9, GPIO_PIN_13);

    /* configure CAN2_RX GPIO */
    gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_6);
    gpio_mode_set(GPIOF, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6);
    gpio_af_set(GPIOF, GPIO_AF_2, GPIO_PIN_6);
    /* configure CAN2_TX GPIO */
    gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_7);
    gpio_mode_set(GPIOF, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_7);
    gpio_af_set(GPIOF, GPIO_AF_2, GPIO_PIN_7);
}

/*!
    \brief      configure BSP
    \param[in]  none
    \param[out] none
    \retval     none
*/
void bsp_board_config(void)
{
    /* configure USART */
    gd_eval_com_init(EVAL_COM);

    /* configure WAKEUP key */
    gd_eval_key_init(KEY_WAKEUP, KEY_MODE_GPIO);
    /* configure WAKEUP key */
    gd_eval_key_init(KEY_TAMPER, KEY_MODE_GPIO);
    /* configure LEDs */
    gd_eval_led_init(LED3);
    gd_eval_led_init(LED4);
    gd_eval_led_off(LED3);
    gd_eval_led_off(LED4);
}

/*!
    \brief      initialize CAN function
    \param[in]  none
    \param[out] none
    \retval     none
*/
void can_config(void)
{
    uint8_t i = 0U;
    can_parameter_struct can_parameter;
    can_fifo_parameter_struct can_fifo_parameter;

    /* initialize CAN register */
    can_deinit(CAN0);
    can_deinit(CAN1);
    can_deinit(CAN2);
    /* initialize CAN */
    can_struct_para_init(CAN_INIT_STRUCT, &can_parameter);
    can_struct_para_init(CAN_FIFO_INIT_STRUCT, &can_fifo_parameter);
    
    /* initialize CAN0, CAN1 and CAN2 parameters */
    can_parameter.internal_counter_source = CAN_TIMER_SOURCE_BIT_CLOCK;
    can_parameter.self_reception = DISABLE;
    can_parameter.mb_tx_order = CAN_TX_HIGH_PRIORITY_MB_FIRST;
    can_parameter.mb_tx_abort_enable = ENABLE;
    can_parameter.local_priority_enable = DISABLE;
    can_parameter.mb_rx_ide_rtr_type = CAN_IDE_RTR_FILTERED;
    can_parameter.mb_remote_frame = CAN_STORE_REMOTE_REQUEST_FRAME;
    can_parameter.rx_private_filter_queue_enable = DISABLE;
    can_parameter.edge_filter_enable = DISABLE;
    can_parameter.protocol_exception_enable = DISABLE;
    can_parameter.rx_filter_order = CAN_RX_FILTER_ORDER_MAILBOX_FIRST;
    can_parameter.memory_size = CAN_MEMSIZE_32_UNIT;
    /* filter configuration */
    can_parameter.mb_public_filter = 0U;
    /* baud rate 1Mbps, sample point at 80% */
    can_parameter.resync_jump_width = 1U;
    can_parameter.prop_time_segment = 2U;
    can_parameter.time_segment_1 = 5U;
    can_parameter.time_segment_2 = 2U;
    can_parameter.prescaler = 30U;

    /* initialize CAN1 and CAN2 */
    can_init(CAN1, &can_parameter);
    can_init(CAN2, &can_parameter);
    
    /* initialize CAN0 */
    can_parameter.rx_private_filter_queue_enable = ENABLE;
    can_init(CAN0, &can_parameter);
    /* initialize CAN0 FIFO */
    can_fifo_parameter.dma_enable = DISABLE;
    /* mailbox0~7 is occupied by CAN0 FIFO  */
    can_fifo_parameter.filter_format_and_number = CAN_RXFIFO_FILTER_A_NUM_8;
    can_fifo_parameter.fifo_public_filter = 0U;
    can_rx_fifo_config(CAN0, &can_fifo_parameter);
    /* private_filter0~7 used for CAN0 FIFO */
    for(i = 0U; i < 8U; i++){
        can_private_filter_config(CAN0, i, 0x7FFFFFFFU);
    }
    /* private_filter8 is used for CAN0 maibox 8 */
    can_private_filter_config(CAN0, 8U, 0x7FFFFFFFU);

    /* configure CAN1 NVIC */
    nvic_irq_enable(CAN0_Message_IRQn, 0U, 0U);

    /* enable CAN0 MB8 interrupt */
    can_interrupt_enable(CAN0, CAN_INT_MB8);
	/* enable CAN0 RxFIFO interrupt */
    can_interrupt_enable(CAN0,CAN_INT_FIFO_AVAILABLE);
    
    can_operation_mode_enter(CAN2, CAN_NORMAL_MODE);
    can_operation_mode_enter(CAN1, CAN_NORMAL_MODE);
    can_operation_mode_enter(CAN0, CAN_NORMAL_MODE);
}

/*!
    \brief      check received data
    \param[in]  none
    \param[out] none
    \retval     none
*/
void communication_check(void)
{
    uint8_t i = 0U;
    uint8_t *fifo_ptr = NULL;

    /* CAN0 maibox receive data correctly, the received data is printed */
    if(SET == can0_mailbox_receive_flag) {
        can0_mailbox_receive_flag = RESET;
        if(mailbox_receive_message.id == transmit_message[0].id) {
            printf("\r\nCAN0 Maibox receive data: \r\n");
            for(i = 0U; i < mailbox_receive_message.data_bytes; i++) {
                printf("%02x ", mailbox_receive_message.data[i]);
            }
            gd_eval_led_toggle(LED3);
        }
    }
    /* CAN0 FIFO receive data correctly, the received data is printed */
    if(SET == can0_fifo_receive_flag) {
        can0_fifo_receive_flag = RESET;
        if(fifo_receive_message.id == transmit_message[1].id) {
            fifo_ptr = (uint8_t *)fifo_receive_message.data;
            printf("\r\nCAN0 FIFO receive data: \r\n");
            for(i = 0U; i < fifo_receive_message.dlc; i++) {
                printf("%02x ", *fifo_ptr);
                fifo_ptr++;
            }
            gd_eval_led_toggle(LED4);
        }
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
