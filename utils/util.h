#ifndef UTIL_EXAM
#define UTIL_EXAM

#define LIGHT 0
#define TEMP 1

#define SINK 0
#define NODE1 1
#define NODE2 2

#define UDP_PORT_CONFIG 4980
#define UDP_PORT_COMMUN 3444

#define SET_HOST_ADDR(a) do {	uip_ipaddr_t hostAddr;\
				uip_ipaddr_t hostMask;\
				uip_ipaddr(&hostAddr, 172, 16, 78, a);\
				uip_ipaddr(&hostMask, 255, 255, 255, 0);\
				uip_sethostaddr(&hostAddr);\
				uip_setnetmask(&hostMask);} while(0)

#define GET_ADDR(a, b) do {	switch(b) {\
				case SINK:\
					uip_ipaddr(&a, 172, 16, 55, 23);\
					break;\
				case NODE1:\
					uip_ipaddr(&a, 172, 16, 25, 47);\
					break;\
				case NODE2:\
                                        uip_ipaddr(&a, 172, 16, 254, 49);\
                                       break;\
				}} while(0)

#define MAX_MEASURE 31
#define MAX_DELAY 31
#define MAX_COUNT 255

#define GET_MEASURE(a) (a[0] >> 5) & 7
#define GET_DELAY(a) a[0] & 31
#define GET_COUNT(a) a[1]

#define SET_MEASURE(a, b) do {a[0] = (b << 5) | a[0];} while(0)
#define SET_DELAY(a, b) do {a[0] = b | a[0];} while(0)
#define SET_COUNT(a, b) do {a[1] = b;} while(0)

#define COPY_MSG(a, b) do {a[0] = b[0]; a[1] = b[1];} while(0)

#endif
