//*****************************************************************************
//
// enet_uip.c - Sample WebServer Application for Ethernet Demo
//
// Copyright (c) 2013 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.0.1.11577 of the DK-TM4C129X Firmware Package.
//
//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>

#include "contiki.h"
#include "tivaware/inc/hw_emac.h"
#include "tivaware/inc/hw_ints.h"
#include "tivaware/inc/hw_memmap.h"
#include "tivaware/inc/hw_types.h"
#include "tivaware/inc/hw_gpio.h"
#include "tivaware/driverlib/tm4c_debug.h"
#include "tivaware/driverlib/tm4c_pin_map.h"
#include "tivaware/driverlib/tm4c_gpio.h"
#include "tivaware/driverlib/tm4c_emac.h"
#include "tivaware/driverlib/tm4c_flash.h"
#include "tivaware/driverlib/tm4c_interrupt.h"
#include "tivaware/driverlib/tm4c_sysctl.h"
#include "tivaware/driverlib/tm4c_systick.h"

#include "tiva_eth.h"

#define DEBUG 0
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

PROCESS(tiva_eth_watchdog_process, "Tiva C Eth Watchdog");
//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>Ethernet with uIP (enet_uip)</h1>
//!
//! This example application demonstrates the operation of the Tiva C Series
//! Ethernet controller using the uIP TCP/IP Stack.  DHCP is used to obtain
//! an Ethernet address.  A basic web site is served over the Ethernet port.
//! The web site displays a few lines of text, and a counter that increments
//! each time the page is sent.
//!
//! UART0, connected to the ICDI virtual COM port and running at 115,200,
//! 8-N-1, is used to display messages from this application.
//!
//! For additional details on uIP, refer to the uIP web page at:
//! http://www.sics.se/~adam/uip/
//
//*****************************************************************************

//*****************************************************************************
//
// Defines for setting up the system clock.
//
//*****************************************************************************
#define SYSTICKHZ               CLOCK_CONF_SECOND
#define SYSTICKMS               (1000 / SYSTICKHZ)
#define SYSTICKUS               (1000000 / SYSTICKHZ)
#define SYSTICKNS               (1000000000 / SYSTICKHZ)

//*****************************************************************************
//
// Macro for accessing the Ethernet header information in the buffer.
//
//*****************************************************************************
//uint8_t g_pui8UIPBuffer[UIP_BUFSIZE + 2];
//uint8_t *uip_buf = g_pui8UIPBuffer;

//#define BUF                     ((struct uip_eth_hdr *)uip_buf)

//*****************************************************************************
//
// Ethernet DMA descriptors.
//
// Although uIP uses a single buffer, the MAC hardware needs a minimum of
// 3 receive descriptors to operate.
//
//*****************************************************************************
#define NUM_TX_DESCRIPTORS 8
#define NUM_RX_DESCRIPTORS 8
tEMACDMADescriptor g_psRxDescriptor[NUM_RX_DESCRIPTORS];
tEMACDMADescriptor g_psTxDescriptor[NUM_TX_DESCRIPTORS];

uint32_t g_ui32RxDescIndex;
uint32_t g_ui32TxDescIndex;

//*****************************************************************************
//
// Transmit and receive buffers.
//
//*****************************************************************************
#define RX_BUFFER_SIZE 1536
#define TX_BUFFER_SIZE 1536
uint8_t g_pui8RxBuffer[RX_BUFFER_SIZE];
uint8_t g_pui8TxBuffer[TX_BUFFER_SIZE];

static uint8_t initialized = 0;
static uint8_t enc_mac_addr[6];
static int received_packets = 0;
static int sent_packets = 0;
//*****************************************************************************
//
// A set of flags.  The flag bits are defined as follows:
//
//     0 -> An indicator that a SysTick interrupt has occurred.
//     1 -> An RX Packet has been received.
//     2 -> A TX packet DMA transfer is pending.
//     3 -> A RX packet DMA transfer is pending.
//
//*****************************************************************************
#define FLAG_SYSTICK            0
#define FLAG_RXPKT              1
#define FLAG_TXPKT              2
#define FLAG_RXPKTPEND          3
static volatile uint32_t g_ui32Flags;

//*****************************************************************************
//
// A system tick counter, incremented every SYSTICKMS.
//
//*****************************************************************************
volatile uint32_t g_ui32TickCounter = 0;

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}

