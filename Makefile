CC=gcc
CFLAGS=-pthread -lrt
OBJECTS=erp_upd.o
OUT=erp_exe

default: $(OBJECTS)
	$(CC) $(OBJECTS) -o $(OUT) $(CFLAGS) 
	
%.o: %.c
	$(CC) $(CFLAGS) -c $<

server: 
	$(CC) -o server UDPServer.c 
	
client:
	$(CC) -o client UDPClient.c

all: default client server
	
clean:
	rm -f *.o core
	rm -f client server erp_exe
