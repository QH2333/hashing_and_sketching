/**
 * @file topk_algorithms.h
 * @author QH2333 (qi_an_hao@126.com)
 * @brief 
 * @version 0.1
 * @date 2021-04-03
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include "top_k.h"
#include "flow_id.h"
#include "stream_summary.cpp"

/**
 * @brief This is the abstract base class for all top-k algorithms.
 *        This class requires that all algorithms should at least implement the following functions:
 *        insert()
 *        query()
 */
class topk_algo_base
{
public:
    topk_algo_base(){}
    ~topk_algo_base(){}

public:
    virtual bool insert(const char *flow_id_buf) = 0;
    virtual bool insert(const flow_id flow_id_obj) = 0;
    virtual std::vector<std::pair<flow_id, int>> query() = 0;
    virtual const size_t get_byte_size() = 0;
};

/**
 * @brief This class implements the naive precise version of the top-k algorithm
 *        The algorithm is based on a hash map and a heap
 */
class exact_algo: public topk_algo_base
{
private:
    int k;
    std::unordered_map<flow_id, int> hash_table;

public:
    exact_algo(int _k = K) : k(_k) { }

public:
    bool insert(const char *flow_id_buf);
    bool insert(const flow_id flow_id_obj);
    std::vector<std::pair<flow_id, int>> query();
    std::vector<std::pair<flow_id, int>> query_ss();
    const int get_bucket_count() { return hash_table.bucket_count(); }
    const float get_load_factor() { return hash_table.load_factor(); }
    const float get_max_load_factor() { return hash_table.max_load_factor(); }
    const size_t get_byte_size() { return hash_table.size() * sizeof(std::pair<flow_id, int>); };
};

/**
 * @brief This class implements the Count-Min + Stream-summary algorithm for identifying top-k elements
 * 
 */
class count_min_heap: public topk_algo_base
{
private:
    int k;
    int d; // d independent hash functions, d arrays
    int m; // m buckets in an array
    uint32_t *seeds;
    int **sketch;
    stream_summary *ss;
    size_t curr_mem_byte_without_ss = sizeof(count_min_heap);

public:
    count_min_heap(int _d, int _m, int _k = K)
        : k(_k), d(_d), m(_m)
    {
        std::random_device rd;
        seeds = new uint32_t[d];
        curr_mem_byte_without_ss += d * sizeof(uint32_t);
        sketch = new int *[d];
        for (int i = 0; i < d; i++)
        {
            seeds[i] = uint32_t(rd());
            sketch[i] = new int[m];
            for (int j = 0; j < m; j++)
            {
                sketch[i][j] = 0;
            }
        }
        curr_mem_byte_without_ss += d * m * sizeof(int);
        ss = new stream_summary(k);
    }

    ~count_min_heap()
    {
        for (int i = 0; i < d; i++)
        {
            delete[] sketch[i];
        }
        delete[] sketch;
        delete ss;
    }

public:
    bool insert(const char *flow_id_buf);
    bool insert(const flow_id flow_id_obj);
    std::vector<std::pair<flow_id, int>> query();
    const size_t get_byte_size() { return curr_mem_byte_without_ss + ss->get_byte_size(); };
};
