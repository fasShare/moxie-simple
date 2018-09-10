
moxie:
	make -C base
	make -C example
	
.PHONY : clean
clean:
	make clean -C base 
	make clean -C example 
