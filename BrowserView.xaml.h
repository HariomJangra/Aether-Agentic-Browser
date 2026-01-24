#pragma once

#include "BrowserView.g.h"

namespace winrt::Agentic_Browser::implementation
{
    struct BrowserView : BrowserViewT<BrowserView>
    {
        BrowserView()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        
    };
}

namespace winrt::Agentic_Browser::factory_implementation
{
    struct BrowserView : BrowserViewT<BrowserView, implementation::BrowserView>
    {
    };
}
