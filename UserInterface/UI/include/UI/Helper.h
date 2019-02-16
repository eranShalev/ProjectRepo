#pragma once
#include <string>
#include <algorithm>
#include <sstream>

namespace UI
{   
    class Helper
    {
    public:
        static bool IsNumber(const std::string& number);
        static void ParseByChar(std::string& data, char divider, std::vector<std::string>& vec);
    };
}
