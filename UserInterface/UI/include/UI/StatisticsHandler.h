#pragma once

#include "UI/Helper.h"
#include "UI/ProcCommunicator.h"
#include "UI/ThreadManager.h"

#include <ctime>
#include <fstream>
#include <string>
#include <vector>

#define COUNTER_PROC_PATH "/proc/firewall_counter"
#define DATA_FLOW_MESSAGE "10"
#define GENERAL_RULES_MESSAGE "11"
#define SPECIFIC_RULE_MESSAGE "12 "
#define ERROR_ID_NOT_FOUND "Error: no rule with this id exists"

namespace UI
{
    class StatisticsHandler
    {
    public:
        StatisticsHandler();
        ~StatisticsHandler();

        bool SetStatistics(std::string stats);
        std::string GetDataFlowStatistics();
        std::string GetGeneralRulesStatistics();
        std::string GetSpecificRuleStatistics();
        std::string GetRuleStatistics(std::string id);
        std::string Help();
        void ParseDataFlow(std::string& stats);
        void ParseGeneralStatistics(std::string& stats);
        void ParseSpecifcRuleStatistics(std::string& stats);
        bool isRunning();
        std::string Start();
        std::string Exit();
        bool getId(std::vector<std::string>& id_list);
        void UpdateTime(time_t new_time);
    
        ThreadManager _threadManager;
    
    private:
        bool _running;
        float _multiplier;
        time_t _time;                       
    
    };    
}
