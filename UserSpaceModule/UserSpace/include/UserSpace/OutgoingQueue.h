#pragma once

#include "PacketQueue.h"

class OutgoingQueue : public PacketQueue
{
 public:
    OutgoingQueue(int id);
    ~OutgoingQueue();

    static int CallBackFunc(struct nfq_q_handle* qh, struct nfgenmsg* nfmsg, struct nfq_data* nfa, void* param);
    void InitializeQueue();
};
