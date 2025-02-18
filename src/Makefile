# Make file for ov103 - OSDP discovery

PROGS=discovery-client discovery-server

CC = clang
LINK = clang
CFLAGS = -c -g -I../include -Wall -Werror
LDFLAGS = -g -ljansson

all:	${PROGS}
	mkdir -p ../opt/tester/bin
	cp ${PROGS} ../opt/tester/bin

clean:
	rm -f core *.o ${PROGS}

discovery-client:	discovery-client.o discovery-utils.o Makefile
	${LINK} -o discovery-client discovery-client.o discovery-utils.o ${LDFLAGS}

discovery-server:	discovery-server.o \
	  discovery-osdp.o discovery-utils.o osdp-crc.o Makefile
	${LINK} -o discovery-server discovery-server.o \
	  discovery-osdp.o discovery-utils.o osdp-crc.o ${LDFLAGS}

discovery-server.o:	discovery-server.c
	${CC} ${CFLAGS} discovery-server.c

discovery-osdp.o:	discovery-osdp.c
	${CC} ${CFLAGS} discovery-osdp.c

discovery-utils.o:	discovery-utils.c
	${CC} ${CFLAGS} discovery-utils.c

osdp-crc.o:	osdp-crc.c
	${CC} ${CFLAGS} osdp-crc.c

