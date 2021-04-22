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

// #define ALGORITHM_TO_BENCH exact_algo
// #define ALGORITHM_PARAMETER (K)

// #define ALGORITHM_TO_BENCH count_min_heap
// #define ALGORITHM_PARAMETER (3, 2800, K)

#define ALGORITHM_TO_BENCH heavy_keeper
#define ALGORITHM_PARAMETER (2, 2500, 1.08, K)

// === End of behavior control section ===

bool
read_packets(std::vector<flow_id> &packets, const size_t rss_before_invoke);

bool
insert_packets(const std::vector<flow_id> &packets, topk_algo_base &algo_obj, const size_t rss_before_invoke);

std::vector<std::pair<flow_id, int>>
query_topk(topk_algo_base &algo_obj);

void
print_topk(const std::vector<std::pair<flow_id, int>> &topk_result);

std::vector<std::pair<flow_id, int>>
calc_answer(const std::vector<flow_id> &packets, topk_algo_base &ans_obj);

bool
calc_metrics(const std::vector<std::pair<flow_id, int>> &result, const std::vector<std::pair<flow_id, int>> &ans, topk_algo_base &ans_obj);

bool
benchmarking(const std::vector<flow_id> &packets);

int main()
{
    std::vector<flow_id> packets;

    read_packets(packets, getCurrentRSS());
    std::cout << std::endl;

    benchmarking(packets);
    return 0;
}

bool benchmarking(const std::vector<flow_id> &packets)
{
    std::cout << "===== Benchmarking =====" << std::endl;
    std::cout <<
        "Algorithm           " // 20
        "Round     "           // 10
        "Ins.Time(s)    "      // 15
        "Ins.Thp.(pkt/s)     " // 20
        "Ins.Mem.(KB)   "      // 15
        "Query Time(s)  "      // 15
        "AAE       "           // 10
        "ARE            "      // 15
        "Precision      "      // 15
        "Recall         "      // 15
        "F1" << std::endl;

    exact_algo topk_ans_obj(K);
    std::vector<std::pair<flow_id, int>> topk_ans = calc_answer(packets, topk_ans_obj);

    size_t rss_before_insertion = getCurrentRSS();
    for (int i = 0; i < REPEAT_CNT; i++)
    {
        std::cout << std::left << std::setw(20) << TO_STR(ALGORITHM_TO_BENCH) << std::setw(10) << i << std::flush;
        ALGORITHM_TO_BENCH *algo_obj = new ALGORITHM_TO_BENCH ALGORITHM_PARAMETER;
        insert_packets(packets, *algo_obj, rss_before_insertion);

        auto topk_result = query_topk(*algo_obj);
        calc_metrics(topk_result, topk_ans, topk_ans_obj);

        std::cout << std::endl;
        delete algo_obj;
    }
    std::cout << "===== Benchmark finished =====" << std::endl;
    return true;
}

bool read_packets(std::vector<flow_id> &packets, const size_t rss_before_invoke)
{
    int pkt_count = 0;
    FILE *fp = fopen(PARSED_FILE, "rb");
    std::cout << "===== Reading from file =====" << std::endl;
    uint8_t flow_id_buf[13];
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

bool insert_packets(const std::vector<flow_id> &packets, topk_algo_base &algo_obj, const size_t rss_before_invoke)
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
    std::cout << std::left << std::setw(15) << algo_obj.get_byte_size() / 1024.0 << std::flush;
    return true;
}

std::vector<std::pair<flow_id, int>> query_topk(topk_algo_base &algo_obj)
{
    auto start = std::chrono::steady_clock::now();
    // ==========
    std::vector<std::pair<flow_id, int>> topk_result = algo_obj.query();
    // ==========
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << std::left << std::setw(15) << elapsed_seconds.count() << std::flush;
    return topk_result;
}

void print_topk(const std::vector<std::pair<flow_id, int>> &topk_result)
{
    for (auto iter = topk_result.begin(); iter != topk_result.end(); iter++)
    {
        std::cout << "[" << std::setw(5) << iter->second << "]" << iter->first.print_detail() << std::endl;
    }
}

std::vector<std::pair<flow_id, int>> calc_answer(const std::vector<flow_id> &packets, topk_algo_base &ans_obj)
{
    size_t rss_before_insertion = getCurrentRSS();
    std::cout << std::left << std::setw(20) << "answer" << std::setw(10) << "-" << std::flush;
    insert_packets(packets, ans_obj, rss_before_insertion);

    auto topk_ans = query_topk(ans_obj);
    calc_metrics(topk_ans, topk_ans, ans_obj);

    std::cout << std::endl;
    return topk_ans;
}

bool calc_metrics(const std::vector<std::pair<flow_id, int>> &result, const std::vector<std::pair<flow_id, int>> &ans, topk_algo_base &ans_obj)
{
    // AAE & ARE
    uint64_t absolute_error_cnt = 0;
    float relative_error_cnt = 0;
    for (auto iter = result.begin(); iter != result.end(); iter++)
    {
        int err = abs(iter->second - ans_obj.query_item(iter->first));
        absolute_error_cnt += err;
        relative_error_cnt += (float)err / ans_obj.query_item(iter->first);
    }
    float AAE = (float)absolute_error_cnt / result.size();
    float ARE = (float)relative_error_cnt / result.size();

    // Precision = TP / (TP + FP)
    std::unordered_map<flow_id, int> hashed_ans(ans.begin(), ans.end());
    int precision_cnt = 0;
    for (auto iter = result.begin(); iter != result.end(); iter++)
    {
        if (hashed_ans.find(iter->first) != hashed_ans.end()) // How many items in the result are true top-ks?
            precision_cnt++;
    }
    float precision = (float)precision_cnt / result.size();
    
    // Recall = TP / (TP + FN)
    std::unordered_map<flow_id, int> hashed_result(result.begin(), result.end());
    int recall_cnt = 0;
    for (auto iter = ans.begin(); iter != ans.end(); iter++)
    {
        if (hashed_result.find(iter->first) != hashed_result.end()) // How many true top-ks are identified?
            recall_cnt++;
    }
    float recall = (float)recall_cnt / ans.size();

    // F-1 measure
    float f1 = 2 * precision * recall / (precision + recall);

    std::cout << std::left << std::setprecision(5) << std::setw(10) << AAE;
    std::cout << std::left << std::setprecision(5) << std::setw(15) << ARE;
    std::cout << std::left << std::setprecision(5) << std::setw(15) << precision;
    std::cout << std::left << std::setprecision(5) << std::setw(15) << recall;
    std::cout << std::left << std::setprecision(5) << std::setw(15) << f1 << std::flush;
    return true;
}
