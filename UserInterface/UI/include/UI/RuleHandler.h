#pragma once

#include "UI/ProcCommunicator.h"
#include "UI/UrlHandler.h"

#include <string>

#define FILE_NAME "rule_file.txt"
#define PROC_PATH "/proc/my_proc"

namespace UI
{
    class RuleHandler
    {
    public:
        RuleHandler();
        ~RuleHandler();
    
        std::string Rules();
        std::string Exit();
        static std::string Print();
        std::string GetPacketSource();
        std::string GetPacketParameter();
        std::string GetPort();
        std::string GetIp();
        std::string GetProtocol();
        std::string GetPacketType();
        int ExtractId(const std::string& s);
        std::string Add();
        std::string Remove(std::string removeRule);
        std::string Help();
        std::string Clean(std::string command);
        std::vector<std::string> InitializeRules();
        bool Active();
        std::string AddUrl();
        std::string RemoveUrl();
        std::string PrintUrl();
    
    private:
        bool _isActive;
        UrlHandler _urlHandler;
    };
}

