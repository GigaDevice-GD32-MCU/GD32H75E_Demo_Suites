/*!
    \file    readme.txt
    \brief   description of esc cia402 demo

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

  This demo cannot be directly compiled and downloaded for use.

  This demo is based on the GD32H75EY-EVAL-V1.0 board, it provides a description EtherCAT. 
after the program is running, connect the Ethernet cable to the development board and use 
the main device software to communicate with the current subdevice board. Once the system 
is operating normally, the LED2 indicator on the board will remain steadily lit.

  The esc cia402 example is used to demonstrate the functionality of implementing 
 an esc cia402 slave based on the esc peripheral and slave stack code (SSC).

  Before testing this demo, users are required to be members of the ETG organization 
in order to obtain access to the SSC TOOL for generating protocol stack code.
  Users must first complete the download of the SSC TOOL. Refer to the ETG official website 
for download instructions: 
    https://www.ethercat.org/en/downloads/downloads_01DCC32A10294F2EA866F7E46FB0285F.htm.

  after the download of the SSC TOOL,Refer to the steps in the AN246 document to generate 
the protocol stack code and port the stack code into the current project for testing.
  AN264 document path: 
    GD32H75E_Demo_Suites\GD32H75EY_EVAL_Demo_Suites\Projects\23_EtherCAT\ethercat


