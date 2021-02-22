all: test

test: hash_chaining.cpp libhash.a
	gcc hash_chaining.cpp -L . -l hash -o hash_chaining
	./hash_chaining

libhash.a: lookup3.o
	ar -rc libhash.a lookup3.o

lookup3.o: lookup3.cpp
	gcc -c lookup3.cpp -w

clean:
	rm *.o
	rm *.a