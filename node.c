#include "contiki-net.h"
#include "contiki.h"
#include "dev/light-sensor.h"
#include "dev/sht11-sensor.h"
#include <stdlib.h>

#define LIGHT 0
#define TEMP 1
#define UDP_PORT 1234

static struct simple_udp_connection udp_connection;
static process_event_t ready;

static int physical_quantity; //grandezza fisica da campionare
static int n_samples;         //numero di campioni da produrre
static int period;            //periodo di campionamento espresso in secondi

PROCESS(node_process, "Node process");
AUTOSTART_PROCESSES(&node_process);

static void
receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  printf("Data received on port %d from port %d with length %d\n",
         receiver_port, sender_port, datalen);
 
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
}

PROCESS_THREAD(node_process, ev, data){

	static struct etimer et;
	
	PROCESS_BEGIN();
	simple_udp_register(&udp_connection, UDP_PORT, NULL, UDP_PORT, receiver);

	while(1){
		PROCESS_WAIT_EVENT_UNTIL(ev == ready);
		if(physical_quantity == LIGHT){
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
			SENSORS_DEACTIVATE(sht11_sensor);
	}
	PROCESS_END();
}
			
















