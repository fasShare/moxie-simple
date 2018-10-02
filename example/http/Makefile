libs := -lmoxie -lutils -ltcmalloc_minimal -lpthread 
cflags := -O2 -std=c++11 -static -Wall
CC = g++

LDDIR += -L../../lib

INCLUDEDIR += -I./ \
			  -I../../base \
			  -I../../ 

moxie_base := ../../base
moxie_libs := ../../base

objects := $(patsubst %.cpp, %.o, $(wildcard *.cpp))
headers := $(wildcard *.h)

all:
	make HttpServer
HttpServer:$(objects)
	$(CC) $^ -o $@ $(LDDIR) $(libs)
%.o:%.cpp $(headers)
	echo $(LDDIR)
	$(CC) -c $< $(INCLUDEDIR) -I./ $(cflags) -o $@  

.PHONY : clean
clean:
	rm *.o -rf
