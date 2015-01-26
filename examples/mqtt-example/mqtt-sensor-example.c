#include <string.h>
#include "mist.h"
// #include "dev/button-sensor.h"
#include "netstack-aes.h"

#include "mqtt.h"

#define SEND_INTERVAL		(60 * CLOCK_SECOND)
#define MIN(a, b) ((a) < (b) ? (a) : (b))

static struct mqtt_connection conn;

//#define APP_BUFFER_SIZE 128
//static uint8_t app_buffer[128];
static struct mqtt_message* msg_ptr = 0;
//static uint8_t first_flag = 1;
//static struct etimer periodic_timer;
static struct etimer sensor_timer;
static struct etimer reconnect_timer;
// static uint8_t led_status = 0;
static uint16_t sensor_value;
static uint16_t stream_id;
static char str_topic_state[20];
static char str_topic_sensor[30];
static char str_topic_relay[30];
static char app_buffer[128];
//static char rssi_str[5];
static process_event_t reconnect_event;
static uint8_t reconnecting = 0;
static uip_ipaddr_t google_ipv4_dns_server = {
    .u8 = {
      /* Google's IPv4 DNS in mapped in an IPv6 address (::ffff:8.8.8.8) */
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0xff, 0xff,
      0x08, 0x08, 0x08, 0x08,
    }
};


#define HOST "85.119.83.194"

#define SHORT_PAYLOAD "Hello, world!\n"
#define LONG_PAYLOAD \
"Lorem ipsum dolor sit amet, consectetur adipiscing elit. \
Etiam in auctor risus. Praesent at lorem rutrum, sagittis \
metus gravida, vulputate dolor. Nunc convallis id augue nec \
varius. Maecenas eget aliquet nunc. Aliquam convallis quis \
tortor et congue. Sed feugiat, enim at condimentum mollis, \
nibh magna varius felis, vel luctus leo odio at diam. \
Vivamus vitae lorem nec odio varius semper. Suspendisse \
lacus nisl, vehicula a lectus non, condimentum ullamcorper \
leo. Ut augue tellus, condimentum sed metus id, rutrum \
lacinia nisl. Mauris faucibus tortor vel erat consectetur, \
vitae vulputate est hendrerit. Aenean dapibus metus odio, \
eu molestie orci egestas ac. Praesent semper mi neque, a \
aliquam ipsum dictum non. Proin varius metus diam. Aenean \
non purus sit amet diam molestie laoreet quis et quam."

#define PAYLOAD SHORT_PAYLOAD


/*---------------------------------------------------------------------------*/
PROCESS(mqtt_example_process, "MQTT Example");
PROCESS(sensor_process, "Sensor");
AUTOSTART_PROCESSES(&sensor_process);
/*---------------------------------------------------------------------------*/
uint16_t get_random(void)
{
  uint32_t g_ui32RandomSeed;
  uint16_t g_ui16Random;
  g_ui32RandomSeed = clock_time();
  g_ui32RandomSeed = (g_ui32RandomSeed * 1664525) + 1013904223;
  g_ui16Random = g_ui32RandomSeed;
  return g_ui16Random;
}
/*---------------------------------------------------------------------------*/
static void
route_callback(int event, uip_ipaddr_t *route,
               uip_ipaddr_t *ipaddr, int numroutes)
{
  if (event == UIP_DS6_NOTIFICATION_DEFRT_ADD) {
    //leds_off(LEDS_ALL);
    //printf("APP - Got a RPL route\n");
  }
}

