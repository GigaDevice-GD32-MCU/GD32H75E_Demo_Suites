/*!
    \file    gdesc.c
    \brief   gdesc driver basic configuration

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

#include <string.h>
#include "systick.h"
#include "ecat_def.h"
#include "ecatslv.h"
#include "ecatappl.h"
#include "gdesc.h"

typedef union 
{
    UINT8           Byte[2];
    UINT16          Word;
} UALEVENT;

UALEVENT         EscALEvent;            /* contains the content of the ALEvent register (0x220), this variable is updated on each Access to the Esc */
UINT16           nAlEventMask;          /* current ALEventMask (content of register 0x204:0x205) */

/*! \brief      operates access without addressing for ISR
    \param[in]  none
    \param[out] none
    \retval     none
    \note       The first two bytes of an access to the EtherCAT ASIC always deliver the AL_Event register (0x220).
                It will be saved in the global "EscALEvent"
*/
static void GetInterruptRegister(void)
{
    DISABLE_AL_EVENT_INT;

    HW_EscReadIsr((MEM_ADDR *)&EscALEvent.Word, 0x220, 2);

    ENABLE_AL_EVENT_INT;

}

/*!
    \brief      operate access without addressing for ISR
    \param[in]  none
    \param[out] none
    \retval     none
    \note       Shall be implemented if interrupts are supported else this function is equal to "GetInterruptRegsiter()"
                The first two bytes of an access to the EtherCAT ASIC always deliver the AL_Event register (0x220).
                It will be saved in the global "EscALEvent"
*/
#if !INTERRUPTS_SUPPORTED
#define ISR_GetInterruptRegister GetInterruptRegister
#else
static void ISR_GetInterruptRegister(void)
{

    HW_EscReadIsr((MEM_ADDR *)&EscALEvent.Word, 0x220, 2);

}
#endif /* #else #if !INTERRUPTS_SUPPORTED */

/*!
    \brief      enable esc func
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void esc_enable(void)
{

    volatile uint32_t temp = 0;

    ospi_hw_init(OSPI_INTERFACE, &ospi_init_struct);
    ospi_enable_ospi_mode(OSPI_INTERFACE, &ospi_init_struct, SPI_MODE);

        /*ByteTest and  PMU ready*/
    ospi_read(PMU_PDIREFVAL,(uint8_t*)&temp,4);
          if(temp != 0x76543210)
    {
        while(1);
    }

    temp = 0x301;
    ospi_write(ESC_RCU_RSTCFG, (uint8_t *)&temp, 4);
    delay_1ms(4);

    ospi_enable_ospi_mode(OSPI_INTERFACE, &ospi_init_struct, SPI_MODE);

    ospi_read(PMU_CTRL0,(uint8_t*)&temp,4);
      if( (temp & BIT(0)) != BIT(0))
        {
         while(1);
        }
}

/*!
    \brief      initialize the Process Data Interface (PDI) and the host controller
    \param[in]  none
    \param[out] none
    \retval     0 if initialization was successful
*/
UINT8 HW_Init(void)
{
    UINT32 intMask;

    /*enabe esc*/
    esc_enable();

    do {
        intMask = 0x0093;
        HW_EscWriteDWord(intMask, ESC_AL_EVENTMASK_OFFSET);
        intMask = 0x02;
        HW_EscReadDWord(intMask, ESC_AL_EVENTMASK_OFFSET);
    } while(intMask != 0x0093);

    intMask = 0x0;
    HW_EscWriteDWord(intMask, ESC_AL_EVENTMASK_OFFSET);

    /* IRQ enable,IRQ polarity, IRQ buffer type in Interrupt Configuration register. */
    intMask = INTC_CTL_IRQEN | INTC_CTL_IRQMODE ;
    ospi_write(INTC_CTL, (uint8_t *)&intMask, 4);

    /*EtherCAT IRQ*/
    intMask = INTC_EN_ECATIE;
    ospi_write(INTC_EN, (uint8_t *)&intMask, 4);

#if AL_EVENT_ENABLED
    INIT_ESC_INT;
    HW_ResetALEventMask(0x0);
    ENABLE_ESC_INT();
#endif

#if DC_SUPPORTED
    INIT_SYNC0_INT;
    INIT_SYNC1_INT;

    ENABLE_SYNC0_INT;
    ENABLE_SYNC1_INT;
#endif

    INIT_ECAT_TIMER;
    START_ECAT_TIMER;

    return 0;
}

