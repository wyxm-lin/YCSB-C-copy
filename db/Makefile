CC=g++
# CFLAGS=-std=c++11 -c -g -Wall -fgnu-tm -DERPC_INFINIBAND=true
CFLAGS=-std=c++11 -c -O2 -Wall -fgnu-tm -DERPC_INFINIBAND=true
INCLUDES=-I../ -I../../eRPC/src -I../../eRPC/third_party/asio/include
LDFLAGS=-L ../../eRPC/build -lerpc -lpthread -lnuma -ldl -libverbs
SOURCES=$(wildcard *.cc)
OBJECTS=$(SOURCES:.cc=.o)

all: $(SOURCES) $(OBJECTS)

.cc.o:
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) $< -o $@

clean:
	$(RM) $(OBJECTS)