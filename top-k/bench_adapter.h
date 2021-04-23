#pragma once

#include "top_k.h"
#include "topk_algorithms.h"

#define EXACT  0
#define CMHEAP 1
#define HK     2
class bench_adapter
{
private:
    std::ifstream plan_file;
    char last_line_buf[100];

private:
    topk_algo_base *parse_last_line()
    {
        std::stringstream last_line;
        last_line << last_line_buf;
        topk_algo_base *algo_obj;
        std::string algo_name;
        last_line >> algo_name;
        if (algo_name == "exact_algo")
        {
            int k;
            last_line >> k;
            algo_obj = new exact_algo(k);
        }
        else if (algo_name == "count_min_heap")
        {
            int d;
            int m;
            int k;
            last_line >> d >> m >> k;
            algo_obj = new count_min_heap(d, m, k);
        }
        else if (algo_name == "heavy_keeper")
        {
            int d;
            int w;
            float b;
            int k;
            last_line >> d >> w >> b >> k;
            algo_obj = new heavy_keeper(d, w, b, k);
        }
        return algo_obj;
    }

public:
    bench_adapter() : plan_file("bench_plan.sh"){};

    const bool read_next_algo()
    {
        if (plan_file.eof())
            return false;
        plan_file.getline(last_line_buf, 100);
        while (last_line_buf[0] == '#')
        {
            if (plan_file.eof())
                return false;
            plan_file.getline(last_line_buf, 100);
        }
        return true;
    }

    topk_algo_base *get_bench_algo()
    {
        return parse_last_line();
    }
};