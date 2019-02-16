#pragma once

#include "UI/ProcCommunicator.h"

#include <chrono>
#include <thread>

#define PROC_RESET_PATH "/proc/logs_proc"
#define DEFAULT_TIMER_TIME 20
#define DEFAULT_RESET_HOSTS_TIME 2

namespace UI
{   
    class ThreadManager
    {
    private:
        int _seconds_count;    
        bool _activated;
        int _timer;
        int _reset_time;
        int _reset_suspicious_time;
        int _reset_suspicious_timer;

    public:
        ThreadManager();
        ~ThreadManager();
        void CountSeconds();
        int GetSeconds();
        void SendReset();
        void SetResetTime(int minutes);
        void ResetSuspiciousHosts();
        
    };
}
