#pragma once

#include <iostream>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <arpa/inet.h>

#define TCP 6
#define UDP 17

struct PacketData
{
    std::string srcIp;
    std::string dstIp;
    unsigned int srcPort;
    unsigned int dstPort;
    std::string protocol;
    std::string url;
    bool incomming;
    char * data;
};

class Packet
{
 public:
    Packet(PacketData* data);
    ~Packet();
    
    std::string GetSrcIp();
    std::string GetDstIp();
    int GetSrcPort();
    int GetDstPort();
    std::string GetProtocol();
    std::string GetUrl();
    bool IsIncomming();

    static PacketData* ExtractData(unsigned char* payloadData, bool incoming);
    
 protected:
    PacketData* _data;
};

