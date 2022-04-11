#include "UI/StatisticsHandler.h"

#include <iostream>

namespace UI
{
    StatisticsHandler::StatisticsHandler() :_running(false), _multiplier(1.0/100000)
                                           ,_time(time(nullptr))
    {
    }

    StatisticsHandler::~StatisticsHandler()
    {
    }

    void StatisticsHandler::UpdateTime(time_t new_time)
    {
        _time = new_time;
    }

    std::string StatisticsHandler::Exit()
    {
        _running = false;
        return "Exiting Statistics Interface";
    }

    std::string StatisticsHandler::Start()
    {
        _running = true;
        return "Entered Statistics Interface";
    }

    bool StatisticsHandler::isRunning()
    {
        return _running;
    }

    void StatisticsHandler::getId(std::vector<std::string>& id_list, std::string min, std::string max)
    {
        for (int i = std::stoi(min); i <=std::stoi(max); i++)
        {
            id_list.push_back(std::to_string(i));
        }
    }

    void StatisticsHandler::ParseDataFlow(std::string& stats)
    {
        std::vector<std::string> data;   
    
        Helper::ParseByChar(stats, ',' ,data);

        double seconds = difftime(time(nullptr), _time);
    
        if (std::stoi(data[0]) == 0)
        {
            stats = "\nno packets captured\n";
        }
        else
        {
            stats = "\nAmount of packets: " + data[0] + "\n";
            stats += "Incoming Packets: " + data[1] + " ---> " + std::to_string( 100 * std::stoi(data[1]) / std::stoi(data[0])) + "%\n";
            stats += "Outoing Packets: " + data[2] + " ---> " + std::to_string( 100 * std::stoi(data[2]) / std::stoi(data[0])) + "%\n";
            stats += "Accepted Packets: " + data[3] + " ---> " + std::to_string( 100 * std::stoi(data[3]) / std::stoi(data[0])) + "%\n";
            stats += "Dropped Packets: " + data[4] + " ---> " + std::to_string( 100 * std::stoi(data[4]) / std::stoi(data[0])) + "%\n";
            stats += "Accepted Incoming Packets: " + data[5] + "\n";
            stats += "Dropped Incoming Packets: " + data[6] + "\n";
            stats += "Accepted Outgoing Packets: " + data[7] + "\n";
            stats += "Dropped Outgoing Packets: " + data[8] + "\n";
            stats += "Incmoming Data Speed (Mb/s): " +  std::to_string((float)(std::stoi(data[9]) * _multiplier / seconds)) + "\n";
            stats += "Outgoing Data Speed (Mb/s): " + std::to_string((float)(std::stoi(data[10]) * _multiplier / seconds)) + "\n";
        }
    
    }

    void StatisticsHandler::ParseGeneralStatistics(std::string& stats)
    {
        std::vector<std::string> data;

        Helper::ParseByChar(stats, ',' ,data);

        int packets_filtered = std::stoi(data[0]) + std::stoi(data[1]);
    
        if (packets_filtered == 0)
        {
            stats = "\nAmount of packets filterd by rules: 0\n";
        }
        else
        {
            stats = "\nAmount of packet filtered by rules :" + std::to_string(packets_filtered) + "\n";
            stats += "Packets Accepted: " + data[0] + " ---> " + std::to_string(std::stoi(data[0]) * 100 / packets_filtered) + "%\n";
            stats += "Packets dropped: " + data[1] + " ---> " + std::to_string(std::stoi(data[1]) * 100 / packets_filtered) + "%\n";
            stats += "accepted for good ip: " + data[2] + " ---> " + std::to_string(std::stoi(data[2]) * 100 / packets_filtered) + "%\n";
            stats += "dropped for bad ip: " + data[3] + " ---> " + std::to_string(std::stoi(data[3]) * 100 / packets_filtered) + "%\n";
            stats += "accepted for good port: " + data[4] + " ---> " + std::to_string(std::stoi(data[4]) * 100 / packets_filtered) + "%\n";
            stats += "dropped for bad port: " + data[5] + " ---> " + std::to_string(std::stoi(data[5]) * 100 / packets_filtered) + "%\n";
            stats += "accepted for good protocol: " + data[6] + " ---> " + std::to_string(std::stoi(data[6]) * 100 / packets_filtered) + "%\n";
            stats += "dropped for bad protocol: " + data[7] + " ---> " + std::to_string(std::stoi(data[7]) * 100 / packets_filtered) + "%\n";     
        }
    }

