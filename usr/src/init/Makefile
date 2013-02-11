
#CFLAGS = -mmcu=msp430g2553 -Wall

all: init

init: init.o crysco.o
	$(CC) $(CFLAGS) -o $@ $^

# = Generic Utility Definitions =

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm *.o
	rm init

