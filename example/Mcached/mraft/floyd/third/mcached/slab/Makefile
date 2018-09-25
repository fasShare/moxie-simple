libs := -lpthread
cflags := -static -Wall -std=c++11
CC = g++

objects := $(patsubst %.cpp, %.o, $(wildcard *.cpp))
headers := $(wildcard *.h)

McachedSlab:$(objects)
	$(CC) -o $@ $^ $(cflags)
%.o:%.cpp $(headers)
	$(CC) -c $<  -I./  $(cflags) -o $@  $(libs)

.PHONY : clean
clean:
	rm *.o -rf 
