#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#include "BrowserView.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::Agentic_Browser::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();

        // Title Bar Customization
        ExtendsContentIntoTitleBar(true);
        SetTitleBar(DragRegion());

        // Create initial tab
        CreateNewTab(L"");
    }

    //  New Tab button
    void MainWindow::TabView_AddTabButtonClick(
        Microsoft::UI::Xaml::Controls::TabView const&,
        winrt::Windows::Foundation::IInspectable const&)
    {
        CreateNewTab(L"");
    }

    // Close Tab
    void MainWindow::TabView_TabCloseRequested(
        Controls::TabView const& sender,
        Controls::TabViewTabCloseRequestedEventArgs const& args)
    {
        uint32_t index{};
        auto items = sender.TabItems();

        if (items.IndexOf(args.Tab(), index))
        {
            // Clean up the BrowserView before removing the tab
            if (auto tab = args.Tab())
            {
                if (auto browserView = tab.Content().try_as<Agentic_Browser::BrowserView>())
                {
                    browserView.Cleanup();
                }
            }

            items.RemoveAt(index);
        }

        if (items.Size() == 0)
        {
            this->Close();
        }
    }

    // SINGLE SOURCE OF TRUTH
    void MainWindow::CreateNewTab(winrt::hstring const& initialUrl)
    {
        using namespace Microsoft::UI::Xaml::Controls;
        using namespace Microsoft::UI::Xaml::Media;

        auto tab = TabViewItem{};
        auto browserView = winrt::Agentic_Browser::BrowserView{};

        tab.Header(winrt::box_value(L"New Tab"));
        tab.Content(browserView);

        browserView.TitleChanged([tab](auto const&, winrt::hstring const& newTitle)
            {
                tab.Header(winrt::box_value(newTitle));
            });

        browserView.FaviconChanged([tab](auto const&, winrt::hstring const& uri)
            {
                auto bitmapIcon = BitmapIconSource();
                bitmapIcon.UriSource(winrt::Windows::Foundation::Uri(uri));
                bitmapIcon.ShowAsMonochrome(false);
                tab.IconSource(bitmapIcon);
            });

        // Handle Ctrl+Click / target=_blank
        browserView.NewTabRequested(
            [this](auto const&, winrt::hstring const& url)
            {
                CreateNewTab(url);
            });

        Tabs().TabItems().Append(tab);
        Tabs().SelectedItem(tab);

        // Navigate immediately
        browserView.NavigateTo(initialUrl);
    }
}
