#include "contiki-net.h"

#include "contiki.h"

#include "dev/light-sensor.h"

#include "dev/sht11-sensor.h"

#include "../utils/util.h"

#include <stdlib.h>

#include <stdint.h>



static struct simple_udp_connection udp_connection;

static process_event_t ready;

static uint8_t msg[2];



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
		printf("Ricevuto\n");

	} else {

		printf("SCARTATO\n");

	}

}



PROCESS_THREAD(node_process, ev, data) {

	static struct etimer et;

	static uip_ipaddr_t addr; //indirizzo del sink

	static uint8_t localMsg[2];



	PROCESS_BEGIN();



	// The sink will have a "1" as the last byte of its address (used default subnet and network address)

	GET_ADDR(addr, SINK);



	simple_udp_register(&udp_connection, UDP_PORT_COMMUN, NULL, UDP_PORT_COMMUN, receiver);



	while(1){

		PROCESS_WAIT_EVENT_UNTIL(ev == ready);

		localMsg[0] = msg[0];

		localMsg[1] = msg[1];

		

		printf("MISURA mis: %u tim: %u qut: %u\n", GET_MEASURE(localMsg), GET_DELAY(localMsg), GET_COUNT(localMsg));

		printf("Set Timer %u\n", GET_DELAY(localMsg));
		etimer_set(&et, GET_DELAY(localMsg) * CLOCK_SECOND);

if (GET_MEASURE(localMsg) == LIGHT) {

				printf("Attivazione sensori luce\n");
				SENSORS_ACTIVATE(light_sensor);



				while(GET_COUNT(localMsg) > 0){
					int temp = light_sensor.value(LIGHT_SENSOR_TOTAL_SOLAR);
					printf("Produzione %u misura %d\n", GET_COUNT(localMsg), temp);
					simple_udp_sendto(&udp_connection, &temp, sizeof(int), &addr);

					SET_COUNT(localMsg, GET_COUNT(localMsg) - 1);

					printf("In attesa su timer\n");
					PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
					etimer_reset(&et);
					printf("End Timer\n");

				}
				printf("Disattivazione sensori luce\n");
				SENSORS_DEACTIVATE(light_sensor);
} else {
	printf("Attivazione sensori temp\n");
				SENSORS_ACTIVATE(sht11_sensor);

				while(GET_COUNT(localMsg) > 0){
					int temp = sht11_sensor.value(SHT11_SENSOR_TEMP);
					printf("Produzione %u misura %d\n", GET_COUNT(localMsg), temp);
					simple_udp_sendto(&udp_connection, &temp, sizeof(int), &addr);

				//	simple_udp_sendto(&udp_connection, sht11_sensor.value(SHT11_SENSOR_TEMP), sizeof(int), &addr);

					SET_COUNT(localMsg, GET_COUNT(localMsg) - 1);

					printf("In attesa su timer\n");
					PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
					etimer_reset(&et);
					printf("End Timer\n");

				}
				printf("Disattivazione sensori temp\n");
				SENSORS_DEACTIVATE(sht11_sensor);

		}

	}

	PROCESS_END();

}
