.PHONY: clean

build_flags = -O3 `pkg-config --cflags protobuf grpc++`
linked_lib = -L./common/ -lhash -lmemstat -lpthread -lpcap `pkg-config --libs protobuf grpc++`
prebuilt_obj = topk.o flow_id.o stream_summary.o topk_algorithms.o live_capture.o benchmark.o tele_service.pb.o tele_service.grpc.pb.o

all: clean compile run

compile: topk

recompile: clean topk

run:
	./topk > result.txt
	less result.txt

topk: $(prebuilt_obj) common/common.h common/pkt_headers.h
	g++ $(build_flags) $(prebuilt_obj) $(linked_lib) -o topk

flow_id.o: flow_id.h flow_id.cpp
	g++ -O3 -c flow_id.cpp -o flow_id.o

stream_summary.o: stream_summary.h stream_summary.cpp flow_id.o
	g++ -O3 -c stream_summary.cpp -o stream_summary.o

topk_algorithms.o: topk_algorithms.h topk_algorithms.cpp memory_tracker.h stream_summary.o
	g++ -O3 -c topk_algorithms.cpp -o topk_algorithms.o

topk.o: topk.h topk.cpp
	g++ $(build_flags) -c topk.cpp -o topk.o

live_capture.o: live_capture.h live_capture.cpp topk_algorithms.o
	g++ -O3 -c live_capture.cpp -o live_capture.o

benchmark.o: benchmark.h benchmark.cpp topk_algorithms.o bench_adapter.h
	g++ -O3 -c benchmark.cpp -o benchmark.o

tele_service.pb.o: tele_service.pb.h tele_service.pb.cc
	g++ $(build_flags) -c tele_service.pb.cc -o tele_service.pb.o

tele_service.grpc.pb.o: tele_service.grpc.pb.h tele_service.grpc.pb.cc
	g++ $(build_flags) -c tele_service.grpc.pb.cc -o tele_service.grpc.pb.o

clean:
	rm *.o
	rm topk