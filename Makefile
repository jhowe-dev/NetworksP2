CC= /usr/bin/gcc
all:	udpserver nonblock-udpclient

udpserver: udpserver.c;
	${CC} udpserver.c -o udpserver

nonblock-udpclient:	nonblock-udpclient.c;
	$(CC) nonblock-udpclient.c -o nonblock-udpclient -lm

clean:
	rm  udpserver nonblock-udpclient 
