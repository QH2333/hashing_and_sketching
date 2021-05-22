/**
 * @file benchmark.h
 * @author QH2333 (qi_an_hao@126.com)
 * @brief 
 * @version 0.1
 * @date 2021-04-26
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <utility>
#include <cstdio>
#include <cstring>
#include <string>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <functional>
#include <chrono>
#include <random>

#include "flow_id.h"
#include "algo_performance.h"
#include "topk_algorithms.h"
#include "bench_adapter.h"
#include "common/get_memstat.h"

bool read_packets(std::vector<flow_id> &packets, const size_t rss_before_invoke);

bool insert_packets(const std::vector<flow_id> &packets, topk_algo_base &algo_obj, int cnt, algo_performance_t &performance);

std::vector<std::pair<flow_id, int>> query_topk(topk_algo_base &algo_obj, algo_performance_t &performance);

void print_topk(const std::vector<std::pair<flow_id, int>> &topk_result);

void print_performance(std::ostream &writer, algo_performance_t performance);

std::vector<std::pair<flow_id, int>> calc_answer(const std::vector<flow_id> &packets, topk_algo_base &ans_obj, int cnt);

bool calc_metrics(const std::vector<std::pair<flow_id, int>> &result, const std::vector<std::pair<flow_id, int>> &ans, topk_algo_base &ans_obj, algo_performance_t &performance);

bool benchmarking(const std::vector<flow_id> &packets);

