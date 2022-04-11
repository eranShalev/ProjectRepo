#pragma once

#include <string>
#include <vector>

class Rules
{
 private:
    std::vector<std::string> _urls;

 public:
    Rules();
    ~Rules();
    
    bool Check(std::string url);
    void AddUrl(std::string url);
    void DelUrl(std::string index);
    void PrintUrls();
};
