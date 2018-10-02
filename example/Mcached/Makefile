libs := -lmoxie -lfloyd -lslash -lpink -lmcached -lprotobuf -lrocksdb -ltcmalloc_minimal -lpthread 
cflags := -O2 -std=c++11 -static -Wall
CC = g++

LDDIR += -L../../lib \
		 -L./mraft/floyd/lib \
		 -L./mraft/floyd/third/mcached \
		 -L./mraft/floyd/third/pink/pink/lib \
		 -L./mraft/floyd/third/slash/slash/lib \
		 -L/opt/protobuf/lib \
		 -L/opt/rocksdb/lib \
		 -L/opt/gperftools-2.7/lib

INCLUDEDIR += -I./ \
			  -I../../base \
			  -I./mraft \
			  -I./marft/floyd \
			  -I./mraft/floyd/include \
			  -I./mraft/floyd/third/mcached \
			  -I./mraft/floyd/third/pink \
			  -I./mraft/floyd/third/slash \
			  -I/opt/protobuf/include \
			  -I/opt/rocksdb/include/

moxie_base := ../../base
moxie_libs := ../../base

objects := $(patsubst %.cpp, %.o, $(wildcard *.cpp))
headers := $(wildcard *.h)

all:
	make -C ./mraft/floyd/third/mcached
	make -C ./mraft/floyd/third/pink/pink
	make -C ./mraft/floyd/third/slash/slash
	make -C ./mraft/floyd/
	make Mcached
Mcached:$(objects)
	$(CC) $^ -o $@ $(LDDIR) $(libs)
%.o:%.cpp $(headers)
	echo $(LDDIR)
	$(CC) -c $< $(INCLUDEDIR) -I./ $(cflags) -o $@  

.PHONY : clean
clean:
	rm *.o -rf
	make clean -C ./mraft/floyd/third/mcached
	make clean -C ./mraft/floyd/third/pink/pink 
	make clean -C ./mraft/floyd/third/slash/slash
	make clean -C ./mraft/floyd/
