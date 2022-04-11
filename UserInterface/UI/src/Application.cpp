#include "bake_config.h"
#include "UI/Application.h"
#include "Jobs.h"

#include <sys/types.h>
#include <unistd.h>
#include <iostream>

namespace UI
{
    std::string Application::_currCommand;
    
    Application::Application()
        : _isRunning(true), _sentRules(false)
    {
        _pipe.CreatePipe();
    }

    Application::~Application()
    {
        _isRunning = false;
    }
 
    void Application::Run()
    {
        OnStartup();
        _ruleHandler.SetPath(_isGUI);

        // Main UI Run Loop
        while (_isRunning)
        {
            OnUpdate();
        }
        OnShutdown();
    }
    
    void Application::OnStartup()
    {
        std::cout << "Welcome To The Firewall!" << std::endl;
        
        
        Jobs::Every(2).Minutes().Do(BIND_METHOD(ThreadManager::SendReset, _statisticsHandler._threadManager));
        Jobs::Every(20).Minutes().Do(BIND_METHOD(ThreadManager::ResetSuspiciousHosts, _statisticsHandler._threadManager));
        
        int pid = fork();
        
        if (pid == -1)
        {
            std::cout << "Error couldnt Fork Proccess" << std::endl;
        }
        else if (pid == 0)
        {
            //std::cout << "Forked Successfully the proccess and the id is: " << std::to_string(getpid()) << std::endl;            
            
            execl("../UserSpaceModule/Sandbox/bin/x64-Linux-debug/Sandbox", "Sandbox", nullptr);
        }
        else
        {
            //std::cout << "In Parent" << std::endl;
        }
        
        Jobs::Run();
    }

    std::string Application::OnUpdateImpl()
    {
        return ProcessCommand();
    }

    void Application::OnShutdown()
    {
        //Jobs::Stop();        
        std::cout << "Firewall Is Shut Down!" << std::endl;
    }

    std::string Application::ProcessCommand()
    {
        const char* START_COMMAND = "start";
        const char* CLOSE_COMMAND = "close";
        const char* STOP_COMMAND = "stop";
        const char* HELP_COMMAND = "help";
        const char* RESTART_COMMAND = "restart";
        const char* RULES_COMMAND = "rules";
        const char* LOGS_COMMAND = "logs";
        const char* SETTINGS_COMMAND = "settings";
        const char* UNKNOWN_COMMAND_MSG =  "Unknown Command (type help for list of commands)";
        const char* ADD_COMMAND = "add";
        const char* REMOVE_COMMAND = "remove";
        const char* EXIT_COMMAND = "exit";
        const char* PRINT_COMMAND = "print";
        const char* STATISTICS_COMMAND = "stats";
        const char* DATA_STATISTICS_COMMAND = "data";
        const char* GENERAL_RULES_COMMAND = "general";
        const char* SPECIFIC_RULE_COMMAND = "rule";

        std::string result;
        
        _commandParts.clear();        
        Helper::ParseByChar(_currCommand, ' ', _commandParts);
                
        if (_commandParts.size() == 1 && _commandParts[0] == PRINT_COMMAND)
        {
            return _ruleHandler.Print();
        }        
        else if (!_ruleHandler.Active() && !_statisticsHandler.isRunning())
        {
            if (_commandParts.size() == 1)
            {
                // Checking for start command
                if (_commandParts[0] == START_COMMAND)
                {
                    return Start();
                }
                // Checking for close command
                else if (Application::_currCommand == CLOSE_COMMAND)
                {
                    return Close();
                }
                // Checking for stop command
                else if (Application::_currCommand == STOP_COMMAND)
                {
                    return Stop();
                }
                // Checking for help command
                else if (Application::_currCommand == HELP_COMMAND)
                {
                    return _handler.HandleHelp();
                }
                // Checking for restart command
                else if (Application::_currCommand == RESTART_COMMAND)
                {
                    return Restart();
                }
                // Checking for logs command
                else if (Application::_currCommand == LOGS_COMMAND)
                {
                    return LogManager::ShowLogs();
                }
                // Checking for settings command
                else if (Application::_currCommand == SETTINGS_COMMAND)
                {
                    return _handler.HandleSettings();
                }
                // Checking for rules command
                else if (Application::_currCommand == RULES_COMMAND)
                {
                    return _ruleHandler.Rules();
                }

                else if (Application::_currCommand == STATISTICS_COMMAND)
                {
                    return _statisticsHandler.Start();
                }
            }
        }
        else if (_statisticsHandler.isRunning())
        {
            if (_commandParts.size() > 0)
            {
                if (_commandParts.size() == 1 && _commandParts[0] == DATA_STATISTICS_COMMAND)
                {
                    return _statisticsHandler.GetDataFlowStatistics();
                }
                else if (_commandParts.size() == 1 && _commandParts[0] == GENERAL_RULES_COMMAND)
                {
                    return _statisticsHandler.GetGeneralRulesStatistics();
                }
                else if (_commandParts[0] == SPECIFIC_RULE_COMMAND)
                {
                    return _statisticsHandler.GetSpecificRuleStatistics(_commandParts);
                }
                else if (_commandParts.size() == 1 && _commandParts[0] == EXIT_COMMAND)
                {
                    return _statisticsHandler.Exit();
                }
                else if (_commandParts.size() == 1 && _commandParts[0] == HELP_COMMAND)
                {
                    return _statisticsHandler.Help();
                }
            }
        }
        else
        {
            if (_commandParts.size() > 0)
            {
                if (_commandParts.size() == 1 && _commandParts[0] == HELP_COMMAND)
                {
                    return _ruleHandler.Help();
                }
                else if (_commandParts.size() > 1)
                {
                    if (_commandParts[0] == ADD_COMMAND && _commandParts[1] == "url")
                    {
                        return AddUrlRule();
                    }
                    else if (_commandParts[0] == ADD_COMMAND)
                    {               
                        return AddRule();
                    }
                    else if (_commandParts[0] == REMOVE_COMMAND && _commandParts[1] == "url")
                    {
                        return RemoveUrlRule();
                    }
                    else if (_commandParts[0] == REMOVE_COMMAND)
                    {
                        return RemoveRule();
                    }
                    else if (_commandParts.size() == 2 && _commandParts[0] == PRINT_COMMAND && _commandParts[1] == "url")
                    {
                        return _ruleHandler.PrintUrl();
                    }
                }
                else if (_commandParts.size() == 1 && _commandParts[0] == EXIT_COMMAND)
                {
                    return _ruleHandler.Exit();
                }
            }
        }
        
        // Unknown Command
        return UNKNOWN_COMMAND_MSG;       
    }

