all: compile run

compile: data_cleaner

run:
	./data_cleaner

data_cleaner: data_cleaner.cpp
	g++ data_cleaner.cpp -o data_cleaner -lpcap