/*!
    \brief      release hardware resources when the sample application stops
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HW_Release(void)
{

}

/*!
    \brief      get the current content of ALEvent register
    \param[in]  none
    \param[out] none
    \retval     first two bytes of ALEvent register (0x220)
*/
UINT16 HW_GetALEventRegister(void)
{
    GetInterruptRegister();
    return EscALEvent.Word;
}

#if INTERRUPTS_SUPPORTED
/*!
    \brief      get ALEvent register content from ISR
    \param[in]  none
    \param[out] none
    \retval     first two bytes of ALEvent register (0x220)
    \note       The PDI requires an extra ESC read access functions from interrupts service routines.
                The behaviour is equal to "HW_GetALEventRegister()"
*/
UINT16 HW_GetALEventRegister_Isr(void)
{
    ISR_GetInterruptRegister();
    return EscALEvent.Word;
}
#endif

/*!
    \brief      reset AL event mask with logical and operation
    \param[in]  intMask: interrupt mask (disabled interrupt shall be zero)
    \param[out] none
    \retval     none
    \note       This function makes an logical and with the AL Event Mask register (0x204)
*/
void HW_ResetALEventMask(UINT16 intMask)
{
    UINT16 mask;

    HW_EscReadWord(mask, ESC_AL_EVENTMASK_OFFSET);

    mask &= intMask;
    DISABLE_AL_EVENT_INT;
    HW_EscWriteWord(mask, ESC_AL_EVENTMASK_OFFSET);
    HW_EscReadWord(nAlEventMask, ESC_AL_EVENTMASK_OFFSET);
    ENABLE_AL_EVENT_INT;
}

/*!
    \brief      operate the read access to the EtherCAT ASIC
    \param[in]  pData: pointer to a byte array which holds data to write or saves read data
    \param[in]  Address: EtherCAT ASIC address (upper limit is 0x1FFF) for access
    \param[in]  Len: access size in bytes
    \param[out] none
    \retval     none
*/
void HW_EscRead(MEM_ADDR *pData, UINT16 Address, UINT16 Len)
{

    /* HBu 24.01.06: if the access will be read by an interrupt routine too the
                     mailbox reading may be interrupted but an interrupted
                     reading will remain in a transmission fault that will
                     reset the internal Sync Manager status. Therefore the reading
                     will be divided in 1-byte reads with disabled interrupt */
    UINT16 i;
    UINT8 *pTmpData = (UINT8 *)pData;

    /* loop for all bytes to be read */
    while(Len > 0) {
        if(Address >= 0x1000) {
            i = Len;
        } else {
            i = (Len > 4) ? 4 : Len;

            if(Address & 01) {
                i = 1;
            } else if(Address & 02) {
                i = (i & 1) ? 1 : 2;
            } else if(i == 03) {
                i = 1;
            }
        }

        DISABLE_AL_EVENT_INT;

        ospi_read_register(pTmpData, Address, i);

        ENABLE_AL_EVENT_INT;

        Len -= i;
        pTmpData += i;
        Address += i;
    }
}


#if INTERRUPTS_SUPPORTED
/*!
    \brief      read access to EtherCAT ASIC from ISR
    \param[in]  pData: pointer to a byte array which holds data to write or saves read data
    \param[in]  Address: EtherCAT ASIC address (upper limit is 0x1FFF) for access
    \param[in]  Len: access size in bytes
    \param[out] none
    \retval     none
    \note       The PDI requires an extra ESC read access functions from interrupts service routines.
                The behaviour is equal to "HW_EscRead()"
*/
void HW_EscReadIsr(MEM_ADDR *pData, UINT16 Address, UINT16 Len)
{

    UINT16 i;
    UINT8 *pTmpData = (UINT8 *)pData;

    /* send the address and command to the ESC */

    /* loop for all bytes to be read */
    while(Len > 0) {

        if(Address >= 0x1000) {
            i = Len;
        } else {
            i = (Len > 4) ? 4 : Len;

            if(Address & 01) {
                i = 1;
            } else if(Address & 02) {
                i = (i & 1) ? 1 : 2;
            } else if(i == 03) {
                i = 1;
            }
        }

        ospi_read_register(pTmpData, Address, i);

        Len -= i;
        pTmpData += i;
        Address += i;
    }
}
#endif /* #if INTERRUPTS_SUPPORTED */

