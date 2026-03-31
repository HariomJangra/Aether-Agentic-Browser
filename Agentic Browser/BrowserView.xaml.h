#pragma once

#include "BrowserView.g.h"

namespace winrt::Agentic_Browser::implementation
{
    struct BrowserView : BrowserViewT<BrowserView>
    {
        BrowserView();

        void NavigateTo(winrt::hstring const& url);
        void Cleanup(); // WebView close fxn before Tab removes

        // Reload animation
        void StartReloadAnimation();
        void StopReloadAnimation();
        void UpdateReloadIcon();

        bool m_isAssistantOpen{ false };
        bool m_isLoading{ false };
        Microsoft::UI::Xaml::Media::Animation::Storyboard m_reloadStoryboard{ nullptr };

        void AssistantButton_Click(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

        void Settings_Click(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

        

        void NewTab_Click(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

        void UrlBarContainer_PointerEntered(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);

        void UrlBarContainer_PointerExited(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);

        // TOP BAR CLICK Function

        void ToggleDownloadButton_Click(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

        void CopyURL_Click(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

        void AddBookmark_Click(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

        void ViewSiteInfo_Click(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);


        // --- Event Registrations ---

        // --- NEW: Added for Hover Cards ---
        //winrt::event_token UrlChanged(
        //    Windows::Foundation::TypedEventHandler<Agentic_Browser::BrowserView, winrt::hstring> const& handler)
        //{
        //    return m_urlChangedEvent.add(handler);
        //}

        //void UrlChanged(winrt::event_token const& token) noexcept
        //{
        //    m_urlChangedEvent.remove(token);
        //}
        // ----------------------------------

        winrt::event_token TitleChanged(
            Windows::Foundation::TypedEventHandler<Agentic_Browser::BrowserView, winrt::hstring> const& handler)
        {
            return m_titleChangedEvent.add(handler);
        }

        void TitleChanged(winrt::event_token const& token) noexcept
        {
            m_titleChangedEvent.remove(token);
        }

        winrt::event_token FaviconChanged(
            Windows::Foundation::TypedEventHandler<Agentic_Browser::BrowserView, winrt::hstring> const& handler)
        {
            return m_faviconChangedEvent.add(handler);
        }

        void FaviconChanged(winrt::event_token const& token) noexcept
        {
            m_faviconChangedEvent.remove(token);
        }

        winrt::event_token NewTabRequested(
            Windows::Foundation::TypedEventHandler<Agentic_Browser::BrowserView, winrt::hstring> const& handler)
        {
            return m_newTabRequestedEvent.add(handler);
        }

        void NewTabRequested(winrt::event_token const& token) noexcept
        {
            m_newTabRequestedEvent.remove(token);
        }

    private:

        winrt::hstring NormalizeUrl(winrt::hstring const& input);
        void UpdateUrlBarFromWebView();
        void HookCoreWebViewEvents();
        void UpdateNavigationButtonStates();

        winrt::hstring m_pendingNavigationUrl;

        // Short-delay timer for URL bar hover (prevents flicker on fast mouse pass-through)
        Microsoft::UI::Xaml::DispatcherTimer m_urlBarHoverTimer{ nullptr };

        // Assistant panel drag state
        bool m_isDragging{ false };

        void OnSplitterPointerPressed(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);

        void OnSplitterPointerMoved(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);

        void OnSplitterPointerReleased(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);

        // --- Event Backing Fields ---
        //winrt::event<Windows::Foundation::TypedEventHandler<Agentic_Browser::BrowserView, winrt::hstring>> m_urlChangedEvent; // NEW
        winrt::event<Windows::Foundation::TypedEventHandler<Agentic_Browser::BrowserView, winrt::hstring>> m_titleChangedEvent;
        winrt::event<Windows::Foundation::TypedEventHandler<Agentic_Browser::BrowserView, winrt::hstring>> m_faviconChangedEvent;
        winrt::event<Windows::Foundation::TypedEventHandler<Agentic_Browser::BrowserView, winrt::hstring>> m_newTabRequestedEvent;
    };
}

namespace winrt::Agentic_Browser::factory_implementation
{
    struct BrowserView : BrowserViewT<BrowserView, implementation::BrowserView>
    {
    };
}