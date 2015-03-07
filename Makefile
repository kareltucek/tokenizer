CC=g++
CFLAGS= 
PREFIX=/usr/local

all: lib tokenizer 

lib: tokenizer.o 
	ar rvs tokenizer.a tokenizer.o 

tokenizer.o: tokenizer.cpp tokenizer.h automaton.cpp automaton.h datastructs.cpp datastructs.h tokenizer_lib.cpp
	${CC} ${CFLAGS} ${LDFLAGS} -c -o tokenizer.o tokenizer_lib.cpp

tokenizer: main.cpp tokenizer.o 
	${CC} ${CFLAGS} ${LDFLAGS} main.cpp tokenizer.a -o tokenizer

install: lib
	sudo mkdir -p ${PREFIX}/lib/
	sudo mkdir -p ${PREFIX}/include/drgxtokenizer
	sudo cp tokenizer.a ${PREFIX}/lib/libdrgxtokenizer.a
	sudo cp *.h *.cpp ${PREFIX}/include/drgxtokenizer/
	sudo chmod a+x ${PREFIX}/lib/libdrgxtokenizer.a
	sudo chmod a+r ${PREFIX}/include/drgxtokenizer/*

uninstall:
	rm -rf ${PREFIX}/include/drgxtokenizer
	rm ${PREFIX}/lib/libdrgxtokenizer.a

clean:
	rm *.o tokenizer *.a

