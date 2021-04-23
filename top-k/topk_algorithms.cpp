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

bool exact_algo::insert(const uint8_t* flow_id_buf)
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

bool count_min_heap::insert(const uint8_t* flow_id_buf)
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

bool heavy_keeper::insert(const uint8_t *flow_id_buf)
{
    flow_id flow_id_obj(flow_id_buf);
    insert(flow_id_obj);
}

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

bool heavy_keeper::insert(const flow_id flow_id_obj) // software min version
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