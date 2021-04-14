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

bool exact_algo::insert(const char* flow_id_buf)
{
    if (hash_table.find(flow_id(flow_id_buf)) == hash_table.end())
        hash_table.insert(std::make_pair(flow_id(flow_id_buf), 1));
    else
        hash_table[flow_id(flow_id_buf)] += 1;
}

bool exact_algo::insert(const flow_id flow_id_obj)
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

bool count_min_heap::insert(const char* flow_id_buf)
{
    flow_id flow_id_obj(flow_id_buf);
    insert(flow_id_obj);
}

bool count_min_heap::insert(const flow_id flow_id_obj)
{
    int min = -1;
    for (int i = 0; i < d; i++)
    {
        int hashed_bucket_id = flow_id_obj.hash_with_seed(seeds[i]) % m;
        sketch[i][hashed_bucket_id]++;
        if (min > sketch[i][hashed_bucket_id] || min < 0)
        {
            min = sketch[i][hashed_bucket_id];
        }
    }
}

std::vector<std::pair<flow_id, int>> count_min_heap::query()
{
    // std::priority_queue<std::pair<flow_id, int>, std::vector<std::pair<flow_id, int>>, std::greater<std::pair<flow_id, int>>> sorted_record;
    // for (auto iter = record.begin(); iter != record.end(); iter++)
    // {
    //     if (sorted_record.size() < k)
    //     {
    //         sorted_record.push(std::make_pair(iter->first, iter->second));
    //     }
    //     else if (sorted_record.top().second < iter->second)
    //     {
    //         sorted_record.pop();
    //         sorted_record.push(std::make_pair(iter->first, iter->second));
    //     }
    // }

    std::vector<std::pair<flow_id, int>> ret_val;
    // while (!sorted_record.empty())
    // {
    //     ret_val.push_back(sorted_record.top());
    //     sorted_record.pop();
    // }
    return std::vector<std::pair<flow_id, int>>(ret_val.rbegin(), ret_val.rend());
}