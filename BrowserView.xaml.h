#pragma once

#include "BrowserView.g.h"

namespace winrt::Agentic_Browser::implementation
{
    struct BrowserView : BrowserViewT<BrowserView>
    {
        BrowserView();

        void NavigateTo(winrt::hstring const& url);
        void SetInitialUrl(winrt::hstring const& url);

    private:
        winrt::hstring NormalizeUrl(winrt::hstring const& input);
        void UpdateUrlBarFromWebView();
        void HookCoreWebViewEvents();
        
    };
}

namespace winrt::Agentic_Browser::factory_implementation
{
    struct BrowserView : BrowserViewT<BrowserView, implementation::BrowserView>
    {
    };
}
