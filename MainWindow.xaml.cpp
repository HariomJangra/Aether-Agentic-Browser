#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#include "App.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Agentic_Browser::implementation
{
    MainWindow::MainWindow() {

        InitializeComponent();

        //Title Bar Customization
        ExtendsContentIntoTitleBar(true);
        SetTitleBar(DragRegion());
    }

    // Add New Tab Functionality
    void MainWindow::TabView_AddTabButtonClick(
        Microsoft::UI::Xaml::Controls::TabView const& sender,
        winrt::Windows::Foundation::IInspectable const&)
    {
        using namespace Microsoft::UI::Xaml::Controls;

        auto tab = TabViewItem{};
        tab.Header(box_value(L"New Tab"));

        sender.TabItems().Append(tab);
        sender.SelectedItem(tab);
    }


    // Close Tab Functionality
    void MainWindow::TabView_TabCloseRequested(
        Controls::TabView const& sender,
        Controls::TabViewTabCloseRequestedEventArgs const& args)
    {
        uint32_t index{};
        auto items = sender.TabItems();

        if (items.IndexOf(args.Tab(), index)) {
            items.RemoveAt(index);
        }

        // Close Application if all tabs removed
        if (items.Size() == 0) {
            this->Close();   
        }
    }
}
