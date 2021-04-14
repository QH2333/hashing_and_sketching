all: compile run

compile: data_cleaner

run:
	./data_cleaner

data_cleaner: data_cleaner.cpp ../common/common.h ../common/pkt_headers.h
	g++ data_cleaner.cpp -o data_cleaner -lpcap
