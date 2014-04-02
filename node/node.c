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
	printf("[RECEIVER] Data received on port %d from %d.%d.%d.%d with length %d, measure %u, delay %u, count %u\n", receiver_port, uip_ipaddr1(sender_addr), uip_ipaddr2(sender_addr), uip_ipaddr3(sender_addr), uip_ipaddr4(sender_addr), datalen, GET_MEASURE(data), GET_DELAY(data), GET_COUNT(data));
	if((int) datalen == 2) {
		COPY_MSG(msg, data);

		ready = process_alloc_event();

		process_post(&node_process, ready, NULL);
		printf("[RECEIVER] Messaggio in Elaborazione\n");
	} else {
		printf("[RECEIVER] Messaggio Scartato\n");
	}

}

/**
 *  The sink will have a "1" as the last byte of its address (used default subnet and network address)
 */
PROCESS_THREAD(node_process, ev, data) {
	static struct etimer et;
	static uip_ipaddr_t addr; //indirizzo del sink
	static uint8_t localMsg[2];
	static int i, measure;

	PROCESS_BEGIN();
	GET_ADDR(addr, SINK);

	simple_udp_register(&udp_connection, UDP_PORT_COMMUN, NULL, UDP_PORT_COMMUN, receiver);

	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(ev == ready);
		COPY_MSG(localMsg, msg);
		
		printf("[NODE    ] mis: %u tim: %u qut: %u\n", GET_MEASURE(localMsg), GET_DELAY(localMsg), GET_COUNT(localMsg));
		printf("[NODE    ] Set Timer %u\n", GET_DELAY(localMsg));
		etimer_set(&et, GET_DELAY(localMsg) * CLOCK_SECOND);

		if (GET_MEASURE(localMsg) == LIGHT) {
			printf("[NODE    ] Attivazione sensori luce\n");
			SENSORS_ACTIVATE(light_sensor);
			for (i = 0; i < GET_COUNT(localMsg); i++) {
				measure = light_sensor.value(LIGHT_SENSOR_TOTAL_SOLAR);
				printf("[NODE    ] Produzione %u misura %d\n", i, measure);
				simple_udp_sendto(&udp_connection, &measure, sizeof(int), &addr);

				printf("[NODE    ] In attesa su timer\n");
				PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
				etimer_reset(&et);
				printf("[NODE    ] End Timer\n");
			}
			printf("[NODE    ] Disattivazione sensori luce\n");
			SENSORS_DEACTIVATE(light_sensor);
		} else {
			printf("[NODE    ] Attivazione sensori temp\n");
			SENSORS_ACTIVATE(sht11_sensor);
			for (i = 0; i < GET_COUNT(localMsg); i++) {
				measure = sht11_sensor.value(SHT11_SENSOR_TEMP);
				printf("[NODE    ] Produzione %u misura %d\n", i, measure);
				simple_udp_sendto(&udp_connection, &measure, sizeof(int), &addr);

				printf("[NODE    ] In attesa su timer\n");
				PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
				etimer_reset(&et);
				printf("[NODE    ] End Timer\n");
			}
			printf("[NODE    ] Disattivazione sensori temp\n");
			SENSORS_DEACTIVATE(sht11_sensor);
		}
	}
	PROCESS_END();
}
