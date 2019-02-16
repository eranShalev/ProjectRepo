#include "UI/UrlHandler.h"

namespace UI
{
    UrlHandler::UrlHandler()
    {
        std::ifstream f(URL_FILE);
        std::string url;
    
        if (f.good())
        {
            while (getline(f, url))
            {
                _urls.push_back(url);
            }
        }
    }

    UrlHandler::~UrlHandler()
    {
        remove(URL_FILE);

        std::ofstream url_file;

        url_file.open(URL_FILE);

        if(url_file.is_open())
        {
            for (std::string url : _urls)
            {
                url_file << url << std::endl;
            }
        }
    }

    std::string UrlHandler::Add()
    {
        std::string url = " ";

        while (url != "c")
        {
            std::cout << "enter the url you wish to block (to cancel press c)" << std::endl;
            std::getline(std::cin, url);

            if (IsUrlValid(url))
            {
                _urls.push_back(url);
                return url;
            }
            else
            {
                std::cout << "Invalid url" << std::endl;
            }
        }
    
        return "no url was added";
    }

    std::string UrlHandler::Remove()
    {
        std::string index = " ";
    
        while (index != "c")
        {
            std::cout << "url index to remvoe (press c to cancel): ";
            std::getline(std::cin, index);

            if (index == "c")
            {
                break;
            }
            else if (Helper::IsNumber(index))
            {
                unsigned int validIndex = std::stoi(index);
                if (validIndex > _urls.size() || validIndex < 1)
                {
                    std::cout << "index out of bonds" << std::endl;
                }
                else
                {
                    _urls.erase(_urls.begin() + validIndex-1);

                    return index;
                }
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

}

