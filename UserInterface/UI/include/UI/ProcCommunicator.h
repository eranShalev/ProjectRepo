#pragma once
#include <string>

#define WRITE_SUCCESS 1
#define ERR_WRITE 0

namespace UI
{
    class ProcCommunicator
    {
    public:
        ProcCommunicator();
        ~ProcCommunicator();

        static bool WriteProc(const std::string& msg, const std::string& proc_path);
    };    
}

