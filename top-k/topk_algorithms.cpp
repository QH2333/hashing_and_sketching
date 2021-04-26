/**
 * @file topk_algorithms.h
 * @author QH2333 (qi_an_hao@126.com)
 * @brief 
 * @version 0.1
 * @date 2021-04-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "topk_algorithms.h"

bool exact_algo::insert(const flow_id &flow_id_obj)
{
    if (hash_table.find(flow_id_obj) == hash_table.end())
        hash_table.insert(std::make_pair(flow_id_obj, 1));
    else
        hash_table[flow_id_obj] += 1;
}

std::vector<std::pair<flow_id, int>> exact_algo::query()
{
    std::priority_queue<std::pair<flow_id, int>, std::vector<std::pair<flow_id, int>>, std::greater<std::pair<flow_id, int>>> sorted_record;
    for (auto iter = hash_table.begin(); iter != hash_table.end(); iter++)
    {
        if (sorted_record.size() < k)
        {
            sorted_record.push(std::make_pair(iter->first, iter->second));
        }
        else if (sorted_record.top().second < iter->second)
        {
            sorted_record.pop();
            sorted_record.push(std::make_pair(iter->first, iter->second));
        }
    }
    std::vector<std::pair<flow_id, int>> ret_val;
    while (!sorted_record.empty())
    {
        ret_val.push_back(sorted_record.top());
        sorted_record.pop();
    }
    return std::vector<std::pair<flow_id, int>>(ret_val.rbegin(), ret_val.rend());
}

int exact_algo::query_item(const flow_id key)
{
    if (hash_table.find(key) != hash_table.end())
        return hash_table.find(key)->second;
    else
        return 0;
}

std::vector<std::pair<flow_id, int>> exact_algo::query_ss()
{
    stream_summary ss(k);
    for (auto iter = hash_table.begin(); iter != hash_table.end(); iter++)
    {
        ss.insert(std::make_pair(iter->first, iter->second));
    }
    std::vector<std::pair<flow_id, int>> ret_val = ss.get_content_vec();
    return std::vector<std::pair<flow_id, int>>(ret_val.rbegin(), ret_val.rend());
}

const std::string exact_algo::get_parameter()
{
    std::stringstream parameter;
    parameter << "k=" << k;
    return parameter.str();
}

/************************************************************************************/

bool count_min_heap::insert(const flow_id &flow_id_obj)
{
    int min = -1;
    for (int i = 0; i < d; i++)
    {
        int hashed_bucket_id = flow_id_obj.hash_with_seed(seeds[i]) % m;
        sketch[i * m + hashed_bucket_id]++;
        if (min > sketch[i * m + hashed_bucket_id] || min < 0)
        {
            min = sketch[i * m + hashed_bucket_id];
        }
    }
    ss->update(std::make_pair(flow_id_obj, min));
}

std::vector<std::pair<flow_id, int>> count_min_heap::query()
{
    std::vector<std::pair<flow_id, int>> ret_val = ss->get_content_vec();
    return std::vector<std::pair<flow_id, int>>(ret_val.rbegin(), ret_val.rend());
}

const std::string count_min_heap::get_parameter()
{
    std::stringstream parameter;
    parameter << "d=" << d << ",m=" << m << ",k=" << k;
    return parameter.str();
}

/************************************************************************************/

bool heavy_keeper::insert_basic_ver(const flow_id flow_id_obj)
{
    for (int i = 0; i < d; i++)
    {
        int hashed_bucket_id = flow_id_obj.hash_with_seed(seeds[i]) % w;
        if (hk[i * w + hashed_bucket_id].second == 0) // case 1
        {
            hk[i * w + hashed_bucket_id].first = flow_id_obj;
            hk[i * w + hashed_bucket_id].second = 1;
        }
        else if (is_equal(hk[i * w + hashed_bucket_id].first, flow_id_obj)) // case 2
        {
            hk[i * w + hashed_bucket_id].second++;
        }
        else // case 3
        {
            if (rand() < INT32_MAX * pow(b, -(hk[i * w + hashed_bucket_id].second)))
            {
                hk[i * w + hashed_bucket_id].second--;
            }
        }
    }
    ss->update(std::make_pair(flow_id_obj, query_item(flow_id_obj)));
}

