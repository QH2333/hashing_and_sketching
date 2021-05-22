/**
 * @file bench_adapter.h
 * @author QH2333 (qi_an_hao@126.com)
 * @brief 
 * @version 0.1
 * @date 2021-04-24
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once

#include "common/common.h"
#include "topk_algorithms.h"

#define EXACT  0
#define CMHEAP 1
#define HK     2
class bench_adapter
{
private:
    struct cm_heap_para_t // Count-Min Heap
    {
        int d;
        int m;
    };
    struct hk_para_t // HeavyKeeper
    {
        int d;
        int w;
        float b;
    };
    struct hkp_para_t // HeavyKeeper parallel
    {
        int d;
        int w;
        float b;
        int th_cnt;
    };

private:
    std::ifstream plan_file;
    char last_line_buf[100];
    int read_cnt;
    int k;
    std::string algo_name;
    cm_heap_para_t cm_heap_para;
    hk_para_t hk_para;
    hkp_para_t hkp_para;

private:
    void parse_last_line()
    {
        std::stringstream last_line;
        last_line << last_line_buf;
        
        last_line >> read_cnt;
        last_line >> k;
        last_line >> algo_name;

        if (algo_name == "count_min_heap")
        {
            last_line >> cm_heap_para.d >> cm_heap_para.m;
        }
        else if (algo_name == "heavy_keeper")
        {
            last_line >> hk_para.d >> hk_para.w >> hk_para.b;
        }
        else if (algo_name == "heavy_keeper_opt")
        {
            last_line >> hk_para.d >> hk_para.w >> hk_para.b;
        }
        else if (algo_name == "heavy_keeper_parallel")
        {
            last_line >> hkp_para.d >> hkp_para.w >> hkp_para.b >> hkp_para.th_cnt;
        }
    }

public:
    bench_adapter() : plan_file("bench_plan.sh"){};

    const bool read_next_algo()
    {
        if (plan_file.eof())
            return false;
        plan_file.getline(last_line_buf, 100);
        while (last_line_buf[0] == '#' || last_line_buf[0] == '\0')
        {
            if (plan_file.eof())
                return false;
            plan_file.getline(last_line_buf, 100);
        }
        parse_last_line();
        return true;
    }

    topk_algo_base *get_bench_algo()
    {
        topk_algo_base *algo_obj;
        if (algo_name == "exact_algo")
        {
            algo_obj = new exact_algo(k);
        }
        else if (algo_name == "count_min_heap")
        {
            algo_obj = new count_min_heap(cm_heap_para.d, cm_heap_para.m, k);
        }
        else if (algo_name == "heavy_keeper")
        {
            algo_obj = new heavy_keeper(hk_para.d, hk_para.w, hk_para.b, k);
        }
        else if (algo_name == "heavy_keeper_opt")
        {
            algo_obj = new heavy_keeper_opt(hk_para.d, hk_para.w, hk_para.b, k);
        }
        else if (algo_name == "heavy_keeper_parallel")
        {
            algo_obj = new heavy_keeper_parallel(hkp_para.d, hkp_para.w, hkp_para.b, hkp_para.th_cnt, k);
        }
        return algo_obj;
    }

    int get_cnt()
    {
        return read_cnt;
    }

    int get_k()
    {
        return k;
    }

    std::string get_last_line()
    {
        return std::string(last_line_buf);
    }
};