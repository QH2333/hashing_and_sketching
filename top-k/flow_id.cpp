/**
 * @file flow_id.cpp
 * @author QH2333 (qi_an_hao@126.com)
 * @brief 
 * @version 0.1
 * @date 2021-04-26
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "flow_id.h"

const std::string flow_id::print_detail() const
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

const uint32_t flow_id::hash_with_seed(uint32_t seed) const
{
    return hashlittle(this->id, 13, seed);
}

const bool flow_id::is_null() const
{
    for (int i = 0; i < 13; i++)
    {
        if (id[i] != 0)
            return false;
    }
    return true;
}