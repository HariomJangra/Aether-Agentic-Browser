#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#include "BrowserView.xaml.h"
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Microsoft.Web.WebView2.Core.h>

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
        
        // Start TCP listener
        InitializeAgentListener();
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

        browserView.UrlChanged([tab](auto const&, winrt::hstring const& url)
            {
                try
                {
                    winrt::Windows::Foundation::Uri uri(url);
                    std::wstring host{ uri.Host() };
                    // Strip www. prefix
                    if (host.starts_with(L"www."))
                        host.erase(0, 4);
                    tab.Tag(winrt::box_value(winrt::hstring{ host }));
                }
                catch (...)
                {
                    tab.Tag(winrt::box_value(url)); // fallback: show full url
                }
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

    winrt::fire_and_forget MainWindow::Tabs_SelectionChanged(
        winrt::Windows::Foundation::IInspectable const& sender,
        Controls::SelectionChangedEventArgs const& args)
    {
        if (args.AddedItems().Size() > 0)
        {
            m_activeTab = args.AddedItems()
                .GetAt(0)
                .try_as<Controls::TabViewItem>();
        }

        if (args.RemovedItems().Size() == 0) co_return;

        auto tab = args.RemovedItems()
            .GetAt(0)
            .try_as<Controls::TabViewItem>();
        if (!tab) co_return;

        auto browserView = tab.Content()
            .try_as<Agentic_Browser::BrowserView>();
        if (!browserView) co_return;

        auto img = co_await browserView.CapturePreviewAsync();
        if (!img) co_return;

        m_tabPreviews[tab] = img;
    }

    void MainWindow::HoverCardPopup_Opened(
        winrt::Windows::Foundation::IInspectable const& sender,
        winrt::Windows::Foundation::IInspectable const&)
    {
        auto popup = sender.try_as<Controls::Primitives::Popup>();
        if (!popup) return;

        Controls::TabViewItem hoveredTab{ nullptr };
        winrt::Microsoft::UI::Xaml::DependencyObject current = popup;
        while (current)
        {
            if (auto tab = current.try_as<Controls::TabViewItem>())
            {
                hoveredTab = tab;
                break;
            }
            current = Media::VisualTreeHelper::GetParent(current);
        }
        if (!hoveredTab) return;

        auto rootGrid = popup.Child().try_as<Controls::Grid>();
        if (!rootGrid) return;

        auto previewBorder = rootGrid.Children().GetAt(2).try_as<Controls::Border>();
        if (!previewBorder) return;

        auto img = previewBorder.Child().try_as<Controls::Image>();
        if (!img) return;

        auto it = m_tabPreviews.find(hoveredTab);
        if (it != m_tabPreviews.end())
            img.Source(it->second);
        else
            img.Source(nullptr);
    }

    winrt::fire_and_forget MainWindow::InitializeAgentListener()
    {
        using namespace winrt::Windows::Networking::Sockets;
        using namespace winrt::Windows::Storage::Streams;

        m_agentListener = StreamSocketListener();

        m_agentListener.ConnectionReceived([this](StreamSocketListener sender, StreamSocketListenerConnectionReceivedEventArgs args) -> winrt::fire_and_forget
            {
                try
                {
                    DataReader reader(args.Socket().InputStream());
                    reader.InputStreamOptions(InputStreamOptions::Partial);
                    unsigned int bytesLoaded = co_await reader.LoadAsync(256);
                    hstring message = reader.ReadString(bytesLoaded);

                    // Marshal to UI Thread
                    this->DispatcherQueue().TryEnqueue([this, message]()
                        {
                            if (!m_activeTab) return;
                            auto browserView = m_activeTab.Content().try_as<Agentic_Browser::BrowserView>();
                            if (!browserView) return;

                            if (message == L"START") {
                                browserView.StartGlow();
                            }
                            else if (message == L"STOP") {
                                browserView.StopGlow();
                            }
                        });
                }
                catch (...) { /* Handle socket errors */ }
            });

        try {
            co_await m_agentListener.BindServiceNameAsync(L"8080");
        }
        catch (...) {
            // Port 8080 might be busy
        }
    }

}