
CFLAGS = -Wall -include hosts/$(HOSTNAME).h

all: init

init: init.o crysco.o
	$(CC) $(CFLAGS) -o $@ $^

# = Generic Utility Definitions =

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm *.o
	rm init