//*****************************************************************************
//
// The interrupt handler for the Ethernet interrupt.
//
//*****************************************************************************
void
EthernetIntHandler(void)
{
    uint32_t ui32Temp;
    //
    // Read and Clear the interrupt.
    //
    ui32Temp = EMACIntStatus(EMAC0_BASE, true);
    EMACIntClear(EMAC0_BASE, ui32Temp);

    //
    // Check to see if an RX Interrupt has occurred.
    //
    if(ui32Temp & EMAC_INT_RECEIVE)
    {
        //
        // Indicate that a packet has been received.
        //
        HWREGBITW(&g_ui32Flags, FLAG_RXPKT) = 1;
    }

    //
    // Has the DMA finished transferring a packet to the transmitter?
    //
    if(ui32Temp & EMAC_INT_TRANSMIT)
    {
        //
        // Indicate that a packet has been sent.
        //
        HWREGBITW(&g_ui32Flags, FLAG_TXPKT) = 0;
    }
    
}
//*****************************************************************************
//
// Read a packet from the DMA receive buffer into the uIP packet buffer.
//
//*****************************************************************************
int32_t
tiva_eth_receive(uint8_t *pui8Buf, uint16_t ui16Bufsize)
{
    uint32_t ui32FrameLen, ui32Loop;
    uint32_t ui32BufLen=0;
    ui32BufLen += ui16Bufsize;
    //
    // Check the arguments.
    //
    ASSERT(pui8Buf != 0);
    ASSERT(ui32BufLen > 0);
if(HWREGBITW(&g_ui32Flags, FLAG_RXPKT))
{
    //
    // Clear the RX Packet event flag.
    //
    HWREGBITW(&g_ui32Flags, FLAG_RXPKT) = 0;
    //
    // By default, we assume we got a bad frame.
    //
    ui32FrameLen = 0;

    //
    // Make sure that we own the receive descriptor.
    //
    if(!(g_psRxDescriptor[g_ui32RxDescIndex].ui32CtrlStatus & DES0_RX_CTRL_OWN))
    {
        //
        // We own the receive descriptor so check to see if it contains a valid
        // frame.  Look for a descriptor error, indicating that the incoming
        // packet was truncated or, if this is the last frame in a packet,
        // the receive error bit.
        //
        if(!(g_psRxDescriptor[g_ui32RxDescIndex].ui32CtrlStatus &
             DES0_RX_STAT_ERR))
        {
            //
            // We have a valid frame so copy the content to the supplied
            // buffer. First check that the "last descriptor" flag is set.  We
            // sized the receive buffer such that it can always hold a valid
            // frame so this flag should never be clear at this point but...
            //
            if(g_psRxDescriptor[g_ui32RxDescIndex].ui32CtrlStatus &
               DES0_RX_STAT_LAST_DESC)
            {
                ui32FrameLen =
                    ((g_psRxDescriptor[g_ui32RxDescIndex].ui32CtrlStatus &
                      DES0_RX_STAT_FRAME_LENGTH_M) >>
                     DES0_RX_STAT_FRAME_LENGTH_S);

                //
                // Sanity check.  This shouldn't be required since we sized the
                // uIP buffer such that it's the same size as the DMA receive
                // buffer but, just in case...
                //
                if(ui32FrameLen > ui32BufLen)
                {
                    ui32FrameLen = ui32BufLen;
                }

                //
                // Copy the data from the DMA receive buffer into the provided
                // frame buffer.
                //
                for(ui32Loop = 0; ui32Loop < ui32FrameLen; ui32Loop++)
                {
                    pui8Buf[ui32Loop] = g_pui8RxBuffer[ui32Loop];
                }
            }
        }

        //
        // Move on to the next descriptor in the chain.
        //
        g_ui32RxDescIndex++;
        if(g_ui32RxDescIndex == NUM_RX_DESCRIPTORS)
        {
            g_ui32RxDescIndex = 0;
        }

        //
        // Mark the next descriptor in the ring as available for the receiver
        // to write into.
        //
        g_psRxDescriptor[g_ui32RxDescIndex].ui32CtrlStatus = DES0_RX_CTRL_OWN;
    }

    //
    // Return the Frame Length
    //
    received_packets++;
    PRINTF("Tiva Eth: rx %d\n", ui32FrameLen);
    PRINTF("Tiva Eth: received_packets %d\n", received_packets);
}
else
{
ui32FrameLen=0;
}
    return(ui32FrameLen);
}

