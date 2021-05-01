.PHONY: clean

CPP_FILES = tele_service.grpc.pb.cc tele_service.grpc.pb.h tele_service.pb.cc tele_service.pb.h
PYTHON_FILES = tele_service_pb2.py tele_service_pb2_grpc.py

all: compile install

compile: $(CPP_FILES) $(PYTHON_FILES)

install: $(CPP_FILES) $(PYTHON_FILES)
	cp tele_service*.cc ../topk_agent
	cp tele_service*.h ../topk_agent
	cp tele_service*.py ../web

$(CPP_FILES): tele_service.proto
	protoc -I. --cpp_out=. tele_service.proto
	protoc -I. --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` tele_service.proto

$(PYTHON_FILES): tele_service.proto
	python3 -m grpc_tools.protoc -I. --python_out=. --grpc_python_out=. tele_service.proto

clean:
	rm -f $(CPP_FILES) $(PYTHON_FILES)
	cd ../topk_agent; rm -f $(CPP_FILES)
	cd ../web; rm -f $(PYTHON_FILES)