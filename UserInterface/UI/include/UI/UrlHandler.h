#pragma once

#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <fstream>
#include "UI/Helper.h"
#define URL_FILE "urls.text"
#define ADD_URL_FAILED "no url was added"
#define REMOVE_URL_FAILED "remove failed"

namespace UI
{
    class UrlHandler
    {
    public:
        UrlHandler();
        ~UrlHandler();
        std::string Add();
        std::string Remove();
        bool IsUrlValid(const std::string& url);
        std::string Print();
        std::string Help();
    
    private:
        std::vector<std::string> _urls;
    };
}