//*****************************************************************************
//
// Transmit a packet from the supplied buffer.
//
//*****************************************************************************
int32_t
tiva_eth_transmit(uint8_t *pui8Buf, uint16_t ui16Bufsize)
{
    uint32_t ui32Loop;
    uint32_t ui32BufLen=0;
    PRINTF("Tiva Eth: tx ui16Bufsize %d\n", ui16Bufsize);
    ui32BufLen += ui16Bufsize;
    PRINTF("Tiva Eth: tx ui32BufLen %d\n", ui32BufLen);
    //
    // Indicate that a packet is being sent.
    //
    HWREGBITW(&g_ui32Flags, FLAG_TXPKT) = 1;

    //00
    // Wait for the previous packet to be transmitted.
    //
    while(g_psTxDescriptor[g_ui32TxDescIndex].ui32CtrlStatus &
          DES0_TX_CTRL_OWN)
    {
        //
        // Spin and waste time.
        //
    }
    //
    // Check that we're not going to over00flow the transmit buffer.  This
    // shouldn't be necessary since the uIP buffer is smaller than our DMA
    // transmit buffer but, just in case...
    //
    if(ui32BufLen > TX_BUFFER_SIZE)
    {
        ui32BufLen = TX_BUFFER_SIZE;
    }

    //
    // Copy the packet data into the transmit buffer.
    //
    for(ui32Loop = 0; ui32Loop < ui32BufLen; ui32Loop++)
    {
        g_pui8TxBuffer[ui32Loop] = pui8Buf[ui32Loop];
    }

    //
    // Move to the next descriptor.
    //
    g_ui32TxDescIndex++;
    if(g_ui32TxDescIndex == NUM_TX_DESCRIPTORS)
    {
        g_ui32TxDescIndex = 0;
    }

    //
    // Fill in the packet size and tell the transmitter to start work.
    //
    g_psTxDescriptor[g_ui32TxDescIndex].ui32Count = ui32BufLen;
    g_psTxDescriptor[g_ui32TxDescIndex].ui32CtrlStatus =
        (DES0_TX_CTRL_LAST_SEG | DES0_TX_CTRL_FIRST_SEG |
         DES0_TX_CTRL_INTERRUPT | DES0_TX_CTRL_IP_ALL_CKHSUMS |
         DES0_TX_CTRL_CHAINED | DES0_TX_CTRL_OWN);

    //
    // Tell the DMA to reacquire the descriptor now that we've filled it in.
    //
    EMACTxDMAPollDemand(EMAC0_BASE);

    //
    // Return the number of bytes sent.
    //
    PRINTF("Tiva Eth: tx %d\n", ui16Bufsize);
    sent_packets++;
    PRINTF("Tiva Eth: sent_packets %d\n", sent_packets);
    return(ui32BufLen);
}

