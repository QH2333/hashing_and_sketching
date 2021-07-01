/**
 * @file common.h
 * @author QH2333 (qi_an_hao@126.com)
 * @brief 
 * @version 0.1
 * @date 2021-04-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#define TO_STR_(x) #x
#define TO_STR(x) TO_STR_(x)

#define BASE_DIR "/home/qh2333/hashing_and_sketching/"
// constexpr const char *INPUT_FILE = "/mnt/i/Project/dataset/imc192_datacenter/univ1_trace/univ1_pt1";
// constexpr const char *PARSED_FILE = BASE_DIR "topk_agent/parsed_data/univ1_pt1.dat";
constexpr const char *INPUT_FILE = "/mnt/i/Project/dataset/imc192_datacenter/univ2_trace/univ2_pt0";
constexpr const char *PARSED_FILE = BASE_DIR "topk_agent/parsed_data/univ2_pt0.dat";
// constexpr const char *INPUT_FILE = "/mnt/i/Project/dataset/MAWI/202012311400.pcap";
// constexpr const char *PARSED_FILE = BASE_DIR "topk_agent/parsed_data/202012311400.dat";
constexpr int MAX_PACKET_CNT = -1; // Used in the data cleaner, -1 for infinite
constexpr int MAX_READ_PKT = 1000 * 10000; // Used in the top-k algo, -1 for infinite
constexpr int REPEAT_CNT = 5;
constexpr int K = 100;