/*---------------------------------------------------------------------------*/
static void
mqtt_event(struct mqtt_connection *m, mqtt_event_t event, void* data)
{
  uint16_t i;

  switch(event) {
    case MQTT_EVENT_CONNECTED: {
      //printf("APP - Application has a MQTT connection\n");
      break;
    }
    case MQTT_EVENT_DISCONNECTED: {
      //printf("APP - Disconnected from MQTT broker\n");
      break;
      }
    case MQTT_EVENT_PUBLISH: {
      msg_ptr = data;
      /* New led value */
      if(strcmp(msg_ptr->topic, str_topic_relay) == 0) {
        msg_ptr->payload_chunk[msg_ptr->payload_length] = 0;

        if(strcmp(msg_ptr->payload_chunk, "on") == 0) {
          //printf("Relay connected"); 
        }
        if(strcmp(msg_ptr->payload_chunk, "off") == 0) {
          //printf("Relay disconnected"); 
        }
      }

      /* Implement first_flag in publish message? */
      if(msg_ptr->first_chunk) {
        msg_ptr->first_chunk = 0;
        //printf("APP - Application received a publish on topic '%s'. Payload "
        //       "size is %i bytes. Content:\n\n", msg_ptr->topic, msg_ptr->payload_length);
      }

      for(i = 0; i < msg_ptr->payload_chunk_length; i++) {
        //printf("%c", msg_ptr->payload_chunk[i]);
      }
      if(msg_ptr->payload_left == 0) {
        //printf("\n");
        //printf("\n");
        //printf("APP - Application received publish content succefully.\n");
      }
      break;
    }
    case MQTT_EVENT_SUBACK: {
      //printf("APP - Application is subscribed to topic successfully\n");
      break;
    }
    case MQTT_EVENT_UNSUBACK: {
      //printf("APP - Application is unsubscribed to topic successfully\n");
      break;
    }
    case MQTT_EVENT_PUBACK: {
      //printf("APP - Publishing complete.\n");
      break;
    }
    default:
      //printf("APP - Application got a unhandled MQTT event: %i\n", event);
      break;
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sensor_process, ev, data)
{
  PROCESS_BEGIN();
  
  /* Delay 1 second */
  etimer_set(&sensor_timer, CLOCK_SECOND*15);
  stream_id = get_random();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sensor_timer));
    /* Reset the etimer to trig again in 1 second */
    // etimer_reset(&sensor_timer);

    sprintf(str_topic_state, "%s%d%s", "ti/iot/device/", stream_id, "/online");
    sprintf(str_topic_relay, "%s%d%s", "ti/iot/device/", stream_id, "/relay");
    //printf("%s\n",str_topic_state);
    sensor_value = get_random();
    //printf("random value %d\n",sensor_value);
    sprintf(str_topic_sensor, "%s%d%s%d", "ti/iot/device/", stream_id, "/sensor/", sensor_value);
    process_start(&mqtt_example_process, NULL);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mqtt_example_process, ev, data)
{
  static struct uip_ds6_notification n;
  static uint8_t i = 0;
  static mqtt_status_t status;
  PROCESS_BEGIN();

  /* Set up DS6 callback and DNS */
  //leds_on(LEDS_ALL);
  uip_ds6_notification_add(&n, route_callback);
  mdns_init();
  mdns_conf(&google_ipv4_dns_server);

  /* Turn off AES */
  netstack_aes_set_active(1);

  /* Allocate events */
  reconnect_event = process_alloc_event();
  /* At the moment it is up to the user to provide the underlying input and
   * output buffer.
   */
  //etimer_set(&network_timer, CLOCK_SECOND*20);
  //etimer_set(&light_sense_timer, CLOCK_SECOND*30);

  mqtt_register(&conn, &mqtt_example_process, "tiot_client", mqtt_event);
  mqtt_set_last_will(&conn, str_topic_state, "0", MQTT_QOS_LEVEL_0);

  //etimer_set(&periodic_timer, CLOCK_SECOND*20);
  /* Reconnect from here */
  while(1) {

    /* Reset reconnecting flag */
    reconnecting = 0;

    /* Connect to MQTT server */
    conn.auto_reconnect = 1;
    mqtt_connect( &conn, HOST, 1883, 100 );
    PROCESS_WAIT_UNTIL(mqtt_connected(&conn));
    conn.auto_reconnect = 0;
    //printf("comes here 1********************\n");
    /* Publish to the online topic that we are online. */
    PROCESS_WAIT_UNTIL(mqtt_ready(&conn));
    mqtt_publish(&conn,
               NULL,
               str_topic_state,
               "online",
               strlen("online"),
               MQTT_QOS_LEVEL_0,
               MQTT_RETAIN_ON);
    //printf("comes here 2**************************\n");
    /* Subscribe to the relay sensor interval topic */
    PROCESS_WAIT_UNTIL(mqtt_ready(&conn));
    mqtt_subscribe(&conn,
                   NULL,
                   str_topic_relay,
                   MQTT_QOS_LEVEL_0);
    //printf("comes here 3***************************\n");
    /* Main loop */
    while(1) {
      PROCESS_WAIT_EVENT();
      if(ev == reconnect_event) {
        mqtt_disconnect(&conn);
        reconnecting = 1;
        etimer_set(&reconnect_timer, CLOCK_SECOND*10);
      }
      if(reconnecting &&
         etimer_expired(&reconnect_timer)) {
        break;
      }

      if(mqtt_ready(&conn)) {
        /* Send sensor data and toggle led */
        DBG("APP - Sending sensor value %d--\n", sensor_value);
        sprintf(app_buffer,"%s%d","Sending sensor value--", sensor_value);
        mqtt_publish(&conn,
               NULL,
               str_topic_sensor,
               app_buffer,
               strlen(app_buffer),
               MQTT_QOS_LEVEL_0,
               MQTT_RETAIN_OFF);
        etimer_restart(&sensor_timer);
      }
    }
  }
  PROCESS_END();
}



/*---------------------------------------------------------------------------*/
