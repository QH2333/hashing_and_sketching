all: compile run

compile: top_k

run:
	./top_k > result.txt
	less result.txt

top_k: top_k.h top_k.cpp flow_id.h topk_algorithms.h topk_algorithms.cpp stream_summary.h memory_tracker.h bench_adapter.h ../common/common.h ../common/pkt_headers.h
	g++ -O3 top_k.cpp topk_algorithms.cpp -L ../common/ -l hash -o top_k

stream_summary: stream_summary.cpp
	g++ stream_summary.cpp -L ../common/ -l hash -o stream_summary
	./stream_summary