bool heavy_keeper::insert(const flow_id &flow_id_obj) // software min version
{
    const std::lock_guard<std::mutex> lock(insert_mutex);
    int first_empty_i = -1;
    int first_empty_j = -1;
    for (int i = 0; i < d; i++) // If the flow is already monitored, just update it
    {
        int hashed_bucket_id = flow_id_obj.hash_with_seed(seeds[i]) % w;
        if (is_equal(hk[i * w + hashed_bucket_id].first, flow_id_obj))
        {
            hk[i * w + hashed_bucket_id].second++;
            ss->update(std::make_pair(flow_id_obj, query_item(flow_id_obj)));
            return true;
        }
        if (hk[i * w + hashed_bucket_id].second == 0 && first_empty_i == -1)
        {
            first_empty_i = i;
            first_empty_j = hashed_bucket_id;
        }
    }
    if (first_empty_i != -1) // The flow is not monitored, but can be inserted
    {
        hk[first_empty_i * w + first_empty_j].first = flow_id_obj;
        hk[first_empty_i * w + first_empty_j].second = 1;
        ss->update(std::make_pair(flow_id_obj, query_item(flow_id_obj)));
    }
    else // Exponential decay
    {
        int min_i = -1;
        int min_j = -1;
        int min_count;
        for (int i = 0; i < d; i++)
        {
            int hashed_bucket_id = flow_id_obj.hash_with_seed(seeds[i]) % w;
            if (min_i == -1 || hk[i * w + hashed_bucket_id].second < min_count)
            {
                min_i = i;
                min_j = hashed_bucket_id;
                min_count = hk[i * w + hashed_bucket_id].second;
            }
        }
        int bucket_addr = min_i * w + min_j;
        if (rand() < INT32_MAX * pow(b, -(hk[bucket_addr].second)))
        {
            hk[bucket_addr].second--;
            if (hk[bucket_addr].second == 0)
            {
                hk[bucket_addr].first = flow_id_obj;
                hk[bucket_addr].second = 1;
            }
        }
        ss->update(std::make_pair(flow_id_obj, query_item(flow_id_obj)));
    }
    return true;
}

std::vector<std::pair<flow_id, int>> heavy_keeper::query()
{
    std::vector<std::pair<flow_id, int>> ret_val = ss->get_content_vec();
    return std::vector<std::pair<flow_id, int>>(ret_val.rbegin(), ret_val.rend());
}

int heavy_keeper::query_item(const flow_id key)
{
    int max = 0;
    for (int i = 0; i < d; i++)
    {
        int hashed_bucket_id = key.hash_with_seed(seeds[i]) % w;
        if (is_equal(hk[i * w + hashed_bucket_id].first, key))
            max = std::max(hk[i * w + hashed_bucket_id].second, max);
    }
    return max;
}

const std::string heavy_keeper::get_parameter()
{
    std::stringstream parameter;
    parameter << "d=" << d << ",w=" << w << ",b=" << b << ",k=" << k;
    return parameter.str();
}

/************************************************************************************/

bool heavy_keeper_opt::insert(const flow_id &flow_id_obj) // software min version
{
    int first_empty_i = -1;
    int first_empty_j = -1;
    for (int i = 0; i < d; i++) // If the flow is already monitored, just update it
    {
        int hashed_bucket_id = flow_id_obj.hash_with_seed(seeds[i]) % w;
        if (is_equal(hk[i * w + hashed_bucket_id].first, flow_id_obj))
        {
            hk[i * w + hashed_bucket_id].second++;
            ss->update(std::make_pair(flow_id_obj, query_item(flow_id_obj)));
            return true;
        }
        if (hk[i * w + hashed_bucket_id].second == 0 && first_empty_i == -1)
        {
            first_empty_i = i;
            first_empty_j = hashed_bucket_id;
        }
    }
    if (first_empty_i != -1) // The flow is not monitored, but can be inserted
    {
        hk[first_empty_i * w + first_empty_j].first = flow_id_obj;
        hk[first_empty_i * w + first_empty_j].second = 1;
        ss->update(std::make_pair(flow_id_obj, query_item(flow_id_obj)));
    }
    else // Exponential decay
    {
        int min_i = -1;
        int min_j = -1;
        int min_count;
        for (int i = 0; i < d; i++)
        {
            int hashed_bucket_id = flow_id_obj.hash_with_seed(seeds[i]) % w;
            if (min_i == -1 || hk[i * w + hashed_bucket_id].second < min_count)
            {
                min_i = i;
                min_j = hashed_bucket_id;
                min_count = hk[i * w + hashed_bucket_id].second;
            }
        }
        int bucket_addr = min_i * w + min_j;
        if (rand() < INT32_MAX * pow(b, -(hk[bucket_addr].second)))
        {
            hk[bucket_addr].second--;
            ss_overflow->insert(std::make_pair(hk[bucket_addr].first, 1));
            if (hk[bucket_addr].second == 0)
            {
                hk[bucket_addr].first = flow_id_obj;
                hk[bucket_addr].second = 1;
            }
            else
            {
                ss_overflow->insert(std::make_pair(flow_id_obj, 1));
            }
        }
        else
        {
            ss_overflow->insert(std::make_pair(flow_id_obj, 1));
        }
        ss->update(std::make_pair(flow_id_obj, query_item(flow_id_obj)));
    }
    return true;
}

