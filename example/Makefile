libs := -lmoxie -lpthread
cflags := -std=c++11 -static -Wall
CC = g++

moxie_base := ../base
moxie_libs := ../base

base_srcs := $(wildcard ${moxie_base}/*.cpp)
base_headers := $(wildcard ${moxie_base}/*.h)

moxie:${base_srcs} ${base_headers}
	$(CC) echo.cpp  -I${moxie_base} -I./ -L${moxie_libs} $(cflags) -o Exemple_echo  $(libs)

.PHONY : clean
clean:
	rm *.o -rf 
