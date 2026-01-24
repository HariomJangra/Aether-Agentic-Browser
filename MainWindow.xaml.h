#pragma once

#include "MainWindow.g.h"

namespace winrt::Agentic_Browser::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();

        void CreateNewTab();


        void TabView_AddTabButtonClick(
            Microsoft::UI::Xaml::Controls::TabView const& sender,
            winrt::Windows::Foundation::IInspectable const&);
        void TabView_TabCloseRequested(
            Microsoft::UI::Xaml::Controls::TabView const& sender,
            Microsoft::UI::Xaml::Controls::TabViewTabCloseRequestedEventArgs const& args);
    };
}

namespace winrt::Agentic_Browser::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
