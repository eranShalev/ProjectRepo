#pragma once

#include "Rules.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/types.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <mutex>
#include <shared_mutex>
#include <set>
#include <string>

#define PROC_FILE_NAME "/proc/user_space_proc"

class PacketQueue 
{
 public:
    PacketQueue(int id);
    ~PacketQueue();
        
    void FilterPackets();
    void StartFilter();
    bool StopFilter();
    virtual void InitializeQueue() = 0;
    void DestroyQueue();
    
    static std::string GetHostNameFromIp(std::string ipAddr, int port);
    static void PrintSet();
    static void WriteRule(int dstPort, int homePort, std::string destIp);
    
    static Rules _rules;
    static std::set<std::pair<std::string, int>> sockets;
    
    
    static std::shared_timed_mutex ruleLock;
    
 protected:    
    struct nfq_handle* library_handle;
    struct nfq_q_handle* queue_handle;
    std::mutex _key;
    static std::mutex _socketLock;
    
    char _buf[4096] __attribute__ ((aligned));
    int _fd;
    int _id;

};
