
#CFLAGS = -Wall -include hosts/`hostname`.h
CFLAGS = -Wall -include hosts/himmel.h

all: init

init: init.o crysco.o bstrlib/bstrlib.o hosts/himmel.o
	$(CC) $(CFLAGS) -static -o $@ $^

# = Generic Utility Definitions =

%.o: %.c
	$(CC) $(CFLAGS) -static -o $@ -c $<

clean:
	find -name "*.o" -exec rm {} \;
	rm init

