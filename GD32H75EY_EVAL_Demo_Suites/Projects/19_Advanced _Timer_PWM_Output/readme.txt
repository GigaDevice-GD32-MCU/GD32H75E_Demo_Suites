/*!
    \file    readme.txt
    \brief   description of Advanced Timer PWM Outout

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

  TIMER0 frequencyis fixed to systemcoreclock, TIMER0 prescaler is equal to 299, so TIMER0
counter frequency is 1MHz. The duty cycle is computed as the following description:
  - the CH0 duty cycle is set to 40 % so MCH0 is set to 60 %.
  - the CH1 duty cycle is set to 60 % so MCH1 is set to 40 %.
  - the CH2 duty cycle is set to 80 % so MCH1 is set to 20 %.

The deadtime of CH0 / MCH0, CH1 / MCH1 and CH2 / MCH2 with different values:
    CH0 and MCH0 insert a dead time equal to ((64+36)/(300MHz/4)*2 =2.67us.
    CH1 and MCH1 insert a dead time equal to ((32+8)/(300MHz/4)*8 =4.26us.
    CH2 and MCH2 insert a dead time equal to ((32+0)/(300MHz/4)*16 =6.83us.

  Configure the break feature, the break function is active when TIMER0_BRKIN0 pin (PE15) input 
  low level, and using the automatic output enable feature. Use the Locking parameters level0.
