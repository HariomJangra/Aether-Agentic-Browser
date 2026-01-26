#include "pch.h"
#include "BrowserView.xaml.h"
#if __has_include("BrowserView.g.cpp")
#include "BrowserView.g.cpp"
#endif

#include <winrt/Microsoft.UI.Xaml.Input.h>
#include <winrt/Windows.System.h>
#include <winrt/Microsoft.Web.WebView2.Core.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Windows.Storage.Streams.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::Agentic_Browser::implementation
{
    BrowserView::BrowserView()
    {
        InitializeComponent();

        // Back
        BackButton().Click([weak_this = get_weak()](auto const&, auto const&)
            {
                if (auto strong_this = weak_this.get())
                {
                    if (strong_this->WebView().CanGoBack())
                    {
                        strong_this->WebView().GoBack();
                    }
                }
            });

        // Forward
        ForwardButton().Click([weak_this = get_weak()](auto const&, auto const&)
            {
                if (auto strong_this = weak_this.get())
                {
                    if (strong_this->WebView().CanGoForward())
                    {
                        strong_this->WebView().GoForward();
                    }
                }
            });

        // Reload
        ReloadButton().Click([weak_this = get_weak()](auto const&, auto const&)
            {
                if (auto strong_this = weak_this.get())
                {
                    if (auto core = strong_this->WebView().CoreWebView2())
                    {
                        core.Reload();
                    }
                }
            });

        // 1. Enter Key Handler
        UrlBox().KeyDown([weak_this = get_weak()](IInspectable const&, Input::KeyRoutedEventArgs const& args)
            {
                if (auto strong_this = weak_this.get())
                {
                    if (args.Key() == Windows::System::VirtualKey::Enter)
                    {
                        strong_this->NavigateTo(strong_this->UrlBox().Text());
                        strong_this->WebView().Focus(FocusState::Programmatic);
                    }
                }
            });

        // 2. Initialize CoreWebView2
        WebView().CoreWebView2Initialized([weak_this = get_weak()](auto&&, auto&&)
            {
                if (auto strong_this = weak_this.get())
                {
                    strong_this->HookCoreWebViewEvents();
                }
            });

        WebView().EnsureCoreWebView2Async();



        // 3. UrlBox focus behavior
        UrlBox().GettingFocus([weak_this = get_weak()](auto const&, auto const&)
            {
                if (auto strong_this = weak_this.get())
                {
                    if (strong_this->WebView().CoreWebView2())
                    {
                        strong_this->UrlBox().Text(
                            strong_this->WebView().Source().AbsoluteUri()
                        );
                        strong_this->UrlBox().SelectAll();
                    }
                }
            });

        UrlBox().LosingFocus([weak_this = get_weak()](auto const&, auto const&)
            {
                if (auto strong_this = weak_this.get())
                {
                    strong_this->UpdateUrlBarFromWebView();
                }
            });

        // 4. FIX: clicking anywhere outside forces focus away
        this->PointerPressed([weak_this = get_weak()](auto const&, auto const&)
            {
                if (auto strong_this = weak_this.get())
                {
                    strong_this->WebView().Focus(FocusState::Programmatic);
                    strong_this->UpdateUrlBarFromWebView();
                }
            });
    }

    void BrowserView::UpdateUrlBarFromWebView()
    {
        auto core = WebView().CoreWebView2();
        if (!core) return;

        try
        {
            auto uri = WebView().Source();
            winrt::hstring host = uri.Host();
            winrt::hstring title = core.DocumentTitle();

            std::wstring hostStr{ host };
            if (hostStr.starts_with(L"www.")) hostStr.erase(0, 4);

            std::wstring titleStr{ title };
            std::transform(titleStr.begin(), titleStr.end(), titleStr.begin(), ::tolower);

            std::wstring cleanHost = hostStr;
            if (auto dot = cleanHost.find_last_of(L'.'); dot != std::wstring::npos)
                cleanHost = cleanHost.substr(0, dot);

            if (!title.empty() && titleStr != hostStr && titleStr != cleanHost)
            {
                UrlBox().Text(winrt::hstring(hostStr) + L" / " + title);
            }
            else
            {
                UrlBox().Text(winrt::hstring(hostStr));
            }
        }
        catch (...) {}
    }

    void BrowserView::HookCoreWebViewEvents()
    {
        auto core = WebView().CoreWebView2();
        if (!core) return;

        core.SourceChanged([weak_this = get_weak()](auto&&, auto&&)
            {
                if (auto strong_this = weak_this.get())
                {
                    strong_this->UpdateUrlBarFromWebView();
                }
            });

        core.DocumentTitleChanged([weak_this = get_weak()](auto const& sender, auto const&)
            {
                if (auto strong_this = weak_this.get())
                {
                    strong_this->UpdateUrlBarFromWebView();
                    strong_this->m_titleChangedEvent(*strong_this, sender.DocumentTitle());
                }
            });

        core.FaviconChanged([weak_this = get_weak()](auto const& sender, auto const&)
            {
                if (auto strong_this = weak_this.get())
                {
                    winrt::hstring uri = sender.FaviconUri();
                    if (!uri.empty())
                    {
                        auto bitmap = winrt::Microsoft::UI::Xaml::Media::Imaging::BitmapImage(
                            winrt::Windows::Foundation::Uri(uri)
                        );
                        strong_this->FaviconImage().Source(bitmap);
                        strong_this->m_faviconChangedEvent(*strong_this, uri);
                    }
                }
            });
        core.NewWindowRequested(
            [weak_this = get_weak()](
                auto const&,
                Microsoft::Web::WebView2::Core::CoreWebView2NewWindowRequestedEventArgs const& args)
            {
                if (auto strong_this = weak_this.get())
                {
                    // Stop WebView2 from creating a new OS window
                    args.Handled(true);

                    // Fire our BrowserView event
                    strong_this->m_newTabRequestedEvent(
                        *strong_this,
                        args.Uri()
                    );
                }
            });

        core.HistoryChanged(
            [weak_this = get_weak()](auto const&, auto const&)
            {
                if (auto strong_this = weak_this.get())
                {
                    strong_this->BackButton().IsEnabled(
                        strong_this->WebView().CanGoBack());

                    strong_this->ForwardButton().IsEnabled(
                        strong_this->WebView().CanGoForward());
                }
            });


    }

    void BrowserView::NavigateTo(winrt::hstring const& url)
    {
        auto normalized = NormalizeUrl(url);
        try
        {
            WebView().Source(winrt::Windows::Foundation::Uri{ normalized });
        }
        catch (...) {}
    }

    winrt::hstring BrowserView::NormalizeUrl(winrt::hstring const& input)
    {
        std::wstring text{ input };
        text.erase(0, text.find_first_not_of(L" \t"));
        if (auto last = text.find_last_not_of(L" \t"); last != std::wstring::npos)
            text.erase(last + 1);

        if (text.empty()) return L"about:blank";
        if (text.starts_with(L"http://") || text.starts_with(L"https://") || text.starts_with(L"about:"))
            return winrt::hstring{ text };

        if (text.find(L'.') != std::wstring::npos && text.find(L' ') == std::wstring::npos)
            return winrt::hstring{ L"https://" + text };

        return winrt::hstring{ L"https://www.google.com/search?q=" + text };
    }
}
