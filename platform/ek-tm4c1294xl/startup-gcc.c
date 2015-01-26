/*
 *  Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \addtogroup dk-tm4c129x
 * @{
 *
 * \file
 * Startup code for the DK-TM4C129X platform, to be used when building with gcc
 */
#include "contiki.h"
#include <stdint.h>
#include <stdio.h>

/*---------------------------------------------------------------------------*/
extern int main(void);
/*---------------------------------------------------------------------------*/
/* System handlers provided here */
void reset_handler(void);
void nmi_handler(void);
void fault_handler(void);
void default_handler(void);

/* System Handler and ISR prototypes implemented elsewhere */
void clock_isr(void); /* SysTick Handler */
void rtimer_isr(void);
void gpio_port_a_isr(void);
void gpio_port_b_isr(void);
void gpio_port_c_isr(void);
void gpio_port_d_isr(void);
void gpio_port_e_isr(void);
void gpio_port_f_isr(void);
void gpio_port_g_isr(void);
void gpio_port_h_isr(void);
void gpio_port_j_isr(void);
void gpio_port_k_isr(void);
void gpio_port_l_isr(void);
void gpio_port_m_isr(void);
void gpio_port_n_isr(void);
void gpio_port_p_isr(void);
void gpio_port_q_isr(void);
void gpio_port_r_isr(void);
void gpio_port_s_isr(void);
void gpio_port_t_isr(void);

