#Makefile
CFLAGS=-Wall

all:	polipd polipconfig 

debug:	polipdd

polipd:		polipd.o readconf.o setup.o server.o stats.o  poliputil.o check.o
	$(CC)  -o polipd polipd.o readconf.o setup.o server.o stats.o poliputil.o check.o -lpopt 

polipdd:	polipd.o readconf.o setup.o server.o stats.o poliputil.o check.o
	$(CC)	-g polipd.c readconf.c setup.c server.c stats.c poliputil.o -o polipd check.o

polipconfig:	polipconfig.o readconf.o writeconf.o poliputil.o
	$(CC)  -o polipconfig polipconfig.o readconf.o  writeconf.o poliputil.o -lnewt

polipd.o:	polipd.c polip.h

polipconfig.o:	polipconfig.c polip.h 

readconf.o:	readconf.c polip.h

writeconf.o:	writeconf.c polip.h

setup.o:	setup.c polip.h

server.o: 	server.c polip.h

stats.o:	stats.c polip.h

poliputil.o:	poliputil.c polip.h

install:
	 cp -f polip* /usr/bin/

clean:
	rm -f polipd polipconfig core *.o
	
