/*
 * Copyright (c) 2012, Jens Nielsen
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

#include "contiki.h"
#include "owhal.h" 
#include "ownet.h"
#include "owlink.h"
#include "ds18b20.h"
#include <string.h> /*for memset*/
#include <stdio.h>
 
// #define DBG(...) printf(__VA_ARGS__)
#define DBG(...)

typedef struct
{
    unsigned char serial[ 8 ];
    signed short lastTemp;
}device_t;

static device_t devices[ NUM_DEVICES ];

typedef enum state_e
{
    STATE_SCAN,
    STATE_CONVERT,
    STATE_FINISHED,    
}state_t;

static state_t state;
static void ds18b20_scan(void);
static void ds18b20_startConvert(void);
static void ds18b20_fetchTemp( unsigned char device );
void ds_print_float(float num, uint8_t preci, char *buf);

void ds18b20_init()
{
  DBG("ds18b20_init\r\n");
  memset( devices, 0, sizeof(devices) );
  state = STATE_SCAN;
}

void scan_start()
{
  state = STATE_SCAN;
  while (state != STATE_FINISHED) {
    switch ( state )
    {
      case STATE_SCAN:
        DBG("STATE_SCAN\r\n");
        ds18b20_scan();
        state = STATE_CONVERT;
      break;
      case STATE_CONVERT:
        DBG("STATE_CONVERT\r\n");
        ds18b20_startConvert();
        state = STATE_FINISHED;
      break;
    }
  }
  DBG("finished scan_start\r\n");
}

void read_temperatures() {
  unsigned char fetchDevice = 0;
  DBG("STATE_FETCH_TEMPS\r\n");
  ds18b20_fetchTemp( fetchDevice );
}

static void ds18b20_scan()
{
  unsigned char found;
  unsigned char i;

  found = owFirst( 0, 1, 0 );
  if ( found )
  {
    DBG("ds18b20_scan found\r\n");
    owSerialNum( 0, devices[ 0 ].serial, 1 );
    // DBG("ds18b20_scan serial number: ");
    // int iz;
    // for (iz=0; iz<8; iz++){
    //   DBG("%d.", devices[ 0 ].serial[iz]);
    // }
    // DBG("\r\n");
  }

  for ( i = 1; found && i < NUM_DEVICES; i++ )
  {
    found = owNext( 0, 1, 0 );
    if ( found )
    {
      owSerialNum( 0, devices[ i ].serial, 1 );
    }
  }
}

static void ds18b20_startConvert( )
{

  owTouchReset();
  owWriteByte(0xCC);
  // owWriteByte(0x55);
  // int i;
  // for ( i = 0; i < 8; i++ ) owWriteByte(devices[ 0 ].serial[i]);
  owWriteByte(0x44);
  DBG("finish owWriteByte 44\r\n");
}

static void ds18b20_fetchTemp( unsigned char device )
{
  if ( device < NUM_DEVICES && devices[ device ].serial[ 0 ] != 0 )
  {
    unsigned char i;
    unsigned char b1, b2;

        /*address specified device*/
    owTouchReset();
    owWriteByte(0x55);
    for (i = 0; i < 8; i++)
    {
      owWriteByte(devices[ device ].serial[ i ]);
    }

        /*read the first two bytes of scratchpad*/
    owWriteByte(0xBE);
    b1 = owReadByte();
    b2 = owReadByte();

    devices[ device ].lastTemp = ( (signed short) b2 << 8 ) | ( b1 & 0xFF );
    int16_t raw = devices[ device ].lastTemp;
    unsigned char cfg = 0x40;
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
    float celsius;
    celsius = (float)raw / 16.0;
    DBG("ds18b20_fetchTemp lastTemp: %d\r\n", devices[ device ].lastTemp);
    char ctemp[20];
    ds_print_float(celsius, 2, ctemp); // 2 decimal precision
    DBG("ds18b20_fetchTemp celsius value: %s\r\n", ctemp);
  }
}

void ds_print_float(float num, uint8_t preci, char *buf)
{
  int integer=(int)num, decimal=0;
  preci = preci > 10 ? 10 : preci;
  num -= integer;
  while((num != 0) && (preci-- > 0)) {
    decimal *= 10;
    num *= 10;
    decimal += (int)num;
    num -= (int)num;
  }
  sprintf(buf, "%d.%d", integer, decimal);
  DBG("print float int dec value: %d.%d\r\n", integer, decimal);
  DBG("print float buf value: %s\r\n", buf);
}