//*****************************************************************************
//
// Initialize the transmit and receive DMA descriptors.  We apparently need
// a minimum of 3 descriptors in each chain.  This is overkill since uIP uses
// a single, common transmit and receive buffer so we tag each descriptor
// with the same buffer and will make sure we only hand the DMA one descriptor
// at a time.
//
//*****************************************************************************
void
tiva_eth_initDescriptors(uint32_t ui32Base)
{
    uint32_t ui32Loop;

    //
    // Initialize each of the transmit descriptors.  Note that we leave the OWN
    // bit clear here since we have not set up any transmissions yet.
    //
    for(ui32Loop = 0; ui32Loop < NUM_TX_DESCRIPTORS; ui32Loop++)
    {
        g_psTxDescriptor[ui32Loop].ui32Count =
            (DES1_TX_CTRL_SADDR_INSERT |
             (TX_BUFFER_SIZE << DES1_TX_CTRL_BUFF1_SIZE_S));
        g_psTxDescriptor[ui32Loop].pvBuffer1 = g_pui8TxBuffer;
        g_psTxDescriptor[ui32Loop].DES3.pLink =
            (ui32Loop == (NUM_TX_DESCRIPTORS - 1)) ?
            g_psTxDescriptor : &g_psTxDescriptor[ui32Loop + 1];
        g_psTxDescriptor[ui32Loop].ui32CtrlStatus =
            (DES0_TX_CTRL_LAST_SEG | DES0_TX_CTRL_FIRST_SEG |
             DES0_TX_CTRL_INTERRUPT | DES0_TX_CTRL_CHAINED |
             DES0_TX_CTRL_IP_ALL_CKHSUMS);
    }

    //
    // Initialize each of the receive descriptors.  We clear the OWN bit here
    // to make sure that the receiver doesn't start writing anything
    // immediately.
    //
    for(ui32Loop = 0; ui32Loop < NUM_RX_DESCRIPTORS; ui32Loop++)
    {
        g_psRxDescriptor[ui32Loop].ui32CtrlStatus = 0;
        g_psRxDescriptor[ui32Loop].ui32Count =
            (DES1_RX_CTRL_CHAINED |
             (RX_BUFFER_SIZE << DES1_RX_CTRL_BUFF1_SIZE_S));
        g_psRxDescriptor[ui32Loop].pvBuffer1 = g_pui8RxBuffer;
        g_psRxDescriptor[ui32Loop].DES3.pLink =
            (ui32Loop == (NUM_RX_DESCRIPTORS - 1)) ?
            g_psRxDescriptor : &g_psRxDescriptor[ui32Loop + 1];
    }

    //
    // Set the descriptor pointers in the hardware.
    //
    EMACRxDMADescriptorListSet(ui32Base, g_psRxDescriptor);
    EMACTxDMADescriptorListSet(ui32Base, g_psTxDescriptor);

    //
    // Start from the beginning of both descriptor chains.  We actually set
    // the transmit descriptor index to the last descriptor in the chain
    // since it will be incremented before use and this means the first
    // transmission we perform will use the correct descriptor.
    //
    g_ui32RxDescIndex = 0;
    g_ui32TxDescIndex = NUM_TX_DESCRIPTORS - 1;
}
//*****************************************************************************
//
// Config Ethernet
//
//*****************************************************************************
static void
tiva_eth_init(void)
{
  //uint32_t ui32User0, ui32User1;
  //static struct uip_eth_addr sTempAddr;
  PRINTF("tiva_eth: resetting chip\n");
  //
  // PF1/PK4/PK6 are used for Ethernet LEDs.
  //
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  //SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
  //GPIOPinConfigure(GPIO_PF1_EN0LED2);
  GPIOPinConfigure(GPIO_PF0_EN0LED0);
  GPIOPinConfigure(GPIO_PF4_EN0LED1);
  //GPIOPinTypeEthernetLED(GPIO_PORTF_BASE, GPIO_PIN_1);
  GPIOPinTypeEthernetLED(GPIO_PORTF_BASE, GPIO_PIN_0);
  GPIOPinTypeEthernetLED(GPIO_PORTF_BASE, GPIO_PIN_4);
  
  #if 0
  //
  // Read the MAC address from the user registers.
  //
  FlashUserGet(&ui32User0, &ui32User1);
  if((ui32User0 == 0xffffffff) || (ui32User1 == 0xffffffff))
  {
    //
    // We should never get here.  This is an error if the MAC address has
    // not been programmed into the device.  Exit the program.
    //
    PRINTF("**************************************\n");
    PRINTF("MAC Address Error\n");
  }
  //
  // Convert the 24/24 split MAC address from NV ram into a 32/16 split MAC
  // address needed to program the hardware registers, then program the MAC
  // address into the Ethernet Controller registers.
  //
  enc_mac_addr[0] = ((ui32User0 >> 0) & 0xff);
  enc_mac_addr[1] = ((ui32User0 >> 8) & 0xff);
  enc_mac_addr[2] = ((ui32User0 >> 16) & 0xff);
  enc_mac_addr[3] = ((ui32User1 >> 0) & 0xff);
  enc_mac_addr[4] = ((ui32User1 >> 8) & 0xff);
  enc_mac_addr[5] = ((ui32User1 >> 16) & 0xff);
  #endif
  
  
  //
  // Configure SysTick for a periodic interrupt.
  // 
  /*Have used in clock.c file as the sys tick*/

  //
  // Enable and reset the Ethernet modules.
  //
  SysCtlPeripheralEnable(SYSCTL_PERIPH_EMAC0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_EPHY0);
  SysCtlPeripheralReset(SYSCTL_PERIPH_EMAC0);
  SysCtlPeripheralReset(SYSCTL_PERIPH_EPHY0);
  //
  // Wait for the MAC to be ready.
  //
  PRINTF("Waiting for MAC to be ready...\n");
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_EMAC0))
  {
  }
  //
  // Configure for use with the internal PHY.
  //
  EMACPHYConfigSet(EMAC0_BASE, (EMAC_PHY_TYPE_INTERNAL | EMAC_PHY_INT_MDIX_EN | EMAC_PHY_AN_100B_T_FULL_DUPLEX));
  //
  // Reset the MAC.
  //
  EMACReset(EMAC0_BASE);
  //
  // Initialize the MAC and set the DMA mode.
  //
  EMACInit(EMAC0_BASE,120000000,EMAC_BCONFIG_MIXED_BURST | EMAC_BCONFIG_PRIORITY_FIXED, 4, 4, 0);
  //
  // Set MAC configuration options.
  //
  EMACConfigSet(EMAC0_BASE,
                      (EMAC_CONFIG_FULL_DUPLEX |EMAC_CONFIG_CHECKSUM_OFFLOAD|
                       EMAC_CONFIG_7BYTE_PREAMBLE | EMAC_CONFIG_IF_GAP_96BITS |
                       EMAC_CONFIG_USE_MACADDR0 |
                       EMAC_CONFIG_SA_FROM_DESCRIPTOR |
                       EMAC_CONFIG_BO_LIMIT_1024),
                      (EMAC_MODE_RX_STORE_FORWARD |
                       EMAC_MODE_TX_STORE_FORWARD |
                       EMAC_MODE_TX_THRESHOLD_64_BYTES |
                       EMAC_MODE_RX_THRESHOLD_64_BYTES), 0);
  //
  // Initialize the Ethernet DMA descriptors.
  //
  tiva_eth_initDescriptors(EMAC0_BASE);
  //
  // Program the hardware with its MAC address (for filtering).
  //
  EMACAddrSet(EMAC0_BASE, 0, enc_mac_addr);
  //
  // Wait for the link to become active.
  //
  while((EMACPHYRead(EMAC0_BASE, 0x00, EPHY_BMSR) & EPHY_BMSR_LINKSTAT) == 0)
  {
  }
  //
  // Set MAC filtering options.  We receive all broadcast and multicast
  // packets along with those addressed specifically for us.
  //
  EMACFrameFilterSet(EMAC0_BASE, EMAC_FRMFILTER_PASS_ADDR_CTRL);
    //EMACFrameFilterSet(EMAC0_BASE, (EMAC_FRMFILTER_SADDR |
                      //                  EMAC_FRMFILTER_PASS_MULTICAST |
                               //         EMAC_FRMFILTER_PASS_NO_CTRL));
  //
  // Clear any pending interrupts.
  //
  EMACIntClear(EMAC0_BASE, EMACIntStatus(EMAC0_BASE, false));
  //
  // Enable the Ethernet MAC transmitter and receiver.
  //
  EMACTxEnable(EMAC0_BASE);
  EMACRxEnable(EMAC0_BASE);
  //
  // Enable the Ethernet interrupt.
  //
  IntEnable(INT_EMAC0);
  //
  // Enable the Ethernet RX Packet interrupt source. modify Tx interrupt
  //
  EMACIntEnable(EMAC0_BASE, EMAC_INT_RECEIVE);
  //
  // Mark the first receive descriptor as available to the DMA to start
  // the receive processing.
  //
  g_psRxDescriptor[g_ui32RxDescIndex].ui32CtrlStatus |= DES0_RX_CTRL_OWN;
}
//*****************************************************************************
//
// Initialization of Ethernet
//
//*****************************************************************************
void
tiva_eth_start(uint8_t *mac_addr)
{
  if(initialized) {
    return;
  }

  memcpy(enc_mac_addr, mac_addr, 6);

  /* Start watchdog process */
  process_start(&tiva_eth_watchdog_process, NULL);

  tiva_eth_init();

  initialized = 1;
}
/*****************************************************************************/

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(tiva_eth_watchdog_process, ev, data)
{
  static struct etimer et;
  PROCESS_BEGIN();

  while(1) {
#define RESET_PERIOD (30*CLOCK_SECOND)
    etimer_set(&et, RESET_PERIOD);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    PRINTF("Tiva C Eth: test received_packet %d > sent_packets %d\n", received_packets, sent_packets);
    if(received_packets <= sent_packets) {
      PRINTF("Tiva C Eth: resetting chip\n");
      tiva_eth_init();
    }
    received_packets = 0;
    sent_packets = 0;
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
