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

#include "top_k.h"
#include "flow_id.h"
#include "topk_algorithms.h"
#include "../common/get_memstat.h"

// === Control the program's behavior here ===
constexpr bool VERBOSE = true;
// constexpr bool CONCISE_INFO = true;
// === End of behavior control section ===

bool read_packets(std::vector<flow_id> &packets, int &pkt_count, const size_t rss_before_invoke);
bool insert_packets(topk_algo_base &algo_obj, std::vector<flow_id> &packets, int &pkt_count, const size_t rss_before_invoke);
std::vector<std::pair<flow_id, int>> query_topk(topk_algo_base &algo_obj, const size_t rss_before_invoke);

int main()
{
    std::vector<flow_id> packets;
    int pkt_count = 0;

    read_packets(packets, pkt_count, getCurrentRSS());
    std::cout << std::endl;

    exact_algo algo_obj(K);
    // count_min_heap algo_obj(10, 100, K);
    insert_packets(algo_obj, packets, pkt_count, getCurrentRSS());
    std::cout << std::endl;

    auto top_k_result = query_topk(algo_obj, getCurrentRSS());
    std::cout << std::endl;

    for (auto iter = top_k_result.begin(); iter != top_k_result.end(); iter++)
    {
        if (VERBOSE) std::cout << "[" << std::setw(5) << iter->second << "]" << iter->first.print_detail() << std::endl;
    }
    return 0;
}


bool read_packets(std::vector<flow_id> &packets, int &pkt_count, const size_t rss_before_invoke)
{
    FILE* fp = fopen(PARSED_FILE, "rb");
    std::cout << "===== Reading from file =====" << std::endl;
    char flow_id_buf[13];
    auto start = std::chrono::steady_clock::now();
    // ==========
    while (fread(flow_id_buf, 13, 1, fp))
    {
        packets.push_back(flow_id(flow_id_buf));
        pkt_count++;
    }
    if (!feof(fp))
    {
        perror("Error: ");
        fclose(fp);
        exit(-1);
    }
    // ==========
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << "Packet count: " << pkt_count << "\n";
    std::cout << "Time elapsed: " << elapsed_seconds.count() << " s\n";
    std::cout << "Throughput: " << pkt_count / elapsed_seconds.count() << " pkt/s\n";
    std::cout << "Packet memory occupation: " << (getCurrentRSS() - rss_before_invoke) / 1024 << "KB" << std::endl;
    std::cout << "===== Reading finished =====" << std::endl;
    fclose(fp);
    return true;
}

bool insert_packets(topk_algo_base &algo_obj, std::vector<flow_id> &packets, int &pkt_count, const size_t rss_before_invoke)
{
    std::cout << "===== Start inserting =====" << std::endl;
    auto start = std::chrono::steady_clock::now();
    // ==========
    for (auto pkt_it = packets.begin(); pkt_it != packets.end(); pkt_it++)
    {
        algo_obj.insert(*pkt_it);
    }
    // ==========
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "Time elapsed: " << elapsed_seconds.count() << " s\n";
    std::cout << "Throughput: " << pkt_count / elapsed_seconds.count() << " pkt/s\n";
    std::cout << "Algorithm memory occupation: " << (getCurrentRSS() - rss_before_invoke) / 1024 << "KB" << std::endl;
    std::cout << "===== Insertion finished =====" << std::endl;
    return true;
}

std::vector<std::pair<flow_id, int>> query_topk(topk_algo_base &algo_obj, const size_t rss_before_invoke)
{
    std::cout << "===== Start querying =====" << std::endl;
    auto start = std::chrono::steady_clock::now();
    // ==========
    std::vector<std::pair<flow_id, int>> top_k_result = algo_obj.query();
    // std::vector<std::pair<flow_id, int>> top_k_result = ((exact_algo&)algo_obj).query_ss();
    // ==========
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "Time elapsed: " << elapsed_seconds.count() << " s\n";
    std::cout << "Query memory occupation: " << (getCurrentRSS() - rss_before_invoke) / 1024 << "KB" << std::endl;
    std::cout << "===== Query finished =====" << std::endl;
    return top_k_result;
}