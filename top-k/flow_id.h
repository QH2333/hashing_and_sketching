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

#include "top_k.h"

class flow_id
{
public:
    uint8_t id[13];

public:
    flow_id()
    {
        memset(id, 0, 13);
    }

    flow_id(const uint8_t* _id)
    {
        memcpy(this->id, _id, 13);
    }

    std::string print_detail() const
    {
        std::stringstream src_ip_stream;
        std::stringstream dst_ip_stream;
        std::stringstream detail_stream;
        src_ip_stream << (int)id[0] << "." << (int)id[1] << "." << (int)id[2] << "." << (int)id[3] << ":" << ntohs(*(uint16_t*)(id + 8));
        dst_ip_stream << (int)id[4] << "." << (int)id[5] << "." << (int)id[6] << "." << (int)id[7] << ":" << ntohs(*(uint16_t*)(id + 10));
        if (id[12] == 0x06)
            detail_stream << "[TCP] ";
        else if (id[12] == 0x11)
            detail_stream << "[UDP] ";
        detail_stream << std::internal << std::setw(21) << src_ip_stream.str() << " - " << dst_ip_stream.str();
        return detail_stream.str();
    }

    const uint32_t hash_with_seed(uint32_t seed) const
    {
        return hashlittle(this->id, 13, seed);
    }

    const bool is_null() const
    {
        for (int i = 0; i < 13; i++)
        {
            if (id[i] != 0)
                return false;
        }
        return true;
    }
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