
CC=gcc

CFLAGS=-Wall -g -pedantic

all: mobile_sensor

mobile_sensor: mobile_sensor.c
	$(CC) $(CFLAGS) mobile_sensor.c -o mobile_sensor
	
#Cleanup.
clean:
	rm -f *.o mobile_sensor
	