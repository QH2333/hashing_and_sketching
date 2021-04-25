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
#include "stream_summary.h"

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
    virtual const std::string get_parameter() = 0;
    virtual const std::string get_algo_name() = 0;
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
    exact_algo(int _k = 100) : k(_k) { }

public:
    bool insert(const uint8_t *flow_id_buf) { flow_id flow_id_obj(flow_id_buf); return insert(flow_id_obj); }
    bool insert(const flow_id flow_id_obj);
    std::vector<std::pair<flow_id, int>> query();
    int query_item(const flow_id key);
    const size_t get_byte_size() { return hash_table.get_allocator().get_allocated_mem() + sizeof(exact_algo); };
    const std::string get_parameter();
    const std::string get_algo_name() { return std::string("exact_algo"); };

public:
    std::vector<std::pair<flow_id, int>> query_ss(); // Slower than query()
    const int get_bucket_count() { return hash_table.bucket_count(); }
    const float get_load_factor() { return hash_table.load_factor(); }
    const float get_max_load_factor() { return hash_table.max_load_factor(); }
};

/**
 * @brief This class implements the Count-Min + Stream-summary algorithm for identifying top-k elements
 */
class count_min_heap: public topk_algo_base
{
private:
    int k;
    int d; // d independent hash functions, d arrays
    int m; // m buckets in an array
    uint32_t *seeds;
    int *sketch;
    stream_summary *ss;
    allocator_mt<flow_id> fi_allocator;
    allocator_mt<flow_id>::rebind<int>::other int_allocator = allocator_mt<flow_id>::rebind<int>::other(fi_allocator);
    allocator_mt<flow_id>::rebind<uint32_t>::other seed_allocator = allocator_mt<flow_id>::rebind<uint32_t>::other(fi_allocator);
    allocator_mt<flow_id>::rebind<stream_summary>::other ss_allocator = allocator_mt<flow_id>::rebind<stream_summary>::other(fi_allocator);

public:
    count_min_heap(int _d, int _m, int _k = 100)
        : d(_d), m(_m), k(_k)
    {
        std::random_device rd;
        seeds = seed_allocator.allocate(d);
        sketch = int_allocator.allocate(d * m);
        for (int i = 0; i < d; i++)
        {
            seeds[i] = uint32_t(rd());
        }
        for (int i = 0; i < d * m; i++)
        {
            sketch[i] = 0;
        }
        ss = ss_allocator.allocate(1);
        ss_allocator.construct(ss, k);
    }

    ~count_min_heap()
    {
        seed_allocator.deallocate(seeds, d);
        int_allocator.deallocate(sketch, d * m);
        ss_allocator.destroy(ss);
        ss_allocator.deallocate(ss, 1);
    }

public:
    bool insert(const uint8_t *flow_id_buf) { flow_id flow_id_obj(flow_id_buf); return insert(flow_id_obj); }
    bool insert(const flow_id flow_id_obj);
    std::vector<std::pair<flow_id, int>> query();
    int query_item(const flow_id key) { return 0; };
    const size_t get_byte_size() { return sizeof(count_min_heap) + fi_allocator.get_allocated_mem() + ss->get_byte_size(); };
    const std::string get_parameter();
    const std::string get_algo_name() { return std::string("CMS_heap"); };
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
    heavy_keeper(int _d, int _w, float _b, int _k = 100)
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
        seed_allocator.deallocate(seeds, d);
        kv_allocator.deallocate(hk, d * w);
        ss_allocator.destroy(ss);
        ss_allocator.deallocate(ss, 1);
    }

public:
    bool insert(const uint8_t *flow_id_buf) { flow_id flow_id_obj(flow_id_buf); return insert(flow_id_obj); }
    bool insert(const flow_id flow_id_obj);
    std::vector<std::pair<flow_id, int>> query();
    int query_item(const flow_id key);
    const size_t get_byte_size() { return sizeof(heavy_keeper) + fi_allocator.get_allocated_mem() + ss->get_byte_size(); };
    const std::string get_parameter();
    const std::string get_algo_name() { return std::string("heavy_keeper"); };

public:
    bool insert_basic_ver(const flow_id flow_id_obj);
};

/**
 * @brief Add a overflow table to the original HK algorithm.
 *        The extended_top_k list is 1.3x the size of original top_k, the overflow table is 0.2x the size of original top_k
 * 
 */
