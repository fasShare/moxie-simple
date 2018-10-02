
moxie:
	mkdir -p lib
	make -C base
	make -C utils
	make -C example
	
.PHONY : clean
clean:
	make clean -C base 
	make clean -C example 
