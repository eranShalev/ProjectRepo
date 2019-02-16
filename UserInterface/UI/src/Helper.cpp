#include "UI/Helper.h"
namespace UI
{
    bool Helper::IsNumber(const std::string& number)
    {
        return !number.empty() && std::find_if(number.begin(), 
                                               number.end(), [](char c) { return !std::isdigit(c); }) == number.end();
    }

    void Helper::ParseByChar(std::string& data, char divider, std::vector<std::string>& vec)
    {
        std::string substring;
        std::stringstream data_stream(data);
    
        while(std::getline(data_stream, substring, divider))
        {
            vec.push_back(substring);
        }
    }
}
