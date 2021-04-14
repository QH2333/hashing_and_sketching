/**
 * @file data_cleaner.cpp
 * @author QH2333 (qi_an_hao@126.com)
 * @brief Read in captured packets from file then extract 5-tuples
 * @version 0.1
 * @date 2021-04-03
 * 
 * @copyright Copyright (c) 2021
 * 
 */

// C/CPP standard library
#include <cstdint>
#include <cstring>
#include <iostream>

// C POSIX library
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

// Thrid-party library
#include <pcap.h>

// Project headers
#include "../common/common.h"
#include "../common/pkt_headers.h"

// === Control the program's behavior here ===
constexpr bool VERBOSE = false;
constexpr bool CONCISE_INFO = true;
// === End of behavior control section ===
int packet_cnt = 0;

void packetHandler(u_char *userData, const struct pcap_pkthdr* pkthdr, const u_char* packet);

int main()
{
    char errbuf[PCAP_ERRBUF_SIZE];

    // Create a pcap session
    pcap_t *pcap_session = pcap_open_offline(INPUT_FILE, errbuf);
    if (pcap_session == NULL) {
        fprintf(stderr, "pcap_open_offline() failed: %s\n", errbuf);
        exit(1);
    }
    
    // Construct and apply a BPF program
    struct bpf_program filter;
    if (pcap_compile(pcap_session, &filter, "(udp || tcp) || (vlan && (udp || tcp))", 1, 0) == PCAP_ERROR)
    {
        fprintf(stderr, "pcap_compile() failed: %s\n", pcap_geterr(pcap_session));
        exit(1);
    };
    if (pcap_setfilter(pcap_session, &filter) == PCAP_ERROR)
    {
        fprintf(stderr, "pcap_setfilter() failed: %s\n", pcap_geterr(pcap_session));
        exit(1);
    }
    pcap_freecode(&filter);

    FILE *fp = fopen(PARSED_FILE, "wb");
    // Start packet processing loop, just like live capture
    printf("===== Start capturing =====\n");
    if (pcap_loop(pcap_session, MAX_PACKET_CNT, packetHandler, (u_char *)fp) < 0)
    {
        printf("pcap_loop() failed: %s\n", pcap_geterr(pcap_session));
        exit(1);
    }
    fclose(fp);

    printf("===== Capture finished =====\n");
    pcap_close(pcap_session);
    return 0;
}

#define PARSE_START 0
#define PARSE_ETHER 1
#define PARSE_VLAN  2
#define PARSE_IPV4  3
#define PARSE_IPV6  4
#define PARSE_TCP   5
#define PARSE_UDP   6
#define PARSE_END   255
/**
 * @brief Parse a packet using a state machine and write 13-byte 5-tuple into a file. Prototype is defined in conformance with `pcap_handler`.
 * 
 * @param userData The file discriptor of output file, its type should be converted from FILE* to char* by the caller in `pcap_loop`.
 * @param pkthdr Generic per-packet information, as supplied by libpcap.
 * @param packet The content of a layer-2 packet.
 */
void packetHandler(u_char *userData, const struct pcap_pkthdr* pkthdr, const u_char* packet)
{
    FILE *fp = (FILE *)userData;
    char tuple_buff[13];
    char str_buff[100];
    int parse_state = 0;
    int parse_offset = 0;

    while (parse_state != PARSE_END)
    {
        switch (parse_state)
        {
        case PARSE_START:
        {
            parse_state = PARSE_ETHER;
            break;
        }
        case PARSE_ETHER:
        {
            const ether_hdr *ether = (ether_hdr *)(packet + parse_offset);
            parse_offset += 14;
            if (ntohs(ether->ether_type) == ETHER_TYPE_IPV4)
                parse_state = PARSE_IPV4;
            else if (ntohs(ether->ether_type) == ETHER_TYPE_IPV6)
                parse_state = PARSE_IPV6;
            else if (ntohs(ether->ether_type) == ETHER_TYPE_VLAN)
                parse_state = PARSE_VLAN;
            else
                parse_state = PARSE_END;
            break;
        }
        case PARSE_VLAN:
        {
            const vlan_hdr *vlan = (vlan_hdr *)(packet + parse_offset);
            if (VERBOSE)
                printf("VlanID=%04d   ", ntohs(vlan->vlan_info) & 0x0FFF);
            parse_offset += 4;
            if (ntohs(vlan->ether_type) == ETHER_TYPE_IPV4)
                parse_state = PARSE_IPV4;
            else if (ntohs(vlan->ether_type) == ETHER_TYPE_IPV6)
                parse_state = PARSE_IPV6;
            else
                parse_state = PARSE_END;
            break;
        }
        case PARSE_IPV4:
        {
            const ipv4_hdr *ipv4 = (ipv4_hdr *)(packet + parse_offset);
            if (VERBOSE)
            {
                sprintf(str_buff, "%d.%d.%d.%d", (int)ipv4->src_ip[0], (int)ipv4->src_ip[1], (int)ipv4->src_ip[2], (int)ipv4->src_ip[3]);
                printf("IP.src=%-15s   ", str_buff);
                sprintf(str_buff, "%d.%d.%d.%d", (int)ipv4->dst_ip[0], (int)ipv4->dst_ip[1], (int)ipv4->dst_ip[2], (int)ipv4->dst_ip[3]);
                printf("IP.dst=%-15s  ", str_buff);
            }

            memcpy(tuple_buff + 0, (char *)(ipv4->src_ip), 4);
            memcpy(tuple_buff + 4, (char *)(ipv4->dst_ip), 4);
            memcpy(tuple_buff + 12, (char *)&(ipv4->protocol), 1);
            
            parse_offset += 20;
            if (ipv4->protocol == 6)
                parse_state = PARSE_TCP;
            else if (ipv4->protocol == 17)
                parse_state = PARSE_UDP;
            else
                parse_state = PARSE_END;
            break;
        }
        case PARSE_IPV6:
        {
            parse_state = PARSE_END;
            break;
        }
        case PARSE_TCP:
        {
            const tcp_hdr *tcp = (tcp_hdr *)(packet + parse_offset);
            if (VERBOSE)
                printf("TCP.src=%-5d  TCP.dst=%-5d\n", (int)ntohs(tcp->src_port), (int)ntohs(tcp->dst_port));

            memcpy(tuple_buff + 8, (char *)&(tcp->src_port), 2);
            memcpy(tuple_buff + 10, (char *)&(tcp->dst_port), 2);
            
            parse_offset += 20;
            parse_state = PARSE_END;
            break;
        }
        case PARSE_UDP:
        {
            const udp_hdr *udp = (udp_hdr *)(packet + parse_offset);
            if (VERBOSE)
                printf("UDP.src=%-5d  UDP.dst=%-5d\n", (int)ntohs(udp->src_port), (int)ntohs(udp->dst_port));

            memcpy(tuple_buff + 8, (char *)&(udp->src_port), 2);
            memcpy(tuple_buff + 10, (char *)&(udp->dst_port), 2);
            
            parse_offset += 8;
            parse_state = PARSE_END;
            break;
        }
        case PARSE_END:
            break;
        default:
            break;
        }
    }
    if (CONCISE_INFO)
    {
        packet_cnt++;
        if (!(packet_cnt % 100000))
            printf("%d\n", packet_cnt);
    }
    fwrite(tuple_buff, 13, 1, fp);
}