/**
 * @file top_k.h
 * @author QH2333 (qi_an_hao@126.com)
 * @brief 
 * @version 0.1
 * @date 2021-04-03
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

// C/CPP standard library
#include <utility>
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

// C POSIX library
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

// Project headers
#include "../common/common.h"
#include "../common/lookup3.h"

typedef struct algo_performance_t // Record the performance in a single run
{
    float ins_time = 0;
    float ins_throughput = 0;
    float ins_mem = 0;
    float query_time = 0;
    float AAE = 0;
    float ARE = 0;
    float precision = 0;
    float recall = 0;
    float F1 = 0;
} algo_performance_t;

typedef struct algo_performance_stat_t // Record the performance in all rounds and calculate the average data
{
    std::string algo_detail;
    float ins_time = 0;
    float ins_throughput = 0;
    float ins_mem = 0;
    float query_time = 0;
    float AAE = 0;
    float ARE = 0;
    float precision = 0;
    float recall = 0;
    float F1 = 0;

    int count = 0;

    void put(algo_performance_t new_data)
    {
        ins_time += new_data.ins_time;
        ins_throughput += new_data.ins_throughput;
        ins_mem += new_data.ins_mem;
        query_time += new_data.query_time;
        AAE += new_data.AAE;
        ARE += new_data.ARE;
        precision += new_data.precision;
        recall += new_data.recall;
        F1 += new_data.F1;
        count++;
    }

    algo_performance_t get()
    {
        return (algo_performance_t) {
            ins_time / count, 
            ins_throughput / count, 
            ins_mem / count, 
            query_time / count, 
            AAE / count, 
            ARE / count, 
            precision / count, 
            recall / count, 
            F1 / count
        };
    }
} algo_performance_stat_t;