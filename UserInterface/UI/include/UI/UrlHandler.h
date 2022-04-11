#pragma once

#include <boost/algorithm/string.hpp>
#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <fstream>
#include "UI/Helper.h"

#define ADD_URL_FAILED "no url was added"
#define REMOVE_URL_FAILED "remove failed"

namespace UI
{
    class UrlHandler
    {
    public:
        UrlHandler();
        ~UrlHandler();
        std::string Add(std::vector<std::string>& commandParts);
        std::string Remove(std::vector<std::string>& commandParts);
        bool IsUrlValid(const std::string& url);
        std::string Print();
        std::string Help();
        void InitializeFile();

        std::vector<std::string> GetUrls();
        void SetPath(bool isGUI);
    
    private:
        bool AlreadyExists(const std::string& url);
        
        std::vector<std::string> _urls;
        
        static std::string path;
    };
}