    std::string Application::Start()
    {
        std::string result;
    
        _pipe.WritePipe("00");
        result = _handler.HandleStart();

        if (result != KERNEL_ALREADY_LOADED)
        {                                   
            _rules = _ruleHandler.InitializeRules();
        }

        if (!_sentRules)
        {
                    
            _pipe.SendRules(_ruleHandler.GetUrls());
            _sentRules = true;
        }

        _kernelActive = true;
        _statisticsHandler.UpdateTime(time(nullptr));                
        return result;
    }

    std::string Application::Restart()
    {
        std::string result;
        
        _pipe.WritePipe("02");
        result = _handler.HandleRestart();
        _rules = _ruleHandler.InitializeRules();
        _pipe.SendRules(_ruleHandler.GetUrls());
        _statisticsHandler.UpdateTime(time(nullptr));
                
        return result;
    }

    std::string Application::Close()
    {
        _isRunning = false;   
        _pipe.WritePipe("01");
        return _handler.HandleClose();
    }

    std::string Application::Stop()
    {
        _pipe.WritePipe("03");
        _sentRules = false;
        _kernelActive = false;
        return _handler.HandleStopFiltering();
    }

    std::string Application::AddRule()
    {
        std::string result;
        
        result = _ruleHandler.Add(_commandParts);
                
        if (result != "exit" && result != "Error: Couldnt open File")
        {          
            return "Added Rule!";
        }

        return result;
    }

    std::string Application::RemoveUrlRule()
    {
        std::string result;

        result = _ruleHandler.RemoveUrl(_commandParts);

        if (result != REMOVE_URL_FAILED && result !=  "index out of bonds")
        {
            if (_kernelActive)
            {               
                _pipe.WritePipe("08 " + result);
            }

            return "remove url id: " + result;
        }

        return result;
    }

    std::string Application::RemoveRule()
    {
        std::string result;
        
        result = _ruleHandler.Remove(Application::_currCommand);
                
        if (result != "Remove Failed")
        {                    
            std::string id = Application::_currCommand.substr(Application::_currCommand.find_first_of(' ') + 1);
            _pipe.WritePipe("05 " + id);
        }
                
        return result;
    }

    std::string Application::AddUrlRule()
    {
        std::string result;
        
        result = _ruleHandler.AddUrl(_commandParts);

        if (result != ADD_URL_FAILED && result != "Invalid url")
        {
            if (_kernelActive)
            {               
                _pipe.WritePipe("07 " + result);
            }
            return "added url: " + result;
        }
                
        return result;
    }
}