class heavy_keeper_opt: public topk_algo_base
{
private:
    int k;
    int d; // d independent hash functions, d arrays
    int w; // w buckets in an array
    float b;
    uint32_t *seeds;
    std::pair<flow_id, int> *hk; // Main data structure of HeavyKeeper
    stream_summary *ss;
    stream_summary *ss_overflow;
    allocator_mt<flow_id> fi_allocator;
    allocator_mt<flow_id>::rebind<std::pair<flow_id, int>>::other kv_allocator = allocator_mt<flow_id>::rebind<std::pair<flow_id, int>>::other(fi_allocator);
    allocator_mt<flow_id>::rebind<uint32_t>::other seed_allocator = allocator_mt<flow_id>::rebind<uint32_t>::other(fi_allocator);
    allocator_mt<flow_id>::rebind<stream_summary>::other ss_allocator = allocator_mt<flow_id>::rebind<stream_summary>::other(fi_allocator);
    std::equal_to<flow_id> is_equal;
    std::random_device rd;

public:
    heavy_keeper_opt(int _d, int _w, float _b, int _k = 100)
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
        ss_allocator.construct(ss, k * 1.5);
        ss_overflow = ss_allocator.allocate(1);
        ss_allocator.construct(ss_overflow, k * 0.2);
    }

    ~heavy_keeper_opt()
    {
        seed_allocator.deallocate(seeds, d);
        kv_allocator.deallocate(hk, d * w);
        ss_allocator.destroy(ss);
        ss_allocator.deallocate(ss, 1);
        ss_allocator.destroy(ss_overflow);
        ss_allocator.deallocate(ss_overflow, 1);
    }

public:
    bool insert(const uint8_t *flow_id_buf) { flow_id flow_id_obj(flow_id_buf); return insert(flow_id_obj); }
    bool insert(const flow_id flow_id_obj);
    std::vector<std::pair<flow_id, int>> query();
    int query_item(const flow_id key);
    const size_t get_byte_size() { return sizeof(heavy_keeper_opt) + fi_allocator.get_allocated_mem() + ss->get_byte_size() + ss_overflow->get_byte_size(); };
    const std::string get_parameter();
    const std::string get_algo_name() { return std::string("HK_opt"); };
};

class heavy_keeper_parallel: public topk_algo_base
{
private:
    int k;
    int d;
    int w;
    float b;
    int th_cnt;
    uint32_t dispatcher_seed;
    heavy_keeper **hk_array;
    std::thread **thread_array;
    moodycamel::BlockingReaderWriterQueue<flow_id> **queue_array;
    static std::equal_to<flow_id> is_equal;

public:
    heavy_keeper_parallel(int _d, int _w, float _b, int _th_cnt, int _k)
        : d(_d), w(_w), b(_b), th_cnt(_th_cnt), k(_k)
    {
        hk_array = new heavy_keeper *[th_cnt];
        thread_array = new std::thread *[th_cnt];
        queue_array = new moodycamel::BlockingReaderWriterQueue<flow_id> *[th_cnt];
        for (int i = 0; i < th_cnt; i++)
        {
            hk_array[i] = new heavy_keeper(d, w, b, k * 2 / th_cnt);
            thread_array[i] = new std::thread(heavy_keeper_parallel::thread_handler, i, hk_array, queue_array);
            queue_array[i] = new moodycamel::BlockingReaderWriterQueue<flow_id>(100);
        }
        dispatcher_seed = rand();
    }

    ~heavy_keeper_parallel()
    {
        for (int i = 0; i < th_cnt; i++)
        {
            delete hk_array[i];
            delete thread_array[i];
        }
        delete[] hk_array;
        delete[] thread_array;
        delete[] queue_array;
    }

private:
    static void thread_handler(int i, heavy_keeper **hk_array, moodycamel::BlockingReaderWriterQueue<flow_id> **queue_array)
    {
        flow_id item_to_insert;
        while (true)
        {
            queue_array[i]->wait_dequeue(item_to_insert);
            if (is_equal(NULL_FLOW, item_to_insert))
                return;
            hk_array[i]->insert(item_to_insert);
        }
    }

public:
    bool insert(const uint8_t *flow_id_buf) { flow_id flow_id_obj(flow_id_buf); return insert(flow_id_obj); }
    bool insert(const flow_id flow_id_obj);
    std::vector<std::pair<flow_id, int>> query();
    int query_item(const flow_id key);
    const size_t get_byte_size();
    const std::string get_parameter();
    const std::string get_algo_name() { return std::string("HK_parallel"); };
};