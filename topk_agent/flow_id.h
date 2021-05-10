/**
 * @file flow_id.h
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
#include <cstring>
#include <sstream>
#include <iomanip>

// C POSIX library
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

// Project headers
#include "common/lookup3.h"

class flow_id
{
public:
    uint8_t id[13];

public:
    flow_id()
    {
        memset(id, 0, 13); // Null flow as default
    }

    flow_id(const uint8_t* _id)
    {
        memcpy(this->id, _id, 13);
    }

    const std::string to_string() const;
    const std::string to_json() const;
    const uint32_t hash_with_seed(uint32_t seed) const;
    const bool is_null() const;
};

/**
 * @brief Multiple template callable that will enable stl based flow_id processing
 * 
 */
namespace std
{
    template<> struct hash<flow_id>
    {
        size_t operator()(const flow_id& f_id) const
        {
            return hashlittle(f_id.id, 13, 0x12345678);
        }
    };

    template<> struct equal_to<flow_id>
    {
        bool operator()(const flow_id& lhs, const flow_id& rhs) const
        {
            return !memcmp(lhs.id, rhs.id, 13);
        }
    };

    template<> struct less<std::pair<flow_id, int>>
    {
        bool operator()( const std::pair<flow_id, int>& lhs, const std::pair<flow_id, int>& rhs ) const
        {
            return lhs.second < rhs.second;
        }
    };

    template<> struct greater<std::pair<flow_id, int>>
    {
        bool operator()( const std::pair<flow_id, int>& lhs, const std::pair<flow_id, int>& rhs ) const
        {
            return lhs.second > rhs.second;
        }
    };
}

const flow_id NULL_FLOW;