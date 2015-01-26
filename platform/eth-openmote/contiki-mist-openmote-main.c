/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
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
 */
/**
 * \addtogroup platform
 * @{
 *
 * \defgroup cc2538 The cc2538 Development Kit platform
 *
 * The cc2538DK is the new platform by Texas Instruments, based on the
 * cc2530 SoC with an ARM Cortex-M3 core.
 * @{
 *
 * \file
 *   Main module for the cc2538dk platform
 */
/*---------------------------------------------------------------------------*/ 
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "contiki.h"
#include "dev/leds.h"
#include "dev/sys-ctrl.h"
#include "dev/scb.h"
#include "dev/nvic.h"
#include "dev/uart.h"
#include "dev/watchdog.h"
#include "dev/ioc.h"
#include "dev/button-sensor.h"
#include "dev/serial-line.h"
#include "dev/slip.h"
#include "dev/cc2538-rf.h"
#include "dev/udma.h"
#include "usb/usb-serial.h"
#include "lib/random.h"
#include "net/netstack.h"
#include "net/mac/frame802154.h"
#include "cpu.h"
#include "ieee-addr.h"
#include "lpm.h"
#include "net/rime.h"
#include "sys/autostart.h"
#include "sys/profile.h"

#include "node-id.h"
#include "mist.h"

#include "cpu.h"

#if WITH_UIP6
#include "net/uip-ds6.h"
#endif /* WITH_UIP6 */
/*---------------------------------------------------------------------------*/

#if NETSTACK_AES_KEY_DEFAULT
#warning Using default AES key "thingsquare mist", change it in project-conf.h like this:
#warning #define NETSTACK_AES_KEY {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}
#endif /* NETSTACK_AES_KEY */

#define DEBUG 1

#if STARTUP_CONF_VERBOSE
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#if UART_CONF_ENABLE
#define PUTS(s) puts(s)
#else
#define PUTS(s)
#endif

uint32_t sys_clock = 0;
uint32_t state = 0;

static struct etimer et, et2;

//SENSORS(&button_sensor);
/*---------------------------------------------------------------------------*/
#ifndef RF_CHANNEL
#define RF_CHANNEL              10
#endif

