/**
 * @file stream_summary.cpp
 * @author QH2333 (qi_an_hao@126.com)
 * @brief Implements the stream_summary data structure according to the original paper
 * @version 0.1
 * @date 2021-04-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <utility>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <cassert>

#include "flow_id.h"

class stream_summary
{
private:
    struct node;
    struct bucket
    {
        node *first = nullptr;
        bucket *prev = nullptr;
        bucket *next = nullptr;
        int number = 0;
    };
    struct node
    {
        bucket *buk = nullptr;
        node *prev = nullptr;
        node *next = nullptr;
        flow_id id;
    };
    bucket psudo_head = {nullptr, nullptr, nullptr, -1};
    bucket *buk_head = &psudo_head;
    int k;
    size_t curr_size = 0;
    size_t curr_mem_byte = sizeof(stream_summary);

    std::unordered_map<flow_id, node *> node_locator;

public:
    stream_summary(int _k) : k(_k) { }
    ~stream_summary()
    {
        bucket *last_buk = buk_head->next;
        if (!last_buk) // Empty ss
            return;
        bucket *buk_iter = last_buk->next;
        for (buk_iter = last_buk->next; buk_iter; buk_iter = buk_iter->next)
        {
            delete_node_chain(last_buk->first);
            delete last_buk;
            curr_mem_byte -= sizeof(bucket);
            last_buk = buk_iter;
        }
        delete_node_chain(last_buk->first);
        delete last_buk;
        curr_mem_byte -= sizeof(bucket);
    }

private:
    /**
     * @brief Insert the node pointed by <node_ptr> to the bucket of <new_num>.
     *        The insertion is based on a linear search, which begins with <start_buk>.
     * 
     * @param start_buk The starting point of insertion, its <number> should be equal to or less than <new_num>
     * @param new_num The <number> of <node_ptr>
     * @param node_ptr The node to be inserted
     * @return true if the node is inserted successfully
     * @return false if the <number> of <start_buk> is greater than <new_num>
     */
    bool insert_from_pos(bucket *start_buk, int new_num, node *node_ptr);
    bool delete_node_chain(node *first);

public:
    /**
     * @brief Insert a k-v pair into the stream_summary
     *        If k already exists, increase its original value by v
     *        If stream_summary is full, evict the key with minimal value if k is greater than it
     * 
     * @param kv_pair Interpreted as (key, increment)
     * @return true if the k-v pair is inserted successfully
     * @return false if the k-v pair is not inserted
     */
    bool insert(std::pair<flow_id, int> kv_pair);

    /**
     * @brief Similar to insert, but kv_pair is interpreted as (key, new_value).
     *        This function only allows *incremental* update
     * 
     * @param kv_pair Interpreted as (key, new_value)
     * @return true if the k-v pair is updated successfully
     * @return false if the k-v pair is not updated
     */
    bool update(std::pair<flow_id, int> kv_pair);

    /**
     * @brief Remove the designated key form the stream_summary
     * 
     * @param key The key to be removed
     * @return true if the k-v pair is successfully removed
     * @return false if key is not found in the structure
     */
    bool erase(const flow_id key);

    /**
     * @brief Get the current size of the stream_summary
     * 
     * @return const size_t 
     */
    const size_t size();

    /**
     * @brief Get the current byte size of the stream_summary
     * 
     * @return const size_t 
     */
    const size_t get_byte_size();

    /**
     * @brief Print everything in the stream_summary
     * 
     */
    void print();

    /**
     * @brief Return the content of the stream_summary as a vector
     * 
     * @return std::vector<std::pair<flow_id, int>>: A list of the content of the stream_summary
     */
    std::vector<std::pair<flow_id, int>> get_content_vec();
};
