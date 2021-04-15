# Output binary name
bin=miner

# Set the following to '0' to disable log messages:
LOGGER ?= 1

# Compiler/linker flags
CFLAGS += -g -Wall -fPIC -DLOGGER=$(LOGGER)
LDLIBS += -lm
LDFLAGS += -L. -Wl,-rpath='$$ORIGIN'

src=miner.c sha1.c
obj=$(src:.c=.o)

all: $(bin) libminer.so

$(bin): $(obj)
	$(CC) $(CFLAGS) $(LDLIBS) $(LDFLAGS) $(obj) -o $@

libminer.so: $(obj)
	$(CC) $(CFLAGS) $(LDLIBS) $(LDFLAGS) $(obj) -shared -o $@

miner.o: miner.c miner.h sha1.h logger.h
sha1.o: sha1.c sha1.h logger.h

clean:
	rm -f $(bin) $(obj) libminer.so vgcore.*


# Tests --

test: $(bin) libminer.so ./tests/run_tests
	@DEBUG="$(debug)" ./tests/run_tests $(run)

testupdate: testclean test

./tests/run_tests:
	rm -rf tests
	git clone https://github.com/usf-cs521-sp21/P3-Tests.git tests

testclean:
	rm -rf tests
