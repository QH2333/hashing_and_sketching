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
#define TO_STR_(x) #x
#define TO_STR(x) TO_STR_(x)

// === Control the program's behavior here ===
constexpr bool VERBOSE = true;
constexpr int MAX_READ_PKT = 30000000;
constexpr int REPEAT_CNT = 20;
// #define ALGORITHM_TO_BENCH exact_algo
// #define ALGORITHM_PARAMETER (K)

#define ALGORITHM_TO_BENCH count_min_heap
#define ALGORITHM_PARAMETER (10, 150, K)

// constexpr bool CONCISE_INFO = true;
// === End of behavior control section ===

bool read_packets(std::vector<flow_id> &packets, const size_t rss_before_invoke);
std::vector<std::pair<flow_id, int>> calc_answer(std::vector<flow_id> &packets);
bool benchmarking(std::vector<flow_id> &packets);
bool insert_packets(topk_algo_base &algo_obj, std::vector<flow_id> &packets, const size_t rss_before_invoke);
bool calc_metrics(const std::vector<std::pair<flow_id, int>> &topk_result, const std::vector<std::pair<flow_id, int>> &ans);
std::vector<std::pair<flow_id, int>> query_topk(topk_algo_base &algo_obj, const size_t rss_before_invoke);
void print_topk(const std::vector<std::pair<flow_id, int>> &topk_result);

int main()
{
    std::vector<flow_id> packets;

    read_packets(packets, getCurrentRSS());
    std::cout << std::endl;

    benchmarking(packets);
    return 0;
}

bool benchmarking(std::vector<flow_id> &packets)
{
    std::cout << "===== Benchmarking =====" << std::endl;
    std::cout << "Algorithm           Round     Ins.Time (s)   Ins.Thp. (pkt/s)    Ins.Mem (KB)   Querying Time (s)   Precision      Recall         F1" << std::endl;

    std::vector<std::pair<flow_id, int>> topk_ans = calc_answer(packets);

    size_t rss_before_insertion = getCurrentRSS();
    for (int i = 0; i < REPEAT_CNT; i++)
    {
        std::cout << std::left << std::setw(20) << TO_STR(ALGORITHM_TO_BENCH) << std::setw(10) << i << std::flush;
        ALGORITHM_TO_BENCH *algo_obj = new ALGORITHM_TO_BENCH ALGORITHM_PARAMETER;
        insert_packets(*algo_obj, packets, rss_before_insertion);

        size_t rss_before_query = getCurrentRSS();
        auto topk_result = query_topk(*algo_obj, rss_before_query);
        calc_metrics(topk_result, topk_ans);

        std::cout << std::endl;
        delete algo_obj;
        // print_topk(topk_result);
    }
    std::cout << "===== Benchmark finished =====" << std::endl;
}

std::vector<std::pair<flow_id, int>> calc_answer(std::vector<flow_id> &packets)
{
    size_t rss_before_insertion = getCurrentRSS();
    std::cout << std::left << std::setw(20) << "answer" << std::setw(10) << "-" << std::flush;
    exact_algo *answer_obj = new exact_algo(K);
    insert_packets(*answer_obj, packets, rss_before_insertion);
    size_t rss_before_query = getCurrentRSS();
    auto topk_ans = query_topk(*answer_obj, rss_before_query);
    calc_metrics(topk_ans, topk_ans);
    std::cout << std::endl;
    delete answer_obj;
    return topk_ans;
}

bool read_packets(std::vector<flow_id> &packets, const size_t rss_before_invoke)
{
    int pkt_count = 0;
    FILE *fp = fopen(PARSED_FILE, "rb");
    std::cout << "===== Reading from file =====" << std::endl;
    char flow_id_buf[13];
    auto start = std::chrono::steady_clock::now();
    // ==========
    while (fread(flow_id_buf, 13, 1, fp))
    {
        packets.push_back(flow_id(flow_id_buf));
        pkt_count++;
        if (MAX_READ_PKT == pkt_count)
            break;
    }
    if (MAX_READ_PKT != pkt_count && !feof(fp))
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

bool insert_packets(topk_algo_base &algo_obj, std::vector<flow_id> &packets, const size_t rss_before_invoke)
{
    auto start = std::chrono::steady_clock::now();
    // ==========
    for (auto pkt_it = packets.begin(); pkt_it != packets.end(); pkt_it++)
    {
        algo_obj.insert(*pkt_it);
    }
    // ==========
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << std::left << std::setw(15) << elapsed_seconds.count();
    std::cout << std::left << std::setw(20) << packets.size() / elapsed_seconds.count();
    // std::cout << std::left << std::setw(15) << (getCurrentRSS() - rss_before_invoke) / 1024 << std::flush;
    std::cout << std::left << std::setw(15) << algo_obj.get_byte_size() / 1024.0 << std::flush;
    return true;
}

std::vector<std::pair<flow_id, int>> query_topk(topk_algo_base &algo_obj, const size_t rss_before_invoke)
{
    auto start = std::chrono::steady_clock::now();
    // ==========
    std::vector<std::pair<flow_id, int>> top_k_result = algo_obj.query();
    // std::vector<std::pair<flow_id, int>> top_k_result = ((exact_algo&)algo_obj).query_ss();
    // ==========
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << std::left << std::setw(20) << elapsed_seconds.count() << std::flush;
    return top_k_result;
}

bool calc_metrics(const std::vector<std::pair<flow_id, int>> &topk_result, const std::vector<std::pair<flow_id, int>> &ans)
{
    std::unordered_map<flow_id, int> hashed_result(topk_result.begin(), topk_result.end());
    int precision_cnt = 0;
    for (auto iter = ans.begin(); iter != ans.end(); iter++)
    {
        if (hashed_result.find(iter->first) != hashed_result.end())
            precision_cnt++;
    }
    float precision = precision_cnt / (float)topk_result.size();
    
    std::unordered_map<flow_id, int> hashed_ans(ans.begin(), ans.end());
    int recall_cnt = 0;
    for (auto iter = topk_result.begin(); iter != topk_result.end(); iter++)
    {
        if (hashed_ans.find(iter->first) != hashed_ans.end())
            recall_cnt++;
    }
    float recall = recall_cnt / (float)ans.size();

    float f1 = 2 * precision * recall / (precision + recall);

    std::cout << std::left << std::setprecision(5) << std::setw(15) << precision;
    std::cout << std::left << std::setprecision(5) << std::setw(15) << recall;
    std::cout << std::left << std::setprecision(5) << std::setw(15) << f1 << std::flush;
}

void print_topk(const std::vector<std::pair<flow_id, int>> &topk_result)
{
    for (auto iter = topk_result.begin(); iter != topk_result.end(); iter++)
    {
        std::cout << "[" << std::setw(5) << iter->second << "]" << iter->first.print_detail() << std::endl;
    }
}
