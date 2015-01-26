#include "mist.h"
#include "websocket.h"
#include "dev/gpio.h"
#include "dev/adc.h"

static struct websocket s;

static void callback(struct websocket *s, websocket_result r,
                     uint8_t *data, uint16_t datalen);

#define RECONNECT_INTERVAL 10 * CLOCK_SECOND

#define IN_PORT_BASE GPIO_A_BASE
#define OUT_PORT_BASE GPIO_B_BASE

#define ADC_CONTACT_CH SOC_ADC_ADCCON_CH_AIN0 // PA0
#define ANALOG_CURR_CH SOC_ADC_ADCCON_CH_AIN1 // PA1
#define DC_IN_PIN 0x01

#define DC_OUT1_PIN 0x02
#define DC_OUT2_PIN 0x02
#define OUT_CONTACT_PIN 0x02

int16_t adc_contact_read;
int16_t analog_curr_read;

static struct ctimer reconnect_timer;

typedef struct inputSet
{
  int16_t adc_contact_read;
  int16_t analog_curr_read;
  int16_t dc_in_read;
};

inputSet ionodeInputs;

/*---------------------------------------------------------------------------*/
PROCESS(ionode_process, "Digital output - set digital pin - and input - read digital pin and ADC");
AUTOSTART_PROCESSES(&ionode_process);
/*---------------------------------------------------------------------------*/
static void
reconnect_callback(void *ptr)
{
  websocket_open(&s, "ws://172.16.0.1:8080/",
                 "thingsquare mist", callback);
}
/*---------------------------------------------------------------------------*/
static void
callback(struct websocket *s, websocket_result r,
         uint8_t *data, uint16_t datalen)
{
  if(r == WEBSOCKET_CLOSED ||
     r == WEBSOCKET_RESET ||
     r == WEBSOCKET_HOSTNAME_NOT_FOUND ||
     r == WEBSOCKET_TIMEDOUT) {
    ctimer_set(&reconnect_timer, RECONNECT_INTERVAL, reconnect_callback, s);
  } else if(r == WEBSOCKET_CONNECTED) {
    websocket_send_str(s, "Connected");
  } else if(r == WEBSOCKET_DATA) {
    printf("environode: Received data '%.*s' (len %d)\n", datalen,
           data, datalen);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(environode_process, ev, data)
{
  static struct etimer et;
  PROCESS_BEGIN();
  /* set digital input pins */
  GPIO_SET_INPUT(IN_PORT_BASE, DC_IN_PIN);

  /* set digital output pins */
  GPIO_SET_OUTPUT(OUT_PORT_BASE, DC_OUT1_PIN);
  GPIO_SET_OUTPUT(OUT_PORT_BASE, DC_OUT2_PIN);
  GPIO_SET_OUTPUT(OUT_PORT_BASE, OUT_CONTACT_PIN);

  /* set ADC pins */
  adc_init();

  ctimer_set(&reconnect_timer, RECONNECT_INTERVAL, reconnect_callback, &s);

  while(1) {
    etimer_set(&et, CLOCK_SECOND / 8);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    /* read ADC result */
    ionodeInputs.adc_contact_read = adc_get(ADC_CONTACT_CH, SOC_ADC_ADCCON_REF_INT, SOC_ADC_ADCCON_DIV_64);
    ionodeInputs.analog_curr_read = adc_get(ANALOG_CURR_CH, SOC_ADC_ADCCON_REF_INT, SOC_ADC_ADCCON_DIV_64);
    /* read pin result */
    ionodeInputs.dc_in_read = GPIO_READ_PIN(IN_PORT_BASE, DC_IN_PIN);

    static int count;
    count++;
    websocket_send_str(&s, ctemp);
  }

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
/**
 * @}
 */
