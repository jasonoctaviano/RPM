/**
 * \addtogroup openmote
 * @{
 *
 * \file
 *  Configuration for the OpenMote-CC2538 platform
 */
#ifndef CONTIKI_CONF_H_
#define CONTIKI_CONF_H_

#include <stdint.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
/*************************************/
#include "platform-conf.h"

#include "mist-conf-const.h"

#ifndef MIST_CONF_NETSTACK
#define MIST_CONF_NETSTACK (MIST_CONF_DROWSIE)
#endif /* MIST_CONF_NETSTACK */

#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE 8
#define NULLRDC_CONF_802154_AUTOACK 1
#define NETSTACK_CONF_FRAMER  framer_802154
//#define NETSTACK_CONF_NETWORK sicslowpan_driver

//#ifndef CC2420_CONF_AUTOACK
//#define CC2420_CONF_AUTOACK              1
//#endif /* CC2420_CONF_AUTOACK */

//#ifndef CC2520_CONF_AUTOACK
//#define CC2520_CONF_AUTOACK              1
//#endif /* CC2520_CONF_AUTOACK */

#if WITH_UIP6
/* Network setup for IPv6 */
#define NETSTACK_CONF_NETWORK sicslowpan_driver
/* Specify a minimum packet size for 6lowpan compression to be
   enabled. This is needed for ContikiMAC, which needs packets to be
   larger than a specified size, if no ContikiMAC header should be
   used. */
#define SICSLOWPAN_CONF_COMPRESSION_THRESHOLD 0
//#define SICSLOWPAN_CONF_MAC_MAX_PAYLOAD        40
#define CONTIKIMAC_CONF_WITH_CONTIKIMAC_HEADER 0

#define CXMAC_CONF_ANNOUNCEMENTS         0
#define XMAC_CONF_ANNOUNCEMENTS          0

#ifndef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM                8
#endif

#else /* WITH_UIP6 */

/* Network setup for non-IPv6 (rime). */

#define NETSTACK_CONF_NETWORK rime_driver

#define COLLECT_CONF_ANNOUNCEMENTS       1
#define CXMAC_CONF_ANNOUNCEMENTS         0
#define XMAC_CONF_ANNOUNCEMENTS          0
#define CONTIKIMAC_CONF_ANNOUNCEMENTS    0

#define CONTIKIMAC_CONF_COMPOWER         1
#define XMAC_CONF_COMPOWER               1
#define CXMAC_CONF_COMPOWER              1

#ifndef COLLECT_NEIGHBOR_CONF_MAX_COLLECT_NEIGHBORS
#define COLLECT_NEIGHBOR_CONF_MAX_COLLECT_NEIGHBORS     32
#endif /* COLLECT_NEIGHBOR_CONF_MAX_COLLECT_NEIGHBORS */

#ifndef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM                16
#endif /* QUEUEBUF_CONF_NUM */

#ifndef TIMESYNCH_CONF_ENABLED
#define TIMESYNCH_CONF_ENABLED           0
#endif /* TIMESYNCH_CONF_ENABLED */

#if TIMESYNCH_CONF_ENABLED
/* CC2420 SDF timestamps must be on if timesynch is enabled. */
#undef CC2420_CONF_SFD_TIMESTAMPS
#define CC2420_CONF_SFD_TIMESTAMPS       1
#endif /* TIMESYNCH_CONF_ENABLED */

#endif /* WITH_UIP6 */

#define PACKETBUF_CONF_ATTRS_INLINE 1

#ifndef RF_CHANNEL
#define RF_CHANNEL              26
#endif /* RF_CHANNEL */

#define CONTIKIMAC_CONF_BROADCAST_RATE_LIMIT 0

// #define IEEE802154_CONF_PANID       0xABCD

#define SHELL_VARS_CONF_RAM_BEGIN 0x1100
#define SHELL_VARS_CONF_RAM_END 0x2000

#define PROFILE_CONF_ON 0
#ifndef ENERGEST_CONF_ON
#define ENERGEST_CONF_ON 1
#endif /* ENERGEST_CONF_ON */

