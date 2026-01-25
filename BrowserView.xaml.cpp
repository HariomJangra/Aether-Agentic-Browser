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

        // 3. Comet-style Focus Handlers
        UrlBox().GettingFocus([weak_this = get_weak()](auto const&, auto const&)
            {
                if (auto strong_this = weak_this.get())
                {
                    if (strong_this->WebView().CoreWebView2())
                    {
                        // Switch to full URL for editing
                        strong_this->UrlBox().Text(strong_this->WebView().Source().AbsoluteUri());
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
    }

    void BrowserView::UpdateUrlBarFromWebView()
    {
        auto core = WebView().CoreWebView2();
        if (!core) return;

        try
        {
            // Check focus state so we don't overwrite the URL while the user is typing
            bool isFocused = (UrlBox().FocusState() != Microsoft::UI::Xaml::FocusState::Unfocused);

            if (!isFocused)
            {
                auto uri = WebView().Source();
                winrt::hstring host = uri.Host();
                winrt::hstring title = core.DocumentTitle();

                // 1. Clean up host (remove 'www.')
                std::wstring hostStr{ host };
                if (hostStr.find(L"www.") == 0) hostStr.erase(0, 4);

                // 2. Logic to prevent "youtube.com / YouTube"
                // We convert to lowercase to compare properly
                std::wstring titleStr{ title };
                std::transform(titleStr.begin(), titleStr.end(), titleStr.begin(), ::tolower);

                std::wstring cleanHost = hostStr;
                // Remove the TLD (.com, .org) for a cleaner comparison
                size_t lastDot = cleanHost.find_last_of(L'.');
                if (lastDot != std::wstring::npos) cleanHost = cleanHost.substr(0, lastDot);

                // 3. Only add the title if it's NOT just repeating the domain name
                if (!title.empty() && titleStr != hostStr && titleStr != cleanHost)
                {
                    UrlBox().Text(winrt::hstring(hostStr) + L" / " + title);
                }
                else
                {
                    // If they are the same, just show the domain (youtube.com)
                    UrlBox().Text(winrt::hstring(hostStr));
                }
            }
        }
        catch (...) {}
    }

    void BrowserView::HookCoreWebViewEvents()
    {
        auto core = WebView().CoreWebView2();
        if (!core) return;

        core.SourceChanged([weak_this = get_weak()](auto&&, auto&&) {
            if (auto strong_this = weak_this.get()) {
                strong_this->UpdateUrlBarFromWebView();
            }
            });

        core.DocumentTitleChanged([weak_this = get_weak()](auto const& sender, auto const&) {
            if (auto strong_this = weak_this.get()) {
                strong_this->UpdateUrlBarFromWebView();
                strong_this->m_titleChangedEvent(*strong_this, sender.DocumentTitle());
            }
            });

        core.FaviconChanged([weak_this = get_weak()](auto const& sender, auto const&) {
            if (auto strong_this = weak_this.get()) {
                winrt::hstring uri = sender.FaviconUri();
                if (!uri.empty()) {
                    auto bitmap = winrt::Microsoft::UI::Xaml::Media::Imaging::BitmapImage(winrt::Windows::Foundation::Uri(uri));
                    strong_this->FaviconImage().Source(bitmap);
                    strong_this->m_faviconChangedEvent(*strong_this, uri);
                }
            }
            });
    }

    // Include your NavigateTo, NormalizeUrl, and SetInitialUrl here as they were
    void BrowserView::NavigateTo(winrt::hstring const& url)
    {
        auto normalized = NormalizeUrl(url);
        try { WebView().Source(winrt::Windows::Foundation::Uri{ normalized }); }
        catch (...) {}
    }

    winrt::hstring BrowserView::NormalizeUrl(winrt::hstring const& input)
    {
        std::wstring text{ input };
        text.erase(0, text.find_first_not_of(L" \t"));
        if (auto last = text.find_last_not_of(L" \t"); last != std::wstring::npos) text.erase(last + 1);
        if (text.empty()) return L"about:blank";
        if (text.starts_with(L"http://") || text.starts_with(L"https://") || text.starts_with(L"about:")) return winrt::hstring{ text };
        if (text.find(L'.') != std::wstring::npos && text.find(L' ') == std::wstring::npos) return winrt::hstring{ L"https://" + text };
        return winrt::hstring{ L"https://www.google.com/search?q=" + text };
    }
}