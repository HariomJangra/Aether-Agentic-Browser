#include "pch.h"
#include "BrowserView.xaml.h"

#if __has_include("BrowserView.g.cpp")
#include "BrowserView.g.cpp"
#endif

#include <winrt/Microsoft.UI.Xaml.Input.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.System.h>
#include <winrt/Microsoft.Web.WebView2.Core.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.Media.Animation.h>
#include <winrt/Microsoft.UI.Input.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace winrt::Microsoft::Web::WebView2::Core;

namespace winrt::Agentic_Browser::implementation
{
    // Special home page URL
    constexpr wchar_t HOME_PAGE_URL[] = L"http://localhost:8000/";

    BrowserView::BrowserView()
    {
        InitializeComponent();

        // Back navigation
        BackButton().Click([weak_this = get_weak()](auto const&, auto const&)
            {
                if (auto self = weak_this.get())
                {
                    if (self->WebView().CanGoBack())
                        self->WebView().GoBack();
                }
            });

        // Forward navigation
        ForwardButton().Click([weak_this = get_weak()](auto const&, auto const&)
            {
                if (auto self = weak_this.get())
                {
                    if (self->WebView().CanGoForward())
                        self->WebView().GoForward();
                }
            });

        // Reload (reload or stop)
        ReloadButton().Click([weak_this = get_weak()](auto const&, auto const&)
            {
                if (auto self = weak_this.get())
                {
                    if (auto core = self->WebView().CoreWebView2())
                    {
                        if (self->m_isLoading)
                            core.Stop();
                        else
                            core.Reload();
                    }
                }
            });

        // Navigate on Enter key
        UrlBox().KeyDown(
            [weak_this = get_weak()]
            (IInspectable const&, Input::KeyRoutedEventArgs const& args)
            {
                if (auto self = weak_this.get())
                {
                    if (args.Key() == Windows::System::VirtualKey::Enter)
                    {
                        self->NavigateTo(self->UrlBox().Text());
                        self->WebView().Focus(FocusState::Programmatic);
                    }
                }
            });

        // Create WebView2 environment with extensions enabled for MAIN WebView
        CoreWebView2EnvironmentOptions options;
        options.AreBrowserExtensionsEnabled(true);
        options.AdditionalBrowserArguments(L"--enable-extensions --remote-debugging-port=9222 --disable-features=msExtensionsHub");

        // Main WebView - uses default user data folder
        auto env_task = CoreWebView2Environment::CreateWithOptionsAsync(
            L"",                    // Browser executable folder (default)
            L"WebView2Data",        // User data folder for main WebView
            options
        );

        env_task.Completed(
            [this, weak_this = get_weak()](auto&& operation, auto&& status)
            {
                if (auto self = weak_this.get())
                {
                    if (status == Windows::Foundation::AsyncStatus::Completed)
                    {
                        try
                        {
                            if (auto env = operation.GetResults())
                                self->WebView().EnsureCoreWebView2Async(env);
                        }
                        catch (...) {}
                    }
                }
            });

        // WebView ready
        WebView().CoreWebView2Initialized(
            [weak_this = get_weak()](auto&&, auto&&)
            {
                if (auto self = weak_this.get())
                {
                    self->HookCoreWebViewEvents();

                    if (auto core = self->WebView().CoreWebView2())
                    {
                        auto settings = core.Settings();
                        settings.IsScriptEnabled(true);
                        settings.AreBrowserAcceleratorKeysEnabled(true);
                        settings.AreDefaultContextMenusEnabled(true);
                        settings.AreDevToolsEnabled(true);

                        if (auto profile = core.Profile())
                        {
                            auto ext_task = profile.AddBrowserExtensionAsync(
                                L"F:\\Browser Development\\Backend\\Extensions\\AdGuard"
                            );

                            ext_task.Completed([](auto&& op, auto&& s)
                                {
                                    if (s == Windows::Foundation::AsyncStatus::Completed)
                                    {
                                        try
                                        {
                                            op.GetResults();
                                            OutputDebugString(L"✓ Extension loaded successfully!\n");
                                        }
                                        catch (...)
                                        {
                                            OutputDebugString(L"✗ Extension loading failed\n");
                                        }
                                    }
                                });
                        }
                    }

                    // Navigate to pending URL if exists, otherwise home page
                    if (!self->m_pendingNavigationUrl.empty())
                    {
                        self->NavigateTo(self->m_pendingNavigationUrl);
                        self->m_pendingNavigationUrl = L"";
                    }
                    else
                    {
                        self->NavigateTo(
                            L"http://localhost:8000/"
                        );
                    }
                }
            });

        // Initialize Assistant WebView2 with SEPARATE user data folder
        CoreWebView2EnvironmentOptions assistantOptions;
        auto assistant_env_task = CoreWebView2Environment::CreateWithOptionsAsync(
            L"",                            // Browser executable folder (default)
            L"WebView2Data_Assistant",      // DIFFERENT user data folder for assistant WebView
            assistantOptions
        );

        assistant_env_task.Completed(
            [this, weak_this = get_weak()](auto&& operation, auto&& status)
            {
                if (auto self = weak_this.get())
                {
                    if (status == Windows::Foundation::AsyncStatus::Completed)
                    {
                        try
                        {
                            if (auto env = operation.GetResults())
                            {
                                self->AssistantWebView().EnsureCoreWebView2Async(env);
                            }
                        }
                        catch (hresult_error const& ex)
                        {
                            OutputDebugString((L"✗ Assistant WebView initialization failed: " + ex.message() + L"\n").c_str());
                        }
                    }
                }
            });

        // Assistant WebView ready
        AssistantWebView().CoreWebView2Initialized(
            [weak_this = get_weak()](auto&&, auto&&)
            {
                if (auto self = weak_this.get())
                {
                    OutputDebugString(L"✓ Assistant WebView2 initialized!\n");

                    if (auto core = self->AssistantWebView().CoreWebView2())
                    {
                        auto settings = core.Settings();
                        settings.IsScriptEnabled(true);
                        settings.AreDevToolsEnabled(true);

                        OutputDebugString(L"✓ Navigating to localhost:5050\n");
                        core.Navigate(L"http://localhost:5050/");
                    }
                }
            });

        // Show full URL when focused
        UrlBox().GettingFocus([weak_this = get_weak()](auto const&, auto const&)
            {
                if (auto self = weak_this.get())
                {
                    // Change border to blue when focused
                    if (auto container = self->UrlBarContainer())
                    {
                        container.BorderBrush(Media::SolidColorBrush(
                            Microsoft::UI::ColorHelper::FromArgb(255, 32, 128, 221)
                        ));
                    }

                    if (self->WebView().CoreWebView2())
                    {
                        auto uriStr = self->WebView().Source().AbsoluteUri();

                        // Clear text for home page, show full URL for others
                        if (uriStr == HOME_PAGE_URL)
                        {
                            self->UrlBox().Text(L"");
                        }
                        else
                        {
                            self->UrlBox().Text(uriStr);
                        }

                        self->UrlBox().SelectAll();
                    }
                }
            });

        // Select all text when clicking on the URL bar (even if already focused)
        UrlBox().GotFocus([weak_this = get_weak()](auto const&, auto const&)
            {
                if (auto self = weak_this.get())
                {
                    // Use Dispatcher to ensure SelectAll happens after focus completes
                    self->DispatcherQueue().TryEnqueue([weak_this]()
                        {
                            if (auto self = weak_this.get())
                            {
                                self->UrlBox().SelectAll();
                            }
                        });
                }
            });

        // Show actual URL on hover (with delay)
        UrlBox().PointerEntered([weak_this = get_weak()](auto const&, auto const&)
            {
                if (auto self = weak_this.get())
                {
                    // Create timer if not exists
                    if (!self->m_hoverTimer)
                    {
                        self->m_hoverTimer = Microsoft::UI::Xaml::DispatcherTimer();
                        self->m_hoverTimer.Interval(std::chrono::milliseconds(100)); // 0.1 second delay
                    }

                    // Set up the timer tick event
                    self->m_hoverTimer.Tick([weak_this](auto const&, auto const&)
                        {
                            if (auto self = weak_this.get())
                            {
                                // Stop the timer
                                self->m_hoverTimer.Stop();

                                if (self->WebView().CoreWebView2())
                                {
                                    // Show actual URL when hovering (only if not focused)
                                    if (self->UrlBox().FocusState() == FocusState::Unfocused)
                                    {
                                        auto uriStr = self->WebView().Source().AbsoluteUri();

                                        // Show "Enter address" for home page, show full URL for others
                                        if (uriStr == HOME_PAGE_URL)
                                        {
                                            self->UrlBox().Text(L"Ask Anything...");
                                        }
                                        else
                                        {
                                            self->UrlBox().Text(uriStr);
                                        }
                                    }
                                }
                            }
                        });

                    // Start the timer
                    self->m_hoverTimer.Start();
                }
            });

        // Restore display text when not hovering
        UrlBox().PointerExited([weak_this = get_weak()](auto const&, auto const&)
            {
                if (auto self = weak_this.get())
                {
                    // Cancel the timer if user moves away before delay completes
                    if (self->m_hoverTimer && self->m_hoverTimer.IsEnabled())
                    {
                        self->m_hoverTimer.Stop();
                    }

                    // Only restore if not focused
                    if (self->UrlBox().FocusState() == FocusState::Unfocused)
                    {
                        self->UpdateUrlBarFromWebView();
                    }
                }
            });

        // Restore display text on blur
        UrlBox().LosingFocus([weak_this = get_weak()](auto const&, auto const&)
            {
                if (auto self = weak_this.get())
                {
                    // Restore border to background color when unfocused
                    if (auto container = self->UrlBarContainer())
                    {
                        container.BorderBrush(Media::SolidColorBrush(
                            Microsoft::UI::ColorHelper::FromArgb(255, 253, 252, 249)  // #fdfcf9
                        ));
                    }

                    self->UpdateUrlBarFromWebView();
                }
            });

        // Clicking outside returns focus to WebView
        PointerPressed([weak_this = get_weak()](auto const&, auto const&)
            {
                if (auto self = weak_this.get())
                {
                    self->WebView().Focus(FocusState::Programmatic);
                    self->UpdateUrlBarFromWebView();
                }
            });
    }

