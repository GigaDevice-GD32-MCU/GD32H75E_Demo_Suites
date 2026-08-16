/*!
    \file    main.c
    \brief   SPI SPI-FLASH-DMA example

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
#include "gd25qxx.h"

#define BUFFER_SIZE                    256
#define TX_BUFFER_SIZE                 BUFFER_SIZE
#define RX_BUFFER_SIZE                 BUFFER_SIZE

#define  SFLASH_ID                     0xC84015
#define  FLASH_WRITE_ADDRESS           0x000000
#define  FLASH_READ_ADDRESS            FLASH_WRITE_ADDRESS

uint32_t int_device_serial[3];
uint8_t  count;
__attribute__((aligned(32))) uint8_t  tx_buffer[TX_BUFFER_SIZE];
__attribute__((aligned(32))) uint8_t  rx_buffer[TX_BUFFER_SIZE];
uint32_t flash_id = 0;
uint16_t i = 0;
uint8_t  is_successful = 0;

void cache_enable(void);
void mpu_config(void);
void dma_config(void);
static void get_chip_serial_num(void);
void test_status_led_init(void);
ErrStatus memory_compare(uint8_t *src, uint8_t *dst, uint16_t length);
int fputc(int ch, FILE *f);

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

    /* systick configuration */
    systick_config();

    /* configure the led GPIO */
    test_status_led_init();

    /* USART parameter configuration */
    gd_eval_com_init(EVAL_COM);

    /* configure the DMA peripheral */
    dma_config();

    /* configure SPI4 GPIO and parameter */
    spi_flash_init();

    delay_1ms(10);

    /* gd32h75e Start up */
    printf("###############################################################################\r\n");
    printf("gd32h75e System is Starting up...\r\n");
    printf("gd32h75e SystemCoreClock:%dHz\r", SystemCoreClock);
    printf("gd32h75e Flash:%dK\r\n", *((__IO uint32_t *)(0x1FF0F7E0)) >> 16);

    /* get chip serial number */
    get_chip_serial_num();

    /* printf CPU unique device id */
    printf("gd32h75e The CPU Unique Device ID:[%X-%X-%X]\r\n", int_device_serial[2], int_device_serial[1], int_device_serial[0]);

    printf("gd32h75e SPI Flash:GD25Q16 configured...\r\n");

    /* get flash id */
    flash_id = spi_flash_read_id();
    printf("The Flash_ID:0x%X\n\r", flash_id);

    /* flash id is correct */
    if(SFLASH_ID == flash_id) {
        printf("Write to tx_buffer:\r\n");

        /* printf tx_buffer value */
        for(i = 0; i < BUFFER_SIZE; i ++) {
            tx_buffer[i] = i;
            printf("0x%02X ", tx_buffer[i]);

            if(15 == i % 16) {
                printf("\r\n");
                delay_1ms(1);
            }
        }

        printf("\n\rRead from rx_buffer:\r");

        /* erases the specified flash sector */
        spi_flash_sector_erase(FLASH_WRITE_ADDRESS);
        /* write tx_buffer data to the flash */
        spi_flash_buffer_write(tx_buffer, FLASH_WRITE_ADDRESS, TX_BUFFER_SIZE);

        delay_1ms(10);

        /* read a block of data from the flash to rx_buffer */
        spi_flash_buffer_read(rx_buffer, FLASH_READ_ADDRESS, RX_BUFFER_SIZE);
        /* printf rx_buffer value */
        for(i = 0; i <= 255; i ++) {
            printf("0x%02X ", rx_buffer[i]);
            if(15 == i % 16) {
                printf("\r\n");
                delay_1ms(1);
            }
        }

        if(ERROR == memory_compare(tx_buffer, rx_buffer, 256)) {
            printf("Err:Data Read and Write aren't Matching.\n\r");
            is_successful = 1;
        }

        /* spi flash test passed */
        if(0 == is_successful) {
            printf("SPI-GD25Q16 Test Passed!\n\r");
        }
    } else {
        /* spi flash read id fail */
        printf("SPI Flash: Read ID Fail!\n\r");
    }

    while(1) {
        /* turn on led3 */
        gd_eval_led_on(LED3);
        delay_1ms(500);
        /* turn off led3 */
        gd_eval_led_off(LED3);
        delay_1ms(500);
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
    \brief      configure the DMA peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void dma_config(void)
{
    rcu_periph_clock_enable(RCU_DMA0);
    rcu_periph_clock_enable(RCU_DMAMUX);

    dma_single_data_parameter_struct dma_init_struct;
    /* deinitialize DMA registers of a channel */
    dma_deinit(DMA0, DMA_CH2);
    dma_deinit(DMA0, DMA_CH3);
    dma_single_data_para_struct_init(&dma_init_struct);

    /* SPI4 transmit DMA config: DMA_CH2 */
    dma_init_struct.request = DMA_REQUEST_SPI4_TX;
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;    
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.number = 0;
    dma_init_struct.periph_addr = (uint32_t)&SPI_TDATA(SPI4);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;
    dma_single_data_mode_init(DMA0, DMA_CH2, &dma_init_struct);

    /* SPI4 receive DMA config: DMA_CH3 */
    dma_init_struct.request = DMA_REQUEST_SPI4_RX;
    dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;    
    dma_init_struct.periph_addr = (uint32_t)&SPI_RDATA(SPI4);
    dma_init_struct.priority = DMA_PRIORITY_MEDIUM;
    dma_single_data_mode_init(DMA0, DMA_CH3, &dma_init_struct); 
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
    \brief      get chip serial number
    \param[in]  none
    \param[out] none
    \retval     none
*/
void get_chip_serial_num(void)
{
    int_device_serial[0] = *(__IO uint32_t *)(0x1FF0F7E8);
    int_device_serial[1] = *(__IO uint32_t *)(0x1FF0F7EC);
    int_device_serial[2] = *(__IO uint32_t *)(0x1FF0F7F0);
}

/*!
    \brief      test status led initialize
    \param[in]  none
    \param[out] none
    \retval     none
*/
void test_status_led_init(void)
{
    /* initialize the led1 */
    gd_eval_led_init(LED3);

    /* turn off led1 */
    gd_eval_led_off(LED3);
}

/*!
    \brief      memory compare function
    \param[in]  src: source data pointer
    \param[in]  dst: destination data pointer
    \param[in]  length: the compare data length
    \param[out] none
    \retval     ErrStatus: ERROR or SUCCESS
*/
ErrStatus memory_compare(uint8_t *src, uint8_t *dst, uint16_t length)
{
    while(length --) {
        if(*src++ != *dst++) {
            return ERROR;
        }
    }
    return SUCCESS;
}
