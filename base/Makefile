libs := -lpthread
cflags := -O2 -std=c++11 -static -Wall
CC = g++

moxie_base := ./base
moxie_libs := ./lib

objects := $(patsubst %.cpp, %.o, $(wildcard *.cpp))
headers := $(wildcard *.h)

moxie:$(objects)
	ar -crv libmoxie.a $^
	mv libmoxie.a ../lib
%.o:%.cpp $(headers)
	$(CC) -c $<  -I${moxie_base} -I./ -L${moxie_libs} $(cflags) -o $@  $(libs)

.PHONY : clean
clean:
	rm *.o -rf 
