#pragma once

#include "UI/ProcCommunicator.h"
#include "UI/UrlHandler.h"

#include <iostream>
#include <string>

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
        std::string GetPacketSource(std::string& msg);
        std::string GetPacketParameter(std::vector<std::string>& msg);
        bool CheckPort(std::string& msg);
        bool CheckIp(std::string& msg);
        bool CheckProtocol(std::string& msg);
        std::string GetPacketType(std::string& msg);
        int ExtractId(const std::string& s);
        std::string Add(std::vector<std::string>& commandParts);
        std::string Remove(std::string removeRule);
        std::string Help();
        std::string Clean(std::string command);
        std::vector<std::string> InitializeRules();
        std::vector<std::string> GetUrls();
        bool Active();
        std::string AddUrl(std::vector<std::string>& commandParts);
        std::string RemoveUrl(std::vector<std::string>& commandParts);
        std::string PrintUrl();
        void SetPath(bool isGUI);
        
    private:
        bool _isActive;
        UrlHandler _urlHandler;
        static std::string path;
    };
}