#define ELFLOADER_CONF_TEXT_IN_ROM 0
#ifndef ELFLOADER_CONF_DATAMEMORY_SIZE
#define ELFLOADER_CONF_DATAMEMORY_SIZE 0x400
#endif /* ELFLOADER_CONF_DATAMEMORY_SIZE */
#ifndef ELFLOADER_CONF_TEXTMEMORY_SIZE
#define ELFLOADER_CONF_TEXTMEMORY_SIZE 0x800
#endif /* ELFLOADER_CONF_TEXTMEMORY_SIZE */


#define AODV_COMPLIANCE
#define AODV_NUM_RT_ENTRIES 32

#define WITH_ASCII 1

#define PROCESS_CONF_NUMEVENTS 8
#define PROCESS_CONF_STATS 1
/*#define PROCESS_CONF_FASTPOLL    4*/

#ifdef WITH_UIP6

#define RIMEADDR_CONF_SIZE              8

#define UIP_CONF_LL_802154              1
#define UIP_CONF_LLH_LEN                0

#define UIP_CONF_ROUTER                 1
#ifndef UIP_CONF_IPV6_RPL
#define UIP_CONF_IPV6_RPL               1
#endif /* UIP_CONF_IPV6_RPL */

/* configure number of neighbors and routes */
#ifndef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS     30
#endif /* NBR_TABLE_CONF_MAX_NEIGHBORS */
#ifndef UIP_CONF_DS6_ROUTE_NBU
#define UIP_CONF_DS6_ROUTE_NBU   30
#endif /* UIP_CONF_DS6_ROUTE_NBU */

#define UIP_CONF_ND6_SEND_RA    0
#define UIP_CONF_ND6_REACHABLE_TIME     600000
#define UIP_CONF_ND6_RETRANS_TIMER      10000

#define UIP_CONF_IPV6                   1
#ifndef UIP_CONF_IPV6_QUEUE_PKT
#define UIP_CONF_IPV6_QUEUE_PKT         0
#endif /* UIP_CONF_IPV6_QUEUE_PKT */
#define UIP_CONF_IPV6_CHECKS            1
#define UIP_CONF_IPV6_REASSEMBLY        0
#define UIP_CONF_NETIF_MAX_ADDRESSES    3
#define UIP_CONF_ND6_MAX_PREFIXES       3
#define UIP_CONF_ND6_MAX_NEIGHBORS      4
#define UIP_CONF_ND6_MAX_DEFROUTERS     2
#define UIP_CONF_IP_FORWARD             0
#ifndef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE    280
#endif

#define SICSLOWPAN_CONF_COMPRESSION_IPV6        0
#define SICSLOWPAN_CONF_COMPRESSION_HC1         1
#define SICSLOWPAN_CONF_COMPRESSION_HC01        2
#define SICSLOWPAN_CONF_COMPRESSION             SICSLOWPAN_COMPRESSION_HC06
#ifndef SICSLOWPAN_CONF_FRAG
#define SICSLOWPAN_CONF_FRAG                    1

/* Unit: 1/16th second. 4 => 0.25s timeout */
#ifndef SICSLOWPAN_CONF_MAXAGE
#define SICSLOWPAN_CONF_MAXAGE                  4
#endif /* SICSLOWPAN_CONF_MAXAGE */

#if (MIST_CONF_NETSTACK & MIST_CONF_DROWSIE_MULTICHANNEL)
/* We need to increase the fragmentation timeout, as the multichannel protocol may transmit
 * the same fragment on two channels, causing up to 0.6s delay inbetween fragments. */
#ifdef SICSLOWPAN_CONF_MAXAGE
#if SICSLOWPAN_CONF_MAXAGE < 12
#undef SICSLOWPAN_CONF_MAXAGE
#define SICSLOWPAN_CONF_MAXAGE                  12
#endif /* SICSLOWPAN_CONF_MAXAGE < 12 */
#endif /* SICSLOWPAN_CONF_MAXAGE */
#endif /* (MIST_CONF_NETSTACK & MIST_CONF_DROWSIE_MULTICHANNEL) */

