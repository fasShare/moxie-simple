libs := -lmoxie -lpthread
cflags := -std=c++11 -static -Wall
CC = g++

moxie_base := ../base
moxie_libs := ../base

base_srcs := $(wildcard ${moxie_base}/*.cpp)
base_headers := $(wildcard ${moxie_base}/*.h)

moxie:${base_srcs} ${base_headers}
	$(CC) Echo.cpp  -I${moxie_base} -I./ -L${moxie_libs} $(cflags) -o Example_bin_Echo  $(libs)
	$(CC) MultiThreadEcho.cpp  -I${moxie_base} -I./ -L${moxie_libs} $(cflags) -o Example_bin_MultiThreadEcho  $(libs)
	$(CC) TimerExam.cpp  -I${moxie_base} -I./ -L${moxie_libs} $(cflags) -o Example_bin_TimerExam  $(libs)
	make -C ./Mcached

.PHONY : clean
clean:
	rm *.o -rf
	rm Example_bin_* -rf
