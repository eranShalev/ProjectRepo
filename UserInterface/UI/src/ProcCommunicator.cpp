#include "UI/ProcCommunicator.h"
#include <fstream>

namespace UI
{
    ProcCommunicator::ProcCommunicator()
    {
    }

    ProcCommunicator::~ProcCommunicator()
    {
    }

    bool ProcCommunicator::WriteProc(const std::string& msg, const std::string& proc_path)
    {
        std::fstream proc;
        std::ifstream check_proc(proc_path);

        if (!check_proc.good())
        {
            return false;
        }

        proc.open(proc_path);

        if (proc.is_open())
        {
            proc << msg;
            proc.close();

            return true;
        }

        return false;    
    }   
}
