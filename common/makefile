libhash.a: lookup3.h lookup3.cpp
	g++ -O3 -c lookup3.cpp -o lookup3.o
	ar -cr libhash.a lookup3.o

libmemstat.a: get_memstat.h get_memstat.cpp
	g++ -O3 -c get_memstat.cpp -o get_memstat.o
	ar -cr libmemstat.a get_memstat.o