/*!
    \brief      operate the write access to the EtherCAT ASIC
    \param[in]  pData: pointer to a byte array which holds data to write or saves write data
    \param[in]  Address: EtherCAT ASIC address (upper limit is 0x1FFF) for access
    \param[in]  Len: access size in bytes
    \param[out] none
    \retval     none
*/
void HW_EscWrite(MEM_ADDR *pData, UINT16 Address, UINT16 Len)
{


    UINT16 i;
    UINT8 *pTmpData = (UINT8 *)pData;

    /* loop for all bytes to be written */
    while(Len) {

        if(Address >= 0x1000) {
            i = Len;
        } else {
            i = (Len > 4) ? 4 : Len;

            if(Address & 01) {
                i = 1;
            } else if(Address & 02) {
                i = (i & 1) ? 1 : 2;
            } else if(i == 03) {
                i = 1;
            }
        }

        DISABLE_AL_EVENT_INT;

        /* start transmission */
        ospi_write_register(pTmpData, Address, i);

        ENABLE_AL_EVENT_INT;

        /* next address */
        Len -= i;
        pTmpData += i;
        Address += i;
    }
}

#if INTERRUPTS_SUPPORTED
/*!
    \brief      write access to EtherCAT ASIC from ISR
    \param[in]  pData: pointer to a byte array which holds data to write or saves write data
    \param[in]  Address: EtherCAT ASIC address (upper limit is 0x1FFF) for access
    \param[in]  Len: access size in bytes
    \param[out] none
    \retval     none
    \note       The PDI requires an extra ESC write access functions from interrupts service routines.
                The behaviour is equal to "HW_EscWrite()"
*/
void HW_EscWriteIsr(MEM_ADDR *pData, UINT16 Address, UINT16 Len)
{

    UINT16 i ;
    UINT8 *pTmpData = (UINT8 *)pData;


    /* loop for all bytes to be written */
    while(Len) {
        if(Address >= 0x1000) {
            i = Len;
        } else {
            i = (Len > 4) ? 4 : Len;

            if(Address & 01) {
                i = 1;
            } else if(Address & 02) {
                i = (i & 1) ? 1 : 2;
            } else if(i == 03) {
                i = 1;
            }
        }

        /* start transmission */
        ospi_write_register(pTmpData, Address, i);

        /* next address */
        Len -= i;
        pTmpData += i;
        Address += i;
    }
}
#endif

#if AL_EVENT_ENABLED
/*!
    \brief      interrupt service routine for the PDI interrupt from the EtherCAT Slave Controller
    \param[in]  none
    \param[out] none
    \retval     none
*/
void EcatIsr(void)
{
    if(exti_interrupt_flag_get(EXTI_10) != RESET) {
        PDI_Isr();
        ACK_ESC_INT;
    }
}
#endif

#if DC_SUPPORTED
/*!
    \brief      interrupt service routine for the interrupts from SYNC0
    \param[in]  none
    \param[out] none
    \retval     none
*/
void Sync0Isr(void)
{
    if(exti_interrupt_flag_get(EXTI_4) != RESET) {
        Sync0_Isr();
        ACK_SYNC0_INT;
    }
}
/*!
    \brief      interrupt service routine for the interrupts from SYNC1
    \param[in]  none
    \param[out] none
    \retval     none
*/
void Sync1Isr(void)
{
    if(exti_interrupt_flag_get(EXTI_3) != RESET) {
        Sync1_Isr();
        ACK_SYNC1_INT;
    }
}
#endif

#if ECAT_TIMER_INT
/*!
    \brief      Timer 2 ISR (1ms) - Handles timer-based tasks for EtherCAT
    \param[in]  none
    \param[out] none
    \retval     none
*/
void TimerIsr(void)
{
    if(timer_interrupt_flag_get(TIMER2, TIMER_INT_FLAG_UP) != RESET) {
        ECAT_CheckTimer();
        ECAT_TIMER_ACK_INT;
    }
}
#endif