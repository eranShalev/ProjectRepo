#include "UserSpace/Packet.h"

Packet::Packet(PacketData* data)
    : _data(data)
{
    
}

Packet::~Packet()
{
}

PacketData* Packet::ExtractData(unsigned char* payloadData, bool incoming)
{
    in_addr addr;
    iphdr* ip_hdr = nullptr;
    tcphdr* tcp_hdr;
    udphdr* udp_hdr;

    PacketData* data = new PacketData();
    
    ip_hdr = ((struct iphdr*)payloadData);

    if (ip_hdr != nullptr)
    {
        addr.s_addr = ip_hdr->saddr;
        data->srcIp = inet_ntoa(addr);
        addr.s_addr = ip_hdr->daddr;
        data->dstIp = inet_ntoa(addr);
        data->data = nullptr;

        data->incomming  = incoming;
    
        if(ip_hdr->protocol == TCP)
        {
            data->protocol = "TCP";
            tcp_hdr = (struct tcphdr*)(payloadData + (ip_hdr->ihl << 2));
            data->srcPort = (unsigned int)ntohs(tcp_hdr->source);
            data->dstPort = (unsigned int)ntohs(tcp_hdr->dest);
        
            data->data = (char *)((unsigned char *)tcp_hdr + (tcp_hdr->doff * 4));
        }
        else if(ip_hdr->protocol == UDP)
        {
            data->protocol = "UDP";
            udp_hdr = (struct udphdr*)(payloadData + (ip_hdr->ihl << 2));
            data->srcPort = (unsigned int)ntohs(udp_hdr->source);
            data->dstPort = (unsigned int)ntohs(udp_hdr->dest);
        }
        else
        {
            data->protocol = "else";        
        }    
    }
    else
    {
        delete data;
        data = nullptr;
    }
    return data;
}

std::string Packet::GetSrcIp()
{
    return _data->srcIp;
}

std::string Packet::GetDstIp()
{
    return _data->dstIp;
}

int Packet::GetSrcPort()
{
    return _data->srcPort;
}

int Packet::GetDstPort()
{
    return _data->dstPort;
}

std::string Packet::GetProtocol()
{
    return _data->protocol;
}

bool Packet::IsIncomming()
{
    return _data->incomming;
}
