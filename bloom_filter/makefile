all: compile run

compile: hash_chaining

run:
	./hash_chaining

hash_chaining: hash_chaining.cpp hash_map.h
	g++ hash_chaining.cpp -L ../common -l hash -o hash_chaining

# libhash.a: lookup3.cpp
# 	g++ -c lookup3.cpp -w
# 	ar -rc libhash.a lookup3.o

clean:
	rm *.o
	rm *.a