#pragma once

#include "IncomingQueue.h"
#include "OutgoingQueue.h"

#include <string>
#include <thread>

#define INCOMING_QUEUE_NUM 9
#define OUTGOING_QUEUE_NUM 10
#define MSG_START_FILTERING "00"
#define PIPE_PATH "/tmp/myfifo"

class PipeManager
{
 public:
    PipeManager();
    ~PipeManager();

    void Run();
    void StartFiltering();
    void AddUrl(const std::string& msg);
    void RemoveUrl(const std::string& msg);
    void StopFiltering();
    void Restart();
    void Close();

 private:
    IncomingQueue _incoming;
    OutgoingQueue _outgoing;
};
