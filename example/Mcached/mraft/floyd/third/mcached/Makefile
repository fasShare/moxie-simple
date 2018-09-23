libs := -lpthread
cflags := -std=c++11 -O2 -static -Wall
CC = g++

moxie_base := ./base
moxie_libs := ./lib

objects := $(patsubst %.cpp, %.o, $(wildcard *.cpp))
headers := $(wildcard *.h)

mcached:$(objects)
	ar -crv libmcached.a $^
%.o:%.cpp $(headers)
	$(CC) -c $<  -I${moxie_base} -I./ -I../ -I/opt/protobuf/include -L${moxie_libs} $(cflags) -o $@  $(libs)

.PHONY : clean
clean:
	rm *.o -rf 
