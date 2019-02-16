#pragma once

extern UI::Application* UI::CreateApplication();

int main()
{
    std::cout << "Entered EntryPoint!" << std::endl;

    UI::Application* app = UI::CreateApplication();
    app->Run();
    delete app;

    return 0;
}
