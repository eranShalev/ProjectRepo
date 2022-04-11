#pragma once

#include "PacketQueue.h"

class IncomingQueue : public PacketQueue
{
public:
    IncomingQueue(int id);
    ~IncomingQueue();

    static int CallBackFunc(struct nfq_q_handle* qh, struct nfgenmsg* nfmsg, struct nfq_data* nfa, void* param);
    void InitializeQueue();
};