    void BrowserView::UpdateReloadIcon()
    {
        if (m_isLoading)
        {
            ReloadIcon().Source(Microsoft::UI::Xaml::Media::Imaging::SvgImageSource(
                Windows::Foundation::Uri(L"ms-appx:///Assets/Icons/icons8-close.svg")));
        }
        else
        {
            ReloadIcon().Source(Microsoft::UI::Xaml::Media::Imaging::SvgImageSource(
                Windows::Foundation::Uri(L"ms-appx:///Assets/Icons/rotate-right.svg")));
        }
    }

    void BrowserView::StartReloadAnimation()
    {
        StopReloadAnimation();

        using namespace Microsoft::UI::Xaml::Media::Animation;

        DoubleAnimation rotate{};
        rotate.From(0.0);
        rotate.To(360.0);
        rotate.Duration(DurationHelper::FromTimeSpan(std::chrono::milliseconds(900)));
        rotate.RepeatBehavior(RepeatBehavior{ 1000 });

        Storyboard storyboard{};
        storyboard.Children().Append(rotate);

        Storyboard::SetTarget(rotate, ReloadRotate());
        Storyboard::SetTargetProperty(rotate, L"Angle");

        storyboard.Begin();

        m_reloadStoryboard = storyboard;
    }

    void BrowserView::StopReloadAnimation()
    {
        if (m_reloadStoryboard)
            m_reloadStoryboard.Stop();

        ReloadRotate().Angle(0);
    }

