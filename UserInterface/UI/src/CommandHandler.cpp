#include "UI/CommandHandler.h"
#include <sstream>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

// To Remove
#include <iostream>
// ------------------

#define INIT_MODULE(moduleFile, params, flags) syscall(__NR_finit_module, moduleFile, params, flags)
#define DELETE_MODULE(name, flags) syscall(__NR_delete_module, name, flags)

namespace UI
{
    CommandHandler::CommandHandler()
        : _kernelLoaded(false)
    {
    }

    CommandHandler::~CommandHandler()
    {
        _kernelLoaded = false;
    }
    
    std::string CommandHandler::HandleStart()
    {

        if (!_kernelLoaded)
        {
            int kernelMod = open("../KernelModule/build/Kernel.ko", O_RDONLY);

            // Initializing the kernel module
            if (INIT_MODULE(kernelMod, "", 0) != 0)
            {
                // TODO: Throw a ModuleInitialization exception
                perror("finit_module"); // TODO: Remove this line
            }
            
            // Closing the kernel module file as it is not used any more
            close(kernelMod);

            _kernelLoaded = true;
        
            return "Firewall Started Filtering Packets!";

        }

        return KERNEL_ALREADY_LOADED;
    }

    std::string CommandHandler::HandleClose()
    {
        HandleStopFiltering();
        return "Firewall Closed!";
    }

    std::string CommandHandler::HandleStopFiltering()
    {

        if (_kernelLoaded)
        {
            // Removing the kernel module
            if (DELETE_MODULE("Kernel", O_NONBLOCK) != 0)
            {
                // TODO: Throw a module deletion exception
                perror("delete_module"); // TODO: Remove line
            }

            _kernelLoaded = false;
            
            return "Firewall Stopped Filtering!";
        }
        
        return "kernel isnt loaded";
        
    }

    std::string CommandHandler::HandleRestart()
    {
        HandleStopFiltering();
        HandleStart();
        
        return "Firewall Restarting...";
    }

    std::string CommandHandler::HandleRules()
    {
        return "Entering rules Editor...";
    }    

    std::string CommandHandler::HandleSettings()
    {
        return "Entering Settings Section....";
    }

    std::string CommandHandler::HandleHelp()
    {
        std::stringstream s;
        
        s << std::endl << "start - start filtering packets" << std::endl;
        s << "stop - stop filtering packets" << std::endl;
        s << "restart - restarts the firewall " << std::endl;
        s << "settings - enters the settings" << std::endl;
        s << "logs - show logs of captured packets" << std::endl;
        s << "rules - enter the rule section to add, edit and delete rules" << std::endl;
        s << "close - shutdowns the whole firewall including the user interface" << std::endl;

        return s.str();
    }
}
