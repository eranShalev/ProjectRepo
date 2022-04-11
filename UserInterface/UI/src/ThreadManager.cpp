#include "UI/ThreadManager.h"

#include <iostream>

namespace UI
{
    ThreadManager::ThreadManager()
    {
    
    }

    ThreadManager::~ThreadManager()
    {
    
    }

    void ThreadManager::SendReset()
    {
        std::string message = "reset";
        ProcCommunicator::WriteProc(message, PROC_RESET_PATH);
    }

    void ThreadManager::ResetSuspiciousHosts()
    {
        std::string message = "reset_hosts";
        ProcCommunicator::WriteProc(message, PROC_RESET_PATH);
    }
}
