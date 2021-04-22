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

#include "memory_tracker.h"
#include "top_k.h"
#include "flow_id.h"
#include "stream_summary.cpp"

/**
 * @brief This is the abstract base class for all top-k algorithms.
 *        This class requires that all algorithms should at least implement the following functions:
 *        insert()
 *        query()
 *        get_byte_size()
 */
class topk_algo_base
{
public:
    topk_algo_base(){}
    ~topk_algo_base(){}

public:
    virtual bool insert(const uint8_t *flow_id_buf) = 0;
    virtual bool insert(const flow_id flow_id_obj) = 0;
    virtual std::vector<std::pair<flow_id, int>> query() = 0;
    virtual int query_item(const flow_id key) = 0;
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
    std::unordered_map<flow_id, int, std::hash<flow_id>, std::equal_to<flow_id>, allocator_mt<std::pair<flow_id, int>>> hash_table;

public:
    exact_algo(int _k = K) : k(_k) { }

public:
    bool insert(const uint8_t *flow_id_buf);
    bool insert(const flow_id flow_id_obj);
    std::vector<std::pair<flow_id, int>> query();
    int query_item(const flow_id key);
    const size_t get_byte_size() { return hash_table.get_allocator().get_allocated_mem() + sizeof(exact_algo); };

public:
    std::vector<std::pair<flow_id, int>> query_ss(); // Slower than query()
    const int get_bucket_count() { return hash_table.bucket_count(); }
    const float get_load_factor() { return hash_table.load_factor(); }
    const float get_max_load_factor() { return hash_table.max_load_factor(); }
};

/**
 * @brief This class implements the Count-Min + Stream-summary algorithm for identifying top-k elements
 * TODO: change the memory tracker to allcator_mt
 * TODO: implement the query_item function
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
        : d(_d), m(_m), k(_k)
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
    bool insert(const uint8_t *flow_id_buf);
    bool insert(const flow_id flow_id_obj);
    std::vector<std::pair<flow_id, int>> query();
    int query_item(const flow_id key) { return 0; };
    const size_t get_byte_size() { return curr_mem_byte_without_ss + ss->get_byte_size(); };
};


class heavy_keeper: public topk_algo_base
{
private:
    int k;
    int d; // d independent hash functions, d arrays
    int w; // w buckets in an array
    float b;
    uint32_t *seeds;
    std::pair<flow_id, int> *hk; // Main data structure of HeavyKeeper
    stream_summary *ss;
    allocator_mt<flow_id> fi_allocator;
    allocator_mt<flow_id>::rebind<std::pair<flow_id, int>>::other kv_allocator = allocator_mt<flow_id>::rebind<std::pair<flow_id, int>>::other(fi_allocator);
    allocator_mt<flow_id>::rebind<uint32_t>::other seed_allocator = allocator_mt<flow_id>::rebind<uint32_t>::other(fi_allocator);
    allocator_mt<flow_id>::rebind<stream_summary>::other ss_allocator = allocator_mt<flow_id>::rebind<stream_summary>::other(fi_allocator);
    std::equal_to<flow_id> is_equal;
    std::random_device rd;

public:
    heavy_keeper(int _d, int _w, float _b, int _k = K)
        : d(_d), w(_w), b(_b), k(_k)
    {

        seeds = seed_allocator.allocate(d);
        hk = kv_allocator.allocate(d * w);
        flow_id null_flow;
        for (int i = 0; i < d; i++)
        {
            seeds[i] = uint32_t(rd());
        }
        for (int i = 0; i < d * w; i++)
        {
            hk[i] = std::make_pair(null_flow, 0);
        }
        ss = ss_allocator.allocate(1);
        ss_allocator.construct(ss, k);
    }

    ~heavy_keeper()
    {
        kv_allocator.deallocate(hk, d * w);
        ss_allocator.destroy(ss);
        ss_allocator.deallocate(ss, 1);
    }

public:
    bool insert(const uint8_t *flow_id_buf);
    bool insert(const flow_id flow_id_obj);
    std::vector<std::pair<flow_id, int>> query();
    int query_item(const flow_id key);
    const size_t get_byte_size() { return fi_allocator.get_allocated_mem() + ss->get_byte_size(); };

public:
    bool insert_basic_ver(const flow_id flow_id_obj);
};