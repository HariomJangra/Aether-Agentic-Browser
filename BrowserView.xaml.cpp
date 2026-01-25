#include "pch.h"
#include "BrowserView.xaml.h"
#if __has_include("BrowserView.g.cpp")
#include "BrowserView.g.cpp"
#endif

#include <winrt/Microsoft.UI.Xaml.Input.h>
#include <winrt/Windows.System.h>
#include <winrt/Microsoft.Web.WebView2.Core.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::Agentic_Browser::implementation
{
    BrowserView::BrowserView()
    {
        InitializeComponent();

        // 1. URL Box Input Handler
        // optimization: Used weak_ref to prevent crashes if the view is closed while processing.
        UrlBox().KeyDown([weak_this = get_weak(), this](IInspectable const&, Input::KeyRoutedEventArgs const& args)
            {
                if (auto strong_this = weak_this.get())
                {
                    if (args.Key() == Windows::System::VirtualKey::Enter)
                    {
                        NavigateTo(UrlBox().Text());
                    }
                }
            });

        // 2. Initialize CoreWebView2
        // We only start hooking events after the Core engine is ready.
        WebView().CoreWebView2Initialized(
            [weak_this = get_weak(), this](auto&&, auto&&)
            {
                if (auto strong_this = weak_this.get())
                {
                    HookCoreWebViewEvents();
                }
            });

        // Ensure the backing WebView2 process is initialized explicitly (optional but good practice)
        WebView().EnsureCoreWebView2Async();
    }

    winrt::hstring BrowserView::NormalizeUrl(winrt::hstring const& input)
    {
        std::wstring text{ input };

        // Trim whitespace
        text.erase(0, text.find_first_not_of(L" \t"));
        if (auto last = text.find_last_not_of(L" \t"); last != std::wstring::npos)
        {
            text.erase(last + 1);
        }

        if (text.empty())
            return L"about:blank";

        // If it already looks like a URL
        if (text.starts_with(L"http://") || text.starts_with(L"https://") || text.starts_with(L"about:"))
            return winrt::hstring{ text };

        // If it looks like a domain name (contains dot, no spaces)
        if (text.find(L'.') != std::wstring::npos && text.find(L' ') == std::wstring::npos)
            return winrt::hstring{ L"https://" + text };

        // Otherwise treat as search query
        return winrt::hstring{ L"https://www.google.com/search?q=" + text };
    }

    void BrowserView::NavigateTo(winrt::hstring const& url)
    {
        auto normalized = NormalizeUrl(url);

        try
        {
            // SAFETY: Uri constructor throws if the string is invalid. 
            // We catch it to prevent the entire app from crashing.
            winrt::Windows::Foundation::Uri targetUri{ normalized };
            WebView().Source(targetUri);
        }
        catch (winrt::hresult_error const&)
        {
            // Optional: Set URL box to indicate error or navigate to a search of the invalid string
            // For now, we just ignore the crash-inducing navigation.
        }
    }

    // Keep this only if it is defined in your BrowserView.idl file. 
    // If not in IDL, you can remove this method entirely.
    void BrowserView::SetInitialUrl(winrt::hstring const& url)
    {
        NavigateTo(url);
    }

    void BrowserView::UpdateUrlBarFromWebView()
    {
        // Check if CoreWebView2 is active before accessing Source
        if (!WebView().CoreWebView2()) return;

        try
        {
            // Get the current source from the actual Core engine
            auto uri = WebView().Source();
            if (!uri) return;

            auto url = uri.AbsoluteUri();

            // Prevent feedback loop: only update if different
            if (UrlBox().Text() != url)
            {
                UrlBox().Text(url);
            }
        }
        catch (...)
        {
            // Swallow errors during UI updates to keep interface responsive
        }
    }

    void BrowserView::HookCoreWebViewEvents()
    {
        auto core = WebView().CoreWebView2();
        if (!core) return;

        // OPTIMIZATION: 'SourceChanged' covers both:
        // 1. Traditional Navigation (Page Loads)
        // 2. SPA Navigation (React/YouTube history API changes)
        // We do not need NavigationCompleted or HistoryChanged anymore.
        core.SourceChanged(
            [weak_this = get_weak(), this](auto&&, auto&&)
            {
                if (auto strong_this = weak_this.get())
                {
                    UpdateUrlBarFromWebView();
                }
            });
    }
}