/* Likewise for the UART ISR */
#if UART_CONF_ENABLE
void uart_isr(void);
#else
#define uart_isr default_handler
#endif
void watchdog_isr(void);
void EthernetIntHandler(void);
/*---------------------------------------------------------------------------*/
/* Allocate stack space */
static unsigned long stack[1024];
/*---------------------------------------------------------------------------*/
__attribute__ ((section(".vectors"), used))
void(*const vectors[])(void) =
{
  (void (*)(void))((unsigned long)stack + sizeof(stack)),   /* Stack pointer */
  reset_handler,              /* Reset handler */
  nmi_handler,                /* The NMI handler */
  fault_handler,              /* The hard fault handler */
  fault_handler,              /* 4 The MPU fault handler */
  fault_handler,              /* 5 The bus fault handler */
  fault_handler,              /* 6 The usage fault handler */
  default_handler,            /* 7 Reserved */
  default_handler,            /* 8 Reserved */
  default_handler,            /* 9 Reserved */
  default_handler,            /* 10 Reserved */
  default_handler,            /* 11 SVCall handler */
  default_handler,            /* 12 Debug monitor handler */
  default_handler,            /* 13 Reserved */
  default_handler,            /* 14 The PendSV handler */
  clock_isr,                  /* 15 The SysTick handler */
  gpio_port_a_isr,            /* 16 GPIO Port A */
  gpio_port_b_isr,            /* 17 GPIO Port B */
  gpio_port_c_isr,            /* 18 GPIO Port C */
  gpio_port_d_isr,            /* 19 GPIO Port D */
  gpio_port_e_isr,            /* 20 GPIO Port E */
  uart_isr,                   /* 21 UART0 Rx and Tx */
  default_handler,            /* 22 UART1 Rx and Tx */
  default_handler,            /* 23 SSI0 Rx and Tx */
  default_handler,            /* 24 I2C0 Master and Slave */
  default_handler,            /* 25 PWM Fault */
  default_handler,            /* 26 PWM Generator 0 */
  default_handler,            /* 27 PWM Generator 1 */
  default_handler,            /* 28 PWM Generator 2 */
  default_handler,            /* 29 Quadrature Encoder 0 */
  default_handler,            /* 30 ADC0 Sequence 0 */
  default_handler,            /* 31 ADC0 Sequence 1 */
  default_handler,            /* 32 ADC0 Sequence 2 */
  default_handler,            /* 33 ADC0 Sequence 3 */
  watchdog_isr,               /* 34 Watchdog timer */
  default_handler,            /* 35 Timer 0 subtimer A */
  default_handler,            /* 36 Timer 0 subtimer B */
  default_handler,            /* 37 Timer 1 subtimer A */
  default_handler,            /* 38 Timer 1 subtimer B */
  default_handler,            /* 39 Timer 2 subtimer A */
  default_handler,            /* 40 Timer 2 subtimer B */
  default_handler,            /* 41 Analog Comparator 0 */
  default_handler,            /* 42 Analog Comparator 1*/
  default_handler,            /* 43 Analog Comparator 2 */
  default_handler,            /* 44 System Control */
  default_handler,            /* 45 FLASH Control */
  gpio_port_f_isr,            /* 46 GPIO Port F */
  gpio_port_g_isr,            /* 47 GPIO Port G */
  gpio_port_h_isr,            /* 48 GPIO Port H */
  default_handler,            /* 49 UART2 Rx and Tx */
  default_handler,            /* 50 SSI1 Rx and Tx */
  default_handler,            /* 51 Timer 3 subtimer A */
  default_handler,            /* 52 Timer 3 subtimer B */
  default_handler,            /* 53 I2C1 Master and Slave */
  default_handler,            /* 54 CAN0 */
  default_handler,            /* 55 CAN1 */
  EthernetIntHandler,         /* 56 Ethernet */
  rtimer_isr,                 /* 57 Hibernate */
  default_handler,            /* 58 USB0 */
  default_handler,            /* 59 PWM Generator 3 */
  default_handler,            /* 60 uDMA Software Transfer */
  default_handler,            /* 61 uDMA Error */
  default_handler,            /* 62 ADC1 Sequence 0 */
  default_handler,            /* 63 ADC1 Sequence 1 */
  default_handler,            /* 64 ADC1 Sequence 2 */
  default_handler,            /* 65 ADC1 Sequence 3*/
  default_handler,            /* 66 External Bus Interface 0 */
  gpio_port_j_isr,            /* 67 GPIO Port J */
  gpio_port_k_isr,            /* 68 GPIO Port K */
  gpio_port_l_isr,            /* 69 GPIO Port L */
  default_handler,            /* 70 SSI2 Rx and Tx */
  default_handler,            /* 71 SSI3 Rx and Tx */
  default_handler,            /* 72 UART3 Rx and Tx */
  default_handler,            /* 73 UART4 Rx and Tx */
  default_handler,            /* 74 UART5 Rx and Tx */
  default_handler,            /* 75 UART6 Rx and Tx */
  default_handler,            /* 76 UART7 Rx and Tx */
  default_handler,            /* 77 I2C2 Master and Slave */
  default_handler,            /* 78 I2C3 Master and Slave */
  default_handler,            /* 79 Timer 4 subtimer A */
  default_handler,            /* 80 Timer 4 subtimer B */
  default_handler,            /* 81 Timer 5 subtimer A */
  default_handler,            /* 82 Timer 5 subtimer B */
  default_handler,            /* 83 FPU */
  default_handler,            /* 84 Reserved */
  default_handler,            /* 85 Reserved */
  default_handler,            /* 86 I2C4 Master and Slave */
  default_handler,            /* 87 I2C5 Master and Slave */
  gpio_port_m_isr,            /* 88 GPIO Port M */
  gpio_port_n_isr,            /* 89 GPIO Port N */
  default_handler,            /* 90 Reserved */
  default_handler,            /* 91 Tamper */
  gpio_port_p_isr,            /* 92 GPIO Port P (Summary or P0) */
  default_handler,            /* 93 GPIO Port P1 */
  default_handler,            /* 94 GPIO Port P2 */
  default_handler,            /* 95 GPIO Port P3 */
  default_handler,            /* 96 GPIO Port P4 */
  default_handler,            /* 97 GPIO Port P5 */
  default_handler,            /* 98 GPIO Port P6 */
  default_handler,            /* 99 GPIO Port P7 */
  gpio_port_q_isr,            /* 100 GPIO Port Q (Summary or Q0) */
  default_handler,            /* 101 GPIO Port Q1 */
  default_handler,            /* 102 GPIO Port Q2 */
  default_handler,            /* 103 GPIO Port Q3 */
  default_handler,            /* 104 GPIO Port Q4 */
  default_handler,            /* 105 GPIO Port Q5 */
  default_handler,            /* 106 GPIO Port Q6 */
  default_handler,            /* 107 GPIO Port Q7 */
  gpio_port_r_isr,            /* 108 GPIO Port R */
  gpio_port_s_isr,            /* 109 GPIO Port S */
  default_handler,            /* 110 SHA/MD5 0 */
  default_handler,            /* 111 AES 0 */
  default_handler,            /* 112 DES3DES 0 */
  default_handler,            /* 113 LCD Controller 0 */
  default_handler,            /* 114 Timer 6 subtimer A */
  default_handler,            /* 115 Timer 6 subtimer B */
  default_handler,            /* 116 Timer 7 subtimer A */
  default_handler,            /* 117 Timer 7 subtimer B */
  default_handler,            /* 118 I2C6 Master and Slave */
  default_handler,            /* 119 I2C7 Master and Slave */
  default_handler,            /* 120 Reserved */
  default_handler,            /* 121 One Wire 0 */
  default_handler,            /* 122 Reserved */
  default_handler,            /* 123 Reserved */
  default_handler,            /* 124 Reserved */
  default_handler,            /* 125 I2C8 Master and Slave */
  default_handler,            /* 126 I2C9 Master and Slave */
  gpio_port_t_isr,            /* 127 GPIO Port T*/
};
/*---------------------------------------------------------------------------*/
/* Linker constructs indicating .data and .bss segment locations */
extern unsigned long _etext;
extern unsigned long _data;
extern unsigned long _edata;
extern unsigned long _bss;
extern unsigned long _ebss;
/*---------------------------------------------------------------------------*/
/* Weak interrupt handlers. */
void
fault_handler(void)
{
  printf("*** Fault handler ***\n");
  while(1);
}
/*---------------------------------------------------------------------------*/
void
nmi_handler(void)
{
  reset_handler();
  while(1);
}
/*---------------------------------------------------------------------------*/
void
default_handler(void)
{
  printf("*** Default handler ***\n");
  while(1);
}
/*---------------------------------------------------------------------------*/
void
reset_handler(void)
{
  unsigned long *pul_src, *pul_dst;

  /* Copy the data segment initializers from flash to SRAM. */
  pul_src = &_etext;

  for(pul_dst = &_data; pul_dst < &_edata;) {
    *pul_dst++ = *pul_src++;
  }

  /* Zero-fill the bss segment. */
  __asm("    ldr     r0, =_bss\n"
        "    ldr     r1, =_ebss\n"
        "    mov     r2, #0\n"
        "    .thumb_func\n"
        "zero_loop:\n"
        "        cmp     r0, r1\n"
        "        it      lt\n"
        "        strlt   r2, [r0], #4\n" "        blt     zero_loop");

  /* call the application's entry point. */
  main();

  /* End here if main () returns */
  while(1);
}
/*---------------------------------------------------------------------------*/

/** @} */
