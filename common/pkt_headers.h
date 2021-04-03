/**
 * @file pkt_headers.h
 * @author QH2333 (qi_an_hao@126.com)
 * @brief 
 * @version 0.1
 * @date 2021-04-03
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#pragma once

#include <cstdint>

#define ETHER_TYPE_IPV4 0x0800
#define ETHER_TYPE_IPV6 0x86DD
#define ETHER_TYPE_VLAN 0x8100

typedef struct ether_hdr
{
    uint8_t dst_mac[6];
    uint8_t src_mac[6];
    uint16_t ether_type;
} ether_hdr;

typedef struct vlan_hdr
{
    uint16_t vlan_info;
    uint16_t ether_type : 16;
} vlan_hdr;

typedef struct ipv4_hdr
{
    uint8_t __padding__[9];
    uint8_t protocol;
    uint16_t checksum;
    uint8_t src_ip[4];
    uint8_t dst_ip[4];
} ipv4_hdr;

typedef struct ipv6_hdr
{
    uint8_t __padding__[8];
    uint8_t src_ip[16];
    uint8_t dst_ip[16];
} ipv6_hdr;

typedef struct tcp_hdr
{
    uint16_t src_port;
    uint16_t dst_port;
    uint8_t __padding__[16];
} tcp_hdr;

typedef struct udp_hdr
{
    uint16_t src_port;
    uint16_t dst_port;
    uint8_t __padding__[4];
} udp_hdr;