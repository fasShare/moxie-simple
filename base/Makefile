libs := -lpthread
cflags := -std=c++11 -static -Wall
CC = g++

moxie_base := ./base
moxie_libs := ./lib

objects := $(patsubst %.cpp, %.o, $(wildcard *.cpp))

moxie:$(objects)
	ar -crv libmoxie.a $^
%.o:%.cpp
	$(CC) -c $^  -I${moxie_base} -I./ -L${moxie_libs} $(cflags) -o $@  $(libs)

.PHONY : clean
clean:
	rm *.o -rf 
