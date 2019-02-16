#include "UI/RuleHandler.h"
#include "UI/Helper.h"

#include <arpa/inet.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace UI
{
    RuleHandler::RuleHandler() : _isActive(false)
    {
    }
    
    RuleHandler::~RuleHandler()
    {
    }

    std::string RuleHandler::GetPort()
    {
        std::string port;
        while (true)
        {
            std::cout << "Enter Port number: ";
            std::getline(std::cin, port);
        
            if (Helper::IsNumber(port))
            {
                if (std::stoi(port) < 65535)
                {
                    return port;
                }
            }

            std::cout << "Invalid Port Number!" << std::endl;
        }
    
    }

    std::string RuleHandler::GetIp()
    {
        std::string ip;
        while (true)
        {
            std::cout << "Enter IP address: ";
            std::getline(std::cin, ip);
        
            struct sockaddr_in sa;
            if (inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr)) == 1)
            {
                return ip;
            }
            std::cout << "Invalid IP address" << std::endl;
        }
    
    }

    std::string RuleHandler::GetProtocol()
    {
        std::string protocol;
    
        while (true)
        {
            std::cout << "Enter Protocol (TCP/UDP): ";
            std::getline(std::cin, protocol);
        
            if (protocol == "TCP" || protocol == "UDP")
            {
                return protocol;
            }
        
            std::cout << "Invalid protocol" << std::endl;
        }
    
    }

    std::string RuleHandler::GetPacketSource()
    {
        std::string decision;
    
        while (true)
        {
            std::cout << "Filter entering packets? (y/n/ to cancel press c)" << std::endl;

            std::getline(std::cin, decision);

            if (decision == "y")
            {
                return "incoming,";
            }
            else if (decision == "n")
            {
                return "outgoing,";
            }
            else if (decision == "c")
            {
                return "exit";
            }
        }
    }

    std::string RuleHandler::GetPacketParameter()
    {
        std::string decision;
    
        while (true)
        {
            std::cout << "Choose packet parameter (dip/sip/dport/sport/protocol/ to cancel press c)" << std::endl;

            std::getline(std::cin, decision);

            if (decision == "dip")
            {
                return "dip="+GetIp()+",";
            }
            else if (decision == "sip")
            {
                return "sip="+GetIp()+",";
            }
            else if (decision == "dport")
            {
                return "dport="+GetPort()+",";
            }
            else if (decision == "sport")
            {
                return "sport="+GetPort()+ ",";
            }
            else if (decision == "protocol")
            {
                return "protocol=" + GetProtocol() + ",";
            }
            else if (decision == "c")
            {
                return "exit";
            }
        }
    }

    std::string RuleHandler::GetPacketType()
    {
        std::string decision;
    
        while(true)
        {
            std::cout << "Accept Packet? (y/n/ to cancel press c)" << std::endl;

            std::getline(std::cin, decision);

            if (decision == "y")
            {
                return "accept";
            }
            else if (decision == "n")
            {
                return "drop";
            }
            else if (decision == "c")
            {
                return "exit";
            }
        }
    }

    std::string RuleHandler::Print()
    {
        std::string rule;
        std::string ruleList = "";
        std::fstream fs;
        
        fs.open(FILE_NAME, std::ios::in);
        int ruleId = 0;
    
        if (fs.is_open())
        {
            ruleList += "\n";
        
            while (getline(fs, rule))
            {
                ruleId++;
                ruleList += std::to_string(ruleId);
                ruleList += " ";
                ruleList += rule;
                ruleList += "\n";
            }

            fs.close();

            return ruleList;
        }
        return "couldnt open file";
    }

    std::string RuleHandler::Add()
    {
        std::string retMsg;
        std::fstream fs;
        std::string rule;
    
        retMsg = GetPacketSource();

        if (retMsg == "exit")
        {
            return retMsg;
        }

        rule = retMsg;

        retMsg = GetPacketParameter();

        if (retMsg == "exit")
        {
            return retMsg;
        }

        rule += retMsg;

        retMsg = GetPacketType();

        if (retMsg == "exit")
        {
            return retMsg;
        }

        rule += retMsg;
    
        fs.open(FILE_NAME, std::ios::out | std::ios::app);
    
        if (fs.is_open())
        {
            fs  << rule << "\n";
            fs.close();        

            std::string procMsg = "04 ";

            procMsg += rule;

            ProcCommunicator::WriteProc(procMsg, PROC_PATH);
        }
        else
        {
            rule = "Error: Couldnt open File";
        }
    
        return rule;
    }

    std::string RuleHandler::Rules()
    {
        std::ifstream f(FILE_NAME);

        if (!f.good())
        {
            std::ofstream fs(FILE_NAME);

            if (fs.is_open())
            {
                fs.close();
            }
            else
            {
                return "coudlnt create rules file";
            }
        }
        _isActive = true;

        return "Entered Rules Interface";
    }

    std::string RuleHandler::Remove(std::string removeRule)
    {
        if (removeRule.find_first_of(' ') != std::string::npos)
        {
            removeRule = removeRule.substr (removeRule.find_first_of(' ') + 1);
        
            int removeId = ExtractId(removeRule);        
        
            if (removeId != -1)
            {
                std::fstream fs;
                std::ofstream temp;
                std::string line = "";
                std::ofstream proc;
                int count = 0;
                bool check = false;
            
                temp.open("temp.txt");
                fs.open(FILE_NAME);
            
                if (fs.is_open())
                {
                    while(getline(fs, line))
                    {
                        count++;
                        if (removeId != count)
                        {
                            temp << line << std::endl;
                        }
                        else
                        {
                            check = true;
                        }
                    }

                    fs.close();
                    temp.close();
                    remove(FILE_NAME);
                    rename("temp.txt", FILE_NAME);
                }
            
                std::string procMsg = "05 ";
            
                procMsg += std::to_string(removeId);                                    

                if (check)
                {
                    ProcCommunicator::WriteProc(procMsg, PROC_PATH);
                
                    return "Removed rule ID: " + std::to_string(removeId);
                }
            
                return "Remove Failed";
            }
        }
    
        return "Remove Failed";
    }

    std::vector<std::string> RuleHandler::InitializeRules()
    {
        std::fstream ruleFile;
        std::string line;
        std::ofstream procFile;
        std::vector<std::string> rules;
    
        ruleFile.open(FILE_NAME);

        if (ruleFile.is_open())
        {
            while (getline(ruleFile, line))
            {
                line = "04 " + line;
                rules.push_back(line);
            }
        }

        if (rules.size() != 0)
        {
        
            for (std::string rule : rules)
            {
                ProcCommunicator::WriteProc(rule, PROC_PATH);
            }
        }

        return rules;
    }

    int RuleHandler::ExtractId(const std::string& s)
    {
        if (Helper::IsNumber(s))
        {
            return std::stoi(s);
        }
        else
        {
            return -1;
        }
    }

    bool RuleHandler::Active()
    {
        return _isActive;
    }

    std::string RuleHandler::Exit()
    {
        _isActive = false;

        return "Exited Rule Interface";
    }

    std::string RuleHandler::Help()
    {
        return "\nAdd [Rule] - add the rule\nRemove [line] - remove the speicifc rule in line\nPrint - shows the complete list of rules\nExit - exit rule interface and return to normal one\n";
    }

    std::string RuleHandler::Clean(std::string command)
    {
        if (command.find_first_of(' ') != std::string::npos)
        {
            return command.substr(0, command.find_first_of(' ')); 
        }
        else
        {
            return command;
        }
    
    }

    std::string RuleHandler::AddUrl()
    {
        return _urlHandler.Add();
    }

    std::string RuleHandler::RemoveUrl()
    {
        return _urlHandler.Remove();
    }

    std::string RuleHandler::PrintUrl()
    {
        return _urlHandler.Print();
    }   
}
