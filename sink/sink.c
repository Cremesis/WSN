#include "contiki.h"
#include "contiki-net.h"
#include "serial-line.h"
#include "../utils/util.h"
#include <stdio.h> /* for printf */

static struct simple_udp_connection udp_connection_config, udp_connection_commun;
static short configState;
static uip_ipaddr_t addr; // Address of the chosen node
static unsigned char sendMsg[2];

static uint8_t physical_quantity;

PROCESS(sink_process, "Sink");
AUTOSTART_PROCESSES(&sink_process);

static void configReceiver(
	struct simple_udp_connection *c,
	const uip_ipaddr_t *sender_addr,
	uint16_t sender_port,
	const uip_ipaddr_t *receiver_addr,
	uint16_t receiver_port,
	const uint8_t *data,
	uint16_t datalen) {

	static long input;
	static unsigned char receivedMsg[3];
	static int i = 0;

	receivedMsg[0] = data[0] - '0';
	receivedMsg[1] = data[1] - '0';
	receivedMsg[2] = data[2] - '0';

	if(datalen != 4) { // Not a valid input
		printf("ATTENTION: Exactly 3 digits allowed!\n\n");
		return;
	}

	input = 0;
	for(i = 0; i < datalen-1; i++) {
		input = input * 10 + receivedMsg[i];
	}

	switch(configState) {

	case 0: // choose node
		if(input == NODE1 || input == NODE2 || input == NODE3 || input == NODE4) {
			GET_ADDR(addr, input);
			configState++;
			printf("\nWhat do you want to measure?\n\n\t* %d = Light\n\t* %d = Temperature\n\n", LIGHT, TEMP);
		} else {
			printf("\nATTENTION: ID does not exist\n\n");
			printf("\nAvailable nodes:\n\n\t* %d\n\t* %d\n\t* %d\n\t* %d\n\nInsert the ID of the node you want to communicate with:\n\n", NODE1, NODE2, NODE3, NODE4);
		}
		break;

	case 1: // choose measure
		if(input == LIGHT || input == TEMP) {
			SET_MEASURE(sendMsg, input);
			physical_quantity = input;
			configState++;
			printf("\nWhat delay do you want between measures? (in seconds)\n\n");
		} else {
			printf("\nATTENTION: Measure not available\n\n");
			printf("\nWhat do you want to measure?\n\n\t* %d = Light\n\t* %d = Temperature\n\n", LIGHT, TEMP);
		}
		break;

	case 2: // choose delay
		if(input > 0 && input <= MAX_DELAY) {
			SET_DELAY(sendMsg, input);
			configState++;
			printf("\nHow many measures?\n\n");
		} else {
			printf("\nATTENTION: Delay must must be between 1 and %d\n\n", MAX_DELAY);
			printf("\nWhat delay do you want between measures? (in seconds)\n\n");
		}
		break;

	case 3: // choose count
		if(input > 0 && input <= MAX_COUNT) {
			SET_COUNT(sendMsg, input);
			configState = 0;

			//printf("\nThe final packet is [0] = %u, [1] = %u\n", sendMsg[0], sendMsg[1]);
			simple_udp_sendto(&udp_connection_commun, sendMsg, 2, &addr);
			sendMsg[0] = 0;
			sendMsg[1] = 0;
			printf("\nREQUEST SENT!\n\n");
			printf("\nAvailable nodes:\n\n\t* %d\n\t* %d\n\t* %d\n\t* %d\n\nInsert the ID of the node you want to communicate with:\n\n", NODE1, NODE2, NODE3, NODE4);
		} else {
			printf("\nATTENTION: Number of measures must be between 1 and %d\n\n", MAX_COUNT);
			printf("\nHow many measures?\n\n");
		}
		break;

	}
}

static void communReceiver(
	struct simple_udp_connection *c,
	const uip_ipaddr_t *sender_addr,
	uint16_t sender_port,
	const uip_ipaddr_t *receiver_addr,
	uint16_t receiver_port,
	const uint8_t *data,
	uint16_t datalen) {
		switch(physical_quantity){
			case LIGHT:
				printf("From: %d.%d.%d.%d\tLight: %d\n", uip_ipaddr1(sender_addr), uip_ipaddr2(sender_addr), uip_ipaddr3(sender_addr), uip_ipaddr4(sender_addr), *((int*)data) * 46 / 10);
				break;
			case TEMP:
				printf("From: %d.%d.%d.%d\tTemperature: %d.%d\n", uip_ipaddr1(sender_addr), uip_ipaddr2(sender_addr), uip_ipaddr3(sender_addr), uip_ipaddr4(sender_addr), (*((int*)data) / 10 - 396) / 10, (*((int*)data) / 10 - 396) % 10);
				break;
		}
		
}

PROCESS_THREAD(sink_process, ev, data) {

	configState = 0;

	PROCESS_BEGIN();

	simple_udp_register(&udp_connection_config, UDP_PORT_CONFIG, NULL, UDP_PORT_CONFIG, configReceiver);
	simple_udp_register(&udp_connection_commun, UDP_PORT_COMMUN, NULL, UDP_PORT_COMMUN, communReceiver);

	printf("\nAvailable nodes:\n\n\t* %d\n\t* %d\n\t* %d\n\t* %d\n\nInsert the ID of the node you want to communicate with:\n\n", NODE1, NODE2, NODE3, NODE4);

	PROCESS_END();
}
