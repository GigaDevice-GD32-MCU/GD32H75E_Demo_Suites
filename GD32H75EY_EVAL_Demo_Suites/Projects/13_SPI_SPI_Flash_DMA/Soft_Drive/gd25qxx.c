/*!
    \file    gd25qxx.c
    \brief   SPI flash gd25qxx driver

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

#include "gd25qxx.h"
#include "gd32h75e.h"
#include <string.h>

#define WRITE            0x02     /* write to memory instruction */
#define WRSR             0x01     /* write status register instruction */
#define WREN             0x06     /* write enable instruction */

#define READ             0x03     /* read from memory instruction */
#define RDSR             0x05     /* read status register instruction  */
#define RDID             0x9F     /* read identification */
#define SE               0x20     /* sector erase instruction */
#define CE               0xC7     /* bulk erase instruction */

#define WIP_FLAG         0x01     /* write in progress(wip)flag */
#define DUMMY_BYTE       0xA5
#define  SIZE_ALIGN(size, align)           (((size) + (align) - 1) & ~((align) - 1))
uint32_t dummy = 0;
/*!
    \brief      configure SPI4 GPIO and initialize SPI4 parameters
    \param[in]  none
    \param[out] none
    \retval     none
*/
void spi_flash_init(void)
{
    spi_parameter_struct spi_init_struct;

    /* enable peripheral clocks for GPIO and SPI4 */
    rcu_periph_clock_enable(RCU_GPIOF);
    rcu_periph_clock_enable(RCU_GPIOH);
    rcu_periph_clock_enable(RCU_SPI4);
    rcu_spi_clock_config(IDX_SPI4, RCU_SPISRC_APB2);

    /* configure SPI4 GPIO pins for SCK, MISO, MOSI, and CS */
    gpio_af_set(GPIOH, GPIO_AF_5, GPIO_PIN_6 | GPIO_PIN_7);
    gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6 | GPIO_PIN_7);
    gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_6 | GPIO_PIN_7);

    gpio_af_set(GPIOF, GPIO_AF_5, GPIO_PIN_9);
    gpio_mode_set(GPIOF, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_9);
    gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_9);

    /* SPI4_CS(PH5) GPIO pin configuration */
    gpio_af_set(GPIOH, GPIO_AF_5, GPIO_PIN_5);
    gpio_mode_set(GPIOH, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_5);
    gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_60MHZ, GPIO_PIN_5);

    /* chip select invalid*/
    SPI_FLASH_CS_HIGH();

    /* configure SPI4 parameters for full-duplex communication */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.data_size            = SPI_DATASIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_16;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;;
    spi_init(SPI4, &spi_init_struct);

    /* enable SPI byte access */
    spi_byte_access_enable(SPI4);

    /* enable SPI NSS output */
    spi_nss_output_enable(SPI4);

    /* enable SPI4 */
    spi_enable(SPI4);
}

/*!
    \brief      erase a specific sector in the SPI flash
    \param[in]  sector_addr: address of the sector to erase
    \param[out] none
    \retval     none
*/
void spi_flash_sector_erase(uint32_t sector_addr)
{
    /* send write enable instruction */
    spi_flash_write_enable();

    /* sector erase */
    /* select the flash: chip select low */
    SPI_FLASH_CS_LOW();
    /* send sector erase instruction */
    spi_flash_send_byte(SE);
    /* send sector_addr high nibble address byte */
    spi_flash_send_byte((sector_addr & 0xFF0000) >> 16);
    /* send sector_addr medium nibble address byte */
    spi_flash_send_byte((sector_addr & 0xFF00) >> 8);
    /* send sector_addr low nibble address byte */
    spi_flash_send_byte(sector_addr & 0xFF);
    /* deselect the flash: chip select high */
    SPI_FLASH_CS_HIGH();

    /* wait the end of flash writing */
    spi_flash_wait_for_write_end();
}

