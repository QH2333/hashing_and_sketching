/**
 * @file common.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2021-04-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

// constexpr const char *INPUT_FILE = "/mnt/i/Project/dataset/imc192_datacenter/univ1_trace/univ1_pt1";
// constexpr const char *PARSED_FILE = "../parsed_data/univ1_pt1.dat";
constexpr const char *INPUT_FILE = "/mnt/i/Project/dataset/MAWI/202012311400.pcap";
constexpr const char *PARSED_FILE = "../parsed_data/202012311400.dat";
constexpr int MAX_PACKET_CNT = -1; // Used in the data cleaner,  -1 for infinite
constexpr int MAX_READ_PKT = 10000000; // Userd in the top-k algo, -1 for infinite
constexpr int REPEAT_CNT = 10;
constexpr int K = 500;