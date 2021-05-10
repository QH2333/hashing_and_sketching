/**
 * @file stream_summary.cpp
 * @author QH2333 (qi_an_hao@126.com)
 * @brief 
 * @version 0.1
 * @date 2021-04-26
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "stream_summary.h"

bool stream_summary::insert_from_pos(bucket* start_buk, int new_num, node* node_ptr)
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

bool stream_summary::delete_node_chain(node* first)
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

bool stream_summary::insert(std::pair<flow_id, int> kv_pair)
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

bool stream_summary::update(std::pair<flow_id, int> kv_pair)
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

bool stream_summary::erase(const flow_id key)
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

const size_t stream_summary::size()
{
    return curr_size;
}

const size_t stream_summary::get_byte_size()
{
    return curr_mem_byte;
}

void stream_summary::print()
{
    for (bucket *buk_iter = buk_head->next; buk_iter; buk_iter = buk_iter->next)
    {
        std::cout << buk_iter->number << "\t" << buk_iter->first->id.to_string() << std::endl;
        for (node *node_iter = buk_iter->first->next; node_iter != buk_iter->first; node_iter = node_iter->next)
        {
            std::cout << buk_iter->number << "\t" << node_iter->id.to_string() << std::endl;
        }
    }
}

std::vector<std::pair<flow_id, int>> stream_summary::get_content_vec()
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