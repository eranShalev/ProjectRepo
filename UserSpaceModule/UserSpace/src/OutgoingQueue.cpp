#include "UserSpace/OutgoingQueue.h"
#include "UserSpace/HttpPacket.h"

#include <iostream>
#include <memory>

OutgoingQueue::OutgoingQueue(int id)
    : PacketQueue(id)
{
}

OutgoingQueue::~OutgoingQueue()
{   
}


int OutgoingQueue::CallBackFunc(struct nfq_q_handle* qh, struct nfgenmsg* nfmsg, struct nfq_data* nfa, void* param)
{
    u_int32_t id;
    unsigned char* payload_data;
    struct nfqnl_msg_packet_hdr *ph;
   
    ph = nfq_get_msg_packet_hdr(nfa);
    id = ntohl(ph->packet_id);
    
    nfq_get_payload(nfa, &payload_data);    

    PacketData* data = Packet::ExtractData(payload_data, false);
    //TODO:: read about auto_ptr
    if (data != nullptr)
    {
        try
        {
            std::unique_ptr<HttpPacket> httpPacket(new HttpPacket(data));
            std::shared_lock<std::shared_timed_mutex> guardRules(PacketQueue::ruleLock);
            
            /*if (httpPacket->GetDstPort() == 443)
            {
                std::cout << "host is: " << PacketQueue::GetHostNameFromIp(httpPacket->GetDstIp(), httpPacket->GetDstPort()) << std::endl;
                }*/
            
            if ((httpPacket->GetHostName() != "no host") && PacketQueue::_rules.Check(httpPacket->GetHostName()))
            {
                std::lock_guard<std::mutex> guardSockets(_socketLock);
                
                //std::cout << "blocked url is: " << httpPacket->GetHostName() << std::endl;
                
                if (sockets.find(std::pair<std::string, int>(httpPacket->GetDstIp(),httpPacket->GetDstPort())) == sockets.end())
                {
                    sockets.insert(std::pair<std::string, int>(httpPacket->GetDstIp(), httpPacket->GetDstPort()));
                    PacketQueue::WriteRule(httpPacket->GetSrcPort(), httpPacket->GetDstPort(), httpPacket->GetDstIp());
                }
                                
                return nfq_set_verdict(qh, id, NF_DROP, 0, NULL);
            }
        }
        catch (const std::runtime_error& e)
        {                
            std::unique_ptr<Packet> packet(new Packet(data));            
        }
        
        if (data != nullptr)
        {
            delete data;
        }    
    }        
    return nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
}

void OutgoingQueue::InitializeQueue()
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

