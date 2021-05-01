/**
 * @file top_k.cpp
 * @author QH2333 (qi_an_hao@126.com)
 * @brief 
 * @version 0.1
 * @date 2021-04-03
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "topk.h"

#include "benchmark.h"
#include "live_capture.h"

// === Control the program's behavior here ===

// #define ALGORITHM_TO_BENCH exact_algo
// #define ALGORITHM_PARAMETER (K)
#define RPC_PROTOCOL_VER "0.1"

// === End of behavior control section ===

void run_grpc_server();
void run_capture();
void test_for_join();
bool capture_running = false;
bool last_result_ready = false;
std::thread *capture_thread = nullptr;
topk_algo_base *algo_obj = nullptr;
std::vector<std::pair<flow_id, int>> topk_result;

class tele_service_impl final : public tele_service::Service {
public:
    grpc::Status get_if_list(::grpc::ServerContext* context, const empty_request* request, getif_response* response) override
    {
        std::cout << "[NewRPC]get_if_list()" << std::endl;
        std::vector<if_info_t> if_list = get_interface_list();
        response->set_protocol_version(RPC_PROTOCOL_VER);
        for (auto interface : if_list)
        {
            getif_response::if_info_entry *new_entry = response->add_interface_list();
            new_entry->set_if_name(interface.name);
            new_entry->set_if_description(interface.description);
        }
        return grpc::Status::OK;
    }

    grpc::Status run_capture(::grpc::ServerContext* context, const run_cap_request* request, run_cap_response* response) override
    {
        std::cout << "[NewRPC]run_capture()" << std::endl;
        std::string if_name = request->if_name();
        int pkt_cnt = request->pkt_count();
        int k = request->k();
        algo_obj = new heavy_keeper(3, 1200, 1.08, k);
        bool is_started = false;
        if (!capture_running)
        {
            test_for_join();
            is_started = monitor_pkt_on_if_async(if_name.c_str(), pkt_cnt, algo_obj, capture_thread, []() -> void {
                topk_result = algo_obj->query();
                last_result_ready = true;
                capture_running = false;
            });
            if (is_started)
            {
                last_result_ready = false;
                capture_running = true;
            }
        }
        response->set_protocol_version(RPC_PROTOCOL_VER);
        response->set_is_started(is_started);
        return grpc::Status::OK;
    }

    grpc::Status get_cap_status(::grpc::ServerContext* context, const empty_request* request, get_cap_status_response* response) override
    {
        std::cout << "[NewRPC]get_cap_status()" << std::endl;
        test_for_join();
        response->set_protocol_version(RPC_PROTOCOL_VER);
        response->set_is_finished(last_result_ready);
        response->set_captured_pkt_count(algo_obj->get_current_count());
        return grpc::Status::OK;
    }

    grpc::Status get_topk_result(::grpc::ServerContext* context, const empty_request* request, get_topk_result_response* response) override
    {
        std::cout << "[NewRPC]get_topk_result()" << std::endl;
        test_for_join();
        response->set_protocol_version(RPC_PROTOCOL_VER);
        response->set_is_finished(last_result_ready);
        for (int i = 0; i < topk_result.size(); i++)
        {
            get_topk_result_response::topk_result_entry *entry = response->add_topk_results();
            entry->set_id(i + 1);
            entry->set_flow_description(topk_result[i].first.print_detail());
            entry->set_flow_count(topk_result[i].second);
        }
        return grpc::Status::OK;
    }
};

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        if (!strncmp(argv[1], "-b", 2))
        {
            std::vector<flow_id> packets;
            read_packets(packets, getCurrentRSS());
            std::cout << std::endl;
            benchmarking(packets);
        }
        else if (!strncmp(argv[1], "-i", 2))
        {
            heavy_keeper algo_obj(3, 1200, 1.08, 100);
            monitor_live(MAX_READ_PKT, &algo_obj);
            print_topk(algo_obj.query());
        }
        else if (!strncmp(argv[1], "-s", 2))
        {
            run_grpc_server();
        }
        else if (!strncmp(argv[1], "-h", 2))
        {
            std::cout << "Usage: topk [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  -b     Benchmark mode." << std::endl;
            std::cout << "  -i     Interactive mode." << std::endl;
            std::cout << "  -s     Run the program as a gRPC server." << std::endl;
            std::cout << "  -h     Print this message." << std::endl;
            std::cout << "" << std::endl;
            std::cout << "If the program doesn't run properly, try to run it again with 'sudo'." << std::endl;
        }
        else 
        {
            std::cout << "Usage: topk [options]" << std::endl;
        }
    }
    return 0;
}

void run_grpc_server()
{
    std::string server_address("0.0.0.0:50051");
    tele_service_impl service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}

void test_for_join()
{
    if (!capture_thread)
    {
        return;
    }
    if (capture_thread->joinable())
    {
        capture_thread->join();
        delete capture_thread;
        capture_thread = nullptr;
        delete algo_obj;
        algo_obj = nullptr;
        capture_running = false;
    }
}