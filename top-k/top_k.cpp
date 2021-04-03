/**
 * @file top_k.cpp
 * @author QH2333 (qi_an_hao@126.com)
 * @brief 
 * @version 0.1
 * @date 2021-04-03
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "top_k.h"

constexpr const char *INPUT_FILE = "../data/202012311400.dat";

class flow_id
{
public:
    uint8_t id[13];

public:
    flow_id(char* _id)
    {
        memcpy(this->id, _id, 13);
    }

    std::string print_detail()
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
};

struct cmp
{
    bool operator()(const flow_id id1, const flow_id id2) const
    {
        bool is_same = true;
        for (int i = 0; i < 13; i++)
            if (id1.id[i] != id2.id[i])
                is_same = false;
        return is_same;
    }
};

struct hash_func
{
    size_t operator()(const flow_id f_id) const
    {
        return hashlittle(f_id.id, 13, 0x12345678);
    }
};

bool pair_compare(std::pair<const flow_id, int> &a, std::pair<const flow_id, int> &b)
{
    return a.second > b.second;
}

int main()
{
    FILE* fp = fopen(INPUT_FILE, "rb");
    char flow_id_buf[13];
    std::unordered_map<flow_id, int, hash_func, cmp> record;
    while (fread(flow_id_buf, 13, 1, fp))
    {
        if (record.find(flow_id(flow_id_buf)) == record.end())
            record.insert(std::make_pair(flow_id(flow_id_buf), 1));
        else
            record[flow_id(flow_id_buf)] += 1;
    }

    if (feof(fp))
    {
        printf("End of file.\n");
    }
    else
    {
        perror("Error: ");
        exit(-1);
    }

    std::multimap<int, flow_id, std::greater<int>> sorted_record;
    for (auto iter = record.begin(); iter != record.end(); iter++)
    {
        sorted_record.insert(std::make_pair(iter->second, iter->first));
    }
    int top_k = 1000000;
    for (auto iter = sorted_record.begin(); iter != sorted_record.end(); iter++)
    {
        if (0 >= top_k--)
            break;
        // std::cout << "[" << std::setw(5) << iter->first << "]" << iter->second.print_detail() << std::endl;
        std::cout << iter->first << std::endl;
    }
    fclose(fp);
    return 0;
}