std::vector<std::pair<flow_id, int>> heavy_keeper_opt::query()
{
    std::vector<std::pair<flow_id, int>> overflow_list = ss_overflow->get_content_vec();
    for (auto item: overflow_list)
    {
        // std::cout << item.first.print_detail() << " " << item.second << std::endl;
        ss->insert(item);
    }
    std::vector<std::pair<flow_id, int>> combined_list = ss->get_content_vec();
    std::vector<std::pair<flow_id, int>> ret_val(combined_list.rbegin(), combined_list.rend());
    ret_val.erase(ret_val.begin() + 100, ret_val.end());
    return ret_val;
}

int heavy_keeper_opt::query_item(const flow_id key)
{
    int max = 0;
    for (int i = 0; i < d; i++)
    {
        int hashed_bucket_id = key.hash_with_seed(seeds[i]) % w;
        if (is_equal(hk[i * w + hashed_bucket_id].first, key))
            max = std::max(hk[i * w + hashed_bucket_id].second, max);
    }
    return max;
}

const std::string heavy_keeper_opt::get_parameter()
{
    std::stringstream parameter;
    parameter << "d=" << d << ",w=" << w << ",b=" << b << ",k=" << k;
    return parameter.str();
}

/************************************************************************************/

void heavy_keeper_parallel::thread_handler(thread_para para)
{
    int th_id = para.th_id;
    int th_cnt = para.th_cnt;
    uint32_t dispatcher_seed = para.dispatcher_seed;
    heavy_keeper **hk_array = para.hk_array;
    moodycamel::BlockingConcurrentQueue<flow_id> *queue = para.queue;
    flow_id item_to_insert;
    while (true)
    {
        queue->wait_dequeue(item_to_insert);
        int dispatch_dst = th_id;
        if (is_equal(NULL_FLOW, item_to_insert))
            return;
        hk_array[dispatch_dst]->insert(item_to_insert);
    }
}

bool heavy_keeper_parallel::insert(const flow_id &flow_id_obj)
{
    queue->enqueue(flow_id_obj);
}

std::vector<std::pair<flow_id, int>> heavy_keeper_parallel::query()
{
    stream_summary merged_result(k);
    for (int i = 0; i < th_cnt; i++)
    {
        queue->enqueue(NULL_FLOW);
    }
    for (int i = 0; i < th_cnt; i++)
    {
        if (thread_array[i]->joinable()) thread_array[i]->join();
        std::vector<std::pair<flow_id, int>> partial_result = hk_array[i]->query();
        for (auto item: partial_result)
            merged_result.insert(item);
    }
    std::vector<std::pair<flow_id, int>> ret_val = merged_result.get_content_vec();
    return std::vector<std::pair<flow_id, int>>(ret_val.rbegin(), ret_val.rend());
}

int heavy_keeper_parallel::query_item(const flow_id key)
{
    int dispatch_dst = key.hash_with_seed(dispatcher_seed) % th_cnt;
    return hk_array[dispatch_dst]->query_item(key);
}

const size_t heavy_keeper_parallel::get_byte_size()
{
    size_t byte_size = sizeof(heavy_keeper_parallel);
    for (int i = 0; i < th_cnt; i++)
    {
        byte_size += hk_array[i]->get_byte_size();
    }
    return byte_size;
}

const std::string heavy_keeper_parallel::get_parameter()
{
    std::stringstream parameter;
    parameter << d << "," << w << "," << b << ",tc=" << th_cnt << ",k=" << k;
    return parameter.str();
}
