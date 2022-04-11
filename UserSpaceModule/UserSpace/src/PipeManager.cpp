#include "UserSpace/PipeManager.h"
#include <string.h>

#include <iostream>
#include <memory>

PipeManager::PipeManager()
    : _incoming(INCOMING_QUEUE_NUM), _outgoing(OUTGOING_QUEUE_NUM)
{
}

PipeManager::~PipeManager()
{    
}

void PipeManager::Run()
{
    std::string msg = "";
    char pipe_msg[80] = "";
    int fd;
    
    _incoming.InitializeQueue();
    _outgoing.InitializeQueue();

    std::thread incomingThread(&PacketQueue::FilterPackets, &_incoming);
    std::thread outgoingThread(&PacketQueue::FilterPackets, &_outgoing);
    
    incomingThread.detach();
    outgoingThread.detach();
    
    while (msg != "01")
    {
        memset(pipe_msg, 0, sizeof(pipe_msg));
        fd = open(PIPE_PATH, O_RDONLY);

        if (fd == -1)
        {
            Close();
            std::cout << "file descriptor is wrong" << std::endl;
            std::cout << PIPE_PATH << std::endl;
            return;
        }
        
        read(fd, pipe_msg, 80);
        //std::cout << "message from pipe is : " << pipe_msg << std::endl; 
        msg = pipe_msg;        
        close(fd);
        
        if (msg == "00")
        {
            StartFiltering();
        }
        if (msg == "01")
        {            
            Close();
        }
        else if (msg ==  "02")
        {            
            Restart();
        }
        else if (msg == "03")
        {
            StopFiltering();
        }
        else if (msg.substr(0, 2) == "07")
        {
            AddUrl(msg);
        }
        else if (msg.substr(0, 2) == "08")
        {
            RemoveUrl(msg);
        }
    }    
}

void PipeManager::StartFiltering()
{
    _incoming.StartFilter();
    _outgoing.StartFilter();
}

void PipeManager::AddUrl(const std::string& msg)
{
    std::lock_guard<std::shared_timed_mutex> guardRules(PacketQueue::ruleLock);
    PacketQueue::_rules.AddUrl(msg.substr(3));
    //PacketQueue::_rules.PrintUrls();
}

void PipeManager::RemoveUrl(const std::string& msg)
{
    std::lock_guard<std::shared_timed_mutex> guardRules(PacketQueue::ruleLock);
    PacketQueue::_rules.DelUrl(msg.substr(3));
    //PacketQueue::_rules.PrintUrls();
}

void PipeManager::StopFiltering()
{
    _incoming.StopFilter();
    _outgoing.StopFilter();
}

void PipeManager::Restart()
{
    //Delete all Url Rules;
}

void PipeManager::Close()
{
    _incoming.StopFilter();
    _outgoing.StopFilter();
    _incoming.DestroyQueue();
    _outgoing.DestroyQueue();    
}
