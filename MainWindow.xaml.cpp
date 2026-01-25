#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

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
        
        // Creating Initial Tab
        CreateNewTab();
    }

    // Add New Tab Functionality
    void MainWindow::TabView_AddTabButtonClick(
        Microsoft::UI::Xaml::Controls::TabView const& sender,
        winrt::Windows::Foundation::IInspectable const&)
    {
        CreateNewTab();
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

    void MainWindow::CreateNewTab()
    {
        using namespace Microsoft::UI::Xaml::Controls;

        auto tab = TabViewItem{};

        // Temporary placeholder — later this comes from WebView2
        tab.Header(box_value(L"New Tab"));
        tab.Content(winrt::Agentic_Browser::BrowserView{});

        Tabs().TabItems().Append(tab);
        Tabs().SelectedItem(tab);

        /*auto browserView = winrt::Agentic_Browser::BrowserView{};
        browserView.SetInitialUrl(L"https://www.google.com");

        tab.Content(browserView);*/

    }

}
