/**
 * @file live_capture.h
 * @author QH2333 (qi_an_hao@126.com)
 * @brief 
 * @version 0.1
 * @date 2021-04-25
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <fstream>
#include <vector>
#include <string>
#include <pcap.h>

#include "topk_algorithms.h"
#include "common/pkt_headers.h"

// === Control the program's behavior here ===
constexpr bool VERBOSE = false;
constexpr bool CONCISE_INFO = true;
// === End of behavior control section ===

#define PARSE_START 0
#define PARSE_ETHER 1
#define PARSE_VLAN  2
#define PARSE_IPV4  3
#define PARSE_IPV6  4
#define PARSE_TCP   5
#define PARSE_UDP   6
#define PARSE_END   255

struct if_info_t
{
    std::string name;
    std::string description;
};

std::vector<if_info_t> get_interface_list();
void monitor_pkt_on_if(const char *if_name, int max_pkt_cnt, topk_algo_base *algo_obj);

/**
 * @brief Used in parameter -s
 * 
 * @param if_name 
 * @param max_pkt_cnt 
 * @param algo_obj 
 * @param capture_thread 
 * @param on_finish 
 * @return true 
 * @return false 
 */
pcap_t * monitor_pkt_on_if_async(const char *if_name, int max_pkt_cnt, topk_algo_base *algo_obj, std::thread* capture_thread, void (*on_finish)(void));

/**
 * @brief Parse a packet using a state machine and write 13-byte 5-tuple into a file. Prototype is defined in conformance with `pcap_handler`.
 * 
 * @param userData The file discriptor of output file, its type should be converted from FILE* to char* by the caller in `pcap_loop`.
 * @param pkthdr Generic per-packet information, as supplied by libpcap.
 * @param packet The content of a layer-2 packet.
 */
void packetHandler(u_char *userData, const struct pcap_pkthdr *pkthdr, const u_char *packet);

/**
 * @brief Used in parameter -i
 * 
 * @param max_pkt_cnt 
 * @param algo_obj 
 */
void monitor_live(int max_pkt_cnt, topk_algo_base *algo_obj);
