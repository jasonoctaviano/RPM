
#ifndef PLATFORM_CONF_H
#define PLATFORM_CONF_H

#define OPENMOTE_ROUTER 1

#undef UIP_FALLBACK_INTERFACE
#define UIP_FALLBACK_INTERFACE ip64_uip_fallback_interface

#undef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE 600
//#define UIP_CONF_BUFFER_SIZE 1300
#undef UIP_CONF_TCP_MSS
#define UIP_CONF_TCP_MSS 500
//#define UIP_CONF_TCP_MSS 1000
#undef UIP_CONF_RECEIVE_WINDOW
#define UIP_CONF_RECEIVE_WINDOW UIP_CONF_TCP_MSS

/* Make HTTP/TCP connection way faster. */
#undef UIP_CONF_TCP_SPLIT
#define UIP_CONF_TCP_SPLIT 1
#undef UIP_SPLIT_CONF_SIZE
#define UIP_SPLIT_CONF_SIZE 8

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

// #undef UIP_CONF_MAX_ROUTES
// #define UIP_CONF_MAX_ROUTES                  64

/* Make HTTP/TCP connection way faster. */
// #undef UIP_CONF_TCP_SPLIT
// #define UIP_CONF_TCP_SPLIT 1
// #undef UIP_SPLIT_CONF_SIZE
// #define UIP_SPLIT_CONF_SIZE 8

#define NULLRDC_CONF_ACK_WAIT_TIME                RTIMER_SECOND / 500
#define NULLRDC_CONF_AFTER_ACK_DETECTED_WAIT_TIME RTIMER_SECOND / 250

#define NETSTACK_RADIO_MAX_PAYLOAD_LEN 125

#include "mist-conf-const.h"
#define MIST_CONF_NETSTACK (MIST_CONF_NULLRDC | MIST_CONF_AES)
//#define MIST_CONF_NETSTACK      MIST_CONF_DROWSIE
//#define MIST_CONF_NETSTACK      (MIST_CONF_AES | MIST_CONF_DROWSIE)

#undef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM        8
#undef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS     20
#undef UIP_CONF_DS6_ROUTE_NBU
#define UIP_CONF_DS6_ROUTE_NBU   20

#undef SICSLOWPAN_CONF_MAXAGE
#define SICSLOWPAN_CONF_MAXAGE 16

/*#undef IEEE_ADDR_CONF_HARDCODED
#define IEEE_ADDR_CONF_HARDCODED             1

#undef IEEE_ADDR_CONF_IN_FLASH
#define IEEE_ADDR_CONF_IN_FLASH              0*/

#endif /* PLATFORM_CONF_H */
