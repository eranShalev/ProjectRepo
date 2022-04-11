#include "UI/Communicator.h"

namespace UI
{
    Communicator::Communicator()
    {

    }

    Communicator::~Communicator()
    {
        unlink(PATH);
    }

    void Communicator::CreatePipe()
    {
        mkfifo(PATH, 0660);
    }

    void Communicator::WritePipe(std::string msg)
    {
        int fd;
        
        fd = open(PATH, O_WRONLY);

        if (fd < 0)
        {
            return;
        }
    

        write(fd, msg.c_str(), msg.size());
    
        close(fd);

    }

    void Communicator::SendRules(std::vector<std::string> rules)
    {
        for (std::string rule : rules)
        {
            std::this_thread::sleep_for (std::chrono::milliseconds(10));
            WritePipe("07 " + rule);
        }
    }
        
}

