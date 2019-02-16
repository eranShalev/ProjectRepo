#include "Sandbox.h"
#include <iostream>
#include <string>

class Sandbox : public UI::Application
{
public:
    Sandbox()
    {
    }

    ~Sandbox()
    {
    }

    void OnStartup() override
    {
        UI::Application::OnStartup();
    }

    void OnUpdate() override
    {
        RcvCommand();
        std::cout << OnUpdateImpl() << std::endl;
    }

    void OnShutdown() override
    {
        UI::Application::OnShutdown();
    }
    
private:
    void RcvCommand()
    {
        std::cout << "-> ";
        std::getline(std::cin, _currCommand);
    }    
};

UI::Application* UI::CreateApplication()
{
    return new Sandbox();
}
