#pragma once

#include "MainWindow.g.h"

namespace winrt::Agentic_Browser::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();
    
    };
}

namespace winrt::Agentic_Browser::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
