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
        std::cout << "writring to Kernel: " << message << std::endl;
        ProcCommunicator::WriteProc(message, PROC_RESET_PATH);
    }

    void ThreadManager::ResetSuspiciousHosts()
    {
        std::string message = "reset_hosts";
        std::cout << "writring to Kernel: " << message << std::endl;
        ProcCommunicator::WriteProc(message, PROC_RESET_PATH);
    }
}
