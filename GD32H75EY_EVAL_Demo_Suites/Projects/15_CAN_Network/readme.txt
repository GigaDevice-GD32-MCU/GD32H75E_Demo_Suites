/*!
    \file    readme.txt
    \brief   description of CAN network demo

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

  This demo is based on the GD32H75EY-EVAL-V1.0 board, it shows how to configure the 
CAN1 and CAN2 peripheral to send CAN frames, and CAN0 peripheral to receive CAN frames
in normal mode. CAN0 uses the mailbox to receive data from CAN1 and uses the FIFO to 
receive data from CAN2.

  When TAMPERbutton is pressed, CAN1 sends a message to CAN0, when CAN0 receives correctly 
this message,the receive data will be printed and LED3 toggles one time.

  When WAKEUP button is pressed, CAN2 sends a message to CAN0, when CAN0 receives correctly 
this message,the receive data will be printed and LED4 toggles one time.

  Connect the CANL pins of JP14, JP15, and JP16 together, and connect the CANH pins together.

  GD32H75EY-EVAL-V1.0 CAN transceiver needs 5V power supply.