/*!
    \brief      erase the entire flash
    \param[in]  none
    \param[out] none
    \retval     none
*/
void spi_flash_chip_erase(void)
{
    /* send write enable instruction */
    spi_flash_write_enable();

    /* bulk erase */
    /* select the flash: chip select low */
    SPI_FLASH_CS_LOW();
    /* send chip erase instruction  */
    spi_flash_send_byte(CE);
    /* deselect the flash: chip select high */
    SPI_FLASH_CS_HIGH();

    /* wait the end of flash writing */
    spi_flash_wait_for_write_end();
}

/*!
    \brief      write a page of data to the SPI flash
    \param[in]  pbuffer: pointer to the data buffer
    \param[in]  write_addr: address in flash to write to
    \param[in]  num_byte_to_write: number of bytes to write
    \param[out] none
    \retval     none
*/
void spi_flash_page_write(uint8_t *pbuffer, uint32_t write_addr, uint16_t num_byte_to_write)
{
    SCB_CleanDCache_by_Addr((uint32_t *)pbuffer, SIZE_ALIGN(num_byte_to_write, 32));
    __DSB();

    /* enable the write access to the flash */
    spi_flash_write_enable();

    /* select the flash: chip select low */
    SPI_FLASH_CS_LOW();

    /* send "write to memory" instruction */
    spi_flash_send_byte(WRITE);
    /* send write_addr high nibble address byte to write to */
    spi_flash_send_byte((write_addr & 0xFF0000) >> 16);
    /* send write_addr medium nibble address byte to write to */
    spi_flash_send_byte((write_addr & 0xFF00) >> 8);
    /* send write_addr low nibble address byte to write to */
    spi_flash_send_byte(write_addr & 0xFF);

    /* disable SPI4 before configuring DMA transfer */
    spi_disable(SPI4);

    /* configure DMA transfer number for transmit and receive */
    dma_transfer_number_config(DMA0, DMA_CH2, num_byte_to_write);
    dma_memory_address_config(DMA0, DMA_CH2, DMA_MEMORY_0, (uint32_t)pbuffer);
    dma_transfer_number_config(DMA0, DMA_CH3, num_byte_to_write);
    dma_memory_address_config(DMA0, DMA_CH3, DMA_MEMORY_0, (uint32_t)&dummy);
    dma_memory_address_generation_config(DMA0, DMA_CH3, DMA_MEMORY_INCREASE_DISABLE);

    /* configure SPI current transfer number */
    spi_current_data_num_config(SPI4, num_byte_to_write);

    /* enable SPI DMA receive and transmit */
    spi_dma_enable(SPI4, SPI_DMA_RECEIVE);
    spi_dma_enable(SPI4, SPI_DMA_TRANSMIT);

    /* enable SPI4 and DMA channels */
    spi_enable(SPI4);
    dma_channel_enable(DMA0, DMA_CH3);
    dma_channel_enable(DMA0, DMA_CH2);

    /* start SPI master transfer */
    spi_master_transfer_start(SPI4, SPI_TRANS_START);

    /* wait for DMA transfer to complete */
    while(dma_flag_get(DMA0, DMA_CH2, DMA_FLAG_FTF) == RESET);
    while(dma_flag_get(DMA0, DMA_CH3, DMA_FLAG_FTF) == RESET);
    dma_flag_clear(DMA0, DMA_CH2, DMA_FLAG_FTF);
    dma_flag_clear(DMA0, DMA_CH3, DMA_FLAG_FTF);

    while(spi_i2s_flag_get(SPI4, SPI_FLAG_TC) == RESET);
    /* deselect the flash: chip select high */
    SPI_FLASH_CS_HIGH();

    dma_channel_disable(DMA0, DMA_CH2);
    dma_channel_disable(DMA0, DMA_CH3);
    spi_dma_disable(SPI4, SPI_DMA_RECEIVE);
    spi_dma_disable(SPI4, SPI_DMA_TRANSMIT);
    dma_memory_address_generation_config(DMA0, DMA_CH3, DMA_MEMORY_INCREASE_ENABLE);

    /* wait the end of flash writing */
    spi_flash_wait_for_write_end();
}

