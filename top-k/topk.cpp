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

#include "topk.h"

#include "benchmark.h"
#include "live_capture.h"

// === Control the program's behavior here ===

// #define ALGORITHM_TO_BENCH exact_algo
// #define ALGORITHM_PARAMETER (K)

// === End of behavior control section ===

sockaddr_in get_ip_addr(uint32_t host, uint16_t port);
void cleanup(int);

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        if (!strncmp(argv[1], "-b", 2))
        {
            std::vector<flow_id> packets;
            read_packets(packets, getCurrentRSS());
            std::cout << std::endl;
            benchmarking(packets);
        }
        else if (!strncmp(argv[1], "-c", 2))
        {
            heavy_keeper algo_obj(3, 1200, 1.08, 100);
            monitor_live(MAX_READ_PKT, &algo_obj);
            print_topk(algo_obj.query());
        }
        else if (!strncmp(argv[1], "-f", 2))
        {
            std::ifstream input_file("input");
            std::ofstream output_file("output");
            std::string command;
            input_file >> command;
            if (command == "getiflist")
            {
                std::vector<if_info_t> if_list = get_interface_list();
                for (auto interface: if_list)
                {
                    output_file << interface.name << "\t" << interface.description << std::endl;
                }
            }
            else if (command == "capture")
            {
                std::string if_name;
                int pkt_cnt;
                int k;
                input_file >> if_name >> pkt_cnt >> k;
                heavy_keeper algo_obj(3, 1200, 1.08, k);
                monitor_pkt_on_if(if_name.c_str(), pkt_cnt, &algo_obj);
                std::vector<std::pair<flow_id, int>> result = algo_obj.query();
                for (auto entry: result)
                {
                    output_file << entry.first.print_detail() << "\t" << entry.second << std::endl;
                }
                std::ofstream log_file("log", std::ios::app);
                log_file << "Done";
            }
        }
    }
    return 0;
}
