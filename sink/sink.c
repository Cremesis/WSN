#include "contiki.h"
#include "contiki-net.h"
#include "uart1.h"
#include "slip.h"
#include "serial-line.h"
#include "../utils/util.h"
#include <stdio.h>
#include <stdlib.h> /*for atoi*/

PROCESS(sink_process, "Sink");
AUTOSTART_PROCESSES(&sink_process);

PROCESS_THREAD(sink_process, ev, data) {
	PROCESS_BEGIN();
	
	while(1) {
		printf("Inserisci il numero del nodo con cui vuoi comunicare:>\n");

		uart1_set_input(slip_input_byte);

		PROCESS_WAIT_EVENT_UNTIL(ev == serial_line_event_message);

     		printf("received number: %s\n", (char *) data);
	}
	PROCESS_END(); 
}