/*!
    \brief      write block of data to the flash
    \param[in]  pbuffer: pointer to the buffer
    \param[in]  write_addr: flash's internal address to write
    \param[in]  num_byte_to_write: number of bytes to write to the flash
    \param[out] none
    \retval     none
*/
void spi_flash_buffer_write(uint8_t *pbuffer, uint32_t write_addr, uint16_t num_byte_to_write)
{
    if (pbuffer == NULL || num_byte_to_write == 0) {
        return; 
    }
    uint8_t num_of_page = 0, num_of_single = 0, addr = 0, count = 0, temp = 0;

    addr          = write_addr % SPI_FLASH_PAGE_SIZE;
    count         = SPI_FLASH_PAGE_SIZE - addr;
    num_of_page   = num_byte_to_write / SPI_FLASH_PAGE_SIZE;
    num_of_single = num_byte_to_write % SPI_FLASH_PAGE_SIZE;

    /* write_addr is SPI_FLASH_PAGE_SIZE aligned  */
    if(0 == addr) {
        /* num_byte_to_write < SPI_FLASH_PAGE_SIZE */
        if(0 == num_of_page) {
            spi_flash_page_write(pbuffer, write_addr, num_byte_to_write);
        }
        /* num_byte_to_write > SPI_FLASH_PAGE_SIZE */
        else {
            while(num_of_page--) {
                spi_flash_page_write(pbuffer, write_addr, SPI_FLASH_PAGE_SIZE);
                write_addr += SPI_FLASH_PAGE_SIZE;
                pbuffer += SPI_FLASH_PAGE_SIZE;
            }
            if(0 != num_of_single) {
                spi_flash_page_write(pbuffer, write_addr, num_of_single);
            }
        }
    } else {
        /* write_addr is not SPI_FLASH_PAGE_SIZE aligned  */
        if(0 == num_of_page) {
            /* (num_byte_to_write + write_addr) > SPI_FLASH_PAGE_SIZE */
            if(num_of_single > count) {
                temp = num_of_single - count;
                spi_flash_page_write(pbuffer, write_addr, count);
                write_addr += count;
                pbuffer += count;
                spi_flash_page_write(pbuffer, write_addr, temp);
            } else {
                spi_flash_page_write(pbuffer, write_addr, num_byte_to_write);
            }
        } else {
            /* num_byte_to_write > SPI_FLASH_PAGE_SIZE */
            num_byte_to_write -= count;
            num_of_page = num_byte_to_write / SPI_FLASH_PAGE_SIZE;
            num_of_single = num_byte_to_write % SPI_FLASH_PAGE_SIZE;

            spi_flash_page_write(pbuffer, write_addr, count);
            write_addr += count;
            pbuffer += count;

            while(num_of_page--) {
                spi_flash_page_write(pbuffer, write_addr, SPI_FLASH_PAGE_SIZE);
                write_addr += SPI_FLASH_PAGE_SIZE;
                pbuffer += SPI_FLASH_PAGE_SIZE;
            }

            if(0 != num_of_single) {
                spi_flash_page_write(pbuffer, write_addr, num_of_single);
            }
        }
    }
}

