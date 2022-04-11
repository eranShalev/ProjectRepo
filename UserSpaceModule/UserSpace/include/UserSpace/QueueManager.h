#include "IncomingQueue.h"

class QueueManager
{
 public:
    QueueManager();
    ~QueueManager();

    void InitQueues();
    void DelQueues();
    void StopQueues();
    void StartQueues();
    
 private:
    IncomingQueue _incoming_queue;
    OutgoingQueue _outgoing_queue;
};
