#pragma once

#include "UI/Helper.h"
#include "UI/ProcCommunicator.h"


#include <iostream>
#include <string>
#include <vector>

#define LOG_PROC_PATH "/proc/logs_proc"
#define ERR_FW_NOT_RUNNING "Error: firewall not running"

namespace UI
{
    class LogManager
    {
    public:
        LogManager();
        ~LogManager();

        static void ParseLogs(std::string& logs);
        static std::string ShowLogs();
    };
}
