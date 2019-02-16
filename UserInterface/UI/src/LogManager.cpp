#include "UI/LogManager.h"
#include <fstream>

namespace UI
{
    LogManager::LogManager() { }

    LogManager::~LogManager() { }

    void LogManager::ParseLogs(std::string& logs)
    {
        std::vector<std::string> data;
    
        Helper::ParseByChar(logs, ',', data);

        logs = ("\nSource Ip: " + data[0] + "\ndest Ip: " + data [1] + "\n");
        logs += ("Source Port: " +data[2] + "\ndest Port: " + data[3] + "\n");
        logs += ("packet is : " + data[4] + "\nPacket Status: " + data[5]);
        logs += "\n";

        for (unsigned int i = 6; i < data.size(); i++)
        {
            logs += data[i] + "\n";
        }
    }

    std::string LogManager::ShowLogs()
    {
        std::string logs;
        std::ifstream proc(LOG_PROC_PATH);
        std::string sortedLogs = "Logs Captured:\n";
    
        if (!proc.good())
        {
            return ERR_FW_NOT_RUNNING;
        }

        while (std::getline(proc, logs))
        {
            proc >> logs;
        
            if (logs.find(",") == std::string::npos)
            {
                return sortedLogs;
            }
        
            ParseLogs(logs);
            sortedLogs += logs;
        }
    
        proc.close();

        return sortedLogs;
    }
}
