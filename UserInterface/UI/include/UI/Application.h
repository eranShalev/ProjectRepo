#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include "UI/CommandHandler.h"
#include "UI/Communicator.h"
#include "UI/RuleHandler.h"
#include "UI/StatisticsHandler.h"
#include "UI/LogManager.h"
#include "UI/ThreadManager.h"

namespace UI
{
    class Application
    {
    public:
        Application();
        virtual ~Application();

        void Run();

        // Virtual Methods
        virtual void OnStartup();
        virtual void OnUpdate() = 0;
        virtual void OnShutdown();

        // Other Methods
        std::string ProcessCommand();

        //        void Test();
        
    protected:
        std::string _currCommand;

        std::string OnUpdateImpl();
        
    private:
        bool _isRunning;
        bool _sentRules;
        std::vector<std::string> _rules;
        std::vector<std::thread*> _threads;
        CommandHandler _handler;
        Communicator _pipe;
        RuleHandler _ruleHandler;
        StatisticsHandler _statisticsHandler;
        
        std::string Start();
        std::string Restart();
        std::string Close();
        std::string Stop();
        std::string AddRule();
        std::string AddUrlRule();
        std::string RemoveRule();
        std::string RemoveUrlRule();
        
        void TerminateThreads();
    };

    // To Be Defined In Sandbox
    Application* CreateApplication();
}
