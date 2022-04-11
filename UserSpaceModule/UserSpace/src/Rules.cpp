#include "UserSpace/Rules.h"

#include <iostream>

Rules::Rules()
{
    
}

Rules::~Rules()
{
}

void Rules::AddUrl(std::string url)
{
    _urls.push_back(url);
}

void Rules::DelUrl(std::string index)
{
    int i = stoi(index);
    _urls.erase(_urls.begin() + i - 1);
}

bool Rules::Check(std::string url)
{
    for(std::string urlIt : _urls)
    {
        if(urlIt == url)
        {
            return true;
        }
    }
    return false;
}

void Rules::PrintUrls()
{
    int i = 1;
    
    for (std::string url : _urls)
    {        
        std::cout << "url number " << i << ": " <<  url << std::endl;
        i++;
    }
}


