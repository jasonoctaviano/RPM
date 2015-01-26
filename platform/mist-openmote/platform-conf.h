#ifndef PLATFORM_CONF_H
#define PLATFORM_CONF_H

#undef OPENMOTE_ROUTER

#undef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE 200

#undef UIP_CONF_TCP_MSS
#define UIP_CONF_TCP_MSS (UIP_CONF_BUFFER_SIZE - 70)
#undef UIP_CONF_RECEIVE_WINDOW
#define UIP_CONF_RECEIVE_WINDOW UIP_CONF_TCP_MSS

/* CPU target speed in Hz; works fine at 8, 16, 18 MHz but not higher. */
// #define F_CPU 16000000uL

/* Our clock resolution, this is the same as Unix HZ. */
// #define CLOCK_CONF_SECOND 128UL

#define BAUD2UBR(baud) ((F_CPU/baud))

#define CCIF
#define CLIF

#define HAVE_STDINT_H
#include "board.h"

/* Types for clocks and uip_stats */
typedef unsigned short uip_stats_t;
typedef unsigned long clock_time_t;
//typedef unsigned long off_t;

#define NULLRDC_CONF_ACK_WAIT_TIME                RTIMER_SECOND / 500
#define NULLRDC_CONF_AFTER_ACK_DETECTED_WAIT_TIME RTIMER_SECOND / 250

#define NETSTACK_RADIO_MAX_PAYLOAD_LEN 125

#include "mist-conf-const.h"

#ifndef MIST_CONF_NETSTACK
#define MIST_CONF_NETSTACK (MIST_CONF_NULLRDC | MIST_CONF_AES)
//#define MIST_CONF_NETSTACK      MIST_CONF_NULLRDC
//#define MIST_CONF_NETSTACK      MIST_CONF_DROWSIE
//#define MIST_CONF_NETSTACK      (MIST_CONF_AES | MIST_CONF_DROWSIE)
#endif /* MIST_CONF_NETSTACK */

#undef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM        8
#undef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS     20
#undef UIP_CONF_DS6_ROUTE_NBU
#define UIP_CONF_DS6_ROUTE_NBU   20

// #include "mist-default-conf.h"

// #define NETSTACK_AES_KEY "thingsquare mist"

// #undef ENERGEST_CONF_ON
// #define ENERGEST_CONF_ON            1

// /* Allow the device to join two multicast groups. */
// #undef UIP_CONF_DS6_MADDR_NBU
// #define UIP_CONF_DS6_MADDR_NBU 2

// #undef NBR_TABLE_CONF_MAX_NEIGHBORS
// #define NBR_TABLE_CONF_MAX_NEIGHBORS                 20

// #undef UIP_CONF_DS6_ROUTE_NBU
// #define UIP_CONF_DS6_ROUTE_NBU               20

// #undef UIP_CONF_BUFFER_SIZE
// #define UIP_CONF_BUFFER_SIZE 200

#endif /* PLATFORM_CONF_H */
