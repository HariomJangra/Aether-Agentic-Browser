#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#include "BrowserView.xaml.h"

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
        using namespace Microsoft::UI::Xaml::Media;

        // 1. Create the Tab and the BrowserView component
        auto tab = TabViewItem{};
        auto browserView = winrt::Agentic_Browser::BrowserView{};

        // 2. Set initial state
        tab.Header(winrt::box_value(L"New Tab"));
        tab.Content(browserView);

        // 3. Hook the TitleChanged event to update the Tab Header
        browserView.TitleChanged([tab](auto const&, winrt::hstring const& newTitle)
            {
                tab.Header(winrt::box_value(newTitle));
            });

        // 4. Hook the FaviconChanged event to update the Tab Icon
        browserView.FaviconChanged([tab](auto const&, winrt::hstring const& uri)
            {
                auto bitmapIcon = winrt::Microsoft::UI::Xaml::Controls::BitmapIconSource();
 
                bitmapIcon.UriSource(winrt::Windows::Foundation::Uri(uri));
                bitmapIcon.ShowAsMonochrome(false);

                tab.IconSource(bitmapIcon);
            });

        // 5. Add to the TabView and select it
        Tabs().TabItems().Append(tab);
        Tabs().SelectedItem(tab);

        // Optional: Set an initial URL if you have that method enabled
        // browserView.NavigateTo(L"https://www.google.com");
    }

}