    void BrowserView::UpdateUrlBarFromWebView()
    {
        auto core = WebView().CoreWebView2();
        if (!core) return;

        try
        {
            auto uri = WebView().Source();
            auto uriStr = uri.AbsoluteUri();

            // Check if this is the special home page URL
            if (uriStr == HOME_PAGE_URL)
            {
                UrlBox().Text(L"Ask Anything...");
                return;
            }

            auto host = uri.Host();
            auto title = core.DocumentTitle();

            std::wstring hostStr{ host };
            if (hostStr.starts_with(L"www."))
                hostStr.erase(0, 4);

            std::wstring titleStr{ title };
            std::transform(titleStr.begin(), titleStr.end(),
                titleStr.begin(), ::tolower);

            std::wstring cleanHost = hostStr;
            if (auto dot = cleanHost.find_last_of(L'.');
                dot != std::wstring::npos)
            {
                cleanHost = cleanHost.substr(0, dot);
            }

            if (!title.empty() &&
                titleStr != hostStr &&
                titleStr != cleanHost)
            {
                UrlBox().Text(
                    winrt::hstring(hostStr) + L" / " + title
                );
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

        core.NavigationStarting([weak_this = get_weak()](auto&&, auto&&)
            {
                if (auto self = weak_this.get())
                {
                    self->m_isLoading = true;
                    self->UpdateReloadIcon(); // Change to X
                    self->StartReloadAnimation();
                }
            });

        core.SourceChanged([weak_this = get_weak()](auto&&, auto&&)
            {
                if (auto self = weak_this.get())
                    self->UpdateUrlBarFromWebView();
            });

        core.NavigationCompleted([weak_this = get_weak()](auto&&, auto&&)
            {
                if (auto self = weak_this.get())
                {
                    //// RELOAD FEATURE
                    self->m_isLoading = false;
                    self->UpdateReloadIcon(); // Revert to rotate-right
                    self->StopReloadAnimation();

                    self->UpdateNavigationButtonStates();
                }
            });

        core.DocumentTitleChanged(
            [weak_this = get_weak()](auto const& sender, auto const&)
            {
                if (auto self = weak_this.get())
                {
                    self->UpdateUrlBarFromWebView();
                    self->m_titleChangedEvent(*self, sender.DocumentTitle());
                }
            });

        core.FaviconChanged(
            [weak_this = get_weak()](auto const& sender, auto const&)
            {
                if (auto self = weak_this.get())
                {
                    auto uri = sender.FaviconUri();
                    if (!uri.empty())
                    {
                        auto bitmap =
                            Media::Imaging::BitmapImage(
                                Windows::Foundation::Uri(uri)
                            );

                        self->FaviconImage().Source(bitmap);
                        self->m_faviconChangedEvent(*self, uri);
                    }
                }
            });

        core.NewWindowRequested(
            [weak_this = get_weak()]
            (auto const&,
                CoreWebView2NewWindowRequestedEventArgs const& args)
            {
                if (auto self = weak_this.get())
                {
                    args.Handled(true);
                    self->m_newTabRequestedEvent(*self, args.Uri());
                }
            });

        core.HistoryChanged([weak_this = get_weak()](auto&&, auto&&)
            {
                if (auto self = weak_this.get())
                {
                    self->UpdateNavigationButtonStates();
                }
            });
    }

    void BrowserView::NavigateTo(winrt::hstring const& url)
    {
        // Store URL if WebView2 not ready yet
        if (!WebView().CoreWebView2())
        {
            m_pendingNavigationUrl = url;
            return;
        }

        auto normalized = NormalizeUrl(url);

        try
        {
            WebView().Source(
                Windows::Foundation::Uri{ normalized }
            );
        }
        catch (...) {}
    }

    winrt::hstring BrowserView::NormalizeUrl(
        winrt::hstring const& input)
    {
        std::wstring text{ input };

        text.erase(0, text.find_first_not_of(L" \t"));
        if (auto last = text.find_last_not_of(L" \t");
            last != std::wstring::npos)
        {
            text.erase(last + 1);
        }

        if (text.empty())
            return L"about:blank";

        if (text.starts_with(L"http://") ||
            text.starts_with(L"https://") ||
            text.starts_with(L"about:") ||
            text.starts_with(L"file://") ||
            text.starts_with(L"chrome://") ||
            text.starts_with(L"edge://") ||
            text.starts_with(L"ms-appx-web://"))
        {
            return winrt::hstring{ text };
        }

        if (text.find(L'.') != std::wstring::npos &&
            text.find(L' ') == std::wstring::npos)
        {
            return winrt::hstring{ L"https://" + text };
        }

        return winrt::hstring{
            L"https://www.google.com/search?q=" + text
        };
    }

    void BrowserView::UpdateNavigationButtonStates()
    {
        if (WebView().CoreWebView2())
        {
            BackButton().IsEnabled(WebView().CanGoBack());
            ForwardButton().IsEnabled(WebView().CanGoForward());
        }
        else
        {
            BackButton().IsEnabled(false);
            ForwardButton().IsEnabled(false);
        }
    }

    void BrowserView::Cleanup()
    {
        try
        {
            // Stop the hover timer if running
            if (m_hoverTimer)
            {
                m_hoverTimer.Stop();
                m_hoverTimer = nullptr;
            }

            // Stop any media playback and close the WebView2
            if (auto core = WebView().CoreWebView2())
            {
                // Navigate to about:blank to stop all content
                core.Navigate(L"about:blank");
            }

            // Close the WebView2 control
            WebView().Close();
        }
        catch (...) {}
    }

    void BrowserView::AssistantButton_Click(IInspectable const& sender, RoutedEventArgs const& e)
    {
        if (AssistantColumn().Width().Value == 0)
        {
            // Show assistant panel
            AssistantColumn().Width(GridLengthHelper::FromValueAndType(350, GridUnitType::Pixel));
            GridSplitterBorder().Visibility(Visibility::Visible);

            // Add splitter interaction handlers
            GridSplitterBorder().PointerPressed({ this, &BrowserView::OnSplitterPointerPressed });
            GridSplitterBorder().PointerMoved({ this, &BrowserView::OnSplitterPointerMoved });
            GridSplitterBorder().PointerReleased({ this, &BrowserView::OnSplitterPointerReleased });
            GridSplitterBorder().PointerCaptureLost({ this, &BrowserView::OnSplitterPointerReleased });
        }
        else
        {
            // Hide assistant panel
            AssistantColumn().Width(GridLengthHelper::FromValueAndType(0, GridUnitType::Pixel));
            GridSplitterBorder().Visibility(Visibility::Collapsed);
        }
    }

    bool m_isDragging = false;

    void BrowserView::OnSplitterPointerPressed(
        winrt::Windows::Foundation::IInspectable const& sender,
        winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e)
    {
        auto border = sender.as<winrt::Microsoft::UI::Xaml::Controls::Border>();
        border.CapturePointer(e.Pointer());
        m_isDragging = true;
    }

    void BrowserView::OnSplitterPointerMoved(
        winrt::Windows::Foundation::IInspectable const& sender,
        winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e)
    {
        if (!m_isDragging) return;

        auto position = e.GetCurrentPoint(*this).Position();

        double newWidth = ActualWidth() - position.X;

        if (newWidth >= 250 && newWidth <= ActualWidth() * 0.7)
        {
            AssistantColumn().Width(
                winrt::Microsoft::UI::Xaml::GridLengthHelper::FromPixels(newWidth));
        }
    }

    void BrowserView::OnSplitterPointerReleased(
        winrt::Windows::Foundation::IInspectable const& sender,
        winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e)
    {
        if (!m_isDragging) return;

        auto border = sender.as<winrt::Microsoft::UI::Xaml::Controls::Border>();
        border.ReleasePointerCapture(e.Pointer());
        m_isDragging = false;
    }

    void BrowserView::Settings_Click(
        winrt::Windows::Foundation::IInspectable const& sender,
        winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        m_newTabRequestedEvent(*this, L"settings://");
    }

}