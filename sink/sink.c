#include "contiki.h"
#include "dev/sht11-sensor.h"
#include "dev/light-sensor.h"

#include <stdio.h> /* For printf() */

PROCESS(sink_process, "Sink");
AUTOSTART_PROCESSES(&sink_process);

PROCESS_THREAD(sink_process, ev, data) {
	PROCESS_BEGIN(); 
	printf("Hello, world!\n");
	
	static struct etimer et;
	
	etimer_set(&et, CLOCK_SECOND*4);
	
	while(1) { 
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		printf("Hello, world!\n");
		etimer_reset(&et);
	}
	PROCESS_END(); 
}