#endif /* SICSLOWPAN_CONF_FRAG */
#define SICSLOWPAN_CONF_CONVENTIONAL_MAC  1
#define SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS       2
#ifndef SICSLOWPAN_CONF_MAX_MAC_TRANSMISSIONS
#define SICSLOWPAN_CONF_MAX_MAC_TRANSMISSIONS   5
#endif /* SICSLOWPAN_CONF_MAX_MAC_TRANSMISSIONS */
#else /* WITH_UIP6 */
#define UIP_CONF_IP_FORWARD      1
#define UIP_CONF_BUFFER_SIZE     108
#endif /* WITH_UIP6 */

#define UIP_CONF_ICMP_DEST_UNREACH 1

#define UIP_CONF_DHCP_LIGHT
#define UIP_CONF_LLH_LEN         0
#ifndef UIP_CONF_RECEIVE_WINDOW
#define UIP_CONF_RECEIVE_WINDOW  100
#endif
#ifndef UIP_CONF_TCP_MSS
#define UIP_CONF_TCP_MSS         100
#endif
#define UIP_CONF_MAX_CONNECTIONS 4
#define UIP_CONF_MAX_LISTENPORTS 8
#define UIP_CONF_UDP_CONNS       12
#define UIP_CONF_FWCACHE_SIZE    30
#define UIP_CONF_BROADCAST       1

#define UIP_ARCH_IPCHKSUM        0

#define UIP_CONF_UDP             1
#define UIP_CONF_UDP_CHECKSUMS   1
#define UIP_CONF_PINGADDRCONF    0
#define UIP_CONF_LOGGING         0

/* include the project config */
/* PROJECT_CONF_H might be defined in the project Makefile */
#ifdef PROJECT_CONF_H
#include PROJECT_CONF_H
#endif /* PROJECT_CONF_H */

#include "mist-default-conf.h"

#if ((MIST_CONF_NETSTACK) & MIST_CONF_AES)
#ifndef NETSTACK_AES_KEY
#define NETSTACK_AES_KEY "thingsquare mist" /* 16 bytes */
#define NETSTACK_AES_KEY_DEFAULT 1
#endif /* NETSTACK_AES_KEY */
#endif /* ((MIST_CONF_NETSTACK) & MIST_CONF_AES) */


#define RIMESTATS_CONF_ON 1
#define RIMESTATS_CONF_ENABLED 1
/*************************************/
/*---------------------------------------------------------------------------*/
/* Compiler configuration and platform-specific type definitions */
//Define in platform.h
//#define CLOCK_CONF_SECOND 128

/* Compiler configurations */
#define CCIF
#define CLIF

/* Platform typedefs */
typedef uint32_t clock_time_t;
//typedef uint32_t uip_stats_t;

/*
 * rtimer.h typedefs rtimer_clock_t as unsigned short. We need to define
 * RTIMER_CLOCK_LT to override this
 */
typedef uint32_t rtimer_clock_t;
#define RTIMER_CLOCK_LT(a,b)     ((int32_t)((a)-(b)) < 0)

#define BV(x) (1<<(x))
/*---------------------------------------------------------------------------*/
/* Energest Module */
#ifndef ENERGEST_CONF_ON
#define ENERGEST_CONF_ON            0
#endif

/* Verbose Startup? */
#ifndef STARTUP_CONF_VERBOSE
#define STARTUP_CONF_VERBOSE        1
#endif
/*---------------------------------------------------------------------------*/
/* board.h assumes that basic configuration is done */
/* #include "board.h" */
/*---------------------------------------------------------------------------*/


/* Include Project Specific conf */
#ifdef PROJECT_CONF_H
#include PROJECT_CONF_H
#endif /* PROJECT_CONF_H */
/*---------------------------------------------------------------------------*/
/**
 * \name Compiler configuration and platform-specific type definitions
 *
 * Those values are not meant to be modified by the user
 * @{
 */
#define CLOCK_CONF_SECOND 128

