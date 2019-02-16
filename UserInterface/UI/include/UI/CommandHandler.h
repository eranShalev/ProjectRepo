#pragma once

#include <string>
#define KERNEL_ALREADY_LOADED "Kernel is already loaded"
namespace UI
{
    class CommandHandler
    {
    public:
        CommandHandler();
        ~CommandHandler();

        // Handler Methods
        std::string HandleStart();
        std::string HandleClose();
        std::string HandleStopFiltering();
        std::string HandleRestart();
        std::string HandleRules();        
        std::string HandleSettings();
        std::string HandleHelp();
        
    private:
        bool _kernelLoaded;
    };  
}
