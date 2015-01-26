#include <string.h>
#include "mist.h"
#include "dev/button-sensor.h"
#include "dev/board.h"
#include "dev/i2c.h"
#include "netstack-aes.h"
#include "mqtt.h"
#include "../../dev/sht21/sht21.h"
#include "../../dev/ds18b20/ds18b20.h"
#include "../../dev/ds18b20/owhal.h"
char ctemp[20];
char chum[20];
char ctemps[20];
#define SEND_INTERVAL (60 * CLOCK_SECOND)
#define MIN(a, b) ((a) < (b) ? (a) : (b))
static struct mqtt_connection conn;
//#define APP_BUFFER_SIZE 128
//static uint8_t app_buffer[128];
static struct mqtt_message* msg_ptr = 0;
//static uint8_t first_flag = 1;
//static struct etimer periodic_timer;
static struct etimer light_sense_timer;
static struct etimer reconnect_timer;
static uint8_t led_status = 0;
static char str_topic_state[20];
static char str_topic_sensor[30];
static char str_topic_led[30];
static char app_buffer[128];
static uint16_t button_sensor_value=0;
//static char rssi_str[5];
static process_event_t led_updates_changed_event;
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
#define HOST "198.41.30.241" // m2m.eclipse.org
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
PROCESS(mqtt_example_process, "MQTT Example");
PROCESS(environode_process, "Environode sensing and sending");
AUTOSTART_PROCESSES(&environode_process);
/*---------------------------------------------------------------------------*/
static void
route_callback(int event, uip_ipaddr_t *route,
uip_ipaddr_t *ipaddr, int numroutes)
{
if (event == UIP_DS6_NOTIFICATION_DEFRT_ADD) {
//leds_off(LEDS_ALL);
printf("APP - Got a RPL route\r\n");
}
}
/*---------------------------------------------------------------------------*/
static void
mqtt_event(struct mqtt_connection *m, mqtt_event_t event, void* data)
{
uint16_t i;
switch(event) {
case MQTT_EVENT_CONNECTED: {
printf("APP - Application has a MQTT connection\r\n");
break;
}
case MQTT_EVENT_DISCONNECTED: {
printf("APP - Disconnected from MQTT broker\r\n");
break;
}
case MQTT_EVENT_PUBLISH: {
msg_ptr = data;
/* New led value */
if(strcmp(msg_ptr->topic, str_topic_led) == 0) {
msg_ptr->payload_chunk[msg_ptr->payload_length] = 0;
if(strcmp((const char *)msg_ptr->payload_chunk, "on") == 0) {
fade(LEDS_GREEN);
}
if(strcmp((const char *)msg_ptr->payload_chunk, "off") == 0) {
fade(LEDS_RED);
}
}
/* Implement first_flag in publish message? */
if(msg_ptr->first_chunk) {
msg_ptr->first_chunk = 0;
printf("APP - Application received a publish on topic '%s'. Payload "
"size is %i bytes. Content:\r\n", msg_ptr->topic, msg_ptr->payload_length);
}
for(i = 0; i < msg_ptr->payload_chunk_length; i++) {
printf("%c", msg_ptr->payload_chunk[i]);
}
if(msg_ptr->payload_left == 0) {
printf("\r\n");
printf("\r\n");
printf("APP - Application received publish content succefully.\r\n");
}
break;
}
case MQTT_EVENT_SUBACK: {
printf("APP - Application is subscribed to topic successfully\r\n");
break;
}
case MQTT_EVENT_UNSUBACK: {
printf("APP - Application is unsubscribed to topic successfully\r\n");
break;
}
case MQTT_EVENT_PUBACK: {
printf("APP - Publishing complete\r\n");
break;
}
default:
printf("APP - Application got a unhandled MQTT event: %d\r\n", event);
break;
}
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(environode_process, ev, data)
{
PROCESS_BEGIN();
printf("Environode process\r\n");
static uint8_t reset_flag = 1;
SENSORS_ACTIVATE(button_user_sensor);
SENSORS_ACTIVATE(button_sw1_sensor);
SENSORS_ACTIVATE(button_sw2_sensor);
while(1) {
PROCESS_WAIT_EVENT_UNTIL((ev == sensors_event) && (
data == &button_sw1_sensor||data == &button_sw2_sensor||data == &button_user_sensor));
if(data == &button_user_sensor){
if(reset_flag){
reset_flag = 0;
sprintf(str_topic_state, "%s", "bontorgate/node2");
printf("%s\r\n",str_topic_state);
sprintf(str_topic_sensor, "%s", "bontorgate/node2/sensor");
sprintf(str_topic_led, "%s", "bontorgate/node2/led");
process_start(&mqtt_example_process, NULL);
}else{
printf("%s\r\n", str_topic_state);
}
}
else if(data == &button_sw1_sensor){
/* We must init I2C each time, because the module lose his state when enter PM2 */
i2c_init(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SCL_PORT, I2C_SCL_PIN, I2C_SCL_FAST_BUS_SPEED);
read_temperature(ctemp); // temperature SHT21
printf("SHT21 temperature value: %s\r\n", ctemp);
read_humidity(chum); // humidity SHT21
printf("SHT21 humidity value: %s\r\n", chum);
}
else if(data == &button_sw2_sensor){
button_sensor_value++;
printf("Button sensor value: %d\r\n", button_sensor_value);
}
}
PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mqtt_example_process, ev, data)
{
static struct uip_ds6_notification n;
PROCESS_BEGIN();
/* Set up DS6 callback and DNS */
//leds_on(LEDS_ALL);
uip_ds6_notification_add(&n, route_callback);
mdns_init();
mdns_conf(&google_ipv4_dns_server);
/* Turn off AES */
netstack_aes_set_active(1);
/* Allocate events */
led_updates_changed_event = process_alloc_event();
reconnect_event = process_alloc_event();
/* At the moment it is up to the user to provide the underlying input and
* output buffer.
*/
//etimer_set(&network_timer, CLOCK_SECOND*20);
etimer_set(&light_sense_timer, CLOCK_SECOND*30);
mqtt_register(&conn, &mqtt_example_process, "node2_client", mqtt_event);
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
(uint8_t*)"online",
strlen("online"),
MQTT_QOS_LEVEL_0,
MQTT_RETAIN_ON);
//printf("comes here 2**************************\n");
/* Subscribe to the light sensor interval topic */
PROCESS_WAIT_UNTIL(mqtt_ready(&conn));
mqtt_subscribe(&conn,
NULL,
str_topic_led,
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
if(etimer_expired(&light_sense_timer) && mqtt_ready(&conn)) {
/* Send light sensor data and toggle led */
led_status ^= 1;
if(led_status) {
//leds_on(LEDS_D1_GREEN);
} else {
//leds_off(LEDS_D1_GREEN);
}
DBG("APP - Sending button sensor value %d--\r\n", button_sensor_value);
sprintf(app_buffer,"%s%d","Sending button sensor value--", button_sensor_value);
DBG("%s\r\n", app_buffer);
mqtt_publish(&conn,
NULL,
str_topic_sensor,
(uint8_t*)app_buffer,
strlen(app_buffer),
MQTT_QOS_LEVEL_0,
MQTT_RETAIN_ON);
etimer_restart(&light_sense_timer);
}
}
}
PROCESS_END();
}