/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Serial Boot Loader Backdoor configuration
 *
 * @{
 */
#ifndef FLASH_CCA_CONF_BOOTLDR_BACKDOOR
#define FLASH_CCA_CONF_BOOTLDR_BACKDOOR	1 /**<Enable the boot loader backdoor */
#endif

#ifndef FLASH_CCA_CONF_BOOTLDR_BACKDOOR_PORT_A_PIN
#define FLASH_CCA_CONF_BOOTLDR_BACKDOOR_PORT_A_PIN 6 /**< Pin PA_6 (Select button) activates the boot loader */
#endif

#ifndef FLASH_CCA_CONF_BOOTLDR_BACKDOOR_ACTIVE_HIGH
#define FLASH_CCA_CONF_BOOTLDR_BACKDOOR_ACTIVE_HIGH 0 /**< A logic low level activates the boot loader */
#endif
/** @} */

/*---------------------------------------------------------------------------*/
/**
 * \name Flash Memory configuration
 *
 * @{
 */
#ifndef FLASH_CONF_ORIGIN
#define FLASH_CONF_ORIGIN  0x00200000
#endif

#ifndef FLASH_CONF_SIZE
#define FLASH_CONF_SIZE    0x00080000 /* 512 KiB */
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Watchdog Timer configuration
 *
 * @{
 */
#ifndef WATCHDOG_CONF_ENABLE
#define WATCHDOG_CONF_ENABLE	      1 /**< Enable the watchdog timer */
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name USB 'core' configuration
 *
 * Those values are not meant to be modified by the user, except where stated
 * otherwise
 * @{
 */
#define CTRL_EP_SIZE                8
#define USB_EP1_SIZE               32
#define USB_EP2_SIZE               64
#define USB_EP3_SIZE               64
#define USB_ARCH_WRITE_NOTIFY       0

#ifndef USB_ARCH_CONF_DMA
#define USB_ARCH_CONF_DMA           1 /**< Change to Enable/Disable USB DMA */

#endif

/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name uDMA Configuration and channel allocations
 *
 * @{
 */
#define USB_ARCH_CONF_RX_DMA_CHAN   0 /**< USB -> RAM DMA channel */
#define USB_ARCH_CONF_TX_DMA_CHAN   1 /**< RAM -> USB DMA channel */
#define CC2538_RF_CONF_TX_DMA_CHAN  2 /**< RF -> RAM DMA channel */
#define CC2538_RF_CONF_RX_DMA_CHAN  3 /**< RAM -> RF DMA channel */
#define UDMA_CONF_MAX_CHANNEL       CC2538_RF_CONF_RX_DMA_CHAN
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Character I/O Configuration
 *
 * @{
 */
#ifndef UART_CONF_ENABLE
#define UART_CONF_ENABLE            1 /**< Enable/Disable UART I/O */
#endif

#ifndef UART0_CONF_BAUD_RATE
#define UART0_CONF_BAUD_RATE   115200 /**< Default UART0 baud rate */
#endif

#ifndef UART1_CONF_BAUD_RATE
#define UART1_CONF_BAUD_RATE   115200 /**< Default UART1 baud rate */
#endif

#ifndef SLIP_ARCH_CONF_USB
#define SLIP_ARCH_CONF_USB          0 /**< SLIP over UART by default */
#endif

#ifndef CC2538_RF_CONF_SNIFFER_USB
#define CC2538_RF_CONF_SNIFFER_USB  0 /**< Sniffer out over UART by default */
#endif

#ifndef DBG_CONF_USB
#define DBG_CONF_USB                0 /**< All debugging over UART by default */
#endif

#ifndef SERIAL_LINE_CONF_UART
#define SERIAL_LINE_CONF_UART       0 /**< UART to use with serial line */
#endif

#if !SLIP_ARCH_CONF_USB
#ifndef SLIP_ARCH_CONF_UART
#define SLIP_ARCH_CONF_UART         0 /**< UART to use with SLIP */
#endif
#endif

#if !CC2538_RF_CONF_SNIFFER_USB
#ifndef CC2538_RF_CONF_SNIFFER_UART
#define CC2538_RF_CONF_SNIFFER_UART 0 /**< UART to use with sniffer */
#endif
#endif

#if !DBG_CONF_USB
#ifndef DBG_CONF_UART
#define DBG_CONF_UART               0 /**< UART to use for debugging */
#endif
#endif

#ifndef UART1_CONF_UART
#define UART1_CONF_UART             0 /**< UART to use for examples relying on
                                           the uart1_* API */
#endif

/* Turn off example-provided putchars */
#define SLIP_BRIDGE_CONF_NO_PUTCHAR 1
#define SLIP_RADIO_CONF_NO_PUTCHAR  1

#ifndef SLIP_ARCH_CONF_ENABLED
/*
 * Determine whether we need SLIP
 * This will keep working while UIP_FALLBACK_INTERFACE and CMD_CONF_OUTPUT
 * keep using SLIP
 */
#if defined (UIP_FALLBACK_INTERFACE) || defined (CMD_CONF_OUTPUT)
#define SLIP_ARCH_CONF_ENABLED      1
#endif
#endif

/*
 * When set, the radio turns off address filtering and sends all captured
 * frames down a peripheral (UART or USB, depending on the value of
 * CC2538_RF_CONF_SNIFFER_USB)
 */
#ifndef CC2538_RF_CONF_SNIFFER
#define CC2538_RF_CONF_SNIFFER      0
#endif

/**
 * \brief Define this as 1 to build a headless node.
 *
 * The UART will not be initialised its clock will be gated, offering some
 * energy savings. The USB will not be initialised either
 */
#ifndef CC2538_CONF_QUIET
#define CC2538_CONF_QUIET           0
#endif

/* CC2538_CONF_QUIET is hard and overrides all other related defines */
#if CC2538_CONF_QUIET
#undef USB_SERIAL_CONF_ENABLE
#define USB_SERIAL_CONF_ENABLE      0

#undef UART_CONF_ENABLE
#define UART_CONF_ENABLE            0

#undef STARTUP_CONF_VERBOSE
#define STARTUP_CONF_VERBOSE        0

/* Little sanity check: We can't have quiet sniffers */
#if CC2538_RF_CONF_SNIFFER
#error "CC2538_RF_CONF_SNIFFER == 1 and CC2538_CONF_QUIET == 1"
#error "These values are conflicting. Please set either to 0"
#endif
#endif /* CC2538_CONF_QUIET */

/**
 * \brief Enable the USB core only if we need it
 */
#ifndef USB_SERIAL_CONF_ENABLE
#define USB_SERIAL_CONF_ENABLE \
  ((SLIP_ARCH_CONF_USB & SLIP_ARCH_CONF_ENABLED) | \
   DBG_CONF_USB | \
   (CC2538_RF_CONF_SNIFFER & CC2538_RF_CONF_SNIFFER_USB))
#endif

/*
 * If debugging and SLIP use the same peripheral, this will be 1. Don't modify
 * this
 */
#if SLIP_ARCH_CONF_ENABLED
#define DBG_CONF_SLIP_MUX (SLIP_ARCH_CONF_USB == DBG_CONF_USB && \
                           (SLIP_ARCH_CONF_USB || \
                            SLIP_ARCH_CONF_UART == DBG_CONF_UART))
#endif

/*
 * Automatic detection of whether a specific UART is in use
 */
#define UART_IN_USE_BY_SERIAL_LINE(u) (SERIAL_LINE_CONF_UART == (u))
#define UART_IN_USE_BY_SLIP(u)        (SLIP_ARCH_CONF_ENABLED && \
                                       !SLIP_ARCH_CONF_USB && \
                                       SLIP_ARCH_CONF_UART == (u))
#define UART_IN_USE_BY_RF_SNIFFER(u)  (CC2538_RF_CONF_SNIFFER && \
                                       !CC2538_RF_CONF_SNIFFER_USB && \
                                       CC2538_RF_CONF_SNIFFER_UART == (u))
#define UART_IN_USE_BY_DBG(u)         (!DBG_CONF_USB && DBG_CONF_UART == (u))
#define UART_IN_USE_BY_UART1(u)       (UART1_CONF_UART == (u))

#define UART_IN_USE(u) ( \
  UART_CONF_ENABLE && \
  (UART_IN_USE_BY_SERIAL_LINE(u) || \
   UART_IN_USE_BY_SLIP(u) || \
   UART_IN_USE_BY_RF_SNIFFER(u) || \
   UART_IN_USE_BY_DBG(u) || \
   UART_IN_USE_BY_UART1(u)) \
)

/*---------------------------------------------------------------------------*/


/** @} */
/*---------------------------------------------------------------------------*/
/* board.h assumes that basic configuration is done */
#include "board.h"
/*---------------------------------------------------------------------------*/
/**
 * \name Network Stack Configuration
 *
 * @{
 */  


/*
#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE 8
#define NULLRDC_CONF_802154_AUTOACK 1
#define NETSTACK_CONF_FRAMER  framer_802154

#ifndef NETSTACK_CONF_NETWORK
#if UIP_CONF_IPV6
#define NETSTACK_CONF_NETWORK sicslowpan_driver
#else
#define NETSTACK_CONF_NETWORK rime_driver*/
// #endif /* UIP_CONF_IPV6 */
// #endif /* NETSTACK_CONF_NETWORK */

/*#ifndef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     csma_driver
#endif

#ifndef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     contikimac_driver
#endif*/

/* Configure NullRDC for when it's selected */
/*#define NULLRDC_802154_AUTOACK                  1
#define NULLRDC_802154_AUTOACK_HW               1*/

/* Configure ContikiMAC for when it's selected */
/*#define CONTIKIMAC_CONF_WITH_CONTIKIMAC_HEADER  0
#define CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION 0
#define WITH_FAST_SLEEP                         1

#ifndef NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE
#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE    8
#endif

#ifndef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER  framer_802154
#endif*/

#define NETSTACK_CONF_RADIO   cc2538_rf_driver
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name LPM configuration
 * @{
 */
#ifndef LPM_CONF_ENABLE
#define LPM_CONF_ENABLE       1 /**< Set to 0 to disable LPM entirely */
#endif

/**
 * \brief Maximum PM
 *
 * The SoC will never drop to a Power Mode deeper than the one specified here.
 * 0 for PM0, 1 for PM1 and 2 for PM2
 */
#ifndef LPM_CONF_MAX_PM
#define LPM_CONF_MAX_PM       1
#endif

#ifndef LPM_CONF_STATS
#define LPM_CONF_STATS        0 /**< Set to 1 to enable LPM-related stats */
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name IEEE address configuration
 *
 * Used to generate our RIME & IPv6 address
 * @{
 */
/**
 * \brief Location of the IEEE address
 * 0 => Read from InfoPage,
 * 1 => Use a hardcoded address, configured by IEEE_ADDR_CONF_ADDRESS
 */
#ifndef IEEE_ADDR_CONF_HARDCODED
#define IEEE_ADDR_CONF_HARDCODED             0
#endif

/**
 * \brief The hardcoded IEEE address to be used when IEEE_ADDR_CONF_HARDCODED
 * is defined as 1
 */
#ifndef IEEE_ADDR_CONF_ADDRESS
#define IEEE_ADDR_CONF_ADDRESS { 0x00, 0x12, 0x4B, 0x00, 0x89, 0xAB, 0xCD, 0xEF }
#endif

/**
 * \brief Location of the IEEE address in the InfoPage when
 * IEEE_ADDR_CONF_HARDCODED is defined as 0
 * 0 => Use the primary address location
 * 1 => Use the secondary address location
 */
// #ifndef IEEE_ADDR_CONF_USE_SECONDARY_LOCATION
// #define IEEE_ADDR_CONF_USE_SECONDARY_LOCATION 0
// #endif
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name RF configuration
 *
 * @{
 */
/* RF Config */
#ifndef IEEE802154_CONF_PANID
#define IEEE802154_CONF_PANID           0x5449 /**< Default PAN ID: TI */
#endif

#ifndef CC2538_RF_CONF_CHANNEL
#define CC2538_RF_CONF_CHANNEL              25
#endif /* CC2538_RF_CONF_CHANNEL */

#ifndef CC2538_RF_CONF_AUTOACK
#define CC2538_RF_CONF_AUTOACK               1 /**< RF H/W generates ACKs */
#endif /* CC2538_CONF_AUTOACK */

#ifndef CC2538_RF_CONF_TX_USE_DMA
#define CC2538_RF_CONF_TX_USE_DMA            1 /**< RF TX over DMA */
#endif

#ifndef CC2538_RF_CONF_RX_USE_DMA
#define CC2538_RF_CONF_RX_USE_DMA            1 /**< RF RX over DMA */
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name IPv6, RIME and network buffer configuration
 *
 * @{
 */

/* Don't let contiki-default-conf.h decide if we are an IPv6 build */
/*#ifndef UIP_CONF_IPV6
#define UIP_CONF_IPV6                        0
#endif*/

// #if UIP_CONF_IPV6
/* Addresses, Sizes and Interfaces */
/* 8-byte addresses here, 2 otherwise */
/*#define LINKADDR_CONF_SIZE                   8
#define UIP_CONF_LL_802154                   1
#define UIP_CONF_LLH_LEN                     0
#define UIP_CONF_NETIF_MAX_ADDRESSES         3*/

/* TCP, UDP, ICMP */
/*#ifndef UIP_CONF_TCP
#define UIP_CONF_TCP                         1
#endif
#ifndef UIP_CONF_TCP_MSS
#define UIP_CONF_TCP_MSS                    64
#endif
#define UIP_CONF_UDP                         1
#define UIP_CONF_UDP_CHECKSUMS               1
#define UIP_CONF_ICMP6                       1*/

/* ND and Routing */
/*#ifndef UIP_CONF_ROUTER
#define UIP_CONF_ROUTER                      1
#endif

#ifndef UIP_CONF_IPV6_RPL
#define UIP_CONF_IPV6_RPL                    1
#endif

#define UIP_CONF_ND6_SEND_RA                 0
#define UIP_CONF_IP_FORWARD                  0
#define RPL_CONF_STATS                       0
#define RPL_CONF_MAX_DAG_ENTRIES             1
#ifndef RPL_CONF_OF
#define RPL_CONF_OF rpl_mrhof
#endif

#define UIP_CONF_ND6_REACHABLE_TIME     600000
#define UIP_CONF_ND6_RETRANS_TIMER       10000

#ifndef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS                20
#endif
#ifndef UIP_CONF_MAX_ROUTES
#define UIP_CONF_MAX_ROUTES                 20
#endif*/

/* uIP */
/*#ifndef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE              1300
#endif

#define UIP_CONF_IPV6_QUEUE_PKT              0
#define UIP_CONF_IPV6_CHECKS                 1
#define UIP_CONF_IPV6_REASSEMBLY             0
#define UIP_CONF_MAX_LISTENPORTS             8*/

/* 6lowpan */
/*#define SICSLOWPAN_CONF_COMPRESSION          SICSLOWPAN_COMPRESSION_HC06
#ifndef SICSLOWPAN_CONF_COMPRESSION_THRESHOLD
#define SICSLOWPAN_CONF_COMPRESSION_THRESHOLD 63
#endif
#ifndef SICSLOWPAN_CONF_FRAG
#define SICSLOWPAN_CONF_FRAG                 1
#endif
#define SICSLOWPAN_CONF_MAXAGE               8*/

/* Define our IPv6 prefixes/contexts here */
/*#define SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS    1
#ifndef SICSLOWPAN_CONF_ADDR_CONTEXT_0
#define SICSLOWPAN_CONF_ADDR_CONTEXT_0 { \
  addr_contexts[0].prefix[0] = 0xaa; \
  addr_contexts[0].prefix[1] = 0xaa; \
}
#endif

#define MAC_CONF_CHANNEL_CHECK_RATE          8

#ifndef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM                    8
#endif*/
/*---------------------------------------------------------------------------*/
// #else /* UIP_CONF_IPV6 */
/* Network setup for non-IPv6 (rime). */
/*#define UIP_CONF_IP_FORWARD                  1

#ifndef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE               108
#endif

#define RIME_CONF_NO_POLITE_ANNOUCEMENTS     0

#ifndef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM                    8
#endif*/

//#endif /* UIP_CONF_IPV6 */
/** @} */
/*---------------------------------------------------------------------------*/

#endif /* CONTIKI_CONF_H_ */

/** @} */
