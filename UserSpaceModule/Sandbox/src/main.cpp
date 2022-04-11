#include "Sandbox.h"

#include <iostream>

int main(int argc, char **argv)
{
    PipeManager pipe;
    
    try
    {
        pipe.Run(); 
    }
    catch (std::exception e)
    {
        std::cout << e.what() << std::endl;
    }
    
    std::cout << "finishing program" << std::endl;  
    return 0;
}
