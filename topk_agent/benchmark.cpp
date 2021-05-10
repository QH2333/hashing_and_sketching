/**
 * @file benchmark.cpp
 * @author QH2333 (qi_an_hao@126.com)
 * @brief 
 * @version 0.1
 * @date 2021-04-26
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "benchmark.h"

bool benchmarking(const std::vector<flow_id> &packets)
{
    std::ofstream output_file("bench_output");
    std::cout << "===== Benchmarking =====" << std::endl;
    std::string result_header =
        "Algorithm      "           // 15
        "Parameter                " // 25
        "Round     "                // 10
        "Ins.Time(s)    "           // 15
        "Ins.Thp.(p/s)  "           // 15
        "Ins.Mem.(KB)   "           // 15
        "Query Time(s)  "           // 15
        "AAE       "                // 10
        "ARE            "           // 15
        "Precision      "           // 15
        "Recall         "           // 15
        "F1";
    std::cout << result_header << std::endl;
    output_file << result_header << std::endl;

    exact_algo topk_ans_obj(K);
    std::vector<std::pair<flow_id, int>> topk_ans = calc_answer(packets, topk_ans_obj);
    std::cout << std::endl;

    topk_algo_base *algo_obj;
    bench_adapter adapter;
    while (adapter.read_next_algo())
    {
        algo_performance_stat_t performance_stat;
        for (int i = 0; i < REPEAT_CNT; i++)
        {
            // ALGORITHM_TO_BENCH *algo_obj = new ALGORITHM_TO_BENCH ALGORITHM_PARAMETER;
            // std::cout << std::left << std::setw(15) << TO_STR(ALGORITHM_TO_BENCH);
            algo_obj = adapter.get_bench_algo();
            algo_performance_t performance_one_run;
            std::stringstream algo_detail;
            algo_detail << std::left << std::setw(15) << algo_obj->get_algo_name();
            algo_detail << std::left << std::setw(25) << algo_obj->get_parameter();
            performance_stat.algo_detail = algo_detail.str();
            std::cout << std::left << performance_stat.algo_detail;
            std::cout << std::left << std::setw(10) << i << std::flush;

            insert_packets(packets, *algo_obj, performance_one_run);
            auto topk_result = query_topk(*algo_obj, performance_one_run);
            calc_metrics(topk_result, topk_ans, topk_ans_obj, performance_one_run);

            print_performance(std::cout, performance_one_run);
            std::cout << std::endl;
            performance_stat.put(performance_one_run);
            delete algo_obj;
        }
        output_file << std::left << std::setw(15) << performance_stat.algo_detail;
        output_file << std::left << std::setw(10) << performance_stat.count;
        algo_performance_t average_performance = performance_stat.get();
        print_performance(output_file, average_performance);
        output_file << std::endl;
        std::cout << std::endl;
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

bool insert_packets(const std::vector<flow_id> &packets, topk_algo_base &algo_obj, algo_performance_t &performance)
{
    auto start = std::chrono::steady_clock::now();
    // ==========
    for (auto pkt_it = packets.begin(); pkt_it != packets.end(); pkt_it++)
    {
        algo_obj.insert(*pkt_it);
    }
    if (algo_obj.get_algo_name() == "HK_parallel")
    {
        ((heavy_keeper_parallel *)&algo_obj)->join_all();
    }
    // ==========
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    performance.ins_time = elapsed_seconds.count();
    performance.ins_throughput = packets.size() / elapsed_seconds.count();
    performance.ins_mem = algo_obj.get_byte_size() / 1024.0;
    return true;
}

std::vector<std::pair<flow_id, int>> query_topk(topk_algo_base &algo_obj, algo_performance_t &performance)
{
    auto start = std::chrono::steady_clock::now();
    // ==========
    std::vector<std::pair<flow_id, int>> topk_result = algo_obj.query();
    // ==========
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    performance.query_time = elapsed_seconds.count();
    return topk_result;
}

void print_topk(const std::vector<std::pair<flow_id, int>> &topk_result)
{
    for (auto iter = topk_result.begin(); iter != topk_result.end(); iter++)
    {
        std::cout << "[" << std::setw(5) << iter->second << "]" << iter->first.to_string() << std::endl;
    }
}

void print_performance(std::ostream &writer, algo_performance_t performance)
{
    writer << std::left << std::setw(15) << performance.ins_time;
    writer << std::left << std::setw(15) << performance.ins_throughput;
    writer << std::left << std::setw(15) << performance.ins_mem;
    writer << std::left << std::setw(15) << performance.query_time;
    writer << std::left << std::setprecision(4) << std::setw(10) << performance.AAE;
    writer << std::left << std::setprecision(4) << std::setw(15) << performance.ARE;
    writer << std::left << std::setprecision(5) << std::setw(15) << performance.precision;
    writer << std::left << std::setprecision(5) << std::setw(15) << performance.recall;
    writer << std::left << std::setprecision(5) << std::setw(15) << performance.F1;
}

std::vector<std::pair<flow_id, int>> calc_answer(const std::vector<flow_id> &packets, topk_algo_base &ans_obj)
{
    algo_performance_t ans_performance;
    std::cout << std::left << std::setw(15) << "answer";
    std::cout << std::left << std::setw(25) << ans_obj.get_parameter() << std::flush;
    std::cout << std::left << std::setw(10) << "-" << std::flush;
    insert_packets(packets, ans_obj, ans_performance);

    auto topk_ans = query_topk(ans_obj, ans_performance);
    calc_metrics(topk_ans, topk_ans, ans_obj, ans_performance);

    print_performance(std::cout, ans_performance);
    std::cout << std::endl;
    return topk_ans;
}

bool calc_metrics(const std::vector<std::pair<flow_id, int>> &result, const std::vector<std::pair<flow_id, int>> &ans, topk_algo_base &ans_obj, algo_performance_t &performance)
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
    float F1 = 2 * precision * recall / (precision + recall);

    performance.AAE = AAE;
    performance.ARE = ARE;
    performance.precision = precision;
    performance.recall = recall;
    performance.F1 = F1;
    return true;
}