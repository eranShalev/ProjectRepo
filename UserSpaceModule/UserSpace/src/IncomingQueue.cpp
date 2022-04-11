#include "UserSpace/IncomingQueue.h"
#include "UserSpace/HttpPacket.h"

#include <iostream>
#include <memory>

IncomingQueue::IncomingQueue(int id)
    : PacketQueue(id)
{    
}

IncomingQueue::~IncomingQueue()
{   
}

int IncomingQueue::CallBackFunc(struct nfq_q_handle* qh, struct nfgenmsg* nfmsg, struct nfq_data* nfa, void* param)
{
    u_int32_t id;
    unsigned char* payload_data;
    struct nfqnl_msg_packet_hdr *ph;
   
    ph = nfq_get_msg_packet_hdr(nfa);
    id = ntohl(ph->packet_id);
    
    nfq_get_payload(nfa, &payload_data);

    PacketData* data = Packet::ExtractData(payload_data, true);
        
    /*try
    {
        std::unique_ptr<HttpPacket> httpPacket(new HttpPacket(data));
        std::shared_lock<std::shared_timed_mutex> guardRules(PacketQueue::ruleLock);
        
        if (httpPacket->GetSrcPort() == 443)
        {
            std::cout << "host is: " << PacketQueue::GetHostNameFromIp(httpPacket->GetSrcIp(), httpPacket->GetSrcPort()) << std::endl;
            }
        
        if (httpPacket->GetHostName() != "no host" && PacketQueue::_rules.Check(httpPacket->GetHostName()))
        {
            std::lock_guard<std::mutex> guardSockets(_socketLock);
            
            if (sockets.find(std::pair<std::string, int>(httpPacket->GetSrcIp(),httpPacket->GetSrcPort())) == sockets.end())
            {
                sockets.insert(std::pair<std::string, int>(httpPacket->GetSrcIp(), httpPacket->GetSrcPort()));
                PacketQueue::WriteRule(httpPacket->GetDstPort(), httpPacket->GetSrcPort(), httpPacket->GetSrcIp());
            }
            
            return nfq_set_verdict(qh, id, NF_DROP, 0, NULL);
        }
    }
    catch (const std::runtime_error& e)
    {
        std::unique_ptr<Packet> packet(new Packet(data));
    }*/
    
    if (data != nullptr)
    {
        delete data;
    }
    
    return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);        
}

void IncomingQueue::InitializeQueue()
{
    library_handle = nfq_open();

    if (!library_handle)
    {
        //TODO: throw exception
        //std::cout << "Couldnt Initialize Library Handle in Queue Num: " << _id << std::endl;
        return;
    }

    _fd = nfq_fd(library_handle);

    queue_handle = nfq_create_queue(library_handle, _id, &CallBackFunc, NULL);

    if (!queue_handle)
    {
        //TODO: throw exception
        //std::cout << "Couldnt Initialize Queue Handle in Queue NUm: " << _id << std::endl;
        return;
    }

    if (nfq_set_mode(queue_handle, NFQNL_COPY_PACKET, 0xffff) < 0)
    {
        //TODO: throw exception
        //std::cout << "couldnt set mode of Queue num: " << _id << std::endl;
        return;
    }

}
