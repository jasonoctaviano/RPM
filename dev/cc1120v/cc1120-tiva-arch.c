/*
 * Copyright (c) 2012, Thingsquare, http://www.thingsquare.com/.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "sys/energest.h"

#include "dev/tivaware/inc/hw_memmap.h"
#include "dev/tivaware/inc/hw_gpio.h"
#include "dev/tivaware/inc/hw_types.h"
#include "dev/tivaware/inc/hw_ssi.h"
#include "dev/tivaware/inc/hw_ints.h"

#include "dev/tivaware/driverlib/tm4c_gpio.h"
#include "dev/tivaware/driverlib/tm4c_pin_map.h"
#include "dev/tivaware/driverlib/tm4c_ssi.h"
#include "dev/tivaware/driverlib/tm4c_sysctl.h"

#include "contiki.h"
#include "contiki-net.h"
#include "dev/serial-line.h"
//#include "dev/cc11xx-arch.h"
#include "dev/spi.h"
#include "dev/leds.h"
#include "lpm.h"

#if defined(TARGET_IS_BLIZZARD_RA1) || defined(TARGET_IS_BLIZZARD_RA3) || \
    defined(TARGET_IS_BLIZZARD_RB0) || defined(TARGET_IS_BLIZZARD_RB1)

#define CC1120_GDO0_PORT GPIO_PORTB_BASE
#define CC1120_GDO0_PIN  GPIO_PIN_2

#define CC1120_GDO2_PORT GPIO_PORTB_BASE
#define CC1120_GDO2_PIN  GPIO_PIN_5

#define CC1120_GDO3_PORT GPIO_PORTA_BASE
#define CC1120_GDO3_PIN  GPIO_PIN_2

#define CC1120_SPI_CSN_PORT GPIO_PORTE_BASE
#define CC1120_SPI_CSN_PIN  GPIO_PIN_0

#define CC1120_SPI_MOSI_PORT GPIO_PORTB_BASE
#define CC1120_SPI_MOSI_PIN  GPIO_PIN_7

#define CC1120_SPI_MISO_PORT GPIO_PORTB_BASE
#define CC1120_SPI_MISO_PIN  GPIO_PIN_6

#define CC1120_SPI_SCLK_PORT GPIO_PORTB_BASE
#define CC1120_SPI_SCLK_PIN  GPIO_PIN_4

#define CC1120_RESET_PORT GPIO_PORTA_BASE
#define CC1120_RESET_PIN GPIO_PIN_4

#elif defined(PART_TM4C129XNCZAD)

#define CC1120_GDO0_PORT GPIO_PORTD_BASE
#define CC1120_GDO0_PIN  GPIO_PIN_1

#define CC1120_GDO2_PORT GPIO_PORTD_BASE
#define CC1120_GDO2_PIN  GPIO_PIN_0

#define CC1120_GDO3_PORT GPIO_PORTD_BASE
#define CC1120_GDO3_PIN  GPIO_PIN_5

#define CC1120_SPI_CSN_PORT GPIO_PORTJ_BASE
#define CC1120_SPI_CSN_PIN  GPIO_PIN_3

#define CC1120_SPI_MOSI_PORT GPIO_PORTA_BASE
#define CC1120_SPI_MOSI_PIN  GPIO_PIN_4

#define CC1120_SPI_MISO_PORT GPIO_PORTA_BASE
#define CC1120_SPI_MISO_PIN  GPIO_PIN_5

#define CC1120_SPI_SCLK_PORT GPIO_PORTA_BASE
#define CC1120_SPI_SCLK_PIN  GPIO_PIN_2

#elif defined(PART_TM4C1294NCPDT)

#define CC1120_GDO0_PORT GPIO_PORTM_BASE
#define CC1120_GDO0_PIN  GPIO_PIN_3

#define CC1120_GDO2_PORT GPIO_PORTE_BASE
#define CC1120_GDO2_PIN  GPIO_PIN_4

#define CC1120_GDO3_PORT GPIO_PORTN_BASE
#define CC1120_GDO3_PIN  GPIO_PIN_3

#define CC1120_SPI_CSN_PORT GPIO_PORTH_BASE
#define CC1120_SPI_CSN_PIN  GPIO_PIN_2

#define CC1120_SPI_MOSI_PORT GPIO_PORTD_BASE
#define CC1120_SPI_MOSI_PIN  GPIO_PIN_1

#define CC1120_SPI_MISO_PORT GPIO_PORTD_BASE
#define CC1120_SPI_MISO_PIN  GPIO_PIN_0

#define CC1120_SPI_SCLK_PORT GPIO_PORTD_BASE
#define CC1120_SPI_SCLK_PIN  GPIO_PIN_3

#define CC1120_RESET_PORT GPIO_PORTN_BASE
#define CC1120_RESET_PIN GPIO_PIN_2

#endif

int cc11xx_rx_interrupt(void);
/*---------------------------------------------------------------------------*/
void
cc1120_arch_spi_enable(void)
{
  /* Set CSn to low (0) */
  GPIOPinTypeGPIOOutput(CC1120_SPI_CSN_PORT, CC1120_SPI_CSN_PIN);
  GPIOPinWrite(CC1120_SPI_CSN_PORT, CC1120_SPI_CSN_PIN, ~CC1120_SPI_CSN_PIN);

  /* The MISO pin should go high before chip is fully enabled.*/
  GPIODirModeSet(CC1120_SPI_MISO_PORT, CC1120_SPI_MISO_PIN, GPIO_DIR_MODE_HW);
  while((GPIOPinRead(CC1120_SPI_MISO_PORT, CC1120_SPI_MISO_PIN)&CC1120_SPI_MISO_PIN) != 0);
}
/*---------------------------------------------------------------------------*/
void
cc1120_arch_spi_disable(void)
{
  /* Set CSn to high (1) */
  GPIOPinTypeGPIOOutput(CC1120_SPI_CSN_PORT, CC1120_SPI_CSN_PIN);
  GPIOPinWrite(CC1120_SPI_CSN_PORT, CC1120_SPI_CSN_PIN, CC1120_SPI_CSN_PIN);
}
/*---------------------------------------------------------------------------*/
uint8_t
cc1120_arch_spi_rw_byte(unsigned char c)
{
  uint32_t ui32sendData=0x00000000;
  uint32_t ui32dummyData;
  ui32sendData += c;
  //
  // Wait until SSI0 is done transferring all the data in the transmit FIFO.
  //
  while(SSIBusy(SSI_BASE))
  {
  }
  //
  // Send the data using the "blocking" put function.  This function
  // will wait until there is room in the send FIFO before returning.
  // This allows you to assure that all the data you send makes it into
  // the send FIFO.
  //
  SSIDataPut(SSI_BASE, ui32sendData);
  while(!((HWREG(SSI_BASE + SSI_O_SR) & SSI_SR_TFE) && ((HWREG(SSI_BASE + SSI_O_SR) & SSI_SR_RNE))));
  //
  // Wait until SSI0 is done transferring all the data in the transmit FIFO.
  // Wait for TX ready
  while(SSIBusy(SSI_BASE))
  {
  }
  //
  // Receive the data using the "blocking" Get function. This function
  // will wait until there is data in the receive FIFO before returning.
  //
  SSIDataGet(SSI_BASE, &ui32sendData);
  while(SSIBusy(SSI_BASE))
  {
  }

  while(SSIDataGetNonBlocking(SSI_BASE, &ui32dummyData))
  {
  }
  //SSIDataGetNonBlocking(SSI0_BASE, &ui32sendData);
  //printf("rxbytes here %d\n",ui32sendData);
  ui32sendData &= 0x00FF;
  // transfer to unsigned char
  c = (unsigned char)ui32sendData;
  //printf("out put value: %u\n",c);
  return c;
}
/*---------------------------------------------------------------------------*/
int
cc1120_arch_spi_rw(unsigned char *inbuf, unsigned char *outbuf, int len)
{

  int i;
  if(inbuf == NULL && outbuf == NULL) {
    /* error: both buffers are NULL */
    return 1;
  } 
  else if(inbuf == NULL)
  {
    for(i = 0; i < len; i++) {
      spi_write(outbuf[i]);
    }
  } 
  else if(outbuf == NULL) 
  {
    for(i = 0; i < len; i++)
    {
      spi_read(&inbuf[i]);
    }
  } 
  else 
  {
    for(i = 0; i < len; i++) 
    {
      spi_write_fast(outbuf[i]);
      spi_read(&inbuf[i]);
    }
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
void
cc1120_arch_init(void)
{

  spi_init();
#if defined(TARGET_IS_BLIZZARD_RA1) || defined(TARGET_IS_BLIZZARD_RA3) || \
    defined(TARGET_IS_BLIZZARD_RB0) || defined(TARGET_IS_BLIZZARD_RB1)
  /*Enable the clock for PortE*/
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
#elif defined(PART_TM4C129XNCZAD)
  /*Enable the clock for PortJ*/
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
#elif defined(PART_TM4C1294NCPDT)
  /*Enable the clock for PortJ*/
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
  printf("software comes here 0\n");
#endif
  /* Unselect radio. */
  cc1120_arch_spi_disable();
printf("software comes here 1\n");
#if defined(TARGET_IS_BLIZZARD_RA1) || defined(TARGET_IS_BLIZZARD_RA3) || \
    defined(TARGET_IS_BLIZZARD_RB0) || defined(TARGET_IS_BLIZZARD_RB1)
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  //SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
  /*Set RESET pin high on CC1120*/
  GPIOPinTypeGPIOOutput(CC1120_RESET_PORT, CC1120_RESET_PIN);
  GPIOPinWrite(CC1120_RESET_PORT, CC1120_RESET_PIN, CC1120_RESET_PIN);

#elif defined(PART_TM4C129XNCZAD)
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

#elif defined(PART_TM4C1294NCPDT)
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
  /*Set RESET pin high on CC1120*/
  GPIOPinTypeGPIOOutput(CC1120_RESET_PORT, CC1120_RESET_PIN);
  GPIOPinWrite(CC1120_RESET_PORT, CC1120_RESET_PIN, CC1120_RESET_PIN);
printf("software comes here 2\n");
#endif


  GPIOPinTypeGPIOInput(CC1120_GDO0_PORT, CC1120_GDO0_PIN);
  GPIOPinTypeGPIOInput(CC1120_GDO2_PORT, CC1120_GDO2_PIN);
  GPIOPinTypeGPIOInput(CC1120_GDO3_PORT, CC1120_GDO3_PIN);
printf("software comes here 3\n");
  /* Reset procedure */
  GPIOPinWrite(CC1120_SPI_SCLK_PORT, CC1120_SPI_SCLK_PIN, CC1120_SPI_SCLK_PIN);
  GPIOPinWrite(CC1120_SPI_MOSI_PORT, CC1120_SPI_MOSI_PIN, CC1120_SPI_MOSI_PIN);

  GPIOPinWrite(CC1120_SPI_CSN_PORT, CC1120_SPI_CSN_PIN, ~CC1120_SPI_CSN_PIN);
  GPIOPinWrite(CC1120_SPI_CSN_PORT, CC1120_SPI_CSN_PIN, CC1120_SPI_CSN_PIN);

  clock_delay_usec(400);

  /* Rising edge interrupt; note that GPIO-pins are hardwired to 0/1/tristate
   * (depending on settings) when cc1120 is in SLEEP state (SPWD strobe). See
   * CC1120 datasheet, GPIO section.
   */
  /*Modify the trrigger edge*/
  GPIOIntTypeSet(CC1120_GDO0_PORT, CC1120_GDO0_PIN, GPIO_RISING_EDGE);
  //GPIOIntTypeSet(CC1120_GDO2_PORT, CC1120_GDO2_PIN, GPIO_RISING_EDGE);
  //GPIOIntTypeSet(CC1120_GDO3_PORT, CC1120_GDO3_PIN, GPIO_RISING_EDGE)
  GPIOPinWrite(CC1120_SPI_CSN_PORT, CC1120_SPI_CSN_PIN, ~CC1120_SPI_CSN_PIN);

  GPIODirModeSet(CC1120_SPI_MISO_PORT, CC1120_SPI_MISO_PIN, GPIO_DIR_MODE_HW);
  while((GPIOPinRead(CC1120_SPI_MISO_PORT, CC1120_SPI_MISO_PIN)&CC1120_SPI_MISO_PIN) != 0);

}
/*---------------------------------------------------------------------------*/
void
cc1120_arch_interrupt_enable(void)
{
  /* Reset interrupt trigger */
  GPIOIntClear(CC1120_GDO0_PORT,CC1120_GDO0_PIN);
  //GPIOIntClear(CC1120_GDO2_PORT,CC1120_GDO2_PIN);
  //GPIOIntClear(CC1120_GDO3_PORT,CC1120_GDO3_PIN);

  /* Enable interrupt on the GDO0 pin */
  GPIOIntEnable(CC1120_GDO0_PORT, CC1120_GDO0_PIN);
  //GPIOIntEnable(CC1120_GDO2_PORT, CC1120_GDO2_PIN);
  //GPIOIntEnable(CC1120_GDO3_PORT, CC1120_GDO3_PIN);
#if defined(TARGET_IS_BLIZZARD_RA1) || defined(TARGET_IS_BLIZZARD_RA3) || \
    defined(TARGET_IS_BLIZZARD_RB0) || defined(TARGET_IS_BLIZZARD_RB1)
  IntEnable(INT_GPIOB_BLIZZARD);

#elif defined(PART_TM4C129XNCZAD)
  IntEnable(INT_GPIOD_BLIZZARD);

#elif defined(PART_TM4C1294NCPDT)
  IntEnable(INT_GPIOM);
#endif
}
/*---------------------------------------------------------------------------*/
#if 0
void
clock_delay_usec(uint16_t usec)
{
  clock_delay(usec / 100);
}
#endif
/*---------------------------------------------------------------------------*/
/** \brief Interrupt service routine for Port D */
void
gpio_port_d_isr()
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  if(GPIOIntStatus(CC1120_GDO0_PORT,1) == CC1120_GDO0_PIN)
  {
    //printf("Comes into interrupt all the time\n");
    if(cc11xx_rx_interrupt( )) 
    {
      //printf("Finishe receive packet\n");
      lpm_exit();
    }
  }
  HWREG(GPIO_PORTD_BASE | GPIO_O_ICR) = 0xFF;
  /* Reset interrupt trigger */
  GPIOIntClear(CC1120_GDO0_PORT,CC1120_GDO0_PIN);
  //GPIOIntClear(CC1120_GDO2_PORT,CC1120_GDO2_PIN);
  //GPIOIntClear(CC1120_GDO3_PORT,CC1120_GDO3_PIN);
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}

/** \brief Interrupt service routine for Port D */
void
gpio_port_b_isr()
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  if(GPIOIntStatus(CC1120_GDO0_PORT,1) == CC1120_GDO0_PIN)
  {
    //printf("Comes into interrupt all the time\n");
    if(cc11xx_rx_interrupt( )) 
    {
      //printf("Finishe receive packet\n");
      lpm_exit();
    }
  }
  HWREG(GPIO_PORTB_BASE | GPIO_O_ICR) = 0xFF;
  /* Reset interrupt trigger */
  GPIOIntClear(CC1120_GDO0_PORT,CC1120_GDO0_PIN);
  //GPIOIntClear(CC1120_GDO2_PORT,CC1120_GDO2_PIN);
  //GPIOIntClear(CC1120_GDO3_PORT,CC1120_GDO3_PIN);
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/** \brief Interrupt service routine for Port m */
void
gpio_port_m_isr()
{
  ENERGEST_ON(ENERGEST_TYPE_IRQ);
  if(GPIOIntStatus(CC1120_GDO0_PORT,1) == CC1120_GDO0_PIN)
  {
    if(cc11xx_rx_interrupt( )) 
    {
      //printf("Finishe receive packet\n");
      lpm_exit();
    }
  }
  HWREG(GPIO_PORTM_BASE | GPIO_O_ICR) = 0xFF;
  /* Reset interrupt trigger */
  GPIOIntClear(CC1120_GDO0_PORT,CC1120_GDO0_PIN);
  //GPIOIntClear(CC1120_GDO2_PORT,CC1120_GDO2_PIN);
  //GPIOIntClear(CC1120_GDO3_PORT,CC1120_GDO3_PIN);
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
/*---------------------------------------------------------------------------*/

