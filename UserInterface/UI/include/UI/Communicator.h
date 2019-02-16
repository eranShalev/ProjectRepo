#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <string>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

#define PORT 17002
#define PATH "/tmp/myfifo"

namespace UI
{
    class Communicator
    {
    public:
        Communicator(); // constructor
        ~Communicator(); // destructor

        void CreatePipe(); // creates the pipe 
        void WritePipe(std::string msg); // write a message into the pipe
        void SendRules(std::vector<std::string> rules);

    };
}
