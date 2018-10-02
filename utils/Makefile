libs := -lpthread
cflags := -std=c++11 -static -Wall
CC = g++

moxie_utils := ./utils

objects := $(patsubst %.cpp, %.o, $(wildcard *.cpp))
headers := $(wildcard *.h)

utils:$(objects)
	ar -crv libutils.a $^
	mv libutils.a ../lib
%.o:%.cpp $(headers)
	$(CC) -c $<  -I${moxie_utils} -I./ $(cflags) -o $@  $(libs)

.PHONY : clean
clean:
	rm *.o -rf 