    void StatisticsHandler::ParseSpecifcRuleStatistics(std::string& stats)
    {
        std::vector<std::string> data;

        Helper::ParseByChar(stats, ',' ,data);

        if (std::stoi(data[0]) == 0)
        {
            stats = "\nAmount of packets filtered by rule: 0\n";
        }
        else
        {
            stats = "\nAmount of packets filtered by rule: " + data[0] + "\n";
            stats += "Influence of rule on network: " + std::to_string(100 * std::stoi(data[0]) / std::stoi(data[1])) +"%\n";
            stats += "Influence of rule on filtered network: " + std::to_string(100 * std::stoi(data[0]) / std::stoi(data[2])) + "%\n";
            stats += "Rule ranking: " + data[3] + "/" + data[4] + "\n";
        }
        
    }

    bool StatisticsHandler::SetStatistics(std::string stats)
    {
        return ProcCommunicator::WriteProc(stats, COUNTER_PROC_PATH);
    }

    std::string StatisticsHandler::GetRuleStatistics(std::string id)
    {
        std::fstream proc;
        std::string line = "";
        std::string message = SPECIFIC_RULE_MESSAGE;
    
        if(!SetStatistics(message + id))
        {
            return "Error: firewall not running";
        }
    
        proc.open(COUNTER_PROC_PATH);    
    
        if (proc.is_open())
        {
            while(!proc.eof())
            {
                proc >> line;
            }

            proc.close();
        }

        if (line == "exists")
        {
            return "Error: no rule with this id exists";
        }

        ParseSpecifcRuleStatistics(line);
    
        return line;
    }

    std::string StatisticsHandler::GetDataFlowStatistics()
    {
        std::fstream proc;
        std::string data = "";    
    
        if (!SetStatistics(DATA_FLOW_MESSAGE))
        {
            return "Error: firewall not running";
        }
    
        proc.open(COUNTER_PROC_PATH);

        if (proc.is_open())
        {
            while(std::getline(proc, data))
            {
                proc >> data;
            }

            proc.close();
        }

        ParseDataFlow(data);
    
        return data;
    }

    std::string StatisticsHandler::GetGeneralRulesStatistics()
    {
        std::fstream proc;
        std::string line = "";

        if(!SetStatistics(GENERAL_RULES_MESSAGE))
        {
            return "Error: firewall not running";
        }
    
        proc.open(COUNTER_PROC_PATH);

        if (proc.is_open())
        {
            while(!proc.eof())
            {
                proc >> line;
            }

            proc.close();
        }

        ParseGeneralStatistics(line);
    
        return line;
    }

    std::string StatisticsHandler::GetSpecificRuleStatistics(std::vector<std::string>& commandParts)
    {
        std::vector<std::string> id_list;
        std::string message = "";
        std::string retMsg = "";
    
        if (commandParts.size() == 3 && Helper::IsNumber(commandParts[1]) && Helper::IsNumber(commandParts[2]))
        {
            if (std::stoi(commandParts[1]) > std::stoi(commandParts[2]))
            {
                return "aborted action";
            }
            
            getId(id_list, commandParts[1], commandParts[2]);
            
            for (std::string id : id_list)
            {
                message = GetRuleStatistics(id);
                
                if (message == "Error: firewall not running")
                {
                    retMsg = "\n" + message + "\n";
                    break;
                }

                if (message != ERROR_ID_NOT_FOUND)
                {
                    retMsg += "\nstats of rule: " + id = "\n";
                    retMsg += message;
                }
                else if (retMsg == "")
                {
                    return message;
                }
                else
                {
                    return retMsg;
                }
            }
            return retMsg;
        }
        return "Aborted Action";
    }

    std::string StatisticsHandler::Help()
    {
        return "\nStatistics options:\ndata - shows data flow statistics\ngeneral - shows general rules statistics\nrule [x-y] - shows statistics about a specific range of rules\nExit - exits the statistics Interface";
    }   
}
