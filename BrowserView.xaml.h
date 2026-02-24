#pragma once

#include "BrowserView.g.h"

namespace winrt::Agentic_Browser::implementation
{
    struct BrowserView : BrowserViewT<BrowserView>
    {
        BrowserView();

        void NavigateTo(winrt::hstring const& url);
        void Cleanup(); 

        // --- Event Registrations ---
        winrt::event_token TitleChanged(Windows::Foundation::TypedEventHandler<Agentic_Browser::BrowserView, winrt::hstring> const& handler) { return m_titleChangedEvent.add(handler); }
        void TitleChanged(winrt::event_token const& token) noexcept { m_titleChangedEvent.remove(token); }

        winrt::event_token FaviconChanged(Windows::Foundation::TypedEventHandler<Agentic_Browser::BrowserView, winrt::hstring> const& handler) { return m_faviconChangedEvent.add(handler); }
        void FaviconChanged(winrt::event_token const& token) noexcept { m_faviconChangedEvent.remove(token); }

        winrt::event_token NewTabRequested(Windows::Foundation::TypedEventHandler<Agentic_Browser::BrowserView, winrt::hstring> const& handler){ return m_newTabRequestedEvent.add(handler); }
        void NewTabRequested(winrt::event_token const& token) noexcept { m_newTabRequestedEvent.remove(token); }


    private:
        winrt::hstring NormalizeUrl(winrt::hstring const& input);
        void UpdateUrlBarFromWebView();
        void HookCoreWebViewEvents();
        void UpdateNavigationButtonStates();
        winrt::hstring m_pendingNavigationUrl;

        // Timer for URL hover delay
        Microsoft::UI::Xaml::DispatcherTimer m_hoverTimer{ nullptr };
       
        

        // --- Event Backing Fields ---
        winrt::event<Windows::Foundation::TypedEventHandler<Agentic_Browser::BrowserView, winrt::hstring>> m_titleChangedEvent;
        winrt::event<Windows::Foundation::TypedEventHandler<Agentic_Browser::BrowserView, winrt::hstring>> m_faviconChangedEvent;
        winrt::event<Windows::Foundation::TypedEventHandler<Agentic_Browser::BrowserView,winrt::hstring>> m_newTabRequestedEvent;
    };
}

namespace winrt::Agentic_Browser::factory_implementation
{
    struct BrowserView : BrowserViewT<BrowserView, implementation::BrowserView>
    {
    };
}