/*!
    \brief      read a block of data from the SPI flash
    \param[in]  pbuffer: pointer to the buffer to store read data
    \param[in]  read_addr: address in flash to read from
    \param[in]  num_byte_to_read: number of bytes to read
    \param[out] none
    \retval     none
*/
void spi_flash_buffer_read(uint8_t *pbuffer, uint32_t read_addr, uint16_t num_byte_to_read)
{
    if (pbuffer == NULL || num_byte_to_read == 0) {
        return;
    }
    /* select the flash: chip select low */
    SPI_FLASH_CS_LOW();

    /* send "read from memory" instruction */
    spi_flash_send_byte(READ);

    /* send read_addr high nibble address byte to read from */
    spi_flash_send_byte((read_addr & 0xFF0000) >> 16);
    /* send read_addr medium nibble address byte to read from */
    spi_flash_send_byte((read_addr & 0xFF00) >> 8);
    /* send read_addr low nibble address byte to read from */
    spi_flash_send_byte(read_addr & 0xFF);

    spi_disable(SPI4);

    /* clear DMA FTF flag to start a new DMA transfer */
    dma_flag_clear(DMA0, DMA_CH2, DMA_FLAG_FTF);
    dma_flag_clear(DMA0, DMA_CH3, DMA_FLAG_FTF);

    /* configure DMA transfer number */
    dma_transfer_number_config(DMA0, DMA_CH2, num_byte_to_read);
    dma_transfer_number_config(DMA0, DMA_CH3, num_byte_to_read);

    /* configure SPI current transfer number */
    spi_current_data_num_config(SPI4, num_byte_to_read);

    /* configure DMA transfer memory address */
    dma_memory_address_config(DMA0, DMA_CH3, DMA_MEMORY_0, (uint32_t)pbuffer);
    dma_memory_address_config(DMA0, DMA_CH2, DMA_MEMORY_0, (uint32_t)&dummy);

    /* prohibit address auto-increment to prevent address overflow */
    dma_memory_address_generation_config(DMA0, DMA_CH2, DMA_MEMORY_INCREASE_DISABLE);

    /* enable SPI DMA receive and transmit */
    spi_dma_enable(SPI4, SPI_DMA_RECEIVE);
    spi_dma_enable(SPI4, SPI_DMA_TRANSMIT);
    spi_enable(SPI4);

    /* enable DMA channel */
    dma_channel_enable(DMA0, DMA_CH3);
    dma_channel_enable(DMA0, DMA_CH2);

    /* start SPI transmit */
    spi_master_transfer_start(SPI4, SPI_TRANS_START);

    /* wait until DMA transmit finishes */
    while(dma_flag_get(DMA0, DMA_CH3, DMA_FLAG_FTF) == RESET);
    while(dma_flag_get(DMA0, DMA_CH2, DMA_FLAG_FTF) == RESET);
    dma_flag_clear(DMA0, DMA_CH3, DMA_FLAG_FTF);
    dma_flag_clear(DMA0, DMA_CH2, DMA_FLAG_FTF);

    while(spi_i2s_flag_get(SPI4, SPI_FLAG_TC) == RESET);
    /* disable DMA channel, SPI DMA receive and transmit */
    dma_channel_disable(DMA0, DMA_CH3);
    dma_channel_disable(DMA0, DMA_CH2);
    spi_dma_disable(SPI4, SPI_DMA_RECEIVE);
    spi_dma_disable(SPI4, SPI_DMA_TRANSMIT);

    /* restore DMA address auto-increment */
    dma_memory_address_generation_config(DMA0, DMA_CH2, DMA_MEMORY_INCREASE_ENABLE);

    /* invalidate cache to ensure data consistency */
    SCB_InvalidateDCache_by_Addr((uint32_t *)pbuffer, SIZE_ALIGN(num_byte_to_read, 32));
    __DSB();

    /* deselect the flash: chip select high */
    SPI_FLASH_CS_HIGH();
}

/*!
    \brief      read the identification of the SPI flash
    \param[in]  none
    \param[out] none
    \retval     flash identification value
*/
uint32_t spi_flash_read_id(void)
{
    uint32_t temp = 0, temp0 = 0, temp1 = 0, temp2 = 0;

    /* select the flash: chip select low */
    SPI_FLASH_CS_LOW();

    /* send "RDID " instruction */
    spi_flash_send_byte(0x9F);

    /* read a byte from the flash */
    temp0 = spi_flash_send_byte(DUMMY_BYTE);

    /* read a byte from the flash */
    temp1 = spi_flash_send_byte(DUMMY_BYTE);

    /* read a byte from the flash */
    temp2 = spi_flash_send_byte(DUMMY_BYTE);

    /* deselect the flash: chip select high */
    SPI_FLASH_CS_HIGH();

    temp = (temp0 << 16) | (temp1 << 8) | temp2;

    return temp;
}

