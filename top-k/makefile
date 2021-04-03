all: compile run

compile: top_k

run:
	./top_k

top_k: top_k.cpp
	g++ top_k.cpp -L ../common/ -l hash -o top_k
