#include "contiki-net.h"
#include "contiki.h"
#include "dev/light-sensor.h"
#include "dev/sht11-sensor.h"
#include <stdlib.h>
#include <stdint.h>

#define LIGHT 0
#define TEMP 1

#define UDP_PORT 1234

#define GET_MEASURE(a) ((a[0] >> 5) & 7)
#define GET_DELAY(a) (a[0] & 31)
#define GET_COUNT(a) (a[1])
#define COPY_MSG(a, b) do {a[0] = b[0]; a[1] = b[1];} while(0)

static struct simple_udp_connection udp_connection;
static process_event_t ready;
static unsigned char msg[2];

PROCESS(node_process, "Node process");
AUTOSTART_PROCESSES(&node_process);

static void receiver(struct simple_udp_connection *c, const uip_ipaddr_t *sender_addr, uint16_t sender_port, const uip_ipaddr_t *receiver_addr, uint16_t receiver_port, const uint8_t *data, uint16_t datalen) {
	printf("RICEVI Data received on port %d from %d.%d.%d.%d with length %d, measure %u, delay %u, count %u\n", receiver_port, uip_ipaddr1(sender_addr), uip_ipaddr2(sender_addr), uip_ipaddr3(sender_addr), uip_ipaddr4(sender_addr), datalen, GET_MEASURE(data), GET_DELAY(data), GET_COUNT(data));
	if((int) datalen == 2) {
		COPY_MSG(msg, data);
		/*In questa funzione si andranno a settare physical_quantity, n_samples e period, tali
		 *valori sono stati spediti dal sink e sono passati alla funzione nel parametro data.
		 *
		 *Bisogna vedere il formato dei messaggi che invia il sink per poter estrarre correttamente
		 *i valori dal vettore di uint8
		 *
		 *Una volta fatto questo, la funziona posta un evento al thread per fargli iniziare il campionamento
		 */
		ready = process_alloc_event();
		process_post(&node_process, ready, NULL);
	} else {
		printf("SCARTATO\n");
	}
}

PROCESS_THREAD(node_process, ev, data) {
	static struct etimer et;
	static uip_ipaddr_t addr; //indirizzo del sink
	static unsigned char localMsg[2];

	PROCESS_BEGIN();

	uip_ipaddr(&addr, 172,16,25,47);

	simple_udp_register(&udp_connection, UDP_PORT, NULL, UDP_PORT, receiver);

	while(1){
		PROCESS_WAIT_EVENT_UNTIL(ev == ready);
		localMsg[0] = msg[0];
		localMsg[1] = msg[1];
		etimer_set(&et, 5000);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		printf("MISURA mis: %u tim: %u qut: %u\n", GET_MEASURE(localMsg), GET_DELAY(localMsg), GET_COUNT(localMsg));
		//simple_udp_sendto(&udp_connection, "test", 4, &addr);

		/*if(physical_quantity == LIGHT){
			SENSORS_ACTIVATE(light_sensor);
			while(n_samples > 0){
				simple_udp_sendto(&udp_connection, light_sensor.value(LIGHT_SENSOR_TOTAL_SOLAR), sizeof(int), &addr);
				n_samples--;
				etimer_set(&et, period*CLOCK_SECOND);
				PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
			}
			SENSORS_DEACTIVATE(light_sensor);
		}
		else if(physical_quantity == TEMP){
			SENSORS_ACTIVATE(sht11_sensor);
			while(n_samples > 0){
				simple_udp_sendto(&udp_connection, sht11_sensor.value(SHT11_SENSOR_TEMP), sizeof(int), &addr);
				n_samples--;
				etimer_set(&et, period*CLOCK_SECOND);
				PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
			}
			SENSORS_DEACTIVATE(sht11_sensor);*/
	}
	PROCESS_END();
}