/*!
    \brief      initiate a read data byte (read) sequence from the flash
    \param[in]  read_addr: flash's internal address to read from
    \param[out] none
    \retval     none
*/
void spi_flash_start_read_sequence(uint32_t read_addr)
{
    /* select the flash: chip select low */
    SPI_FLASH_CS_LOW();

    /* send "read from memory " instruction */
    spi_flash_send_byte(READ);

    /* send the 24-bit address of the address to read from */
    /* send read_addr high nibble address byte */
    spi_flash_send_byte((read_addr & 0xFF0000) >> 16);
    /* send read_addr medium nibble address byte */
    spi_flash_send_byte((read_addr & 0xFF00) >> 8);
    /* send read_addr low nibble address byte */
    spi_flash_send_byte(read_addr & 0xFF);
}

/*!
    \brief      read a byte from the SPI flash
    \param[in]  none
    \param[out] none
    \retval     byte read from the SPI flash
*/
uint8_t spi_flash_read_byte(void)
{
    return(spi_flash_send_byte(DUMMY_BYTE));
}

/*!
    \brief      send a byte through the SPI interface and return the byte received from the SPI bus
    \param[in]  byte: byte to send
    \param[out] none
    \retval     the value of the received byte
*/
uint8_t spi_flash_send_byte(uint8_t byte)
{
    spi_current_data_num_config(SPI4, 1);
    spi_master_transfer_start(SPI4, SPI_TRANS_START);

    /* loop while data register is not empty */
    while(RESET == spi_i2s_flag_get(SPI4, SPI_FLAG_TP));

    /* send byte through the SPI4 peripheral */
    spi_i2s_data_transmit(SPI4, byte);

    /* wait to receive a byte */
    while(RESET == spi_i2s_flag_get(SPI4, SPI_FLAG_RP));

    /* return the byte read from the SPI bus */
    return(spi_i2s_data_receive(SPI4));
}

/*!
    \brief      send a half word through the SPI interface and return the half word received from the SPI bus
    \param[in]  half_word: half word to send
    \param[out] none
    \retval     the value of the received byte
*/
uint16_t spi_flash_send_halfword(uint16_t half_word)
{
    /* loop while data register in not emplty */
    while(RESET == spi_i2s_flag_get(SPI4, SPI_FLAG_TP));

    /* send half word through the SPI4 peripheral */
    spi_i2s_data_transmit(SPI4, half_word);

    /* wait to receive a half word */
    while(RESET == spi_i2s_flag_get(SPI4, SPI_FLAG_RP));

    /* return the half word read from the SPI bus */
    return spi_i2s_data_receive(SPI4);
}

/*!
    \brief      enable write operations on the SPI flash
    \param[in]  none
    \param[out] none
    \retval     none
*/
void spi_flash_write_enable(void)
{
    /* select the flash: chip select low */
    SPI_FLASH_CS_LOW();

    /* send write enable instruction */
    spi_flash_send_byte(WREN);

    /* deselect the flash: chip select high */
    SPI_FLASH_CS_HIGH();
}

/*!
    \brief      wait until the SPI flash is no longer busy
    \param[in]  none
    \param[out] none
    \retval     none
*/
void spi_flash_wait_for_write_end(void)
{
    uint8_t flash_status = 0;

    /* select the flash: chip select low */
    SPI_FLASH_CS_LOW();

    /* send "read status register" instruction */
    spi_flash_send_byte(RDSR);

    /* loop as long as the memory is busy with a write cycle */
    do {
        /* send a dummy byte to generate the clock needed by the flash
        and put the value of the status register in flash_status variable */
        flash_status = spi_flash_send_byte(DUMMY_BYTE);
    } while((flash_status & WIP_FLAG) == SET);

    /* deselect the flash: chip select high */
    SPI_FLASH_CS_HIGH();
}
