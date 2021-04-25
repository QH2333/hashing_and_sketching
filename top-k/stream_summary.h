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
#include <unordered_map>
#include <cassert>

#include "top_k.h"
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
    bool insert_from_pos(bucket* start_buk, int new_num, node* node_ptr)
    {
        if (start_buk->number > new_num) // Unable to insert
            return false;
        while (start_buk->next && start_buk->next->number <= new_num)
        {
            start_buk = start_buk->next;
        }
        if (start_buk->number == new_num)
        {
            node_ptr->prev = start_buk->first->prev;
            node_ptr->next = start_buk->first;
            node_ptr->buk = start_buk;
            start_buk->first->prev->next = node_ptr;
            start_buk->first->prev = node_ptr;
        }
        else // start_buk->number < new_num
        {
            bucket *buk_to_insert = new bucket{node_ptr, start_buk, start_buk->next, new_num};
            curr_mem_byte += sizeof(bucket);
            if (start_buk->next)
                start_buk->next->prev = buk_to_insert;
            start_buk->next = buk_to_insert;
            node_ptr->next = node_ptr;
            node_ptr->prev = node_ptr;
            node_ptr->buk = buk_to_insert;
        }
        return true;
    }

    bool delete_node_chain(node* first)
    {
        node *last_node = first;
        assert(last_node != nullptr); // Empty bucket should not exist
        node *node_iter = last_node->next;
        last_node->prev->next = nullptr; // Break the chain
        for (node_iter = last_node->next; node_iter; node_iter = node_iter->next)
        {
            delete last_node;
            curr_mem_byte -= sizeof(node);
            last_node = node_iter;
        }
        delete last_node;
        curr_mem_byte -= sizeof(node);
        return true;
    }

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
    bool insert(std::pair<flow_id, int> kv_pair)
    {
        // return true;
        if (node_locator.find(kv_pair.first) != node_locator.end()) // key already in SS
        {
            node* node_ptr = node_locator.find(kv_pair.first)->second;
            bucket *buk_to_insert_from = node_ptr->buk->prev;
            int new_num = node_ptr->buk->number + kv_pair.second;
            if (node_ptr->next == node_ptr) // key is the only node in a bucket, move the node, delete the bucket
            {
                bucket *buk_to_del = node_ptr->buk;
                buk_to_del->prev->next = buk_to_del->next;
                if (buk_to_del->next)
                    buk_to_del->next->prev = buk_to_del->prev;
                delete buk_to_del;
                curr_mem_byte -= sizeof(bucket);
            }
            else // key is not the only node in a bucket
            {
                node_ptr->next->prev = node_ptr->prev;
                node_ptr->prev->next = node_ptr->next;
                if (node_ptr == node_ptr->buk->first) // key is directly pointed by the bucket
                {
                    // Hand over the direct pointer to the next node
                    node_ptr->buk->first = node_ptr->next;
                }
            }
            insert_from_pos(buk_to_insert_from, new_num, node_ptr);
        }
        else // key not in SS
        {
            if (curr_size == k) // SS is full
            {
                if (buk_head->next->number >= kv_pair.second)
                    return false;
                // Remove the minimal node in case of the new number is greater than it
                node *node_to_del = buk_head->next->first;
                if (node_to_del->next == node_to_del) // The target node is the only node in the bucket, remove the bucket as well
                {
                    bucket *buk_to_del = node_to_del->buk;
                    buk_to_del->prev->next = buk_to_del->next;
                    if (buk_to_del->next)
                        buk_to_del->next->prev = buk_to_del->prev;
                    delete buk_to_del;
                    curr_mem_byte -= sizeof(bucket);
                }
                else // The target node is not the only node in the bucket
                {
                    node_to_del->next->prev = node_to_del->prev;
                    node_to_del->prev->next = node_to_del->next;
                    if (node_to_del == node_to_del->buk->first) // key is directly pointed by the bucket
                    {
                        // Hand over the direct pointer to the next node
                        node_to_del->buk->first = node_to_del->next;
                    }
                }
                node_locator.erase(node_to_del->id);
                curr_size--;
                curr_mem_byte -= sizeof(std::pair<flow_id, node *>);
                delete node_to_del;
                curr_mem_byte -= sizeof(node);
            }
            // Insert
            node *node_to_insert = new node{nullptr, nullptr, nullptr, kv_pair.first};
            curr_mem_byte += sizeof(node);
            insert_from_pos(buk_head, kv_pair.second, node_to_insert);
            node_locator.insert(std::make_pair(kv_pair.first, node_to_insert));
            curr_size++;
            curr_mem_byte += sizeof(std::pair<flow_id, node *>);
        }
        return true;
    }

    /**
     * @brief Similar to insert, but kv_pair is interpreted as (key, new_value).
     *        This function only allows *incremental* update
     * 
     * @param kv_pair Interpreted as (key, new_value)
     * @return true if the k-v pair is updated successfully
     * @return false if the k-v pair is not updated
     */
    bool update(std::pair<flow_id, int> kv_pair)
    {
        if (node_locator.find(kv_pair.first) != node_locator.end()) // Found!
        {
            node* node_ptr = node_locator.find(kv_pair.first)->second;
            if (node_ptr->buk->number >= kv_pair.second)
                return false;
            else
            {
                return insert(std::make_pair(kv_pair.first, kv_pair.second - node_ptr->buk->number));
            }
        }
        return insert(kv_pair);
    }

    /**
     * @brief Remove the designated key form the stream_summary
     * 
     * @param key The key to be removed
     * @return true if the k-v pair is successfully removed
     * @return false if key is not found in the structure
     */
    bool erase(const flow_id key)
    {
        if (node_locator.find(key) != node_locator.end())
        {
            node* node_ptr = node_locator.find(key)->second;
            bucket *buk_to_insert_from = node_ptr->buk->prev;
            if (node_ptr->next == node_ptr) // Key is the only node in a bucket, remove the bucket as well
            {
                bucket *buk_to_del = node_ptr->buk;
                buk_to_del->prev->next = buk_to_del->next;
                if (buk_to_del->next)
                    buk_to_del->next->prev = buk_to_del->prev;
                delete buk_to_del;
                curr_mem_byte -= sizeof(bucket);
            }
            else // Key is not the only node in a bucket
            {
                node_ptr->next->prev = node_ptr->prev;
                node_ptr->prev->next = node_ptr->next;
                if (node_ptr == node_ptr->buk->first) // key is directly pointed by the bucket
                {
                    // Hand over the direct pointer to the next node
                    node_ptr->buk->first = node_ptr->next;
                }
            }
            node_locator.erase(key);
            curr_size--;
            curr_mem_byte -= sizeof(std::pair<flow_id, node *>);
            return true;
        }
        return false;
    }

    /**
     * @brief Get the current size of the stream_summary
     * 
     * @return const size_t 
     */
    const size_t size()
    {
        return curr_size;
    }

    const size_t get_byte_size()
    {
        return curr_mem_byte;
    }

    /**
     * @brief Print everything in the stream_summary
     * 
     */
    void print()
    {
        for (bucket *buk_iter = buk_head->next; buk_iter; buk_iter = buk_iter->next)
        {
            std::cout << buk_iter->number << "\t" << buk_iter->first->id.print_detail() << std::endl;
            for (node *node_iter = buk_iter->first->next; node_iter != buk_iter->first; node_iter = node_iter->next)
            {
                std::cout << buk_iter->number << "\t" << node_iter->id.print_detail() << std::endl;
            }
        }
    }

    /**
     * @brief Return the content of the stream_summary as a vector
     * 
     * @return std::vector<std::pair<flow_id, int>>: A list of the content of the stream_summary
     */
    std::vector<std::pair<flow_id, int>> get_content_vec()
    {
        std::vector<std::pair<flow_id, int>> ret_val;
        for (bucket *buk_iter = buk_head->next; buk_iter; buk_iter = buk_iter->next)
        {
            ret_val.push_back(std::make_pair(buk_iter->first->id, buk_iter->number));
            for (node *node_iter = buk_iter->first->next; node_iter != buk_iter->first; node_iter = node_iter->next)
            {
                ret_val.push_back(std::make_pair(node_iter->id, buk_iter->number));
            }
        }
        return ret_val;
    }
};
