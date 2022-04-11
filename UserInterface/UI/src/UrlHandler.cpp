#include "UI/UrlHandler.h"

namespace UI
{
    std::string UrlHandler::path = "none";
    
    UrlHandler::UrlHandler()
    {
        
    }

    UrlHandler::~UrlHandler()
    {
        remove(path.c_str());

        std::ofstream url_file;

        url_file.open(path);

        if(url_file.is_open())
        {
            for (std::string url : _urls)
            {
                url_file << url << std::endl;
            }
        }
    }   
    std::string UrlHandler::Add(std::vector<std::string>& commandParts)
    {
        
        if (commandParts.size() == 3)
        {
            if (!AlreadyExists(commandParts[2]))
            {
                _urls.push_back(commandParts[2]);
                return commandParts[2];
            }
   
        }
        return "no url was added";
    }

    std::string UrlHandler::Remove(std::vector<std::string>& commandParts)
    {
        if (Helper::IsNumber(commandParts[2]))
        {
            unsigned int validIndex = std::stoi(commandParts[2]);
            if (validIndex > _urls.size() || validIndex < 1)
            {
                return "index out of bonds";
            }
            else
            {
                _urls.erase(_urls.begin() + validIndex-1);

                return commandParts[2];
            }
        }
        
        return "remove failed";
    }

    std::string UrlHandler::Print()
    {
        std::string urls = "";
    
        for (unsigned int i = 0; i < _urls.size(); i++)
        {
            urls += std::to_string(i+1);
            urls +=": ";
            urls +=  _urls[i];
            urls += "\n";
        }

        return urls;
    }

    bool UrlHandler::IsUrlValid(const std::string& url)
    {
        std::string pattern = "https?:\\/\\/(www\\.)?[-a-zA-Z0-9@:%._\\+~#=]{2,256}\\.[a-z]{2,4}\\b([-a-zA-Z0-9@:%_\\+.~#?&//=]*)";

        std::regex regex_url(pattern);

        return std::regex_match(url, regex_url);
    }

    std::string UrlHandler::Help()
    {
        return "Add url - add a url to block\nRemove url - removes a url from the list\nprint url - prints all the blocked urls";
    }

    std::vector<std::string> UrlHandler::GetUrls()
    {        
        return _urls;
    }

    bool UrlHandler::AlreadyExists(const std::string& url)
    {
        for (std::string nowUrl : _urls)
        {
            if (nowUrl == url)
            {
                return true;
            }
        }

        return false;
    }
    
    void UrlHandler::SetPath(bool isGUI)
    {
        if (isGUI)
        {
            UrlHandler::path = "../UserInterface/urls.text";
        }
        else
        {
            UrlHandler::path = "urls.text";
        }
    }
    
    void UrlHandler::InitializeFile()
    {
        std::ifstream f(path);
        std::string url;
    
        if (f.good())
        {
            while (getline(f, url))
            {
                _urls.push_back(url);
            }
        }
    }
}

