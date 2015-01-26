/*
 * Copyright (c) 2013, Jens Nielsen
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL JENS NIELSEN BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef OWHAL_H
#define OWHAL_H

#define DS_PORT_BASE GPIO_D_BASE // PORT D
#define DS_PIN_MASK  GPIO_PIN_MASK(3) // PIN 3

#define T_PORT_BASE GPIO_D_BASE // PORT D
#define T_PIN_MASK  GPIO_PIN_MASK(2) // PIN 2

#define T1_PORT_BASE GPIO_A_BASE // PORT A
#define T1_PIN_MASK  GPIO_PIN_MASK(5) // PIN 5

#include "dev/gpio.h"

#define T_OUTPUT() GPIO_SET_OUTPUT(T_PORT_BASE, T_PIN_MASK)
#define T_HIGH() GPIO_SET_PIN(T_PORT_BASE, T_PIN_MASK)
#define T_LOW() GPIO_CLR_PIN(T_PORT_BASE, T_PIN_MASK)

#define T1_OUTPUT() GPIO_SET_OUTPUT(T1_PORT_BASE, T1_PIN_MASK)
#define T1_HIGH() GPIO_SET_PIN(T1_PORT_BASE, T1_PIN_MASK)
#define T1_LOW() GPIO_CLR_PIN(T1_PORT_BASE, T1_PIN_MASK) 

#define OW_OUTPUT() GPIO_SET_OUTPUT(DS_PORT_BASE, DS_PIN_MASK)
#define OW_INPUT() GPIO_SET_INPUT(DS_PORT_BASE, DS_PIN_MASK)

#define OW_HIGH() GPIO_SET_PIN(DS_PORT_BASE, DS_PIN_MASK)
#define OW_LOW() GPIO_CLR_PIN(DS_PORT_BASE, DS_PIN_MASK)

#define OW_READ() GPIO_READ_PIN(DS_PORT_BASE, DS_PIN_MASK)>>3 // need >>3 because GPIO_READ_PIN returns port reading

#endif
