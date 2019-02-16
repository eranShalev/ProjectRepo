#include "bake_config.h"
#include "UI/Application.h"
#include <iostream>

namespace UI
{
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

        // Main UI Run Loop
        while (_isRunning)
        {
            OnUpdate();
        }

        OnShutdown();
    }

    void Test()
    {
        std::cout << "Hi" << std::endl;
    }

    void Test2()
    {
        std::cout << "Hello World" << std::endl;
    }
    
    void Application::OnStartup()
    {
        std::cout << "Welcome To The Firewall!" << std::endl;

        //Jobs::Every(1).Seconds().Do(BIND_FN(Test));
        //Jobs::Every(3).Seconds().Do(BIND_FN(Test2));
        //Jobs::Every(1).Seconds().Do(BIND_METHOD(ThreadManager::SendReset, _statisticsHandler._threadManager));
        //       Jobs::Every(2).Seconds().Do(BIND_METHOD(ThreadManager::ResetSuspiciousHosts, _statisticsHandler._threadManager));
        //        Jobs::Run();
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
        const char* ADD_URL_COMMAND = "add url";
        const char* REMOVE_URL_COMMAND  = "remove url";
        const char* PRINT_URL_COMMAND = "print url";
        const char* STATISTICS_COMMAND = "stats";
        const char* DATA_STATISTICS_COMMAND = "data";
        const char* GENERAL_RULES_COMMAND = "general";
        const char* SPECIFIC_RULE_COMMAND = "rule";

        std::string result;
        
        if (_currCommand == PRINT_COMMAND)
        {
            return _ruleHandler.Print();
        }        
        else if (!_ruleHandler.Active() && !_statisticsHandler.isRunning())
        {
            // Checking for start command
            if (_currCommand == START_COMMAND)
            {
                return Start();
            }
            // Checking for close command
            else if (_currCommand == CLOSE_COMMAND)
            {
                return Close();
            }
            // Checking for stop command
            else if (_currCommand == STOP_COMMAND)
            {
                return Stop();
            }
            // Checking for help command
            else if (_currCommand == HELP_COMMAND)
            {
                return _handler.HandleHelp();
            }
            // Checking for restart command
            else if (_currCommand == RESTART_COMMAND)
            {
                return Restart();
            }
            // Checking for logs command
            else if (_currCommand == LOGS_COMMAND)
            {
                return LogManager::ShowLogs();
            }
            // Checking for settings command
            else if (_currCommand == SETTINGS_COMMAND)
            {
                return _handler.HandleSettings();
            }
            // Checking for rules command
            else if (_currCommand == RULES_COMMAND)
            {
                return _ruleHandler.Rules();
            }
            else if (_currCommand == STATISTICS_COMMAND)
            {
                return _statisticsHandler.Start();
            }
        }
        else if (_statisticsHandler.isRunning())
        {
            if (_currCommand == DATA_STATISTICS_COMMAND)
            {
                return _statisticsHandler.GetDataFlowStatistics();
            }
            else if (_currCommand == GENERAL_RULES_COMMAND)
            {
                return _statisticsHandler.GetGeneralRulesStatistics();
            }
            else if (_currCommand == SPECIFIC_RULE_COMMAND)
            {
                return _statisticsHandler.GetSpecificRuleStatistics();
            }
            else if (_currCommand == EXIT_COMMAND)
            {
                return _statisticsHandler.Exit();
            }
            else if (_currCommand == HELP_COMMAND)
            {
                return _statisticsHandler.Help();
            }
        }
        else
        {                      
            if(_currCommand == HELP_COMMAND)
            {
                return _ruleHandler.Help();
            }
            else if (_currCommand == ADD_COMMAND)
            {               
                return AddRule();
            }
            else if (_currCommand == REMOVE_URL_COMMAND)
            {
                return RemoveUrlRule();
            }
            else if (_ruleHandler.Clean(_currCommand) == REMOVE_COMMAND)
            {
                return RemoveRule();
            }            
            else if (_currCommand == EXIT_COMMAND)
            {
                return _ruleHandler.Exit();
            }
            else if (_currCommand == ADD_URL_COMMAND)
            {
                return AddUrlRule();
            }
            else if (_currCommand == PRINT_URL_COMMAND)
            {
                return _ruleHandler.PrintUrl();
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
                    
            _pipe.SendRules(_rules);
            _sentRules = true;
        }
        
        _statisticsHandler.UpdateTime(time(nullptr));                
        return result;
    }

    std::string Application::Restart()
    {
        std::string result;
        
        _pipe.WritePipe("02");
        result = _handler.HandleRestart();
        _rules = _ruleHandler.InitializeRules();
        _pipe.SendRules(_rules);
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
        return _handler.HandleStopFiltering();
    }

    std::string Application::AddRule()
    {
        std::string result;
        
        result = _ruleHandler.Add();
                
        if (result != "exit" && result != "Error: Couldnt open File")
        {
            _pipe.WritePipe("04 " + result);
            return "Added Rule!";
        }

        return result;
    }

    std::string Application::RemoveUrlRule()
    {
        std::string result;

        result = _ruleHandler.RemoveUrl();

        if (result != REMOVE_URL_FAILED)
        {
            _pipe.WritePipe("08 " + result);

            return "remove url id: " + result;
        }

        return result;
    }

    std::string Application::RemoveRule()
    {
        std::string result;
        
        result = _ruleHandler.Remove(_currCommand);
                
        if (result != "Remove Failed")
        {                    
            std::string id = _currCommand.substr(_currCommand.find_first_of(' ') + 1);
            _pipe.WritePipe("05 " + id);
                    
        }
                
        return result;
    }

    std::string Application::AddUrlRule()
    {
        std::string result;
        
        result =  _ruleHandler.AddUrl();

        if (result != ADD_URL_FAILED)
        {
            _pipe.WritePipe("07 " + result);

            return "added url: " + result;
        }
                
        return result;                

    }
}
