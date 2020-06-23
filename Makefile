CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=gnu99

all: traceroute

traceroute: trace_helpers.o traceroute.o
	$(CC) traceroute.o trace_helpers.o -o traceroute $(CFLAGS)

clean:
	rm -f *.o traceroute