/*---------------------------------------------------------------------------*/
static void
fade(unsigned char l)
{
  volatile int i;
  int k, j;
  for(k = 0; k < 800; ++k) {
    j = k > 400 ? 800 - k : k;

    leds_on(l);
    for(i = 0; i < j; ++i) {
      asm("nop");
    }
    leds_off(l);
    for(i = 0; i < 400 - j; ++i) {
      asm("nop");
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
set_rime_addr(void)
{
  rimeaddr_t addr;
  uint8_t i;
  memset(&addr, 0, sizeof(rimeaddr_t));
#if UIP_CONF_IPV6
  memcpy(addr.u8, node_mac, sizeof(addr.u8));
#else
  if(node_id == 0) {
    for(i = 0; i < sizeof(rimeaddr_t); ++i) {
      addr.u8[i] = node_mac[7 - i];
    }
  } else {
    addr.u8[0] = node_id & 0xff;
    addr.u8[1] = node_id >> 8;
  }
#endif
  rimeaddr_set_node_addr(&addr);
  printf("Rime addr ");
  for(i = 0; i < sizeof(addr.u8) - 1; i++) {
    printf("%u.", addr.u8[i]);
  }
  printf("%u\r\n", addr.u8[i]);
}

/*---------------------------------------------------------------------------*/
/**
 * \brief Main routine for the cc2538dk platform
 */
int
main(int argc, char **argv)
{
  nvic_init();
  sys_ctrl_init();
  clock_init();

  /*Disable EA*/
  dint();

  watchdog_init();
  /*Enable EA*/
  //eint();

  rtimer_init();
  lpm_init();
  gpio_init();
  ioc_init();  
  leds_init();
  fade(LEDS_YELLOW);

  /*
   * Character I/O Initialisation.
   * When the UART receives a character it will call serial_line_input_byte to
   * notify the core. The same applies for the USB driver.
   *
   * If slip-arch is also linked in afterwards (e.g. if we are a border router)
   * it will overwrite one of the two peripheral input callbacks. Characters
   * received over the relevant peripheral will be handled by
   * slip_input_byte instead
   */
#if UART_CONF_ENABLE
  uart_init(0);
  uart_init(1);
  uart_set_input(SERIAL_LINE_CONF_UART, serial_line_input_byte);
#endif

  /*Enable EA*/
  eint();

#if USB_SERIAL_CONF_ENABLE
  usb_serial_init();
  usb_serial_set_input(serial_line_input_byte);
#endif

  serial_line_init();

  INTERRUPTS_ENABLE();
  fade(LEDS_GREEN);
  PRINTF("=================================\r\n");
  PUTS(CONTIKI_VERSION_STRING);
  PRINTF("======================\r\n");  
  PRINTF("\r\n");  
  PUTS(BOARD_STRING);
  PRINTF("\r\n");

  /* Restore node id if such has been stored in external mem */
#ifdef NODEID
  node_id = NODEID;
#ifdef BURN_NODEID
  node_id_burn(node_id);
  node_id_restore(); /* also configures node_mac[] */
#endif /* BURN_NODEID */
#else
  node_id_restore(); /* also configures node_mac[] */
#endif /* NODE_ID */

  /* for setting "hardcoded" IEEE 802.15.4 MAC addresses */
#ifdef MAC_1
  {
    uint8_t ieee[] = { MAC_1, MAC_2, MAC_3, MAC_4, MAC_5, MAC_6, MAC_7, MAC_8 };
    memcpy(node_mac, ieee, sizeof(uip_lladdr.addr));
  }
#endif

  /*
   * Initialize Contiki and our processes.
   */
  process_init();
  process_start(&sensors_process, NULL);
  button_sensor_init();
  process_start(&etimer_process, NULL);
  ctimer_init();

  set_rime_addr();
  // netstack_init();
  // cc2538_rf_set_addr(IEEE802154_PANID);

  // PRINTF(" Net: ");
  // PRINTF("%s\r\n", NETSTACK_NETWORK.name);
  // PRINTF(" MAC: ");
  // PRINTF("%s\r\n", NETSTACK_MAC.name);
  // PRINTF(" RDC: ");
  // PRINTF("%s\r\n", NETSTACK_RDC.name);

  /* Initialise the H/W RNG engine. */
  random_init(0);

  udma_init();

  if(node_id > 0) {
    printf("Node id %u.\r\n", node_id);
  } else {
    printf("Node id not set.\r\n");
  }


#if WITH_UIP6
  memcpy(&uip_lladdr.addr, node_mac, sizeof(uip_lladdr.addr));
  /* Setup nullmac-like MAC for 802.15.4 */

  queuebuf_init();

  netstack_init();
  cc2538_rf_set_addr(IEEE802154_PANID);
  printf("%s/%s %lu %u\r\n",
         NETSTACK_RDC.name,
         NETSTACK_MAC.name,
         CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval() == 0 ? 1:
                         NETSTACK_RDC.channel_check_interval()),
         RF_CHANNEL);

  process_start(&tcpip_process, NULL);

  printf("IPv6 ");
  {
    uip_ds6_addr_t *lladdr;
    int i;
    lladdr = uip_ds6_get_link_local(-1);
    for(i = 0; i < 7; ++i) {
      printf("%02x%02x:", lladdr->ipaddr.u8[i * 2],
             lladdr->ipaddr.u8[i * 2 + 1]);
    }
    printf("%02x%02x\r\n", lladdr->ipaddr.u8[14], lladdr->ipaddr.u8[15]);
  }

  if(1) {
    uip_ipaddr_t ipaddr;
    int i;
    uip_ip6addr(&ipaddr, 0xfc00, 0, 0, 0, 0, 0, 0, 0);
    uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
    uip_ds6_addr_add(&ipaddr, 0, ADDR_TENTATIVE);
    printf("Tentative global IPv6 address ");
    for(i = 0; i < 7; ++i) {
      printf("%02x%02x:",
             ipaddr.u8[i * 2], ipaddr.u8[i * 2 + 1]);
    }
    printf("%02x%02x\r\n",
           ipaddr.u8[7 * 2], ipaddr.u8[7 * 2 + 1]);
  }

#else /* WITH_UIP6 */

  netstack_init();
  cc2538_rf_set_addr(IEEE802154_PANID);
  printf("%s %lu %u\r\n",
         NETSTACK_RDC.name,
         CLOCK_SECOND / (NETSTACK_RDC.channel_check_interval() == 0? 1:
                         NETSTACK_RDC.channel_check_interval()),
         RF_CHANNEL);
#endif /* WITH_UIP6 */

#if !WITH_UIP6
  uart1_set_input(serial_line_input_byte);
  serial_line_init();
#endif

//   NETSTACK_RDC.off(1);

//   {
// #ifndef NETSTACK_AES_KEY
// #define NETSTACK_AES_KEY "thingsquare mist"
// #define NETSTACK_AES_KEY_DEFAULT 1
// #endif /* NETSTACK_AES_KEY */

// #if NETSTACK_AES_KEY_DEFAULT
// #warning Using default AES key "thingsquare mist", change it in project-conf.h like this:
// #warning #define NETSTACK_AES_KEY {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}
// #endif /* NETSTACK_AES_KEY */

//     const uint8_t key[] = NETSTACK_AES_KEY;
//     printf("Setting AES key\r\n");
//     netstack_aes_set_key(key);
//   }

#ifdef NETSTACK_AES_H
#ifndef NETSTACK_AES_KEY
#error Please define NETSTACK_AES_KEY!
#endif /* NETSTACK_AES_KEY */
  {
    const uint8_t key[] = NETSTACK_AES_KEY;
    netstack_aes_set_key(key);
  }
  /*printf("AES encryption is enabled: '%s'\n", NETSTACK_AES_KEY);*/
  printf("AES encryption is enabled\r\n");
#else /* NETSTACK_AES_H */
  printf("Warning: AES encryption is disabled\r\n");
#endif /* NETSTACK_AES_H */

#if TIMESYNCH_CONF_ENABLED
  timesynch_init();
  timesynch_set_authority_level(rimeaddr_node_addr.u8[0]);
#endif /* TIMESYNCH_CONF_ENABLED */

  energest_init();
  ENERGEST_ON(ENERGEST_TYPE_CPU);

  simple_rpl_init();

  /* Reset watchdog and handle polls and events */
  printf("Watchdog start\r\n");
  watchdog_periodic();
  watchdog_start();

  printf("Autostart start\r\n");
  autostart_start(autostart_processes);
  fade(LEDS_ORANGE);

#if IP64_CONF_UIP_FALLBACK_INTERFACE_SLIP && WITH_SLIP
  /* Start the SLIP */
  printf("Initiating SLIP: my IP is 172.16.0.2...\r\n");
  slip_arch_init(0);
  {
    uip_ip4addr_t ipv4addr, netmask;

    uip_ipaddr(&ipv4addr, 172, 16, 0, 2);
    uip_ipaddr(&netmask, 255, 255, 255, 0);
    ip64_set_ipv4_address(&ipv4addr, &netmask);
  }
  uart1_set_input(slip_input_byte);
#endif /* IP64_CONF_UIP_FALLBACK_INTERFACE_SLIP */

  while(1) {
    uint8_t r;
    do {
      watchdog_periodic();
      r = process_run();
    } while(r > 0);

    /* We have serviced all pending events. Enter a Low-Power mode. */
    lpm_enter();
  }
}
/*---------------------------------------------------------------------------*/

/**
 * @}
 * @}
 */
