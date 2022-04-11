#include "UserSpace/PacketQueue.h"

#include <iostream>
#include <fstream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

Rules PacketQueue::_rules;
std::mutex PacketQueue::_socketLock;
std::shared_timed_mutex PacketQueue::ruleLock;
std::set<std::pair<std::string, int>> PacketQueue::sockets;

PacketQueue::PacketQueue(int id)
    : _id(id)
{
    _key.lock();
}

PacketQueue::~PacketQueue()
{
}

void PacketQueue::WriteRule(int dstPort, int homePort, std::string destIp)
{
    std::fstream proc;
    
    if (access (PROC_FILE_NAME, F_OK) == -1)
    {
        std::cout << "Error: file doesnt exist" << std::endl;
        return;
    }

    proc.open(PROC_FILE_NAME);
     
    std::string msg = std::to_string(homePort) + "," + std::to_string(dstPort) + "," + destIp;
    
    if (proc.is_open())
    {
        proc << msg;
        proc.close();
        
        //std::cout << "wrote to proc: " << msg << std::endl;
        return;
    }
}

void PacketQueue::DestroyQueue()
{
    nfq_destroy_queue(queue_handle);
    nfq_close(library_handle);
    //std::cout << "finished destroying queue id: " << _id << std::endl;
}

void PacketQueue::FilterPackets()
{
    int rv;

    //std::cout << "started filtering packets in queue id: " << _id << std::endl;
    
    while (true)
    {
        _key.lock();
        rv = recv(_fd, _buf, sizeof(_buf), 0);
        nfq_handle_packet(library_handle, _buf, rv);
        _key.unlock();
    }
}

void PacketQueue::StartFilter()
{
    //std::cout << "unlocked Queue num: " << _id << std::endl;
    _key.unlock();
}

bool PacketQueue::StopFilter()
{
    //std::cout << "locked Queue num: " << _id << std::endl;
    return _key.try_lock();
}

void PacketQueue::PrintSet()
{
    for (auto it : sockets)
    {
        std::cout << "Ip: " << it.first << " and Port: " << std::to_string(it.second) << std::endl;
    }
}

std::string PacketQueue::GetHostNameFromIp(std::string ipAddr, int port)
{
    struct sockaddr_in sa;
    char hostname[128] = "";
    
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = inet_addr(ipAddr.c_str());
    
    getnameinfo((struct sockaddr *)(&sa), sizeof(sa), hostname, sizeof(hostname), NULL, 0, NI_NAMEREQD);
    return hostname;
}
