#include "contiki.h"
#include "contiki-net.h"
#include "serial-line.h"
#include "../utils/util.h"
#include <stdio.h> /* for printf */
#include <stdlib.h> /* for strtol */

PROCESS(sink_process, "Sink");
AUTOSTART_PROCESSES(&sink_process);

PROCESS_THREAD(sink_process, ev, data) {
	static long input; // Serial port input
	static uip_ipaddr_t addr; // Address of the chosen node
	static unsigned char sendMsg[2];
	static char* end; // Used to check the correctness of the serial input
	static int flag = 1, i;

	PROCESS_BEGIN();

	while(1) {
		while(flag) {
			printf("\nAvailable nodes:\n\n\t* %d\n\t* %d\n\nInsert the ID of the node you want to communicate with:\n\n", NODE1, NODE2);
			
			PROCESS_WAIT_EVENT_UNTIL(ev == serial_line_event_message);

			input = strtol((char *) data, &end, 10);
			if(!*end && (input == NODE1 || input == NODE2)) {
				GET_ADDR(addr, input);
				flag = 0;
			} else {
				printf("\nATTENTION: ID does not exist\n\n");
			}
		}
		flag = 1;
		while(flag) {
			printf("\nWhat do you want to measure?\n\n\t* %d = Light\n\t* %d = Temperature\n\n", LIGHT, TEMP);
			
			PROCESS_WAIT_EVENT_UNTIL(ev == serial_line_event_message);

			input = strtol((char *) data, &end, 10);
			if(!*end && (input == LIGHT || input == TEMP)) {
				SET_MEASURE(sendMsg, input);
				flag = 0;
			} else {
				printf("\nATTENTION: Measure not available\n\n");
			}
		}
		flag = 1;
		while(flag) {
			printf("\nHow long between measures? (in seconds)\n\n");
			
			PROCESS_WAIT_EVENT_UNTIL(ev == serial_line_event_message);

			input = strtol((char *) data, &end, 10);
			if(!*end && (input > 0 && input <= MAX_DELAY)) {
				SET_DELAY(sendMsg, input);
				flag = 0;
			} else {
				printf("\nATTENTION: Delay must must be between 1 and %d\n\n", MAX_DELAY);
			}
		}
		flag=1;
		while(flag) {
			printf("\nHow many measures?\n\n");
			
			PROCESS_WAIT_EVENT_UNTIL(ev == serial_line_event_message);

			input = strtol((char *) data, &end, 10);
			if(!*end && (input > 0 && input <= MAX_COUNT)) {
				SET_COUNT(sendMsg, input);
				flag = 0;
			} else {
				printf("\nATTENTION: Number of measures must be between 1 and %d\n\n", MAX_COUNT);
			}
		}
		flag=1;

		printf("\nthe final packet is [0] = %u, [1] = %u\n", sendMsg[0], sendMsg[1]);
		// TODO: send packet
		printf("\nREQUEST SENT!\n\n");
	}
	PROCESS_END(); 
}
