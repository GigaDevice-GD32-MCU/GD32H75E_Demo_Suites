/*!
    \file    main.c
    \brief   esc cia402 example

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
#include "gd32h75e.h"
#include "systick.h"
#include "gd32h75ey_eval.h"
#include "gdesc.h"
#include "applInterface.h"

extern uint16_t CiA402_Init(void);
extern uint16_t APPL_GenerateMapping(UINT16 *pInputSize, UINT16 *pOutputSize);
extern void CiA402_DeallocateAxis(void);

void cache_enable(void);

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

    /* configure systick */
    systick_config();

    /* EtherCAT */
    HW_Init();
    MainInit();

    CiA402_Init();
    APPL_GenerateMapping(&nPdInputSize, &nPdOutputSize);

    bRunApplication = TRUE;

    do {
        MainLoop();
    } while(bRunApplication == TRUE);

    CiA402_DeallocateAxis();

    HW_Release();

    return 0;
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
