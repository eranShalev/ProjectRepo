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
    std::string RuleHandler::path = "none";
    
    RuleHandler::RuleHandler() : _isActive(false)
    {
    }
    
    RuleHandler::~RuleHandler()
    {
    }

    bool RuleHandler::CheckPort(std::string& msg)
    {        
        if (Helper::IsNumber(msg))
        {
            if (std::stoi(msg) < 65535)
            {
                return true;
            }
        }
        
        return false;
    }

    bool RuleHandler::CheckIp(std::string& msg)
    {   
        struct sockaddr_in sa;
        if (inet_pton(AF_INET, msg.c_str(), &(sa.sin_addr)) == 1)
        {
            return true;
        }
        
        return false;
    }

    bool RuleHandler::CheckProtocol(std::string& msg)
    {
        if (msg == "TCP" || msg == "UDP")
        {
            return true;
        }
        
        return false;
    }

    std::string RuleHandler::GetPacketSource(std::string& msg)
    {
        if (msg == "y")
        {
            return "incoming,";
        }
        else if (msg == "n")
        {
            return "outgoing,";
        }
        else
        {
            return "exit";
        }
    }

    std::string RuleHandler::GetPacketParameter(std::vector<std::string>& msg)
    {
        if (msg[2] == "dip" && CheckIp(msg[3]))
        {
            return "dip=" + msg[3] + ",";
        }
        else if  (msg[2] == "sip" && CheckIp(msg[3]))
        {
            return "sip=" + msg[3] + ",";
        }
        else if (msg[2] == "dport" && CheckPort(msg[3]))
        {
            return "dport=" + msg[3] + ",";
        }
        else if (msg[2] == "sport" && CheckPort(msg[3]))
        {
            return "sport=" + msg[3] + ",";
        }
        else if (msg[2] == "protocol" && CheckProtocol(msg[3]))
        {
            return "protocol=" + msg[3] + ",";
        }
        else
        {
            return "exit";
        }
    }

    std::string RuleHandler::GetPacketType(std::string& msg)
    {
        if (msg == "y")
        {
            return "accept";
        }
        else if (msg == "n")
        {
            return "drop";
        }
        else
        {
            return "exit";
        }
    }

    std::string RuleHandler::Print()
    {
        std::string rule;
        std::string ruleList = "";
        std::fstream fs;
        
        fs.open(path.c_str(), std::ios::in);
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

    std::string RuleHandler::Add(std::vector<std::string>& commandParts)
    {
        std::string retMsg;
        std::fstream fs;
        std::string rule;
        
        if (commandParts.size() != 5)
        {
            return "Error: Wrong Formatting";
        }
    
        retMsg = GetPacketSource(commandParts[1]);

        if (retMsg == "exit")
        {
            return retMsg;
        }

        rule = retMsg;

        retMsg = GetPacketParameter(commandParts);

        if (retMsg == "exit")
        {
            return retMsg;
        }

        rule += retMsg;

        retMsg = GetPacketType(commandParts[4]);

        if (retMsg == "exit")
        {
            return retMsg;
        }

        rule += retMsg;
    
        fs.open(path, std::ios::out | std::ios::app);
    
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
        //std::cout << path << std::endl;
        std::ifstream f(path);

        if (!f.good())
        {
            std::ofstream fs(path);

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
                fs.open(path);
            
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
                    remove(path.c_str());
                    rename("temp.txt", path.c_str());
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
    
        ruleFile.open(path);

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
        return "\nadd [y/n (incoming/outgoing) parameter (sip/dip/sport/dport/protocol)\nvalue(TCP/UDP/ip/port) y/n(accept/drop)] - add the rule\nremove [line number] - remove the specific rule in line\nprint - shows the complete list of rules\nadd url [url] - adds a new url to the blacklist (works for HTTP only)\nremove url [line number] - removes a url from the blacklist\nprint url - prints all the blacklisted urls\nExit - exit rule interface and return to normal one\n";
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

    std::string RuleHandler::AddUrl(std::vector<std::string>& commandParts)
    {
        return _urlHandler.Add(commandParts);
    }

    std::string RuleHandler::RemoveUrl(std::vector<std::string>& commandParts)
    {
        return _urlHandler.Remove(commandParts);
    }

    std::string RuleHandler::PrintUrl()
    {
        return _urlHandler.Print();
    }

    std::vector<std::string> RuleHandler::GetUrls()
    {
        return _urlHandler.GetUrls();
    }
    
    void RuleHandler::SetPath(bool isGUI)
    {
        if (isGUI)
        {
            RuleHandler::path = "../UserInterface/rule_file.txt";
        }
        else
        {
            RuleHandler::path = "rule_file.txt";
        }
        
        
        _urlHandler.SetPath(isGUI);
        _urlHandler.InitializeFile();
    }
}
